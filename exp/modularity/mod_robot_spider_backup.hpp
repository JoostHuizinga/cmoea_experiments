/*
 * Spider.h
 *
 *  Created on: Sep 26, 2013
 *      Author: joost
 */

#ifndef SPIDER_H_
#define SPIDER_H_


#include <boost/config.hpp>

//#include "btBulletDynamicsCommon.h"
#include "mod_robot_ctrnn.hpp"
#include "mod_robot_global.hpp"
#include "mod_robot_exception.hpp"
//#include "mod_robot_sensor.hpp"
#include "mod_robot_object_group.hpp"
#include <iostream>
#include <math.h>
#include <mod_robot_sensor_group.hpp>
#include <bitset>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

#if defined(BULLET_NOT_IN_BULLET_FOLDER)
#include "btBulletDynamicsCommon.h"
#else // defined(BULLET_NOT_IN_BULLET_FOLDER)
#include "bullet/btBulletDynamicsCommon.h"
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

#ifndef M_PI_8
#define M_PI_8     0.5 * M_PI_4
#endif

//#define NUM_LEGS 6
//#define BODYPART_COUNT 2 * NUM_LEGS + 1
//#define KNEE_JOINT_COUNT NUM_LEGS
//#define HIP_JOINT_COUNT NUM_LEGS

//#define FOOD_SENSOR_COUNT 6
//#define PREDATOR_SENSOR_COUNT 6

namespace mod_robot{
struct CollisionCallback : public btCollisionWorld::ContactResultCallback
{
    CollisionCallback(){
        hit = false;
    }

    virtual btScalar addSingleResult(btManifoldPoint& cp,
            const btCollisionObjectWrapper* colObj0,
            int partId0,
            int index0,
            const btCollisionObjectWrapper* colObj1,
            int partId1,
            int index1)
    {
        hit=true;
        return 0;
    }

    bool hit;
};

template<typename NN>
class Spider
{
public:
    typedef NN nn_t;

	/*****************/
	/*** CONSTANTS ***/
	/*****************/
//	SFERES_CONST size_t food_sensor_count = 6;
//	SFERES_CONST size_t predator_sensor_count = 6;
//	SFERES_CONST size_t total_sensor_count = food_sensor_count + predator_sensor_count;

    SFERES_CONST size_t num_legs = 6;
	SFERES_CONST size_t knee_joint_count = num_legs;
	SFERES_CONST size_t hip_joint_count = num_legs;
	SFERES_CONST size_t hip_pitch_joint_count = num_legs;
	SFERES_CONST size_t hip_yaw_joint_count = num_legs;
	SFERES_CONST size_t total_actuator_count = knee_joint_count+hip_pitch_joint_count+hip_yaw_joint_count;

	SFERES_CONST size_t body_part_count = 2*num_legs + 1;

	//This is the 'roll' of the leg considered from the perspective of looking down the length of the leg.
	//It effectively determines how much the leg can twist.
	SFERES_CONST double rollUpperLimit = 0.0;
	SFERES_CONST double rollLowerLimit = 0.0;
	SFERES_CONST int rollAxisIndex = 0;

	//This is the 'yaw' of the leg considered from the perspective of looking down the length of the leg.
	//It effectively determines how much the leg can turn sideways.
	SFERES_CONST double yawLowerLimit = -M_PI_8;
	SFERES_CONST double yawUpperLimit = M_PI_8;
	SFERES_CONST int yawAxisIndex = 1;

	//This is the 'pitch' of the leg considered from the perspective of looking down the length of the leg.
	//It effectively determines how much the leg can turn up and down.
//	SFERES_CONST double pitchLowerLimit = -M_PI_2; //TODO New limit
//	SFERES_CONST double pitchUpperLimit = M_PI_2; //TODO New limit
    SFERES_CONST double pitchLowerLimit = -M_PI_4;
    SFERES_CONST double pitchUpperLimit = M_PI_4;
	SFERES_CONST int pitchAxisIndex = 2;

