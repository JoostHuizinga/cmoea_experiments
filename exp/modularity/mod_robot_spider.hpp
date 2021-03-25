/*
 * Spider.h
 *
 *  Created on: Sep 26, 2013
 *      Author: joost
 */

#ifndef SPIDER_H_
#define SPIDER_H_

//#include <btBulletDynamicsCommon.h>
#include <boost/config.hpp>

#include "mod_robot_ctrnn.hpp"
#include "mod_robot_global.hpp"
#include "mod_robot_exception.hpp"
#include "mod_robot_object_group.hpp"
#include <iostream>
#include <math.h>
#include <mod_robot_sensor_group.hpp>
#include <bitset>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

#ifndef NO_OPEN_GL
#include <modules/opengldraw/opengl_draw_sphere.hpp>
#include <modules/opengldraw/opengl_draw_piramid.hpp>
#endif

#if defined(BULLET_NOT_IN_BULLET_FOLDER)
#include <btBulletDynamicsCommon.h>
#else // defined(BULLET_NOT_IN_BULLET_FOLDER)
#include <bullet/btBulletDynamicsCommon.h>
#endif


// Apparently, using these defines causes the eclipse index to crash, so I am
// not using them below.
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

#define DBO dbg::out(dbg::info, "spider")

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

class SpiderBase{
public:
	SFERES_CONST size_t num_legs = 6;
	SFERES_CONST size_t knee_joint_count = num_legs;
	SFERES_CONST size_t hip_joint_count = num_legs;
	SFERES_CONST size_t hip_pitch_joint_count = num_legs;
	SFERES_CONST size_t hip_yaw_joint_count = num_legs;
	SFERES_CONST size_t total_actuator_count = knee_joint_count +
			hip_pitch_joint_count + hip_yaw_joint_count;
	SFERES_CONST size_t body_part_count = 2*num_legs + 1;
};

template<typename NN, typename Params>
class Spider: public SpiderBase{
public:
    typedef NN nn_t;

	/*****************/
	/*** CONSTANTS ***/
	/*****************/

	//This is the 'roll' of the leg considered from the perspective of looking
    //down the length of the leg.
	//It effectively determines how much the leg can twist.
	SFERES_CONST double rollUpperLimit = 0.0;
	SFERES_CONST double rollLowerLimit = 0.0;
	SFERES_CONST int rollAxisIndex = 0;

	//This is the 'yaw' of the leg considered from the perspective of looking
	//down the length of the leg.
	//It effectively determines how much the leg can turn sideways.
	SFERES_CONST double yawLowerLimit = -(0.5*0.785398163397448309616);
	SFERES_CONST double yawUpperLimit = 0.5*0.785398163397448309616;
//	SFERES_CONST double yawLowerLimit = 0;
//	SFERES_CONST double yawUpperLimit = 0;
	SFERES_CONST int yawAxisIndex = 1;

	//This is the 'pitch' of the leg considered from the perspective of looking
	//down the length of the leg.
	//It effectively determines how much the leg can turn up and down.
	SFERES_CONST double pitchLowerLimit = -0.785398163397448309616;
	SFERES_CONST double pitchUpperLimit = 0.785398163397448309616;
	SFERES_CONST int pitchAxisIndex = 2;

	//This is the pitch of the knee. The knee does not have more than 1 degree
	//of freedom.
	SFERES_CONST double kneeLowerLimit = -(0.5*0.785398163397448309616);
	SFERES_CONST double kneeUpperLimit = 0.2;


	SFERES_CONST btScalar fullCircleDegrees = 360;
	SFERES_CONST btScalar halfCircleDegrees = 180;
	SFERES_CONST btScalar sensorStartingAngle = -180;
	SFERES_CONST float m_fMuscleStrength = 0.5;
	SFERES_CONST float m_friction = Params::robot::friction;
	SFERES_CONST int m_nnDelay = Params::robot::nn_delay;

private:
	/*****************/
	/*** VARIABLES ***/
	/*****************/
	btDynamicsWorld*			m_ownerWorld;
	btCollisionShape*			m_shapes[body_part_count];
	btRigidBody*				m_bodies[body_part_count];
	btGeneric6DofConstraint*	m_hipjoints[hip_joint_count];
	btHingeConstraint*			m_kneejoints[knee_joint_count];
	std::vector<btScalar> nn_output_cache;
	std::vector<SensorGroup> _sensors;
	int steps;
	float energy_used;

