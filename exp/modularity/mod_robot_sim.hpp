/*
Bullet Continuous Collision Detection and Physics Library Copyright (c)
2007 Erwin Coumans
Modularity Robot Simulator

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use
of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
that you wrote the original software. If you use this software in a product, an
acknowledgment in the product documentation would be appreciated but is not
required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef MOD_ROBOT_SIMULATOR_H
#define MOD_ROBOT_SIMULATOR_H

#ifndef NULL
#define NULL 0
#endif



//#if defined(__APPLE__) && !defined (VMDMESA)
#if defined(BULLET_NOT_IN_BULLET_FOLDER)
#ifdef __GNUC__
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6) || __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif //
#endif //__GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#include "btBulletDynamicsCommon.h"
#ifdef __GNUC__
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6) || __clang__
#pragma GCC diagnostic pop
#endif //__GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#endif //__GNUC__


#include "LinearMath/btAlignedObjectArray.h"
#else// defined(__APPLE__) && !defined (VMDMESA)
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#endif


#include <sferes/dbg/dbg.hpp>
#include <sferes/misc/rand.hpp>
#include <sferes/stc.hpp>

#include <boost/lexical_cast.hpp>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/spirit/include/karma.hpp>

#include "mod_robot_object_group.hpp"
#include "mod_robot_spider.hpp"
#include "mod_robot_global.hpp"

#ifndef NO_OPEN_GL

//#include "GL/freeglut_ext.h"
//#include <GL/freeglut.h>
#include "mod_robot_shape_drawer.hpp"
#include <modules/opengldraw/opengl_draw_painter.hpp>
#include <modules/opengldraw/opengl_draw_piramid.hpp>
#include <modules/opengldraw/opengl_draw_diamond.hpp>
#include <modules/opengldraw/opengl_draw_sphere.hpp>
#include <modules/opengldraw/opengl_draw_text.hpp>
#include <modules/opengldraw/opengl_draw_line.hpp>
#include <modules/opengldraw/opengl_draw_curve.hpp>
#include <modules/opengldraw/opengl_draw_drawable_object.hpp>
#include <modules/opengldraw/opengl_draw_overlay.hpp>
#include <modules/bullet/GlutDemoApplication.h>
#endif
#include <modules/datatools/position3.hpp>


#include "svg_writer.hpp"
#include "clock.hpp"

#if defined(PNGW)
#include "pngwriter.h"
#endif

int gPickingConstraintId = 0;
btVector3 gOldPickingPos;
btVector3 gHitPos(-1,-1,-1);
float gOldPickingDist  = 0.f;
btRigidBody* pickedBody = 0;//for deactivation state
extern int gNumClampedCcdMotions;


#ifndef NO_OPEN_GL
using namespace opengl_draw;
#endif

namespace mod_robot{

class Predator: public Object{
public:
	Predator(){
        _type = 1;
	}

    inline void init(const btVector3& position){
    	typedef btRigidBody::btRigidBodyConstructionInfo rbInfor_t;
        _shape = new btBoxShape(btVector3(1.0,1.0,1.0));
        btTransform transformation;
        btDefaultMotionState* myMotionState;
        transformation.setIdentity();
        transformation.setOrigin(position);
        btVector3 localInertia(0.0,0.0,0.0);
        myMotionState = new btDefaultMotionState(transformation);
        rbInfor_t rbInfo(0.f,myMotionState,_shape,localInertia);
        _body = new btRigidBody(rbInfo);
        _body->setCollisionFlags(_body->getCollisionFlags() |
        		btCollisionObject::CF_KINEMATIC_OBJECT);
        _body->setActivationState( DISABLE_DEACTIVATION );
    }
};

class Food: public Object{
public:
	Food(){
        _type = 0;
	}

    inline void init(const btVector3& position){
    	typedef btRigidBody::btRigidBodyConstructionInfo rbInfor_t;
        _shape = new btBoxShape(btVector3(0.5,0.5,0.5));
        btTransform transformation;
        btDefaultMotionState* myMotionState;
        transformation.setIdentity();
        transformation.setOrigin(position);
        btVector3 localInertia(0,0,0);
        myMotionState = new btDefaultMotionState(transformation);
        rbInfor_t rbInfo(0.f,myMotionState,_shape,localInertia);
        _body = new btRigidBody(rbInfo);
    }
};

class Control: public Object{
public:
    Control(int sensorIndex){
        _type = 2;
        _sensorIndex = sensorIndex;
    }
};


class Poison: public Object{
public:
	Poison(){
        _type = 3;
	}

    inline void init(const btVector3& position){
        _shape = new btBoxShape(btVector3(0.5, 1, 0.5));
        btTransform transformation;
        transformation.setIdentity();
        transformation.setOrigin(position);
        btVector3 li(0,0,0);
        btDefaultMotionState* ms = new btDefaultMotionState(transformation);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, ms, _shape, li);
        _body = new btRigidBody(rbInfo);
    }
};


class Target: public Object{
public:
	Target(bool dangerous=false){
        _type = 4;
        _max_impulse = 0;
        _dangerous = dangerous;
        _color = opengl_draw::Color(0.1, 0.9, 0.1);
        _flash_time = 0;
	}

    inline void init(const btVector3& position){
    	if(_dangerous){
    		_shape = new btCylinderShape(btVector3(2.0, 1.0, 2.0));
    		_color = opengl_draw::Color(0.9, 0.1, 0.1);
    	}else{
    		_shape = new btCylinderShape(btVector3(2.0, 0.1, 2.0));
    		_color = opengl_draw::Color(0.1, 0.9, 0.1);
    	}
        btTransform transformation;
        transformation.setIdentity();
        transformation.setOrigin(position);
        btVector3 li(0,0,0);
        btDefaultMotionState* ms = new btDefaultMotionState(transformation);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f, ms, _shape, li);
        _body = new btRigidBody(rbInfo);
//        _body->setCollisionFlags(_body->getCollisionFlags() |
//            btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
#ifndef BULLET_2_81
        _body->setUserIndex(target_index);
#endif
        _body->setUserPointer(this);
    }

    virtual void step(){
    	if(_flash_time > 0){
    		_flash_time--;
    	} else if(!_dangerous){
    		this->setColor(opengl_draw::Color(0.1, 0.9, 0.1));
    	}
    }

    inline void addImpuls(btScalar impulse){
    	if(impulse > _max_impulse){
    		_max_impulse = impulse;
    	}
    	if(impulse > 10 and !_dangerous){
    		_flash_time = 10;
    		this->setColor(opengl_draw::Color(2.0, 2.0, 2.0));
    	} else if(impulse > 5 and !_dangerous){
    		_flash_time = 5;
    		this->setColor(opengl_draw::Color(1.0, 2.0, 1.0));
    	}
    }

    inline btScalar getMaxImpulse(){
    	return _max_impulse;
    }

protected:
    btScalar _max_impulse;
    bool _dangerous;
    int _flash_time;
};


template<typename Sim>
class Tracker {
public:
    typedef Sim sim_t;
	Tracker(){
		_simulator = 0;
	}

	virtual ~Tracker(){

	}

	virtual void track()
	{
	}

	void setSim(sim_t* simulator){
		_simulator = simulator;
	}

protected:
	sim_t* _simulator;
};


#ifndef NO_OPEN_GL
struct NeuronVisualizeData{
	NeuronVisualizeData(float x,
	float y,
	float z,
	Color color,
	size_t index):
		x(x),
		y(y),
		z(z),
		color(color),
		index(index){

	}

	float x;
	float y;
	float z;
	Color color;
	size_t index;
};

struct ConVisualizeData{
    ConVisualizeData(
    Color color,
    size_t index1,
    size_t index2):
        color(color),
        index1(index1),
        index2(index2){

    }

    Color color;
    size_t index1;
    size_t index2;
};
#endif

//class btBroadphaseInterface;
//class btCollisionShape;
//class btCollisionDispatcher;
//class btConstraintSolver;
//class btDefaultCollisionConfiguration;

class btOverlappingPairCache;
struct btCollisionAlgorithmCreateFunc;

//SFERES_CONST float degrees_to_radians = 0.0174532925;
SFERES_CONST float degrees_to_radians = (2*M_PI)/360;
SFERES_CONST float radians_to_degrees = 360/(2*M_PI);

class SimulatorBase{

public:
	const float STEPSIZE = 5;

	virtual ~SimulatorBase(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void keyboardCallback(unsigned char key, int x, int y){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void keyboardUpCallback(unsigned char key, int x, int y) {
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void specialKeyboard(int key, int x, int y){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void specialKeyboardUp(int key, int x, int y){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void reshape(int w, int h){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void mouseFunc(int button, int state, int x, int y){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void mouseMotionFunc(int x,int y){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void displayCallback(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void renderme(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual	void swapBuffers(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual	void updateModifierKeys(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void clientMoveAndDisplay(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void myinit(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	virtual void updateCamera(){
		dbg::trace trace("sim", DBG_HERE);
		// Intentionally left empty
	}

	void moveAndDisplay(){
		clientMoveAndDisplay();
	}

};


mod_robot::SimulatorBase* gDemoApplication = 0;


static  void glutKeyboardCallback(unsigned char key, int x, int y)
{
    gDemoApplication->keyboardCallback(key,x,y);
}

static  void glutKeyboardUpCallback(unsigned char key, int x, int y)
{
    gDemoApplication->keyboardUpCallback(key,x,y);
}

static void glutSpecialKeyboardCallback(int key, int x, int y)
{
    gDemoApplication->specialKeyboard(key,x,y);
}

static void glutSpecialKeyboardUpCallback(int key, int x, int y)
{
    gDemoApplication->specialKeyboardUp(key,x,y);
}


static void glutReshapeCallback(int w, int h)
{
    gDemoApplication->reshape(w,h);
}

static void glutMoveAndDisplayCallback()
{
    gDemoApplication->moveAndDisplay();
}

static void glutMouseFuncCallback(int button, int state, int x, int y)
{
    gDemoApplication->mouseFunc(button,state,x,y);
}


static void glutMotionFuncCallback(int x,int y)
{
    gDemoApplication->mouseMotionFunc(x,y);
}


static void glutDisplayCallback(void)
{
    gDemoApplication->displayCallback();
}

template<typename Robot>
class ModRobotSimulator: public SimulatorBase
{
public:
    typedef ModRobotSimulator<Robot> this_t;
    typedef Robot robot_t;
    typedef typename robot_t::nn_t nn_t;
    typedef typename nn_t::neuron_t neuron_t;
    typedef Tracker<this_t> tracker_t;


private:
	int m_Time;

	btVector3 origin;
	robot_t* individual;



//	DrawableObject* food;
//	DrawableObject* predator;


	std::vector<ObjectGroup> objects;

	float _distance_traveled;

	//keep the collision shapes, for deletion/cleanup
	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;
	btBroadphaseInterface*	m_broadphase;
	btCollisionDispatcher*	m_dispatcher;
	btConstraintSolver*	m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;

	btDynamicsWorld* m_dynamicsWorld;
	btTypedConstraint* m_pickConstraint;
	btScalar        m_defaultContactProcessingThreshold;

	SFERES_CONST double step_size = 0.01;
	SFERES_CONST btScalar mousePickClamping = 30.f;

	Clock _nn_clock;
	Clock _physics_clock;
	Clock _reset_clock;

	int _fixed_sensor_input;
	int _fixed_predator_input;

	// Food parameters
	Object* closestFood;
	float _food_collected;
	bool _replaceFood;
	btVector3 _previous_food_location;
	double _food_x_min;
	double _food_x_max;
	double _food_y_min;
	double _food_y_max;
	double _food_height;
	double _food_distance;
	double _closest_food_distance;

	// Predator parameters
	Object* closestPredator;
	float _captured_by_predator;
	bool _replacePredator;
	bool _spawnPredatorFront;
	btScalar _predator_speed;
	double _predator_distance;
	double distanceToNearestPredator;

	// Poison parameters
	Object* closestPoison;
	float _poison_collected;
	bool _replacePoison;
	btVector3 _previous_poison_location;
	double _poison_distance;
	double _closest_poison_distance;

	int _max_steps;
	bool _has_to_init;

	std::vector<std::vector<float> > neuron_pos;
	std::vector<sferes::SvgColor> _colors;


	bool _neurons_2d;
	bool _show_io;
	bool _show_text;


	btVector3 euler;

	std::vector<btVector3> _indiv_trajectory;
	bool _paused;

	tracker_t* _tracker;

#ifndef NO_OPEN_GL
	//Overlay related attributes (should be moved to separate class)
    ModRobotShapeDrawer* _shapeDrawer;
    Painter* painter;
	btVector3 _overlay_offset;
	btVector3 _overlay_scale;
	Overlay* _overlay_nn;
	std::vector<WorldObject*> _overlay_neurons;
	std::vector<WorldObject*> _overlay_edges;
    DrawableObject* _overlay;
    btMatrix3x3 _overlayRotation;
    bool _overlay_draw_edges;
    std::vector<NeuronVisualizeData> _neuron_vis_data;
    std::vector<ConVisualizeData> _con_vis_data;
    btVector3 m_sundirection;
    GL_ShapeDrawer*	m_shapeDrawer;
    bool m_enableshadows;
    float m_cameraDistance;
	btVector3 m_cameraPosition;
	btVector3 m_cameraTargetPosition;//look at
	bool _followRobot;
	float m_ele;
	float m_azi;
	float m_zoomStepSize;
	int	m_debugMode;
	btVector3 m_cameraUp;
	int	m_forwardAxis;
	int m_glutScreenWidth;
	int m_glutScreenHeight;
	float m_frustumZNear;
	float m_frustumZFar;
	int	m_ortho;
	int	m_mouseOldX;
	int	m_mouseOldY;
	int	m_mouseButtons;
#endif

	bool _render;

	int _print_network_state;
	std::string _network_filename;

	size_t _frame_width;
	size_t _frame_height;
	size_t _frame_counter;
	std::string _movie_folder;
	std::string _frame_prefix;

public:
	enum object_types{
		food_type = 0,
		predator_type = 1,
        control_type = 2,
		poison_type = 3,
		target_type = 4
	};



	/**********************************
	 * Initialization and destruction *
	 **********************************/
	ModRobotSimulator(){
#ifndef NO_OPEN_GL
	    _shapeDrawer = new ModRobotShapeDrawer ();
	    _shapeDrawer->enableTexture(true);

		m_shapeDrawer = new GL_ShapeDrawer ();
		m_shapeDrawer->enableTexture(true);

        _overlay_nn = 0;
        _followRobot = false;

	    m_sundirection = btVector3(1,-2,1)*1000;

	    m_dynamicsWorld = 0;
	    m_pickConstraint = 0;
	    m_cameraDistance = 15.0;
	    m_debugMode = 0;
	    m_ele = 20.f;
	    m_azi = 0.f;
	    m_cameraPosition = btVector3(0.f,0.f,0.f);
	    m_cameraTargetPosition = btVector3(0.f,0.f,0.f);
	    m_cameraUp = btVector3(0,1,0);
	    m_forwardAxis = 2;
	    m_zoomStepSize = 0.4;
	    m_glutScreenWidth = 0;
	    m_glutScreenHeight = 0;
	    m_frustumZNear = 1.f;
	    m_frustumZFar = 10000.f;
	    m_ortho = 0;
	    m_enableshadows = true;
#endif
		_has_to_init = true;
		m_defaultContactProcessingThreshold = BT_LARGE_FLOAT;
		m_pickConstraint = 0;

		resetToDefaults();
	}

	void initTrue(){
		_has_to_init = true;
	}

	void resetToDefaults(){
		origin = btVector3(0,0,0);
		_previous_food_location = btVector3(0,0,0);
		_previous_poison_location = btVector3(0,0,0);
		_indiv_trajectory.clear();

		individual = NULL;
		_fixed_sensor_input = -1;
		_fixed_predator_input = -1;
		_replaceFood = false;
		_replacePoison = false;
		_replacePredator = false;
		_spawnPredatorFront = false;
		_predator_distance = 10.0;
		_food_distance = 5.0;
		_poison_distance = 5.0;
		_predator_speed = 2;
		_food_x_min = -10;
		_food_x_max = 10;
		_food_y_min = -10;
		_food_y_max = 10;
		_food_height = 0;
		_tracker = 0;
		_render = true;
		_print_network_state = -1;
		_network_filename = "";

		//Default Youtube format
//		_frame_width = 1920;
//		_frame_height = 1080;
        _frame_width = 1024;
        _frame_height = 768;
		_frame_counter = 0;
		//m_debugMode = btIDebugDraw::DBG_NoHelpText;
//		m_dynamicsWorld->setForceUpdateAllAabbs(true);

		_paused = true;
		_neurons_2d = true;
		_show_io = true;
		_show_text = true;

		gDisableDeactivation = true;
	}

	void initPhysics() {
		dbg::trace trace("sim", DBG_HERE);
		//std::cout << "Init physics..." << std::endl;

		//Stuff for visualizing


		btVector3 worldAabbMin(-10000,-10000,-10000);
		btVector3 worldAabbMax(10000,10000,10000);

		m_solver = new btSequentialImpulseConstraintSolver;
		m_collisionConfiguration = new btDefaultCollisionConfiguration();
		m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
		m_broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax);
		m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
		createStuff();

		// Setup the basic world
		m_Time = 0;