	//This is the pitch of the knee. The knee does not have more than 1 degree of freedom.
//	SFERES_CONST double kneeLowerLimit = -M_PI_2; //TODO New limit
	SFERES_CONST double kneeLowerLimit = -M_PI_8;
	SFERES_CONST double kneeUpperLimit = 0.2;


	SFERES_CONST btScalar fullCircleDegrees = 360;
	SFERES_CONST btScalar halfCircleDegrees = 180;
//	SFERES_CONST btScalar angleIncrement;
//	SFERES_CONST btScalar halfAngleIncrement;
	SFERES_CONST btScalar sensorStartingAngle = -180;
//	SFERES_CONST btScalar firstSensorCenterAngle;
//
//	static btScalar sensor_angles[food_sensor_count];
//	static btScalar sensor_edges[food_sensor_count+1];

	SFERES_CONST float m_fMuscleStrength = 0.5;

private:
	/*****************/
	/*** VARIABLES ***/
	/*****************/
	btDynamicsWorld*			m_ownerWorld;
	btCollisionShape*			m_shapes[body_part_count];
	btRigidBody*				m_bodies[body_part_count];
	btGeneric6DofConstraint*	m_hipjoints[hip_joint_count];
	btHingeConstraint*			m_kneejoints[knee_joint_count];
//	double 						m_foodSensors[food_sensor_count];
//	double	 					m_predatorSensors[predator_sensor_count];
//
//	std::vector<PieSensor> _foodSensors;
//	std::vector<PieSensor> _predatorSensors;

	std::vector<SensorGroup> _sensors;


//	bool usingPredatorSensor;



	btRigidBody* localCreateRigidBody (btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
	{
		dbg::trace trace("spider", DBG_HERE);

//		mass = 0.0;
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			shape->calculateLocalInertia(mass,localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}


public:

	Spider (btDynamicsWorld* ownerWorld, const btVector3& positionOffset = btVector3(0,0,0), nn_t* dnn = 0)
		: m_ownerWorld (ownerWorld), dnn(dnn)
	{
		dbg::trace trace("spider", DBG_HERE);
//		if(!nnIsCorrect()){
//			throw mod_robot_exception(toString("Nr of inputs or outputs is incorrect:\n") +
//					"received inputs: " + toString(dnn->getNumberOfInputs()) + "\n" +
//					"received outputs: " + toString(dnn->getNumberOfOutputs()) + "\n" +
//					"expected inputs: " + toString(food_sensor_count) + " or " + toString(total_sensor_count) + "\n" +
//					"expected outputs: " + toString(food_sensor_count) + "\n" );
//		}

		//Initialize network
		btVector3 vUp(0, 1, 0);

		//
		// Setup geometry
		//
		float fBodySize  = 0.25f;
		float fLegLength = 0.45f;
		float fForeLegLength = 0.75f;
		m_shapes[0] = new btCapsuleShape(btScalar(fBodySize), btScalar(0.10));

		for (size_t i=0; i<num_legs; i++)
		{
			m_shapes[1 + 2*i] = new btCapsuleShape(btScalar(0.10), btScalar(fLegLength));
			m_shapes[2 + 2*i] = new btCapsuleShape(btScalar(0.08), btScalar(fForeLegLength));
		}

		//
		// Setup rigid bodies
		//
		float fHeight = 1.0;
		btTransform offset; offset.setIdentity();
		offset.setOrigin(positionOffset);

		// root
		btVector3 vRoot = btVector3(btScalar(0.), btScalar(fHeight), btScalar(0.));
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(vRoot);

		//Create the main body
		m_bodies[0] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[0]);
//		printPosition(m_bodies[0]->getCenterOfMassPosition());


		// legs
		for (size_t i=0; i<num_legs; i++)
		{
			float fAngle = 2 * M_PI * i / num_legs;
			float fSin = sin(fAngle);
			float fCos = cos(fAngle);

			transform.setIdentity();
			btVector3 vBoneOrigin = btVector3(btScalar(fCos*(fBodySize+0.5*fLegLength)), btScalar(fHeight), btScalar(fSin*(fBodySize+0.5*fLegLength)));
			transform.setOrigin(vBoneOrigin);

			// thigh
			btVector3 vToBone = (vBoneOrigin - vRoot).normalize();
			btVector3 vAxis = vToBone.cross(vUp);
			transform.setRotation(btQuaternion(vAxis, M_PI_2));
			m_bodies[1+2*i] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[1+2*i]);

			// shin
			transform.setIdentity();
			transform.setOrigin(btVector3(btScalar(fCos*(fBodySize+fLegLength)), btScalar(fHeight-0.5*fForeLegLength), btScalar(fSin*(fBodySize+fLegLength))));
			m_bodies[2+2*i] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[2+2*i]);
		}

