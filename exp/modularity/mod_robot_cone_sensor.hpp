/*
 * mod_robot_cone_sensor.hpp
 *
 *  Created on: Oct 18, 2014
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_MOD_ROBOT_CONE_SENSOR_HPP_
#define EXP_MODULARITY_MOD_ROBOT_CONE_SENSOR_HPP_

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#include <set>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <modules/opengldraw/opengl_draw_color.hpp>
#include <sferes/dbg/dbg.hpp>
#include "mod_robot_sensor.hpp"

namespace mod_robot{

class ConeSensor: public Sensor{

public:
	ConeSensor(btCollisionObject* parent, btMatrix3x3 base_rotation, btScalar angle):
		_parent(parent), _base_rotation(base_rotation), _angle(angle){
		dbg::trace trace("sensor", DBG_HERE);
		_max_activation = 3.0f;
		_min_activation = 1.0f;
		update();
	}

	virtual ~ConeSensor(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void activate(Object* object){
		dbg::trace trace("sensor", DBG_HERE);
        btVector3 object_position = object->getPosition();
        btVector3 parent_position = _parent->getWorldTransform().getOrigin();
        btVector3 vector_to_object = object_position - parent_position;

		btVector3 vector = _current_rotation*btVector3(1,0,0);
		dbg::out(dbg::info, "sensor") 	<< " _current_orientation: " << vector
		        						<< " modified_vector_to_object " << vector_to_object
		        						<< " required angle: " << _angle
		        						<< " angle: " << vector.angle(vector_to_object) <<std::endl;
		if(vector.angle(vector_to_object) < _angle){
			btScalar dist_to_object = vector_to_object.length();
			if(dist_to_object <= 1.0f){
				_activation = _max_activation;
			} else {
				btScalar raw_activation = btScalar(_min_activation) + btScalar((1.0f/dist_to_object)*_max_activation);
				_activation = std::max(raw_activation, _activation);
			}
		}
	}

	virtual void update(){
		dbg::trace trace("sensor", DBG_HERE);
		btMatrix3x3 parent_rotation_matrix = _parent->getWorldTransform().getBasis();
		_current_rotation = parent_rotation_matrix*_base_rotation;
		_activation = 0;
	}

	void setActiveColor(opengl_draw::Color active_color){
		_active_color = active_color;
	}

#ifndef NO_OPEN_GL
	virtual void draw(opengl_draw::Painter* painter){
		dbg::trace trace("sensor", DBG_HERE);
		btVector3 forward(1, 0, 0);
		btVector3 up(0, 1, 0);
		btVector3 current_forward = _current_rotation*forward;
		btVector3 current_up = _current_rotation*up;
		btVector3 start = current_forward.rotate(current_up, _angle)*10;
		btScalar granularity = M_PI/10.0f;

		painter->disableLighting();
		painter->pushTransformation();
		painter->translate(_parent->getWorldTransform().getOrigin());

//		painter->setColor(opengl_draw::Color(255, 0, 0));
//		painter->setLineWidth(3.0f);
//		painter->beginLine();
//		painter->addPoint(btVector3(0, 0, 0));
//		painter->addPoint(current_forward*10);
//		painter->end();

		if(_activation>0.01){
			painter->setColor(_active_color);
			painter->setLineWidth(3.0f);

		} else {
			painter->setColor(opengl_draw::Color(0, 0, 0));
			painter->setLineWidth(1.0f);
		}

		painter->beginLine();
		painter->addPoint(btVector3(0, 0, 0));
		painter->addPoint(start);
		painter->end();

		for(btScalar i=0.0; i<M_PI*2; i+=granularity){
//			painter->beginLineLoop();
			painter->beginLineStrip();
			painter->addPoint(start);
			start = start.rotate(current_forward, granularity);
			painter->addPoint(start);
			painter->addPoint(btVector3(0, 0, 0));
			painter->end();
		}
//		painter->end();
		painter->popTransformation();
		painter->enableLighting();
		painter->setLineWidth(1.0f);
	}
#endif

private:
	btCollisionObject* _parent;
	btMatrix3x3 _base_rotation;
	btMatrix3x3 _current_rotation;

//	btVector3 _base_orientation;
//	btVector3 _current_orientation;

	btScalar _angle;
	btScalar _max_activation;
	btScalar _min_activation;
	opengl_draw::Color _active_color;
};

}



#endif /* EXP_MODULARITY_MOD_ROBOT_CONE_SENSOR_HPP_ */