#ifndef NO_OPEN_GL
        setTexturing(true);
        setShadows(true);
        //setCameraDistance(btScalar(8.));

		painter = new Painter();
		_overlayRotation.setIdentity();
		_overlay_draw_edges = true;
        //Overlay related attributes (should be moved to separate class)
        euler = btVector3(0,0,0);
        _overlay = new DrawableObject();
        _overlay_offset = btVector3(3, 0, -5);
        _overlay_scale = btVector3(1,1,1);
#endif

		objects.clear();
		objects.push_back(ObjectGroup(food_type));
		objects.push_back(ObjectGroup(predator_type));
		objects.push_back(ObjectGroup(control_type));
		objects.push_back(ObjectGroup(poison_type));
		objects.push_back(ObjectGroup(target_type));



		resetToDefaults();



//		setOverlay();

		clientResetScene();
	}

	void showText(bool flag=true){
		dbg::trace trace("sim", DBG_HERE);
	    _show_text = flag;
	    setOverlay();
	}

	void neurons2d(bool flag=true){
		dbg::trace trace("sim", DBG_HERE);
	    _neurons_2d = flag;
	    setOverlayNN();
	}

    void pause(bool flag=true){
        _paused = flag;
    }

    void setMovieFolder(std::string folder){
        _movie_folder = folder;
    }

    void setFramePrefix(std::string prefix){
        _frame_prefix = prefix;
    }

	void exitPhysics(){
		dbg::trace trace("sim", DBG_HERE);
		removePickingConstraint();

		//Delete all objects
	    removeAllItems();

		//Delete the individual
		if(individual != NULL ) deleteIndividual();

		//Delete rigid bodies
		if(m_dynamicsWorld != NULL){
			for (int i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
			{
				btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				if (body && body->getMotionState())
				{
					delete body->getMotionState();
				}
				m_dynamicsWorld->removeCollisionObject( obj );
				delete obj;
			}
		}

		//Delete all other collision shapes
		for (int j=0;j<m_collisionShapes.size();j++)
		{
			btCollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}
		m_collisionShapes.clear();

#ifndef NO_OPEN_GL
		if(_overlay != NULL){
			delete _overlay;
			_overlay = 0;
		}

		if(painter != NULL){
			delete painter;
			painter = 0;
		}
#endif

		if(m_dynamicsWorld != NULL){
			delete m_dynamicsWorld;
			m_dynamicsWorld = 0;
		}

		if(m_broadphase != NULL){
			delete m_broadphase;
			m_broadphase = 0;
		}

		if(m_dispatcher != NULL){
			delete m_dispatcher;
			m_dispatcher = 0;
		}

		if(m_collisionConfiguration != NULL){
			delete m_collisionConfiguration;
			m_collisionConfiguration = 0;
		}

		if(m_solver != NULL){
			delete m_solver;
			m_solver = 0;
		}
	}


	void deleteStuff()
	{
		dbg::trace trace("sim", DBG_HERE);
		//cleanup in the reverse order of creation/initialization
		if(individual != NULL ) deleteIndividual();

		//delete collision shapes
		for (int j=0;j<m_collisionShapes.size();j++)
		{
			btCollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}

		m_collisionShapes.clear();

//		delete m_solver;
		if(m_broadphase != NULL){
			delete m_broadphase;
			m_broadphase = 0;
		}
	}


	/**
	 * Creates the ground bounding-box.
	 */
	void createStuff(){
		dbg::trace trace("sim", DBG_HERE);

		// Setup a big ground box
		{
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(200.),btScalar(10.),btScalar(200.)));
			m_collisionShapes.push_back(groundShape);
#ifndef BULLET_2_81
			groundShape->setUserIndex(-42);
#endif
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-10,0));
			localCreateRigidBody(btScalar(0.),groundTransform,groundShape);
		}
	}


	virtual ~ModRobotSimulator(){
		exitPhysics();
	}



	void reset(){
	    removeAllItems();
		m_Time = 0;
		btVector3 worldAabbMin(-10000,-10000,-10000);
		btVector3 worldAabbMax(10000,10000,10000);

		//
		if(individual != NULL ) deleteIndividual();
		for (int i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
		{
			btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			m_dynamicsWorld->removeCollisionObject( obj );
			delete obj;
		}

		//
		for (int j=0;j<m_collisionShapes.size();j++)
		{
			btCollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}
		m_collisionShapes.clear();


		delete m_broadphase;
		m_broadphase = new btAxisSweep3 (worldAabbMin, worldAabbMax);
		m_dynamicsWorld->setBroadphase(m_broadphase);

		// Setup a big ground box
		{
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(200.),btScalar(10.),btScalar(200.)));
			m_collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-10,0));
			localCreateRigidBody(btScalar(0.),groundTransform,groundShape);
		}

		_food_collected=0;
		_captured_by_predator=0;
		_poison_collected=0;
		_indiv_trajectory.clear();
	}

	void deleteIndividual(){
		dbg::trace trace("sim", DBG_HERE);
		delete individual;
		individual = NULL;
	}

	void removePickingConstraint(){
	    if (m_pickConstraint && m_dynamicsWorld)
	    {
	        m_dynamicsWorld->removeConstraint(m_pickConstraint);
	        delete m_pickConstraint;
	        //printf("removed constraint %i",gPickingConstraintId);
	        m_pickConstraint = 0;
	        pickedBody->forceActivationState(ACTIVE_TAG);
	        pickedBody->setDeactivationTime( 0.f );
	        pickedBody = 0;
	    }
	}

	void clientResetScene(){
	    removePickingConstraint();

	#ifdef SHOW_NUM_DEEP_PENETRATIONS
	    gNumDeepPenetrationChecks = 0;
	    gNumGjkChecks = 0;
	#endif //SHOW_NUM_DEEP_PENETRATIONS

	    gNumClampedCcdMotions = 0;
	    int numObjects = 0;
	    int i;

	    if (m_dynamicsWorld)
	    {
	        int numConstraints = m_dynamicsWorld->getNumConstraints();
	        for (i=0;i<numConstraints;i++)
	        {
	            m_dynamicsWorld->getConstraint(0)->setEnabled(true);
	        }
	        numObjects = m_dynamicsWorld->getNumCollisionObjects();

	        ///create a copy of the array, not a reference!
	        btCollisionObjectArray copyArray = m_dynamicsWorld->getCollisionObjectArray();




	        for (i=0;i<numObjects;i++)
	        {
	            btCollisionObject* colObj = copyArray[i];
	            btRigidBody* body = btRigidBody::upcast(colObj);
	            if (body)
	            {
	                if (body->getMotionState())
	                {
	                    btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
	                    myMotionState->m_graphicsWorldTrans = myMotionState->m_startWorldTrans;
	                    body->setCenterOfMassTransform( myMotionState->m_graphicsWorldTrans );
	                    colObj->setInterpolationWorldTransform( myMotionState->m_startWorldTrans );
	                    colObj->forceActivationState(ACTIVE_TAG);
	                    colObj->activate();
	                    colObj->setDeactivationTime(0);
	                    //colObj->setActivationState(WANTS_DEACTIVATION);
	                }
	                //removed cached contact points (this is not necessary if all objects have been removed from the dynamics world)
	                btBroadphaseInterface* bp = m_dynamicsWorld->getBroadphase();
	                if (bp->getOverlappingPairCache()){
	                    bp->getOverlappingPairCache()->cleanProxyFromPairs(
	                    		colObj->getBroadphaseHandle(),
								m_dynamicsWorld->getDispatcher());
	                }

	                btRigidBody* body = btRigidBody::upcast(colObj);
	                if (body && !body->isStaticObject())
	                {
	                    btRigidBody::upcast(colObj)->setLinearVelocity(btVector3(0,0,0));
	                    btRigidBody::upcast(colObj)->setAngularVelocity(btVector3(0,0,0));
	                }
	            }

	        }

	        ///reset some internal cached data in the broadphase
	        m_dynamicsWorld->getBroadphase()->resetPool(m_dynamicsWorld->getDispatcher());
	        m_dynamicsWorld->getConstraintSolver()->reset();

	    }
	}



	btRigidBody* localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape)
	{
	    btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	    //rigidbody is dynamic if and only if mass is non zero, otherwise static
	    bool isDynamic = (mass != 0.f);

	    btVector3 localInertia(0,0,0);
	    if (isDynamic)
	        shape->calculateLocalInertia(mass,localInertia);

	    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

	#define USE_MOTIONSTATE 1
	#ifdef USE_MOTIONSTATE
	    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	    btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	    btRigidBody* body = new btRigidBody(cInfo);
	    body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

	#else
	    btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);
	    body->setWorldTransform(startTransform);
	#endif//

	    m_dynamicsWorld->addRigidBody(body);

	    return body;
	}


	/**********************************
	 ****** Neuron visualization ******
	 **********************************/