		// Setup some damping on the m_bodies
		for (size_t i = 0; i < body_part_count; ++i)
		{
			m_bodies[i]->setDamping(0.05, 0.85);
			m_bodies[i]->setDeactivationTime(.0f);
			//m_bodies[i]->setSleepingThresholds(1.6, 2.5);
//			m_bodies[i]->setSleepingThresholds(0.5f, 0.5f);
		}


		//
		// Setup joints
		//
		btHingeConstraint* kneeConstraint;
		btGeneric6DofConstraint * hipConstraint;
		btTransform localA, localB, localC;

		for (size_t i=0; i<num_legs; i++)
		{
			float fAngle = 2 * M_PI * i / num_legs;
			float fSin = sin(fAngle);
			float fCos = cos(fAngle);

			// hip joints
			localA.setIdentity(); localB.setIdentity();
			localA.getBasis().setEulerZYX(0,-fAngle,0);	localA.setOrigin(btVector3(btScalar(fCos*fBodySize), btScalar(0.), btScalar(fSin*fBodySize)));
			localB = m_bodies[1+2*i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;

			//Create hip
			hipConstraint = new btGeneric6DofConstraint(*m_bodies[0], *m_bodies[1+2*i], localA, localB, true);
			hipConstraint->setLinearLowerLimit(btVector3(0.0,0.0,0.0));
			hipConstraint->setLinearUpperLimit(btVector3(0.0,0.0,0.0));
			hipConstraint->setAngularLowerLimit(btVector3(rollLowerLimit,yawLowerLimit,pitchLowerLimit));
			hipConstraint->setAngularUpperLimit(btVector3(rollUpperLimit,yawUpperLimit,pitchUpperLimit));

			//Add hip to the world
			m_hipjoints[i] = hipConstraint;
			m_ownerWorld->addConstraint(m_hipjoints[i], true);

			// knee joints
			localA.setIdentity(); localB.setIdentity(); localC.setIdentity();
			localA.getBasis().setEulerZYX(0,-fAngle,0);	localA.setOrigin(btVector3(btScalar(fCos*(fBodySize+fLegLength)), btScalar(0.), btScalar(fSin*(fBodySize+fLegLength))));
			localB = m_bodies[1+2*i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[2+2*i]->getWorldTransform().inverse() * m_bodies[0]->getWorldTransform() * localA;

			//Create knee
			kneeConstraint = new btHingeConstraint(*m_bodies[1+2*i], *m_bodies[2+2*i], localB, localC);
			kneeConstraint->setLimit(kneeLowerLimit, kneeUpperLimit);


			m_kneejoints[i] = kneeConstraint;
			m_ownerWorld->addConstraint(m_kneejoints[i], true);
		}
	}

	inline bool collisionTest(Object* object){
	    btRigidBody* body = object->getRigidBody();
	    CollisionCallback callback;
	    for(size_t i=0; i<body_part_count; ++i){
	        m_ownerWorld->contactPairTest(body, m_bodies[i], callback);
	        if(callback.hit) return true;
	    }
	    return false;
	}

//	inline static void init(){
//		std::cout << "Spider init " << std::endl; //Debug stream does not exists yet
//		//Set sensor angles
//		for(size_t i =0; i<food_sensor_count; i++){
//			sensor_angles[i] = firstSensorCenterAngle + angleIncrement*i;
//			sensor_edges[i] = sensorStartingAngle + angleIncrement*i;
//		}
//		sensor_edges[food_sensor_count] = sensorStartingAngle + angleIncrement*food_sensor_count;
//	}

	inline size_t getNumberOfSensors(){
		dbg::trace trace("spider", DBG_HERE);
		size_t total_number_of_sensors = 0;
		for(size_t i=0; i<_sensors.size(); ++i){
			total_number_of_sensors += _sensors[i].size();
		}
		return total_number_of_sensors;
	}

	inline size_t getNumberOfSensorGroups(){
		dbg::trace trace("spider", DBG_HERE);
		return _sensors.size();
	}

	inline size_t getNumberOfSensorsInGroup(size_t index){
		dbg::trace trace("spider", DBG_HERE);
		return _sensors[index].size();
	}

	inline void setSensors(const std::vector<ObjectGroup>& object_groups){
		dbg::trace trace("spider", DBG_HERE);
		for(size_t i=0; i<_sensors.size(); ++i){
			_sensors[i].update(object_groups);
		}
	}

	inline SensorGroup& getSensors(size_t index){
		dbg::trace trace("spider", DBG_HERE);
		return _sensors[index];
	}

	inline SensorGroup& getLastSensorGroup(){
		dbg::trace trace("spider", DBG_HERE);
		return _sensors.back();
	}

	inline void addSensorGroup(){
		dbg::trace trace("spider", DBG_HERE);
		_sensors.push_back(SensorGroup(getMainBody()));
	}

	bool nnIsCorrect(){
		dbg::trace trace("spider", DBG_HERE);
		if(dnn->getNumberOfInputs() != getNumberOfSensors()){
			return false;
		}

		return dnn->getNumberOfOutputs() == total_actuator_count;
	}

	virtual	~Spider ()
	{
		dbg::trace trace("spider", DBG_HERE);

		//Remove all sensors
		for (size_t i = 0; i < _sensors.size(); ++i){
			_sensors[i].clear();
		}
		_sensors.clear();

		// Remove all constraints
		for (size_t i = 0; i < knee_joint_count; ++i)
		{
			m_ownerWorld->removeConstraint(m_kneejoints[i]);
			delete m_kneejoints[i]; m_kneejoints[i] = 0;
		}

		// Remove all constraints
		for (size_t i = 0; i < hip_joint_count; ++i)
		{
			m_ownerWorld->removeConstraint(m_hipjoints[i]);
			delete m_hipjoints[i]; m_hipjoints[i] = 0;
		}

		// Remove all bodies and shapes
		for (size_t i = 0; i < body_part_count; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);

			delete m_bodies[i]->getMotionState();

			delete m_bodies[i]; m_bodies[i] = 0;
			delete m_shapes[i]; m_shapes[i] = 0;
		}

		//Delete its brain
//		delete dnn;
	}

//	btTypedConstraint** GetJoints() {return &m_kneejoints[0];}