	btRigidBody* localCreateRigidBody (
			btScalar mass,
			const btTransform& startTransform,
			btCollisionShape* shape)
	{
		typedef btRigidBody::btRigidBodyConstructionInfo rbInfor_t;
		dbg::trace trace("spider", DBG_HERE);
		bool isDynamic = (mass != 0.f);
		btVector3 inertia(0, 0, 0);
		if (isDynamic)
			shape->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* ms = new btDefaultMotionState(startTransform);
		rbInfor_t rbInfo(mass, ms, shape, inertia);
		rbInfo.m_friction = m_friction;
		btRigidBody* body = new btRigidBody(rbInfo);
		m_ownerWorld->addRigidBody(body);
		return body;
	}
//
//
public:

	Spider (btDynamicsWorld* ownerWorld,
			const btVector3& positionOffset = btVector3(0,0,0),
			nn_t* dnn = 0)
		: m_ownerWorld (ownerWorld), dnn(dnn)
	{
		dbg::trace trace("spider", DBG_HERE);
		nn_output_cache.resize(hip_joint_count*2 + knee_joint_count, 0.0);
		steps = 0;
		energy_used = 0;

		//Initialize network
		btVector3 vUp(0, 1, 0);

		//
		// Setup geometry
		//
		float fBodySize  = 0.25f;
		float fLegLength = 0.45f;
		float fForeLegLength = 0.75f;
		m_shapes[0] = new btCapsuleShape(btScalar(fBodySize), btScalar(0.10));
		m_shapes[0]->setUserPointer(0);

		for (size_t i=0; i<num_legs; i++)
		{
			m_shapes[1 + 2*i] = new btCapsuleShape(btScalar(0.10),
					btScalar(fLegLength));
			m_shapes[1 + 2*i]->setUserPointer(0);
			m_shapes[2 + 2*i] = new btCapsuleShape(btScalar(0.08),
					btScalar(fForeLegLength));
			m_shapes[2 + 2*i]->setUserPointer(0);
		}

		//
		// Setup rigid bodies
		//
		float fHeight = 1.0;
		btTransform offset; offset.setIdentity();
		offset.setOrigin(positionOffset);

		// root
		btVector3 vRoot = btVector3(btScalar(0.),
				btScalar(fHeight), btScalar(0.));
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(vRoot);

		//Create the main body
		m_bodies[0] = localCreateRigidBody(btScalar(1.),
				offset*transform, m_shapes[0]);
//		printPosition(m_bodies[0]->getCenterOfMassPosition());


		// legs
		for (size_t i=0; i<num_legs; i++)
		{
			float fAngle = 2 * M_PI * i / num_legs;
			float fSin = sin(fAngle);
			float fCos = cos(fAngle);

			transform.setIdentity();
			btVector3 vBoneOrigin = btVector3(
					btScalar(fCos*(fBodySize+0.5*fLegLength)),
					btScalar(fHeight),
					btScalar(fSin*(fBodySize+0.5*fLegLength)));
			transform.setOrigin(vBoneOrigin);

			// thigh
			btVector3 vToBone = (vBoneOrigin - vRoot).normalize();
			btVector3 vAxis = vToBone.cross(vUp);
			transform.setRotation(btQuaternion(vAxis, M_PI_2));
			m_bodies[1+2*i] = localCreateRigidBody(btScalar(1.),
					offset*transform, m_shapes[1+2*i]);

			// shin
			transform.setIdentity();
			transform.setOrigin(btVector3(
					btScalar(fCos*(fBodySize+fLegLength)),
					btScalar(fHeight-0.5*fForeLegLength),
					btScalar(fSin*(fBodySize+fLegLength))));
			m_bodies[2+2*i] = localCreateRigidBody(btScalar(1.),
					offset*transform, m_shapes[2+2*i]);
		}

		// Setup some damping on the m_bodies
		for (size_t i = 0; i < body_part_count; ++i)
		{
			m_bodies[i]->setDamping(0.05, 0.85);
			m_bodies[i]->setDeactivationTime(.0f);
#ifndef BULLET_2_81
			m_bodies[i]->setUserIndex(spider_index);
#endif
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
			localA.getBasis().setEulerZYX(0,-fAngle,0);
			localA.setOrigin(btVector3(
					btScalar(fCos*fBodySize),
					btScalar(0.),
					btScalar(fSin*fBodySize)));
			localB = m_bodies[1+2*i]->getWorldTransform().inverse() *
					m_bodies[0]->getWorldTransform() * localA;

			//Create hip
			hipConstraint = new btGeneric6DofConstraint(*m_bodies[0],
					*m_bodies[1+2*i],
					localA,
					localB,
					true);
			hipConstraint->setLinearLowerLimit(btVector3(0.0,0.0,0.0));
			hipConstraint->setLinearUpperLimit(btVector3(0.0,0.0,0.0));
			hipConstraint->setAngularLowerLimit(btVector3(
					btScalar(rollLowerLimit),
					btScalar(yawLowerLimit),
					btScalar(pitchLowerLimit)));
			hipConstraint->setAngularUpperLimit(btVector3(
					btScalar(rollUpperLimit),
					btScalar(yawUpperLimit),
					btScalar(pitchUpperLimit)));

			//Add hip to the world
			m_hipjoints[i] = hipConstraint;
			m_ownerWorld->addConstraint(m_hipjoints[i], true);

			// knee joints
			localA.setIdentity(); localB.setIdentity();
			localC.setIdentity();
			localA.getBasis().setEulerZYX(0,-fAngle,0);
			localA.setOrigin(btVector3(btScalar(fCos*(fBodySize+fLegLength)),
					btScalar(0.), btScalar(fSin*(fBodySize+fLegLength))));
			localB = m_bodies[1+2*i]->getWorldTransform().inverse() *
					m_bodies[0]->getWorldTransform() * localA;
			localC = m_bodies[2+2*i]->getWorldTransform().inverse() *
					m_bodies[0]->getWorldTransform() * localA;

			//Create knee
			kneeConstraint = new btHingeConstraint(*m_bodies[1+2*i],
					*m_bodies[2+2*i], localB, localC);
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
	}


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


	inline btScalar getOutput(const size_t index) const{
	    dbg::trace trace("spider", DBG_HERE);
#if defined(USE_OUTPUT_POTENTIAL)
	    btScalar output = dnn->getOutputNeuron(index).getPotential() +
	    		dnn->getOutputNeuron(index).getBias();
	    DBO << "Raw actuator output: " << output << std::endl;
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
	    DBO << "Actuator output: " << output << std::endl;
	    return output;
	}


	void step(btScalar timeStep){
		dbg::trace trace("spider", DBG_HERE);
		btVector3 lowerLimits;
		btVector3 upperLimits;
		size_t behavior_index = 0;
		bool query_nn = (steps%m_nnDelay == 0);
		++steps;

		size_t input_index = 0;
		if(query_nn){
			for(size_t i =0; i<_sensors.size(); ++i){
				for(size_t j=0; j< _sensors[i].size(); ++j){
					DBO << "sensor group: " << i
							<< " sensor: " << j
							<< " act.: " << _sensors[i][j]->getActivation()
							<< std::endl;
					dnn->setInput(input_index, _sensors[i][j]->getActivation());
					input_index++;
				}
			}
			dnn->activate();
		}

		size_t cache_i = 0;
		btScalar output;
		for (size_t i=0; i<knee_joint_count; i++){
			if(query_nn){
				output = getOutput(i);
				nn_output_cache[cache_i] = output;
			} else {
				output = nn_output_cache[cache_i];
			}
			cache_i++;
			btHingeConstraint* kneeJoint = m_kneejoints[i];
			btScalar angularVel = _getDesiredAnglularVel(output,
					kneeJoint->getHingeAngle(),
					kneeJoint->getLowerLimit(),
					kneeJoint->getUpperLimit(),
					timeStep);
			energy_used += fabs(angularVel);
			kneeJoint->enableAngularMotor(true, angularVel, m_fMuscleStrength);
		}

		for (size_t i=0; i<hip_pitch_joint_count; i++){
			if(query_nn){
				output = getOutput(i+knee_joint_count);
				nn_output_cache[cache_i] = output;
			}  else {
				output = nn_output_cache[cache_i];
			}
			cache_i++;
			_actuateMotor(m_hipjoints[i], output, pitchAxisIndex, timeStep);
		}

		for (size_t i=0; i<hip_yaw_joint_count; i++){
			if(query_nn){
				output = getOutput(i+knee_joint_count+hip_pitch_joint_count);
				nn_output_cache[cache_i] = output;
			} else {
				output = nn_output_cache[cache_i];
			}
			cache_i++;
			_actuateMotor(m_hipjoints[i], output, yawAxisIndex, timeStep);
		}
	}


	nn_t* getNN(){
		return dnn;
	}

	void setNN(nn_t* nn){
		dnn = nn;
	}

	template<typename Painter>
	void drawSensors(Painter painter){
#ifndef NO_OPEN_GL
		for(size_t i=0; i<_sensors.size(); ++i){
			_sensors[i].draw(painter);
		}
		btScalar m[16];
		painter->pushTransformation();
		m_bodies[0]->getWorldTransform().getOpenGLMatrix(m);
		painter->mult(m);


//		painter->translate(m_bodies[0]->getWorldTransform().getOrigin());
//		btQuaternion rot = m_bodies[0]->getWorldTransform().getRotation();
//		btVector4 rot_v(rot.w(), rot.x(), rot.y(), rot.z());
//		painter->rotate(rot);
		{
			btVector3 scale = btVector3(0.3, 0.3, 0.3);
			btVector3 pos = btVector3(0, 0, 0);
			opengl_draw::Sphere s;
			s.setScale(scale);
			s.setPosition(pos);
			//1.f,1.0f,0.5f
			s.setDefaultMaterialDiffuseColor(opengl_draw::Color(1.5,1.0,0.5));
			s.draw(painter);
		}
		{
			btVector3 scale = btVector3(0.1, 0.1, 0.1);
			btVector3 pos = btVector3(0.15, 0.15, 0.1);
			opengl_draw::Sphere s;
			s.setScale(scale);
			s.setPosition(pos);
			s.setDefaultMaterialDiffuseColor(opengl_draw::Color(0,0,0));
			s.draw(painter);
		}
		{
			btVector3 scale = btVector3(0.1, 0.1, 0.1);
			btVector3 pos = btVector3(0.15, 0.15, -0.1);
			opengl_draw::Sphere s2;
			s2.setScale(scale);
			s2.setPosition(pos);
			s2.setDefaultMaterialDiffuseColor(opengl_draw::Color(0,0,0));
			s2.draw(painter);
		}

		{
			btVector3 scale = btVector3(0.3, 0.3, 0.3);
			btVector3 pos = btVector3(0, 0.23, 0);
			opengl_draw::Pyramid s2;
			s2.setScale(scale);
			s2.setPosition(pos);
			s2.setDefaultMaterialDiffuseColor(opengl_draw::Color(1.5,0,0));
			s2.draw(painter);
		}
		{
			painter->rotate(30, 0, 0, 1);
			btVector3 scale = btVector3(0.3, 0.3, 0.3);
			btVector3 pos = btVector3(0, 0.23, 0);
			opengl_draw::Pyramid s2;
			s2.setScale(scale);
			s2.setPosition(pos);
			s2.setDefaultMaterialDiffuseColor(opengl_draw::Color(1.5,0,0));
			s2.draw(painter);
		}
		{
			painter->rotate(30, 0, 0, 1);
			btVector3 scale = btVector3(0.3, 0.3, 0.3);
			btVector3 pos = btVector3(0, 0.23, 0);
			opengl_draw::Pyramid s2;
			s2.setScale(scale);
			s2.setPosition(pos);
			s2.setDefaultMaterialDiffuseColor(opengl_draw::Color(1.5,0,0));
			s2.draw(painter);
		}

		painter->popTransformation();
#endif
	}

	float getEnergyUsed(){
		return energy_used;
	}

private:
	nn_t* dnn;


	inline btScalar _getDesiredAnglularVel(
			const btScalar& neuron_value,
			const btScalar& current_angle,
			const btScalar& lower_limit,
			const btScalar& upper_limit,
			const btScalar& timeStep)
	{
		btScalar limit_diff = upper_limit - lower_limit;
		btScalar target_angle = lower_limit + ((neuron_value+1)/2) * limit_diff;
		btScalar fAngleError  = target_angle - current_angle;
		btScalar desired_velocity = 1000000.f * fAngleError/timeStep;
		dbg::out(dbg::info, "motors") << " neuron_value: "<< neuron_value
				<< " lower_limit: "<< lower_limit
				<< " upper_limit: "<< upper_limit
				<< " current_angle: "<< current_angle
				<< " target_angle: "<< target_angle
				<< " fAngleError: " << fAngleError
				<< " desired_velocity: " << desired_velocity
				<< std::endl;
//		return 10000000.f * neuron_value;
		return desired_velocity;
	}

	void _actuateMotor(
			btGeneric6DofConstraint* motor,
			btScalar neuronValue,
			int axis,
			btScalar timeStep)
	{
		dbg::trace trace("spider", DBG_HERE);
		btVector3 lowerLimits;
		btVector3 upperLimits;
		motor->getAngularLowerLimit(lowerLimits);
		motor->getAngularUpperLimit(upperLimits);

		btScalar angularVel = _getDesiredAnglularVel(neuronValue,
				motor->getAngle(axis),
				lowerLimits[axis],
				upperLimits[axis],
				timeStep);
		energy_used += fabs(angularVel);
		motor->getRotationalLimitMotor(axis)->m_enableMotor = true;
		motor->getRotationalLimitMotor(axis)->m_targetVelocity = angularVel;
		motor->getRotationalLimitMotor(axis)->m_maxMotorForce = m_fMuscleStrength;
	}
};

}

#undef DBO

#endif /* SPIDER_H_ */