#ifndef NO_OPEN_GL
	void resetVisData(){
		_neuron_vis_data.clear();
		_con_vis_data.clear();
	}

	void addNeuronVisData(float x, float y, float z, float r, float g, float b, size_t index){
		_neuron_vis_data.push_back(NeuronVisualizeData(x, y, z, Color(r,g,b), index));
	}

    void addConVisData(Color color, size_t index1, size_t index2){
        _con_vis_data.push_back(ConVisualizeData(color, index1, index2));
    }

	void addNeuronPos(float x,float y,float z){
		neuron_pos.push_back(std::vector<float>());
		neuron_pos.back().push_back(x);
		neuron_pos.back().push_back(y);
		neuron_pos.back().push_back(z);
	}


	inline btVector3 getVisualPos(const size_t& neuron_index) const{
        const double layer_distance_2d = 3.5;
        const double layer_y_offset_2d = 0.5;
	    btVector3 result;
        if(_neurons_2d){
            result = btVector3(
                    _neuron_vis_data[neuron_index].x,
                    _neuron_vis_data[neuron_index].z + _neuron_vis_data[neuron_index].y*layer_distance_2d +layer_y_offset_2d,
                    0);
        } else {
            result = btVector3(
                    _neuron_vis_data[neuron_index].x,
                    _neuron_vis_data[neuron_index].y,
                    _neuron_vis_data[neuron_index].z);
        }
        return result;
	}


	void setOverlayOffset(btVector3 offset){
            _overlay_offset = offset;
	}

	inline DrawableObject* getOverlay(){
	    return _overlay;
	}

	inline void setOverlay(){
		dbg::trace trace("sim", DBG_HERE);
		//Print some debug text on screen

	    // Destroy the old overlay (should become a destructor)
	    // Note that the overlay itself handles memory management,
	    // once deleteChildren is called, all objects are destroyed.
		_overlay->deleteChildren();

		if(_show_text){
		    //_overlay->addDrawableChild(new Text("Food collected: " + toString(_food_collected), 160 + 40*6, 20));
		    //		_overlay->addDrawableChild(new Text("Predator sensor: ", 20, 40));
		    //_overlay->addDrawableChild(new Text("Captured by predator: " + toString(_captured_by_predator), 160 + 40*6, 40));
		    _overlay->addDrawableChild(new Text("Time: " + toString(m_Time) + " of max: " + toString(_max_steps), 20, 60));
		}

		_overlay_nn = new Overlay();
		_overlay->addDrawableChild(_overlay_nn);
		setOverlayNN();
	}

    inline void setOverlayNN(){
		dbg::trace trace("sim", DBG_HERE);
        //Print some debug text on screen

        // Destroy the old overlay (should become a destructor)
        // Note that the overlay itself handles memory management,
        // once deleteChildren is called, all objects are destroyed.
        _overlay_neurons.clear();
        _overlay_edges.clear();
        dbg::out(dbg::info, "sim") << "Resetting overlay" << std::endl;
        dbg::check_ptr(_overlay_nn, DBG_HERE);
        _overlay_nn->deleteChildren();
        dbg::out(dbg::info, "sim") << "Children after reset: " <<
        		_overlay_nn->getChildren().size() << std::endl;
        _overlay_nn->setTranslation(_overlay_offset);
        btQuaternion quat;
        _overlayRotation.getRotation(quat);
        btVector3 axis = quat.getAxis();
        btVector4 rotation;
        rotation.setX(axis.getX());
        rotation.setY(axis.getY());
        rotation.setZ(axis.getZ());
        rotation.setW(quat.getAngle()*radians_to_degrees);
        _overlay_nn->setRotation(rotation);
        _overlay_nn->setScale(_overlay_scale);

//      std::cout << "x: " << neuron_pos[0][0] << " y: " << neuron_pos[0][1] << " z: " << neuron_pos[0][2] <<std::endl;

        dbg::out(dbg::info, "sim") << "Retrieving network" << std::endl;
        if(!individual){
        	std::cerr << "WARNING: Robot is 0; can't set overlay." << std::endl;
        	return;
        }
        dbg::check_ptr(individual, DBG_HERE);
        nn_t* nn = individual->getNN();
        dbg::check_ptr(nn, DBG_HERE);

        double base = 0.05;
        double scale = 0.05;

        dbg::out(dbg::info, "sim") << "Adding neurons to overlay: "
        		 << _neuron_vis_data.size() << std::endl;
        for(size_t i=0; i<_neuron_vis_data.size(); i++){
            neuron_t* neuron = nn->getNeuron(_neuron_vis_data[i].index);
            double output = neuron->getOutput()*scale;
            Sphere* sphere = new Sphere();
            sphere->setScale(btVector3(
            		base + output,
            		base + output,
					base + output));
            sphere->setPosition(getVisualPos(i));
            sphere->setDefaultMaterialDiffuseColor(_neuron_vis_data[i].color);
            _overlay_nn->addDrawableChild(sphere);
            _overlay_neurons.push_back(sphere);
        }

        dbg::out(dbg::info, "sim") << "Adding connections to overlay: "
        		<< _neuron_vis_data.size() * _neuron_vis_data.size() << std::endl;
        if(_overlay_draw_edges){
        	size_t con_index = 0;
        	for(size_t i=0; i<_neuron_vis_data.size(); i++){
        		for(size_t j=0; j<_neuron_vis_data.size(); j++){
        			double con = nn->getConnection(_neuron_vis_data[i].index,
        					_neuron_vis_data[j].index);
        			bool isIo = nn->isInput(_neuron_vis_data[i].index) ||
        					nn->isOutput(_neuron_vis_data[j].index);

        			if(con != 0.0 && (!isIo || _show_io)){
        				btVector3 sPos = getVisualPos(i);
        				btVector3 tPos = getVisualPos(j);
        				WorldObject* line;
        				if((sPos.x() == tPos.x() || sPos.z() == tPos.z()) &&
        						sPos.y() == 0 && tPos.y() == 0){
        					btScalar dist = sPos.distance(tPos);
        					btScalar curve = dist*0.1;
        					if(dist < 1.0){
        						line = new Line(sPos, tPos);
        						line->setLineWidth(2.0f);
        					}else if(sPos.x() < 0){
        						line = new Curve(sPos,
        								sPos + btVector3(-curve, 0, 0),
										tPos + btVector3(-curve, 0, 0),
										tPos, 0.01);
        						line->setLineWidth(0.01f);
        					} else {
        						line = new Curve(sPos,
        								sPos + btVector3(curve, 0, 0),
										tPos + btVector3(curve, 0, 0),
										tPos, 0.01);
        						line->setLineWidth(0.01f);
        					}

        				} else {
        					line = new Line(sPos, tPos);
        					line->setLineWidth(2.0f);
        				}
        				Color line_color;
        				if(con_index < _con_vis_data.size()){
        					line_color = _con_vis_data[con_index].color;
        				} else if(_neuron_vis_data[i].color == _neuron_vis_data[j].color){
        					line_color = _neuron_vis_data[i].color;
        				} else {
        					line_color = Color(0, 0, 0);
        				}
        				line_color.setA(0.2);
        				line->setDefaultMaterialFlatColor(line_color);
        				_overlay_nn->addDrawableChild(line);
        				con_index++;
        			}
        		}
        	}
        }

        dbg::out(dbg::info, "sim") << "Children after adding:" <<
        		_overlay_nn->getChildren().size() << std::endl;
    }


    inline void updateOverlay(){
        //Print some debug text on screen

//        Overlay* overlay = new Overlay();
        _overlay_nn->setTranslation(_overlay_offset);
        btQuaternion quat;
        _overlayRotation.getRotation(quat);
        btVector3 axis = quat.getAxis();
        btVector4 rotation;
        rotation.setX(axis.getX());
        rotation.setY(axis.getY());
        rotation.setZ(axis.getZ());
        rotation.setW(quat.getAngle()*radians_to_degrees);
        _overlay_nn->setRotation(rotation);
        _overlay_nn->setScale(_overlay_scale);

//      std::cout << "x: " << neuron_pos[0][0] << " y: " << neuron_pos[0][1] << " z: " << neuron_pos[0][2] <<std::endl;

        nn_t* nn = individual->getNN();

        double base = 0.05;
        double scale = 0.05;


        for(size_t i=0; i<_neuron_vis_data.size(); i++){
            neuron_t* neuron = nn->getNeuron(_neuron_vis_data[i].index);
            double output = neuron->getOutput()*scale;
            _overlay_neurons[i]->setScale(btVector3(base + output, base + output, base + output));
            _overlay_neurons[i]->setPosition(getVisualPos(i));
        }
    }

	bool	setTexturing(bool enable) { return(m_shapeDrawer->enableTexture(enable)); }
	bool	setShadows(bool enable)	{ bool p=m_enableshadows;m_enableshadows=enable;return(p); }

	void setCameraDistance(float dist)
	{
		m_cameraDistance  = dist;
	}

	float getCameraDistance()
	{
		return m_cameraDistance;
	}

	void setFollowRobot(bool follow){
		_followRobot = follow;
	}

	void stepLeft() {
		m_azi -= STEPSIZE; if (m_azi < 0) m_azi += 360; updateCamera();
	}
	void stepRight() {
		m_azi += STEPSIZE; if (m_azi >= 360) m_azi -= 360; updateCamera();
	}
	void stepFront() {
		m_ele += STEPSIZE; if (m_ele >= 360) m_ele -= 360; updateCamera();
	}
	void stepBack() {
		m_ele -= STEPSIZE; if (m_ele < 0) m_ele += 360; updateCamera();
	}
	void zoomIn() {
		m_cameraDistance -= btScalar(m_zoomStepSize); updateCamera();
		if (m_cameraDistance < btScalar(0.1))
			m_cameraDistance = btScalar(0.1);

	}
	void zoomOut() {
		m_cameraDistance += btScalar(m_zoomStepSize); updateCamera();

	}

	int	getDebugMode()
	{
		return m_debugMode ;
	}

	void setDebugMode(int mode){
		m_debugMode = mode;
	}

	void reshape(int w, int h) {
		GLDebugResetFont(w,h);

		m_glutScreenWidth = w;
		m_glutScreenHeight = h;

		glViewport(0, 0, w, h);
		updateCamera();
	}