	btVector3 getPosition(){
		dbg::trace trace("spider", DBG_HERE);
		return m_bodies[0]->getCenterOfMassPosition();
	}

	btQuaternion getRotation(){
		dbg::trace trace("spider", DBG_HERE);
		return m_bodies[0]->getOrientation();
	}

	btVector3 getHeading(){
	    btVector3 forward(1,0,0);
	    btVector3 heading = m_bodies[0]->getWorldTransform().getBasis()*forward;
	    return heading;
	}

    btVector3 getUp(){
        btVector3 up(0,1,0);
        btVector3 currentUp = m_bodies[0]->getWorldTransform().getBasis()*up;
        return currentUp;
    }

	void rotate(){
		dbg::trace trace("spider", DBG_HERE);
		m_bodies[0]->applyTorque(btVector3(0, 100, 0));
	}

	btRigidBody* getMainBody(){
		dbg::trace trace("spider", DBG_HERE);
		return m_bodies[0];
	}

//	void setFoodSensor(int index, btScalar value){
//		dbg::trace trace("spider", DBG_HERE);
//		for(int i = 0; i<food_sensor_count; i++){
//			if(i == index){
//				m_foodSensors[i] = std::max(std::min(3.0f, value), 0.0f);
//			} else {
//				m_foodSensors[i] = 0;
//			}
//		}
//	}
//
//	void setPredatorSensor(int index, btScalar value){
//		dbg::trace trace("spider", DBG_HERE);
//		for(int i = 0; i<predator_sensor_count; i++){
//			if(i == index){
//				m_predatorSensors[i] = std::max(std::min(3.0f, value), 0.0f);
//			} else {
//				m_predatorSensors[i] = 0;
//			}
//		}
//	}


//	std::string getFoodSensorReading(int i){
//		std::stringstream ss;
//		ss << m_foodSensors[i];
//		return ss.str().substr(0, 3);
//	}
//
//	std::string getPredatorSensorReading(int i){
//		std::stringstream ss;
//		ss << m_predatorSensors[i];
//		return ss.str().substr(0, 3);
//	}
//
//
//	std::string foodSensorAsString(){
//		std::stringstream ss;
//		for(int i = 0; i<food_sensor_count; i++){
//			ss << m_foodSensors[i] << " ";
//		}
//		return ss.str();
//	}
//
//	std::string predatorSensorAsString(){
//		std::stringstream ss;
//		for(int i = 0; i<predator_sensor_count; i++){
//			ss << m_predatorSensors[i] << " ";
//		}
//		return ss.str();
//	}
//
//	static inline const btScalar& getSensorAngle(const int& index){
//		dbg::trace trace("spider", DBG_HERE);
//		return sensor_angles[index];
//	}
//
//	static inline const btScalar& getSensorLowerLimit(const int& index){
//		dbg::trace trace("spider", DBG_HERE);
//		return sensor_edges[index];
//	}
//
//	static inline const btScalar& getSensorUpperLimit(const int& index){
//		dbg::trace trace("spider", DBG_HERE);
//		return sensor_edges[index+1];
//	}
//
//	static inline bool inSensorRange(btScalar angle, const int& index){
//		dbg::trace trace("spider", DBG_HERE);
//		return (angle > getSensorLowerLimit(index)) && (angle <= getSensorUpperLimit(index));
//	}

//	void setFoodSensor(btVector3 foodPosition){
//		dbg::trace trace("spider", DBG_HERE);
//		btScalar foodDistance = std::max(getPosition().distance(foodPosition), btScalar(1.0));
//		btVector3 difference = (foodPosition - getPosition());
//		difference.setY(0.0);
//		difference.normalize();
//		btVector3 vectorToFood = difference;
//		btVector3 robotOrientationVector = getRotation().getAxis().normalize();
//		robotOrientationVector.setY(0.0);
//
//		btScalar foodAngle = robotOrientationVector.angle(vectorToFood);
//		foodAngle = (foodAngle/M_PI)*180; //to degrees
//		if (difference.z() < 0){
//			foodAngle *= -1;
//		}
//
//		btScalar pieStartAngle = -180;
//		btScalar pieStopAngle = -180 + angleIncrement;
//
//		for(int i =0; i<food_sensor_count; i++){
//			if(inSensorRange(foodAngle, i)){
//				m_foodSensors[i] = std::min(3.0f, btScalar((1.0f/foodDistance)*3.0f));
//			} else {
//				m_foodSensors[i] = 0;
//			}
//		}
//	}





//	void setFoodSensor(const btVector3& foodPosition){
//		dbg::trace trace("spider", DBG_HERE);
//		_setPieSensor(foodPosition, m_foodSensors, food_sensor_count);
//	}
//
//	void setPredatorSensor(const btVector3& predatorPosition){
//		dbg::trace trace("spider", DBG_HERE);
//		_setPieSensor(predatorPosition, m_predatorSensors, predator_sensor_count);
//	}


//	void getSensorData(){
//		dbg::trace trace("spider", DBG_HERE);
//		for(size_t i =0; i<food_sensor_count; i++){
//			m_foodSensors[i] = 0;
//		}
//
//	}

//	void resetFoodSensor(){
//		dbg::trace trace("spider", DBG_HERE);
//		for(size_t i =0; i<food_sensor_count; i++){
//			m_foodSensors[i] = 0;
//		}
//	}
//
//	void resetPredatorSensor(){
//		dbg::trace trace("spider", DBG_HERE);
//		for(size_t i =0; i<predator_sensor_count; i++){
//			m_predatorSensors[i] = 0;
//		}
//	}