#else
    inline void addNeuronVisData(float x, float y, float z, float r, float g, float b, size_t index){}
    inline void addNeuronPos(float x,float y,float z){}
    inline void setOverlayOffset(btVector3 offset){}
    inline void setOverlay(){}
    inline void setOverlayNN(){}
    inline void updateOverlay(){}
#endif


	/**********************************
	 ******* Getters and Setters ******
	 **********************************/
	inline void setTracker(tracker_t* tracker){
		dbg::trace trace("sim", DBG_HERE);
		_tracker = tracker;
		if(_tracker){
		    _tracker->setSim(this);
		}
	}

	inline void deleteTracker(){
	    if(_tracker){
	        delete _tracker;
	        _tracker = 0;
	    }
	}

	inline btVector3 getFoodPos(){
		dbg::trace trace("sim", DBG_HERE);
		return getClosestObject(objects[food_type])->getPosition();
	}

	inline btVector3 getPreviousFoodPos(){
		dbg::trace trace("sim", DBG_HERE);
		return _previous_food_location;
	}

	inline btVector3 getPredatorPos(){
		dbg::trace trace("sim", DBG_HERE);
		return getClosestObject(objects[predator_type])->getPosition();
	}

	inline double getPredatorDist(){
		dbg::trace trace("sim", DBG_HERE);
		return _predator_distance;
	}

	inline void setFoodDistance(const double& food_distance){
		dbg::trace trace("sim", DBG_HERE);
		_food_distance = food_distance;
	}

	inline void setPredatorDistance(const double& predator_distance){
		dbg::trace trace("sim", DBG_HERE);
		_predator_distance = predator_distance;
	}

	inline void setPoisonDistance(const double& poison_distance){
		dbg::trace trace("sim", DBG_HERE);
		_poison_distance = poison_distance;
	}

	inline btVector3 getPoisonPos(){
		dbg::trace trace("sim", DBG_HERE);
		return getClosestObject(objects[poison_type])->getPosition();
	}

	inline btVector3 getPreviousPoisonPos(){
		dbg::trace trace("sim", DBG_HERE);
		return _previous_poison_location;
	}

	inline void setColors(std::vector<sferes::SvgColor> colors){
		dbg::trace trace("sim", DBG_HERE);
		_colors = colors;
	}

	inline void setReplaceFood(bool replaceFood = true){
		dbg::trace trace("sim", DBG_HERE);
		_replaceFood = replaceFood;
	}

	inline void setReplacePredator(bool replacePredator = true){
		dbg::trace trace("sim", DBG_HERE);
		_replacePredator = replacePredator;
	}

	inline void setReplacePoison(bool replacePoison = true){
		dbg::trace trace("sim", DBG_HERE);
		_replacePoison = replacePoison;
	}

	inline void setSpawnPredatorFront(bool spawnPredatorFront = true){
		dbg::trace trace("sim", DBG_HERE);
		_spawnPredatorFront = spawnPredatorFront;
	}

	inline void setFixedSensorInput(int fixed_sensor_input){
		dbg::trace trace("sim", DBG_HERE);
		_fixed_sensor_input = fixed_sensor_input;
	}

	inline double getFoodHeight(){
		dbg::trace trace("sim", DBG_HERE);
		return _food_height;
	}

	inline void setFoodHeight(const double& food_height){
		dbg::trace trace("sim", DBG_HERE);
		_food_height = food_height;
	}

	inline double getFoodDistance(){
		dbg::trace trace("sim", DBG_HERE);
		return _food_distance;
	}

//	inline void setFoodDistance(const double& food_distance){
//		dbg::trace trace("sim", DBG_HERE);
//		_food_distance = food_distance;
//	}

	inline double getNNTime(){
		dbg::trace trace("sim", DBG_HERE);
		return _nn_clock.time();
	}

	inline double getPhysicsTime(){
		dbg::trace trace("sim", DBG_HERE);
		return _physics_clock.time();
	}

	inline double getResetTime(){
		dbg::trace trace("sim", DBG_HERE);
		return _reset_clock.time();
	}

	inline float getFoodCollected(){
		dbg::trace trace("sim", DBG_HERE);
		return _food_collected;
	}

	float getCapturedByPredator(){
		dbg::trace trace("sim", DBG_HERE);
		return _captured_by_predator;
	}

	inline float getPoisonCollected(){
		dbg::trace trace("sim", DBG_HERE);
		return _poison_collected;
	}


	float getDistanceTraveled(){
		dbg::trace trace("sim", DBG_HERE);
		return _distance_traveled;
	}

    btScalar getDistance(){
    	dbg::trace trace("sim", DBG_HERE);
    	return individual->getPosition().distance(origin);
    }

    btScalar getInverseDistanceToClosestFood(btScalar max_value = 0.9999){
    	dbg::trace trace("sim", DBG_HERE);
    	btScalar distance_to_nearest_fooditem = getDistanceToClosestFood();
    	return distance_to_nearest_fooditem == 0 ? max_value : std::min(1/distance_to_nearest_fooditem, max_value);
    }

    btScalar getInverseDistanceToClosestPredator(btScalar max_value = 0.9999){
    	dbg::trace trace("sim", DBG_HERE);
    	btScalar distance_to_nearest_predator = getDistanceToClosestPredator();
    	return distance_to_nearest_predator == 0 ? max_value : std::min(1/distance_to_nearest_predator, max_value);
    }

    btScalar getDistanceToClosestFood(){
    	dbg::trace trace("sim", DBG_HERE);
//		return getClosestFood()->getPosition().distance(spider->getPosition());
		return getClosestObject(objects[food_type])->getPosition().distance(individual->getPosition());
    }

    btScalar getDistanceToClosestPredator(){
    	dbg::trace trace("sim", DBG_HERE);
		return getClosestObject(objects[predator_type])->getPosition().distance(individual->getPosition());
    }

    btScalar getDistanceToClosestPoison(){
    	dbg::trace trace("sim", DBG_HERE);
		return getClosestObject(objects[poison_type])->getPosition().distance(individual->getPosition());
    }

	robot_t* getIndividual(){
		dbg::trace trace("sim", DBG_HERE);
		return individual;
	}

	void setIndividual(robot_t* _spider){
		dbg::trace trace("sim", DBG_HERE);
		if(individual != NULL ) deleteIndividual();
		individual = _spider;
		setOverlay();
	}

	btVector3 getIndivPos(){
		dbg::trace trace("sim", DBG_HERE);
		return individual->getPosition();
	}

	btVector3 getIndivHeading(){
	    dbg::trace trace("sim", DBG_HERE);

	    return individual->getHeading();
	}

	void setWindowSize(size_t width, size_t height){
		_frame_width = width;
		_frame_height = height;
	}

	Object* getClosestObject(ObjectGroup& objects){
    	dbg::trace trace("sim", DBG_HERE);
		btVector3 spiderPos = individual->getPosition();
		Object* nearestObject = 0;
		double distanceObject = INFINITY;

		if(objects.size()==0){
			return 0;
		}

		for (size_t i=0; i<objects.size(); ++i){
		    Object* object = objects[i];
			double distanceToCurrentObject = object->getPosition().distance(spiderPos);
			if(distanceToCurrentObject < distanceObject){
				nearestObject = object;
				distanceObject = distanceToCurrentObject;
			}
		}
		return nearestObject;
    }

    btScalar getPosInDirection(btScalar direction){
    	dbg::trace trace("sim", DBG_HERE);
    	btVector3 direction_vector(1,0,0);
    	btVector3 up_vector(0,1,0);
    	direction_vector = direction_vector.rotate(up_vector, direction);
    	return getPosInDirection(direction_vector);
    }

    btScalar getPosInDirection(btVector3 direction){
    	dbg::trace trace("sim", DBG_HERE);
    	direction.normalize();
    	return individual->getPosition().dot(direction);
    }

	btDynamicsWorld* getWorld(){
		dbg::trace trace("sim", DBG_HERE);
		return m_dynamicsWorld;
	}


	/**********************************
	 ********* Step functions *********
	 **********************************/
	inline void steps(const int& nrOfSteps, const bool& visual = true){
		dbg::trace trace("sim", DBG_HERE);
		if(visual){
			stepsVisual(nrOfSteps);
		} else {
			stepsNoVisual(nrOfSteps);
		}
	}


	void stepsNoVisual(const int& nrOfSteps){
		dbg::trace trace("sim", DBG_HERE);
		_max_steps = -1;
		for(int i=0; i<nrOfSteps; i++){
			step();
		}
	}


#ifndef NO_OPEN_GL
	void stepsVisual(const int& nrOfSteps){
		dbg::trace trace("sim", DBG_HERE);
		_max_steps = nrOfSteps;
		m_Time = 0;

		int argc = 0;
		char **argv = new char*[0];

		dbg::out(dbg::info, "sim") << "Has to init: " << _has_to_init << std::endl;
		if(_has_to_init){
			try{
//				glutmain(argc, argv,640,480,"Spider visualization", this);
			    gDemoApplication = this;

			    glutInit(&argc, argv);
			    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
			    glutInitWindowPosition(0, 0);
			    dbg::out(dbg::info, "sim") << "Win size: " << _frame_width << _frame_height << std::endl;
			    glutInitWindowSize(_frame_width, _frame_height);
			    glutCreateWindow("Spider visualization");
			    dbg::out(dbg::info, "glversion") << glGetString(GL_VERSION) << std::endl;
#if defined(RECORD)
			    align_network_side();
#endif

			#if BT_USE_FREEGLUT
			    glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
			#endif

			    gDemoApplication->myinit();

			    glutKeyboardFunc(glutKeyboardCallback);
			    glutKeyboardUpFunc(glutKeyboardUpCallback);
			    glutSpecialFunc(glutSpecialKeyboardCallback);
			    glutSpecialUpFunc(glutSpecialKeyboardUpCallback);

			    glutReshapeFunc(glutReshapeCallback);
			    //createMenu();
			    glutIdleFunc(glutMoveAndDisplayCallback);
			    glutMouseFunc(glutMouseFuncCallback);
			    glutPassiveMotionFunc(glutMotionFuncCallback);
			    glutMotionFunc(glutMotionFuncCallback);
			    glutDisplayFunc( glutDisplayCallback );

//			    display();
			    //Perform a single update, but don't step
			    bool temp = _paused;
			    _paused = true;
			    glutMoveAndDisplayCallback();
			    _paused = temp;

			//enable vsync to avoid tearing on Apple (todo: for Windows)
			#if defined(__APPLE__) && !defined (VMDMESA)
			    int swap_interval = 1;
			    CGLContextObj cgl_context = CGLGetCurrentContext();
			    CGLSetParameter(cgl_context, kCGLCPSwapInterval, &swap_interval);
			#endif
				glClearColor(btScalar(1.0),btScalar(1.0),btScalar(1.0),btScalar(0));
				glEnable( GL_MULTISAMPLE );
				glEnable(GL_LINE_SMOOTH); //Antialiasing
				//glEnable(GL_LINE_STIPPLE);
				glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
				
				glEnable(GL_POINT_SMOOTH); //This is important so that the joins are round

				//glutReshapeWindow(_frame_width, _frame_height);
				_has_to_init = false;
				glutMainLoop();
			} catch(bool exit){
				//std::cout << "Exit" <<std::endl;
				_max_steps = -1;
			}

		} else{
			try{

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif
//			    display();
				glutMainLoop();
#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#endif
#endif
			} catch(bool exit){
				//std::cout << "Exit" <<std::endl;
				_max_steps = -1;
			}
		}

		delete[] argv;
	}
#else
	void stepsVisual(const int& nrOfSteps){
	    stepsNoVisual(nrOfSteps);
	}
#endif


	void step(){
		dbg::trace trace("sim", DBG_HERE);
		if (m_dynamicsWorld)
		{
			followRobot();
			m_Time+=1;
			btVector3 spiderPos = individual->getPosition();
			_indiv_trajectory.push_back(spiderPos);

			//Set sensors
			individual->setSensors(objects);

			//Step spider
			_nn_clock.start();
			individual->step(btScalar(step_size)*1000000);
			_nn_clock.stop();

			//Step simulator
			dbg::out(dbg::info, "sim") << "Step simulator " << std::endl;
			_physics_clock.start();
			m_dynamicsWorld->stepSimulation(btScalar(step_size), 1, btScalar(step_size));
			_physics_clock.stop();

			// Process collisions
#ifndef BULLET_2_81
			for(int i=0; i<objects[target_type].size(); ++i){
				objects[target_type][i]->step();
			}

			btPersistentManifold* contactManifold;
			btDispatcher* disp = m_dynamicsWorld->getDispatcher();
			int numManifolds = disp->getNumManifolds();
			for (int i = 0; i < numManifolds; i++){
				contactManifold =  disp->getManifoldByIndexInternal(i);
				const btCollisionObject* obA = contactManifold->getBody0();
				const btCollisionObject* obB = contactManifold->getBody1();
				const btCollisionObject* spider = 0;
				const btCollisionObject* target = 0;
				if(obA->getUserIndex() == spider_index){
					spider = obA;
				} else if(obB->getUserIndex() == spider_index){
					spider = obB;
				}
				if(obA->getUserIndex() == target_index){
					target = obA;
				} else if(obB->getUserIndex() == target_index){
					target = obB;
				}

				if(target && spider){
					int numContacts = contactManifold->getNumContacts();
					for (int j = 0; j < numContacts; j++){
						btManifoldPoint& pt = contactManifold->getContactPoint(j);
						if (pt.getDistance() < 0.f){
							Target* t = (Target*) target->getUserPointer();
							t->addImpuls(pt.m_appliedImpulse);
//							std::cout << "Impact: " << pt.m_appliedImpulse << std::endl;
//							const btVector3& ptA = pt.getPositionWorldOnA();
//							const btVector3& ptB = pt.getPositionWorldOnB();
//							const btVector3& normalOnB = pt.m_normalWorldOnB;
						}
					}
				}
			}
#endif

			//Process food
			closestFood = getClosestObject(objects[food_type]);
			dbg::out(dbg::info, "sim") << "Processing food: " << closestFood << std::endl;
			if(closestFood){
			    _closest_food_distance = closestFood->getPosition().distance(spiderPos);
			    dbg::out(dbg::info, "sim") << "Distance food: " << _closest_food_distance << std::endl;
			    if(individual->collisionTest(closestFood)){
			        _previous_food_location = closestFood->getPosition();
			        removeItem(closestFood);
			        _food_collected += 1;
			        if(_replaceFood){
			            addFood();
			        }
			    }
			}


			//Process poison
			closestPoison = getClosestObject(objects[poison_type]);
			dbg::out(dbg::info, "sim") << "Processing food: " << closestPoison << std::endl;
			if(closestPoison){
				_closest_poison_distance = closestPoison->getPosition().distance(spiderPos);
			    dbg::out(dbg::info, "sim") << "Distance food: " << _closest_poison_distance << std::endl;
			    if(individual->collisionTest(closestPoison)){
			        _previous_poison_location = closestPoison->getPosition();
			        removeItem(closestPoison);
			        _poison_collected += 1;
			        if(_replacePoison){
			            addFood();
			        }
			    }
			}

			//Process predator
			closestPredator = getClosestObject(objects[predator_type]);
			dbg::out(dbg::info, "sim") << "Processing predator" << closestPredator << std::endl;
			if(closestPredator){
				distanceToNearestPredator = closestPredator->getPosition().distance(spiderPos);
				dbg::out(dbg::info, "sim") << "Distance predator: " << distanceToNearestPredator << std::endl;

				//Move predator
				btVector3 vectorToSpider  = spiderPos - closestPredator->getPosition();
				vectorToSpider.normalize();
				closestPredator->move(vectorToSpider*btScalar(step_size)*_predator_speed);

				if(individual->collisionTest(closestPredator)){
				    removeItem(closestPredator);
					_captured_by_predator += 1;
					if(_replacePredator){
						addPredator();
					}
				}
			}

			dbg::out(dbg::info, "sim") << "Step: " <<  m_Time
					<< " " << spiderPos.getX()
					<< " " << spiderPos.getY()
					<< " " << spiderPos.getZ() <<std::endl;
			if(_max_steps != -1 && m_Time > _max_steps){
			    dbg::out(dbg::info, "sim") << "Leaving simulator loop" <<std::endl;

#if BT_USE_FREEGLUT
			    _has_to_init = true;
			    glutLeaveMainLoop();
#else
				throw true;
#endif
			}

			if(_tracker){
			    dbg::out(dbg::info, "sim") << "Tracking..." <<std::endl;
				_tracker->track();
				dbg::out(dbg::info, "sim") << "Tracking done" <<std::endl;
			}
		}
	}

	//Convenience functions
	void addFood(){
		dbg::trace trace("sim", DBG_HERE);
		btVector3 center = individual->getPosition();
		center.setY(_food_height);
		addItemRandomAngle(new Food(), _food_distance, center);
	}

	void addPredator(){
		dbg::trace trace("sim", DBG_HERE);
		if(_spawnPredatorFront){
			addPredatorFront();
		} else {
			btVector3 center = individual->getPosition();
			center.setY(1);
			addItemRandomAngle(new Predator(), _predator_distance, center);
		}
	}

	void addPoison(){
		dbg::trace trace("sim", DBG_HERE);
		btVector3 center = individual->getPosition();
		center.setY(_food_height);
		addItemRandomAngle(new Poison(), _poison_distance, center);
	}

	void addPredatorFront(int hist = 20){
		dbg::trace trace("sim", DBG_HERE);
		btVector3 center = individual->getPosition();
		btVector3 oldPos;
		size_t index = 0;
		if(_indiv_trajectory.size() >= hist){
			oldPos = _indiv_trajectory[_indiv_trajectory.size() - hist];
		} else if(!_indiv_trajectory.empty()) {
			oldPos = _indiv_trajectory[0];
		} else{
			oldPos = center;
		}

		btVector3 indivHeading = center - oldPos;
		if(indivHeading.length() == 0){
			addPredator();
		} else{
			indivHeading.normalize();
			btVector3 newPredatorPos = indivHeading*_predator_distance + center;
			newPredatorPos.setY(1);
			addItem(new Predator(), newPredatorPos);
		}
	}




//	void setupRun(Spider::nn_t* dnn){
//		dbg::trace trace("sim", DBG_HERE);
//		_nn_clock.reset();
//		_physics_clock.reset();
//		setIndividual(dnn);
//	}