	inline btScalar getOutput(const size_t index) const{
	    dbg::trace trace("spider", DBG_HERE);
#if defined(USE_OUTPUT_POTENTIAL)
	    btScalar output = dnn->getOutputNeuron(index).getPotential() + dnn->getOutputNeuron(index).getBias();
	    dbg::out(dbg::info, "spider") << "Raw actuator output: " << output << std::endl;
	    if(output > 1.0) output = 1.0;
	    if(output < -1.0) output = -1.0;
#elif defined(USE_OUTPUT_TANH)
	    const btScalar& potential = dnn->getOutputNeuron(index).getPotential();
	    const btScalar& bias = dnn->getOutputNeuron(index).getBias();
	    const btScalar lambda = 5.0f;
	    btScalar output = tanh((potential + bias) * lambda);
#else
	    btScalar output = dnn->getOutput(index);
#endif
	    dbg::out(dbg::info, "spider") << "Actuator output: " << output << std::endl;
	    return output;
	}



	void step(btScalar timeStep){
		dbg::trace trace("spider", DBG_HERE);
		btVector3 lowerLimits;
		btVector3 upperLimits;
		size_t behavior_index = 0;

		size_t input_index = 0;
		for(size_t sensor_group_index =0; sensor_group_index<_sensors.size(); ++sensor_group_index){
			for(size_t sensor_index=0; sensor_index< _sensors[sensor_group_index].size(); ++sensor_index){
				dbg::out(dbg::info, "spider") << "sensor group: " << sensor_group_index
						<< " sensor: " << sensor_index
						<< " activation: " <<  _sensors[sensor_group_index][sensor_index]->getActivation() << std::endl;
				dnn->setInput(input_index, _sensors[sensor_group_index][sensor_index]->getActivation());
				input_index++;
			}
//			_behavior[behavior_index++] = m_foodSensors[i] > 0 ? 1 : 0;
//			dbg::out(dbg::info, "behavior") << "sensor: " << " " << " " << m_foodSensors[i] << " : " << (m_foodSensors[i] > 0 ? 1 : 0) << std::endl;
//			dnn->setInput(i, _sensors[i].getActivation());
		}
//
//		for(size_t i =0; i<food_sensor_count; i++){
//			_behavior[behavior_index++] = m_foodSensors[i] > 0 ? 1 : 0;
//			dbg::out(dbg::info, "behavior") << "food sensor: " << m_foodSensors[i] << " : " << (m_foodSensors[i] > 0 ? 1 : 0) << std::endl;
//			dnn->setInput(i, m_foodSensors[i]);
//		}
//
//		if(usingPredatorSensor){
//			for(size_t i =0; i<predator_sensor_count; i++){
//				_behavior[behavior_index++] = m_predatorSensors[i] > 0 ? 1 : 0;
//				dbg::out(dbg::info, "behavior") << "predator sensor: " << m_predatorSensors[i] << " : " << (m_predatorSensors[i] > 0 ? 1 : 0) << std::endl;
//				dnn->setInput(i+food_sensor_count, m_predatorSensors[i]);
//			}
//		}

		dnn->activate();


		for (size_t i=0; i<knee_joint_count; i++){
			btScalar output = getOutput(i);

//			_behavior[behavior_index++] = output > 0 ? 1 : 0;
//			dbg::out(dbg::info, "behavior") << "Knee: " << i << " output: " << output << " : " << (output > 0 ? 1 : 0) << std::endl;
			btHingeConstraint* kneeJoint = m_kneejoints[i];
//			btScalar fCurAngle      = kneeJoint->getHingeAngle();
//			btScalar fTargetLimitAngle = kneeJoint->getLowerLimit() + ((output+1)/2) * (kneeJoint->getUpperLimit() - kneeJoint->getLowerLimit());
//			btScalar fAngleError  = fTargetLimitAngle - fCurAngle;
//			btScalar fDesiredAngularVel = 1000000.f * fAngleError/timeStep;
			btScalar fDesiredAngularVel = _getDesiredAnglularVel(output, kneeJoint->getHingeAngle(), kneeJoint->getLowerLimit(), kneeJoint->getUpperLimit(), timeStep);
			kneeJoint->enableAngularMotor(true, fDesiredAngularVel, m_fMuscleStrength);
		}

		for (size_t i=0; i<hip_pitch_joint_count; i++){
			btScalar output = getOutput(i+knee_joint_count);
//			_behavior[behavior_index++] = output > 0 ? 1 : 0;
//			dbg::out(dbg::info, "behavior") << "Hip pitch: " << i << " output: " << output << " : " << (output > 0 ? 1 : 0) << std::endl;
			_actuateMotor(m_hipjoints[i], output, pitchAxisIndex, timeStep);
		}

		for (size_t i=0; i<hip_yaw_joint_count; i++){
			btScalar output = getOutput(i+knee_joint_count+hip_pitch_joint_count);
//			_behavior[behavior_index++] = output > 0 ? 1 : 0;
//			dbg::out(dbg::info, "behavior") << "Hip yaw: " << i << " output: " << output << " : " << (output > 0 ? 1 : 0) << std::endl;
			_actuateMotor(m_hipjoints[i], output, yawAxisIndex, timeStep);
		}

//		dbg::out(dbg::info, "behavior") << "behavior spider: " << _behavior << std::endl;
	}

//	std::bitset<food_sensor_count + total_actuator_count>& getBehavior(){
//		return _behavior;
//	}