//	void evaluate(Spider::nn_t* dnn, int nr_steps = 1000){
//		dbg::trace trace("sim", DBG_HERE);
//		setupRun(dnn);
//		steps(nr_steps);
//	}

	//TODO: This function is really only related to the ctrnn, it should not be a part of the simulator
	template<typename BEHAVIOR>
	void reaction(nn_t* dnn, std::vector<BEHAVIOR>& behavior, int input = 0, int steps = 5){
	    dbg::trace trace("sim", DBG_HERE);
	    dbg::out(dbg::info, "sim") << "Input " << input << " DNN inputs " << dnn->getNumberOfInputs() <<std::endl;
//	    dbg::out(dbg::info, "sim") << "DNN inputs: " << dnn->getNumberOfInputs() << std::endl;
		for(size_t i =0; i<dnn->getNumberOfInputs(); i++){
			if(i == input){
				dnn->setInput(i, 1.0);
			} else {
				dnn->setInput(i, 0.0);
			}
		}

		dbg::out(dbg::info, "sim") << "Steps " << steps <<std::endl;
		for(int i =0; i < steps; i ++){
			dnn->activate();

			BEHAVIOR this_behavior;
			dbg::out(dbg::info, "sim") << "Step " << i << " behavior size " << this_behavior.size() <<std::endl;
			for (size_t j=0; j < this_behavior.size(); j++){
				btScalar output = dnn->getOutput(j);
				this_behavior[j] = output > 0 ? 1 : 0;
			}
			behavior.push_back(this_behavior);
		}
	}

	/**********************************
	 ********* Item functions *********
	 **********************************/
	inline void addItemRandomPosition(Object* object, const size_t& nrOfItems = 1, const double& y = 0){
		dbg::trace trace("sim", DBG_HERE);
		for(int i =0; i<nrOfItems ; i++){
			double x = sferes::misc::rand<double>(_food_x_min, _food_x_max);
			double z = sferes::misc::rand<double>(_food_y_min, _food_y_max);
			addItem(object, x, y, z);
		}
	}

	inline void addItem(Object* object, const double& x, const double& z){
		dbg::trace trace("sim", DBG_HERE);
		addItem(object, x, 0, z);
	}

	inline void addItem(Object* object,
			const double& x,
			const double& y,
			const double& z)
	{
		dbg::trace trace("sim", DBG_HERE);
		addItem(object, btVector3(x, y, z));
	}

	inline void addItem(Object* object, const btVector3& pos){
		dbg::trace trace("sim", DBG_HERE);
		object->init(pos);
//		object->setPosition(pos);
		m_dynamicsWorld->addRigidBody(object->getRigidBody());
//		m_dynamicsWorld->updateSingleAabb(object->getRigidBody());
		objects[object->getType()].add(object);
	}

	/**
	 * Adds an item without a specific position.
	 *
	 * This is especially useful for adding abstract objects like Control objects,
	 * which have no physical place in this world.
	 *
	 * Added objects are passed to the sensors, and each sensor is updated with
	 * every object that matches one of its assigned object groups.
	 */
	inline void addItem(Object* object){
	    dbg::trace trace("sim", DBG_HERE);
	    objects[object->getType()].add(object);
	}

	inline void addItemAngleDistance(Object* object,
			const double& angle,
			const double& distance,
			const btVector3& center = btVector3(0,0,0))
	{
		dbg::trace trace("sim", DBG_HERE);
		dbg::out(dbg::info, "sim") << "add item: " << object <<
				" at angle: " << angle <<
				" distance: " << distance <<
				" around: " << center << std::endl;
		btVector3 vector(distance, 0, 0);
		vector = vector.rotate(btVector3(0, 1, 0), angle*degrees_to_radians);
		vector += center;
		addItem(object, vector.getX(), vector.getY(), vector.getZ());
	}

	inline void addItemRandomAngle(Object* object,
			const double& distance,
			const btVector3& center = btVector3(0,0,0))
	{
		dbg::trace trace("sim", DBG_HERE);
		double angle = sferes::misc::rand<double>(0, 360);
		addItemAngleDistance(object, angle, distance, center);
	}

	inline void removeItem(Object* object){
	    dbg::trace trace("sim", DBG_HERE);
	    if(object->getRigidBody()){
	        m_dynamicsWorld->removeRigidBody(object->getRigidBody());
	    }
	    objects[object->getType()].remove(object);
	}

    inline void removeAllItems(){
        dbg::trace trace("sim", DBG_HERE);
        for(size_t i=0; i<objects.size(); i++){
            for(size_t j=0; j<objects[i].size(); ++j){
                if(objects[i][j]->getRigidBody()){

                    m_dynamicsWorld->removeRigidBody(objects[i][j]->getRigidBody());
                }
            }
            objects[i].clear();
        }
    }



	/**********************************
	 ******* Callback functions *******
	 **********************************/
	virtual void clientMoveAndDisplay(){
		dbg::trace trace("sim", DBG_HERE);
		if(!_paused){
			step();
		}
        if(_print_network_state == 0){
            align_network();
            _print_network_state ++;
        } else if(_print_network_state == 1){
            align_network();
            _print_network_state++;
        } else if(_print_network_state == 2){
            print_network();
            _print_network_state++;
        } else if(_print_network_state == 3){
            _print_network_state++;
            throw true;
        }
        updateOverlay();
		display();
#if defined(RECORD)
        if(!_paused && m_dynamicsWorld){
            saveFrame();
        }
#endif
	}


	virtual void displayCallback(){
		dbg::trace trace("sim", DBG_HERE);
		display();
	}


#ifndef NO_OPEN_GL
	void keyboardCallback(unsigned char key, int x, int y){
		dbg::trace trace("sim", DBG_HERE);
        int width = 500;
        int height = 450;
        double ratio = (double)width/ (double)height;
		switch (key)
		{

		// Rotate layout: []{};'
		case '[':{
			euler.setX(euler.x()-0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
			break;
		}
		case ']':{
			euler.setX(euler.x()+0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
			break;
		}
		case '{':{
			euler.setY(euler.y()-0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());

			break;
		}
		case '}':{
			euler.setY(euler.y()+0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
			break;
		}
		case ';':{
			euler.setZ(euler.z()-0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
			break;
		}
		case '\'':{
			euler.setZ(euler.z()+0.1);
			_overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
			break;
		}
		case '|':{
			_overlay_draw_edges = !_overlay_draw_edges;
			setOverlayNN();
			break;
		}

        // Robot sensor control: 123456!@#$%^
        case '1':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(0);
        	}
            _fixed_sensor_input = 0;
            break;
        }
        case '2':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(1);
        	}
            _fixed_sensor_input = 0;
            break;
        }
        case '3':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(2);
        	}
            _fixed_sensor_input = 0;
            break;
        }
        case '4':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(3);
        	}
        	_fixed_sensor_input = 0;
        	break;
        }
        case '5':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(4);
        	}
        	_fixed_sensor_input = 0;
        	break;
        }
        case '6':{
        	if(objects.size() >= 2){
        		objects[2][0]->setSensorIndex(5);
        	}
        	_fixed_sensor_input = 0;
        	break;
        }
		case '!':{ // Shift + 1
			_fixed_sensor_input = 0;
			break;
		}
		case '@':{ // Shift + 2
			_fixed_sensor_input = 1;
			break;
		}
		case '#':{ // Shift + 3
			_fixed_sensor_input = 2;
			break;
		}
		case '$':{ // Shift + 4
			_fixed_sensor_input = 3;
			break;
		}
		case '%':{ // Shift + 5
			_fixed_sensor_input = 4;
			break;
		}
		case '^':{ // Shift + 6
			_fixed_sensor_input = 5;
			break;
		}

		// Camera control: wasd-=
    	case 'a' : stepLeft(); break;
    	case 'd' : stepRight(); break;
    	case 'w' : stepFront(); break;
    	case 's' : stepBack(); break;
		case '-':{
			zoomOut();
			break;
		}
		case '=':{
			zoomIn();
			break;
		}

		// Simulator control: po>
        case 'o':{
            _max_steps = -1;
            break;
        }
		case 'p':{
		    _paused = !_paused;
			break;
		}
		case '>':{
		    m_Time = _max_steps;
			break;
		}

		// Control visuals: zxcvbnm
		case 'z':{
			_neurons_2d = !_neurons_2d;
			// The changes required here are so extensive that we'll need to
			// redraw the entire overlay.
			setOverlayNN();
			break;
		}
        case 'x':{
            _show_io = !_show_io;
            break;
        }
		case 'c' : _shapeDrawer->enableTexture(!_shapeDrawer->hasTextureEnabled());break;
        case 'v':{
            _render = !_render;
            break;
        }
        case 'b':{
            _overlay_scale *= 1.5;
            break;
        }
        case 'n':{
            _overlay_scale /= 1.5;
            break;
        }

        // Create screenshots: tyui
        case 't':{
            _overlay_scale = btVector3(1, 1, 1);
            _render = false;
            _neurons_2d = false;
            _show_text = false;
            setOverlayOffset(btVector3(0,0,-5));
            setOverlay();
            euler.setZ(0.2);
            _overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
            glutReshapeWindow(width, height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-0.24*ratio, 0.24*ratio, -0.30, 0.22, 1, 100);
            //gluPerspective(70, width/height, 1, 100);
            glMatrixMode(GL_MODELVIEW);
            glutPostRedisplay();
            break;
        }
    	case 'y':{
            _overlay_scale = btVector3(1, 1, 1);
            //_render = false;
            //_neurons_2d = false;
            //_show_text = false;
            setOverlayOffset(btVector3(0,0,-5));
            setOverlay();
            euler.setZ(0.2);
            _overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
            glutReshapeWindow(width, height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-0.24*ratio, 0.24*ratio, -0.30, 0.22, 1, 100);
            //gluPerspective(70, width/height, 1, 100);
            glMatrixMode(GL_MODELVIEW);
            glutPostRedisplay();
    		break;
    	}
        case 'u':{
#if defined(PNGW)
            int nPixels = width * height * 3;
            GLfloat* image = new GLfloat[nPixels];
            display();
            glReadPixels(0, 0, width, height,GL_RGB, GL_FLOAT, image);

            pngwriter PNG(width, height, 1.0, "test.png");
            size_t x = 1;   // start the top and leftmost point of the window
            size_t y = 1;
            double R, G, B;
            for(size_t i=0; i<nPixels; i++){
                  switch(i%3){
                       case 2:{
                             B = (double) image[i];
                             PNG.plot(x, y, R, G, B);
                             if( x == width ){
                                   x=1;
                                   y++;
                              }
                              else{
                                  x++;
                              }
                              break;
                       }
                       case 1:{
                             G = (double) image[i];
                             break;
                       }
                       case 0:{
                             R = (double) image[i];
                             break;
                       }
                  }
            }
            PNG.close();

            delete[] image;
#endif
            break;
        }

        // Control robot: r
		case 'r':{
			individual->rotate();
			break;
		}