	nn_t* getNN(){
		return dnn;
	}

	void setNN(nn_t* nn){
		dnn = nn;
	}

	template<typename Painter>
	void drawSensors(Painter painter){
		for(size_t i=0; i<_sensors.size(); ++i){
			_sensors[i].draw(painter);
		}
	}

private:
	nn_t* dnn;
//	std::bitset<food_sensor_count + total_actuator_count> _behavior;


//	void _setPieSensor(const btVector3& objectPosition, double* sensor, int sensor_count){
//		dbg::trace trace("spider", DBG_HERE);
//
//		//Get the vector to the object
//		btScalar objectDistance = std::max(getPosition().distance(objectPosition), btScalar(1.0));
//		btVector3 vectorToObject = (objectPosition - getPosition());
//		vectorToObject.setY(0.0);
//		vectorToObject.normalize();
//
//		//Get the forward vector of the robot
//		btVector3 forwardVector(1,0,0);
//		btTransform transform;
//		m_bodies[0]->getMotionState()->getWorldTransform(transform);
//		forwardVector = transform.getBasis() * forwardVector;
//		forwardVector.setY(0.0);
//		forwardVector.normalize();
//
//		//Get the angle between the forward vector and the vector to object
//		btScalar objectAngle = forwardVector.angle(vectorToObject);
//		btVector3 crossProduct = forwardVector.cross(vectorToObject);
//		objectAngle = (objectAngle/M_PI)*180; //to degrees
//		if (crossProduct.y() < 0){
//			objectAngle *= -1;
//		}
//
//		dbg::out(dbg::info, "sensor") << "Sensor: " << sensor << " foodAngle: " << objectAngle
//				<< " object position: " << objectPosition.x() << " " << objectPosition.y() << " " << objectPosition.z()
//				<< " forward vector: "<< forwardVector.x() << " " << forwardVector.y() << " " << forwardVector.z()
//				<< " vector to food: " << vectorToObject.x() << " " << vectorToObject.y() << " " << vectorToObject.z() << " " << std::endl;
//
//		//Store which pie sensor records the object
//		for(int i =0; i<sensor_count; i++){
//			if(inSensorRange(objectAngle, i)){
//				sensor[i] = std::min(3.0f, btScalar((1.0f/objectDistance)*3.0f));
//			} else {
//				sensor[i] = 0;
//			}
//		}
//	}