//		case 't':{
//			throw true;
//			break;
//		}
		default:
			break;
			// Intentionally left empty
			//DemoApplication::keyboardCallback(key, x, y);
		}
	}

    void mouseFunc(int button, int state, int x, int y){
    	if (state == 0)
    	{
            m_mouseButtons |= 1<<button;
        } else
    	{
            m_mouseButtons = 0;
        }

    	m_mouseOldX = x;
        m_mouseOldY = y;

    	btVector3 rayTo = getRayTo(x,y);
    	switch (button){
    	case 0:{
    			if (state==0)
    			{


    				//add a point to point constraint for picking
    				if (m_dynamicsWorld)
    				{

    					btVector3 rayFrom;
    					if (m_ortho)
    					{
    						rayFrom = rayTo;
    						rayFrom.setZ(-100.f);
    					} else
    					{
    						rayFrom = m_cameraPosition;
    					}

    					btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom,rayTo);
    					m_dynamicsWorld->rayTest(rayFrom,rayTo,rayCallback);
    					if (rayCallback.hasHit())
    					{


    						btRigidBody* body = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
    						if (body)
    						{
    							//other exclusions?
    							if (!(body->isStaticObject() || body->isKinematicObject()))
    							{
    								pickedBody = body;
    								pickedBody->setActivationState(DISABLE_DEACTIVATION);
    								btVector3 pickPos = rayCallback.m_hitPointWorld;
    								btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
    								btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body,localPivot);
    								m_dynamicsWorld->addConstraint(p2p,true);
    								m_pickConstraint = p2p;
    								p2p->m_setting.m_impulseClamp = mousePickClamping;
    								//very weak constraint for picking
    								p2p->m_setting.m_tau = 0.001f;

    								//save mouse position for dragging
    								gOldPickingPos = rayTo;
    								gHitPos = pickPos;
    								gOldPickingDist  = (pickPos-rayFrom).length();
    							}
    						}
    					}
    				}

    			} else
    			{
    				removePickingConstraint();
    			}
    			break;
    		}
    	default:
    		{
    		}
    	}
    }

    void mouseMotionFunc(int x,int y){
    	if (m_pickConstraint)
    	{
    		//move the constraint pivot
    		if (m_pickConstraint->getConstraintType() == D6_CONSTRAINT_TYPE)
    		{
    			btGeneric6DofConstraint* pickCon = static_cast<btGeneric6DofConstraint*>(m_pickConstraint);
    			if (pickCon)
    			{
    				//keep it at the same picking distance

    				btVector3 newRayTo = getRayTo(x,y);
    				btVector3 rayFrom;
    				btVector3 oldPivotInB = pickCon->getFrameOffsetA().getOrigin();

    				btVector3 newPivotB;
    				if (m_ortho)
    				{
    					newPivotB = oldPivotInB;
    					newPivotB.setX(newRayTo.getX());
    					newPivotB.setY(newRayTo.getY());
    				} else
    				{
    					rayFrom = m_cameraPosition;
    					btVector3 dir = newRayTo-rayFrom;
    					dir.normalize();
    					dir *= gOldPickingDist;

    					newPivotB = rayFrom + dir;
    				}
    				pickCon->getFrameOffsetA().setOrigin(newPivotB);
    			}

    		} else
    		{
    			btPoint2PointConstraint* pickCon = static_cast<btPoint2PointConstraint*>(m_pickConstraint);
    			if (pickCon)
    			{
    				//keep it at the same picking distance

    				btVector3 newRayTo = getRayTo(x,y);
    				btVector3 rayFrom;
    				btVector3 oldPivotInB = pickCon->getPivotInB();
    				btVector3 newPivotB;
    				if (m_ortho)
    				{
    					newPivotB = oldPivotInB;
    					newPivotB.setX(newRayTo.getX());
    					newPivotB.setY(newRayTo.getY());
    				} else
    				{
    					rayFrom = m_cameraPosition;
    					btVector3 dir = newRayTo-rayFrom;
    					dir.normalize();
    					dir *= gOldPickingDist;

    					newPivotB = rayFrom + dir;
    				}
    				pickCon->setPivotB(newPivotB);
    			}
    		}
    	}
    	m_mouseOldX = x;
        m_mouseOldY = y;
    }

    btVector3 getRayTo(int x,int y){
    	if (m_ortho)
    	{
    		btScalar aspect;
    		btVector3 extents;
    		aspect = m_glutScreenWidth / (btScalar)m_glutScreenHeight;
    		extents.setValue(aspect * 1.0f, 1.0f,0);

    		extents *= m_cameraDistance;
    		btVector3 lower = m_cameraTargetPosition - extents;
    		btVector3 upper = m_cameraTargetPosition + extents;

    		btScalar u = x / btScalar(m_glutScreenWidth);
    		btScalar v = (m_glutScreenHeight - y) / btScalar(m_glutScreenHeight);

    		btVector3	p(0,0,0);
    		p.setValue((1.0f - u) * lower.getX() + u * upper.getX(),(1.0f - v) * lower.getY() + v * upper.getY(),m_cameraTargetPosition.getZ());
    		return p;
    	}

    	float top = 1.f;
    	float bottom = -1.f;
    	float nearPlane = 1.f;
    	float tanFov = (top-bottom)*0.5f / nearPlane;
    	float fov = btScalar(2.0) * btAtan(tanFov);

    	btVector3 rayFrom = getCameraPosition();
    	btVector3 rayForward = (getCameraTargetPosition()-getCameraPosition());
    	rayForward.normalize();
    	float farPlane = 10000.f;
    	rayForward*= farPlane;

    	btVector3 rightOffset;
    	btVector3 vertical = m_cameraUp;

    	btVector3 hor;
    	hor = rayForward.cross(vertical);
    	hor.normalize();
    	vertical = hor.cross(rayForward);
    	vertical.normalize();

    	float tanfov = tanf(0.5f*fov);

    	hor *= 2.f * farPlane * tanfov;
    	vertical *= 2.f * farPlane * tanfov;

    	btScalar aspect;

    	aspect = m_glutScreenWidth / (btScalar)m_glutScreenHeight;
    	hor*=aspect;
    	btVector3 rayToCenter = rayFrom + rayForward;
    	btVector3 dHor = hor * 1.f/float(m_glutScreenWidth);
    	btVector3 dVert = vertical * 1.f/float(m_glutScreenHeight);

    	btVector3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
    	rayTo += btScalar(x) * dHor;
    	rayTo -= btScalar(y) * dVert;
    	return rayTo;
    }

	btVector3 getCameraPosition()
	{
		return m_cameraPosition;
	}

	btVector3	getCameraTargetPosition()
	{
		return m_cameraTargetPosition;
	}




#else
	void keyboardCallback(unsigned char key, int x, int y){}
	void mouseFunc(int button, int state, int x, int y){}
	void mouseMotionFunc(int x,int y){}
	btVector3 getRayTo(int x,int y){return btVector3();}
	btVector3 getCameraPosition(){return btVector3();}
	btVector3 getCameraTargetPosition(){return btVector3();}
#endif


	void saveFrame(){
	    #if defined(PNGW)
	    using namespace boost;
	    using namespace boost::spirit::karma;
	    int nPixels = _frame_width * _frame_height * 3;
	    GLfloat* image = new GLfloat[nPixels];
	    display();
	    glReadPixels(0, 0, _frame_width, _frame_height,GL_RGB, GL_FLOAT, image);
//	    std::string frame_name = "movie/frame_" +  lexical_cast<std::string>(format(left_align(5, '0')[maxwidth(5)[int_]], _frame_counter)) + ".png";
	    std::string frame_name = _movie_folder + "/" + _frame_prefix + "_" +  lexical_cast<std::string>(_frame_counter) + ".png";
//	    std::string frame_name = _movie_folder + "/" + _frame_prefix movie/frame_" +  lexical_cast<std::string>(_frame_counter) + ".png";
	    pngwriter PNG(_frame_width, _frame_height, 1.0, frame_name.c_str());
	    size_t x = 1;   // start the top and leftmost point of the window
	    size_t y = 1;
	    double R, G, B;
	    for(size_t i=0; i<nPixels; i++){
	        switch(i%3){
	        case 2:{
	            B = (double) image[i];
	            PNG.plot(x, y, R, G, B);
	            if( x == _frame_width ){
	                x=1;
	                y++;
	            }
	            else{
	                x++;
	            }
	            break;
	        }
	        case 1:{
	            G = (double) image[i];
	            break;
	        }
	        case 0:{
	            R = (double) image[i];
	            break;
	        }
	        }
	    }
	    PNG.close();

	    delete[] image;
	    ++_frame_counter;
	    #endif
	}

	void print_3d_network(std::string filename = ""){
	    _print_network_state = 0;
	    _network_filename = filename;
	}

#ifndef NO_OPEN_GL
	void align_network(){
	    std::cout << "Aligning network" << std::endl;
        int width = 500;
        int height = 450;
        double ratio = (double)width/ (double)height;
        _overlay_scale = btVector3(1, 1, 1);
        _render = false;
        _neurons_2d = false;
        _show_text = false;
        setOverlayOffset(btVector3(0,0,-5));
        setOverlay();
        euler.setZ(0.2);
        _overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
        glutReshapeWindow(width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-0.24*ratio, 0.24*ratio, -0.30, 0.22, 1, 100);
        //gluPerspective(70, width/height, 1, 100);
        glMatrixMode(GL_MODELVIEW);
        glutPostRedisplay();
	}

    void align_network_side(){
        std::cout << "Aligning network" << std::endl;
        boost::filesystem::path full_path( boost::filesystem::current_path() );
        boost::filesystem::path dir(full_path/_movie_folder);
        boost::filesystem::create_directory(dir);
//        _neurons_2d = false;
//        _show_text = false;
        _overlay_scale = btVector3(.8,.8,.8);
        setOverlayOffset(btVector3(0,2.5,-4));

        // Settings for video with large NN
//        _overlay_scale = btVector3(1.5,1.5,1.5);
//        setOverlayOffset(btVector3(0,1.5,-4));
        updateOverlay();
        euler.setZ(0.6);
        //m_ele = 15;
        //euler.setY(-0.4);
        _overlayRotation.setEulerYPR(euler.x(), euler.y(), euler.z());
        setCameraDistance(5);
        setFollowRobot(false);
        //m_cameraTargetPosition.setY(5);
    }
#else
    void align_network(){}
    void align_network_side(){}
#endif

    void print_network(){
#if defined(PNGW)
        int width = 500;
        int height = 450;
        double ratio = (double)width/ (double)height;
        std::cout << "Printing network" << std::endl;
            int nPixels = width * height * 3;
            GLfloat* image = new GLfloat[nPixels];
            display();
            glReadPixels(0, 0, width, height,GL_RGB, GL_FLOAT, image);

            pngwriter PNG(width, height, 1.0, _network_filename.c_str());
            size_t x = 1;   // start the top and leftmost point of the window
            size_t y = 1;
            double R, G, B;
            for(size_t i=0; i<nPixels; i++){
                  switch(i%3){
                       case 2:{
                             B = (double) image[i];
                             PNG.plot(x, y, R, G, B);
                             if( x == width ){
                                   x=1;
                                   y++;
                              }
                              else{
                                  x++;
                              }
                              break;
                       }
                       case 1:{
                             G = (double) image[i];
                             break;
                       }
                       case 0:{
                             R = (double) image[i];
                             break;
                       }
                  }
            }
            PNG.close();

            delete[] image;
#else
            std::cout << "Printing network not supported" << std::endl;
#endif
    }