	inline btScalar _getDesiredAnglularVel(const btScalar& neuron_value, const btScalar& current_angle, const btScalar& lower_limit, const btScalar& upper_limit,  const btScalar& timeStep){
		btScalar fTargetLimitAngle = lower_limit + ((neuron_value+1)/2) * (upper_limit - lower_limit);
		btScalar fAngleError  = fTargetLimitAngle - current_angle;
		btScalar desired_velocity = 1000000.f * fAngleError/timeStep;
		dbg::out(dbg::info, "motors") << " neuron_value: "<< neuron_value
				<< " lower_limit: "<< lower_limit
				<< " upper_limit: "<< upper_limit
				<< " current_angle: "<< current_angle
				<< " fTargetLimitAngle: "<< fTargetLimitAngle
				<< " fAngleError: " << fAngleError
				<< " desired_velocity: " << desired_velocity
				<< std::endl;
		return desired_velocity;
	}

	void _actuateMotor(btGeneric6DofConstraint* motor, btScalar neuronValue, int axis, btScalar timeStep){
		dbg::trace trace("spider", DBG_HERE);
		btVector3 lowerLimits;
		btVector3 upperLimits;
		motor->getAngularLowerLimit(lowerLimits);
		motor->getAngularUpperLimit(upperLimits);

		btScalar fDesiredAngularVel = _getDesiredAnglularVel(neuronValue, motor->getAngle(axis), lowerLimits[axis], upperLimits[axis], timeStep);
		motor->getRotationalLimitMotor(axis)->m_enableMotor = true;
		motor->getRotationalLimitMotor(axis)->m_targetVelocity = fDesiredAngularVel;
		motor->getRotationalLimitMotor(axis)->m_maxMotorForce = m_fMuscleStrength;
	}
};

  //template<typename NN>
  //const btScalar Spider<NN>::fullCircleDegrees = 360;

  //template<typename NN>
  //const btScalar Spider<NN>::halfCircleDegrees = Spider<NN>::fullCircleDegrees/2;
//const btScalar Spider::angleIncrement = Spider::fullCircleDegrees/Spider::food_sensor_count;
//const btScalar Spider::halfAngleIncrement = Spider::halfCircleDegrees/Spider::food_sensor_count;

//template<typename NN>
//const btScalar Spider<NN>::sensorStartingAngle = -180;
//const btScalar Spider::firstSensorCenterAngle = Spider::sensorStartingAngle + Spider::halfAngleIncrement;

//btScalar Spider::sensor_angles[food_sensor_count]  = {0,0,0,0,0,0};
//btScalar Spider::sensor_edges[food_sensor_count+1]  = {0,0,0,0,0,0,0};

#endif /* SPIDER_H_ */