private:

#ifndef NO_OPEN_GL
	void display(){
		dbg::trace trace("sim", DBG_HERE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if(_render){

		    updateCamera();
            myinit();
		    if (m_dynamicsWorld)
		    {
		        if(m_enableshadows)
		        {
		            glClear(GL_STENCIL_BUFFER_BIT);
		            glEnable(GL_CULL_FACE);
		            renderscene(0);

		            glDisable(GL_LIGHTING);
		            glDepthMask(GL_FALSE);
		            glDepthFunc(GL_LEQUAL);
		            glEnable(GL_STENCIL_TEST);
		            glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		            glStencilFunc(GL_ALWAYS,1,0xFFFFFFFFL);
		            glFrontFace(GL_CCW);
		            glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
		            renderscene(1);
		            glFrontFace(GL_CW);
		            glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
		            renderscene(1);
		            glFrontFace(GL_CCW);

		            glPolygonMode(GL_FRONT,GL_FILL);
		            glPolygonMode(GL_BACK,GL_FILL);
		            glShadeModel(GL_SMOOTH);
		            glEnable(GL_DEPTH_TEST);
		            glDepthFunc(GL_LESS);
		            glEnable(GL_LIGHTING);
		            glDepthMask(GL_TRUE);
		            glCullFace(GL_BACK);
		            glFrontFace(GL_CCW);
		            glEnable(GL_CULL_FACE);
		            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

		            glDepthFunc(GL_LEQUAL);
		            glStencilFunc( GL_NOTEQUAL, 0, 0xFFFFFFFFL );
		            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		            //glDisable(GL_LIGHTING);
		            renderscene(2);
		            //glEnable(GL_LIGHTING);
		            glDepthFunc(GL_LESS);
		            glDisable(GL_STENCIL_TEST);
		            glDisable(GL_CULL_FACE);
		        }
		        else
		        {
		            //glDisable(GL_LIGHTING);
		            glDisable(GL_CULL_FACE);
		            renderscene(0);
		        }

		        int xOffset = 10;
		        int yStart = 20;
		        int yIncr = 20;


//		        glDisable(GL_LIGHTING);
		        glColor3f(0, 0, 0);

//		        if ((m_debugMode & btIDebugDraw::DBG_NoHelpText)==0)
//		        {
//		            setOrthographicProjection();
//		            showProfileInfo(xOffset,yStart,yIncr);
//		            resetPerspectiveProjection();
//		        }

//		        glDisable(GL_LIGHTING);
		    }

//		    updateCamera();
		}
//
		glDisable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    glEnable(GL_NORMALIZE);
//
//
	    individual->drawSensors(painter);

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glDepthFunc(GL_ALWAYS);
        glEnable(GL_LIGHT1);
		_overlay->draw(painter);
		glDisable(GL_LIGHT1);
		glDisable (GL_BLEND);

		glFlush();

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif
		glutSwapBuffers();
#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#endif
#endif
	}

	void renderscene(int pass){
		dbg::trace trace("sim", DBG_HERE);
	    btScalar    m[16];
	    btMatrix3x3 rot;rot.setIdentity();
	    const int   numObjects=m_dynamicsWorld->getNumCollisionObjects();
	    btVector3 wireColor(1,0,0);
	    for(int i=0;i<numObjects;i++)
	    {
	        //std::cout << "******** Drawing object " << i << " *********" << std::endl;
	        btCollisionObject*  colObj=m_dynamicsWorld->getCollisionObjectArray()[i];
	        btRigidBody*        body=btRigidBody::upcast(colObj);
	        if(body&&body->getMotionState())
	        {
	            btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
	            myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
	            rot=myMotionState->m_graphicsWorldTrans.getBasis();
	        }
	        else
	        {
	            colObj->getWorldTransform().getOpenGLMatrix(m);
	            rot=colObj->getWorldTransform().getBasis();
	        }
	        btVector3 wireColor(1.f,1.0f,0.5f); //wants deactivation
	        if(i&1) wireColor=btVector3(0.f,0.0f,1.f);
	        ///color differently for active, sleeping, wantsdeactivation states
	        if (colObj->getActivationState() == 1) //active
	        {
	            if (i & 1)
	            {
	                wireColor += btVector3 (1.f,0.f,0.f);
	            }
	            else
	            {
	                wireColor += btVector3 (.5f,0.f,0.f);
	            }
	        }
	        if(colObj->getActivationState()==2) //ISLAND_SLEEPING
	        {
	            if(i&1)
	            {
	                wireColor += btVector3 (0.f,1.f, 0.f);
	            }
	            else
	            {
	                wireColor += btVector3 (0.f,0.5f,0.f);
	            }
	        }

        	mod_robot::Object* obj = (mod_robot::Object*) colObj->getUserPointer();
        	opengl_draw::Color col;
        	//std::cout << "Rendering shape with object: " << obj <<std::endl;
        	if(obj){
        		col = obj->getColor();
        		//std::cout << "Rendering color: " << col.r() << ", " << col.g() << ", " << col.b() <<std::endl;
        		wireColor = btVector3 (col.r(), col.g(), col.b());
        	}

	        btVector3 aabbMin,aabbMax;
	        m_dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

	        aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
	        aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);

	        if (!(getDebugMode()& btIDebugDraw::DBG_DrawWireframe))
	        {
	            //JH: Massive hack, but for now, I will differentiate my objects here

	            //First, only texture the ground
	            if (i == 0){ //0 is the ground
	                _shapeDrawer->enableTexture(true);
	            } else {
	                _shapeDrawer->enableTexture(false);
	            }
	            int special = 0;

	            if(i == 1 and (pass == 0 or pass == 2)){
	                special = 1;
	                continue;
	            }

	            switch(pass)
	            {
	            case    0:  _shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor,getDebugMode(),aabbMin,aabbMax,special);break;
	            case    1:  _shapeDrawer->drawShadow(m,m_sundirection*rot,colObj->getCollisionShape(),aabbMin,aabbMax);break;
	            case    2:  _shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor*btScalar(0.3),0,aabbMin,aabbMax);break;
//	            case    2:
//	                // Draw only light shadows on the head and body
////	            	if (i > 0 ){ //1 is the head
////	                    _shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor*btScalar(0.7),0,aabbMin,aabbMax,special);
////	                } else {
////	                	_shapeDrawer->drawOpenGL(m,colObj->getCollisionShape(),wireColor*btScalar(0.3),0,aabbMin,aabbMax,special);
////	                }
//                	break;
	            }
	        }
	        //std::cout << std::endl;
	    }
	}

	inline void followRobot(){
		if(!_followRobot) return;
		m_cameraTargetPosition = individual->getPosition();
	}


	void updateCamera() {
		dbg::trace trace("sim", DBG_HERE);
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    btScalar rele = m_ele * btScalar(0.01745329251994329547);// rads per deg
	    btScalar razi = m_azi * btScalar(0.01745329251994329547);// rads per deg


	    btQuaternion rot(m_cameraUp,razi);


	    btVector3 eyePos(0,0,0);
	    eyePos[m_forwardAxis] = -m_cameraDistance;

	    btVector3 forward(eyePos[0],eyePos[1],eyePos[2]);
	    if (forward.length2() < SIMD_EPSILON)
	    {
	        forward.setValue(1.f,0.f,0.f);
	    }
	    btVector3 right = m_cameraUp.cross(forward);
	    btQuaternion roll(right,-rele);

	    eyePos = btMatrix3x3(rot) * btMatrix3x3(roll) * eyePos;

	    m_cameraPosition[0] = eyePos.getX();
	    m_cameraPosition[1] = eyePos.getY();
	    m_cameraPosition[2] = eyePos.getZ();
	    m_cameraPosition += m_cameraTargetPosition;

	    if (m_glutScreenWidth == 0 && m_glutScreenHeight == 0)
	        return;

	    btScalar aspect;
	    btVector3 extents;

	    aspect = m_glutScreenWidth / (btScalar)m_glutScreenHeight;
	    extents.setValue(aspect * 1.0f, 1.0f,0);


	    if (m_ortho)
	    {
	        // reset matrix
	        glLoadIdentity();


	        extents *= m_cameraDistance;
	        btVector3 lower = m_cameraTargetPosition - extents;
	        btVector3 upper = m_cameraTargetPosition + extents;
	        //gluOrtho2D(lower.x, upper.x, lower.y, upper.y);
	        glOrtho(lower.getX(), upper.getX(), lower.getY(), upper.getY(),-1000,1000);

	        glMatrixMode(GL_MODELVIEW);
	        glLoadIdentity();
	        //glTranslatef(100,210,0);
	    } else
	    {
	//      glFrustum (-aspect, aspect, -1.0, 1.0, 1.0, 10000.0);
	        glFrustum (-aspect * m_frustumZNear, aspect * m_frustumZNear, -m_frustumZNear, m_frustumZNear, m_frustumZNear, m_frustumZFar);
	        glMatrixMode(GL_MODELVIEW);
	        glLoadIdentity();
	        gluLookAt(m_cameraPosition[0], m_cameraPosition[1], m_cameraPosition[2],
	            m_cameraTargetPosition[0], m_cameraTargetPosition[1], m_cameraTargetPosition[2],
	            m_cameraUp.getX(),m_cameraUp.getY(),m_cameraUp.getZ());
	    }

	}

	void myinit(void){
		dbg::trace trace("sim", DBG_HERE);
	    GLfloat light_ambient[] = { btScalar(0.2), btScalar(0.2), btScalar(0.2), btScalar(1.0) };
	    GLfloat light_diffuse[] = { btScalar(1.0), btScalar(1.0), btScalar(1.0), btScalar(1.0) };
	    GLfloat light_specular[] = { btScalar(1.0), btScalar(1.0), btScalar(1.0), btScalar(1.0 )};
	    /*  light_position is NOT default value */
	    GLfloat light_position0[] = { btScalar(1.0), btScalar(10.0), btScalar(1.0), btScalar(0.0 )};
//	    GLfloat light_position0[] = { btScalar(5.0), btScalar(10.0), btScalar(5.0), btScalar(0.0 )};
	    GLfloat light_position1[] = { btScalar(-1.0), btScalar(-10.0), btScalar(-1.0), btScalar(0.0) };

	    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

	    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	    glEnable(GL_LIGHTING);
	    glEnable(GL_LIGHT0);
	    //glEnable(GL_LIGHT1);


	    glShadeModel(GL_SMOOTH);
	    glEnable(GL_DEPTH_TEST);
	    glDepthFunc(GL_LESS);

	    glClearColor(btScalar(1),btScalar(1),btScalar(1),btScalar(0));
	    //Presumably required to draw texture based text
	    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	    //  glEnable(GL_CULL_FACE);
	    //  glCullFace(GL_BACK);
	}
#else
	inline void followRobot(){}
	void display(){}
	void renderscene(int pass){}
	void updateCamera(){}
	void myinit(void){}
#endif


};

}

#endif
