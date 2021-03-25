/*
 * mod_robot_pie_sensor.hpp
 *
 *  Created on: Sep 16, 2014
 *      Author: joost
 */

#ifndef MOD_ROBOT_PIE_SENSOR_HPP_
#define MOD_ROBOT_PIE_SENSOR_HPP_

#include <set>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <sferes/dbg/dbg.hpp>
#include <modules/opengldraw/opengl_draw_color.hpp>
#include "mod_robot_sensor.hpp"

namespace mod_robot{

class PieSensor: public Sensor{

public:
	/**
	 * Constructs a PieSensor object.
	 *
	 * @param parent The bullet object on which to install this sensor.
	 * @param orientation The direction this sensor is facing.
	 * @param angle The size of the pie slice in terms of the angle.
	 * @param modifier Used to "zero out" certain positions in the vector to the
	 *     object to be detected. For example, a modifier of (1, 0, 1) means
	 *     that this sensors is insensitive to variations along the y-axis. A
	 *     modifier of (1, 1, 1) means the sensor acts as a cone sensor.
	 */
	PieSensor(btCollisionObject* parent, btVector3 orientation, btScalar angle, btVector3 modifier = btVector3(1, 1, 1)):
		_parent(parent), _base_orientation(orientation), _modifier(modifier), _angle(angle){
		dbg::trace trace("sensor", DBG_HERE);
		update();
	}

	virtual ~PieSensor(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void activate(Object* object){
		dbg::trace trace("sensor", DBG_HERE);
        btVector3 object_position = object->getPosition();
        btVector3 parent_position = _parent->getWorldTransform().getOrigin();
        btVector3 vector_to_object = object_position - parent_position;

		btVector3 modified_vector_to_object = _modifier * vector_to_object;
		dbg::out(dbg::info, "sensor") 	<< " _current_orientation: " << _current_orientation
		        						<< " modified_vector_to_object " << modified_vector_to_object
		        						<< " required angle: " << _angle
		        						<< " angle: " << _current_orientation.angle(modified_vector_to_object) <<std::endl;
		if(_current_orientation.angle(modified_vector_to_object) < _angle){
			if(vector_to_object.length() < 1){
				_activation = 3.0;
			} else {
				_activation = std::max(std::min(btScalar(3.0), btScalar(1.0) + btScalar((1.0f/vector_to_object.length())*3.0)), _activation);
			}
		}
	}

	virtual void update(){
		dbg::trace trace("sensor", DBG_HERE);
		btQuaternion parent_rotation = _parent->getWorldTransform().getRotation();
		btMatrix3x3 parent_rotation_matrix;

		btVector3 axis = parent_rotation.getAxis();
		btScalar angle = parent_rotation.getAngle();
		btScalar y = axis.y()*angle;

		parent_rotation_matrix.setEulerZYX(0, y, 0);

		_current_orientation = parent_rotation_matrix*_base_orientation;
		_activation = 0;

		dbg::out(dbg::info, "sensor") << "Rotation around y: " << y <<std::endl;
		dbg::out(dbg::info, "sensor") << "rotation matrix:\n"
				<<  parent_rotation_matrix.getRow(0) << "\n"
				<<  parent_rotation_matrix.getRow(1) << "\n"
				<<  parent_rotation_matrix.getRow(2) << std::endl;
		dbg::out(dbg::info, "sensor") << "_base_orientation: " << _base_orientation
				<< "_current_orientation: "
				<< _current_orientation <<std::endl;
	}

	inline void setModifier(const btVector3& modifier){
		dbg::trace trace("sensor", DBG_HERE);
		_modifier = modifier;
	}

	void setActiveColor(opengl_draw::Color active_color){
		dbg::trace trace("sensor", DBG_HERE);
		_active_color = active_color;
	}

#ifndef NO_OPEN_GL
	virtual void draw(opengl_draw::Painter* painter){
		dbg::trace trace("sensor", DBG_HERE);
		btVector3 up(0, 1, 0);
		btVector3 straight = _current_orientation;
		straight.normalize();

		btVector3 left = straight.rotate(up, _angle);
		btVector3 right = straight.rotate(up, -_angle);

		left.normalize();
		right.normalize();
		left*=10;
		right*=10;
		straight*=10;

		painter->pushTransformation();
		painter->disableLighting();
//		glEnable (GL_BLEND);
//		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glDepthFunc(GL_ALWAYS);
//		if(_activation>0.01){
//			painter->setColor(_active_color);
//		} else {
//			painter->setColor(opengl_draw::Color(0.5, 0.5, 0.5, 0.5));
//		}

		painter->translate(_parent->getWorldTransform().getOrigin());
//		painter->beginPolygon();
//		painter->addPoint(btVector3(0, 0, 0));
//		painter->addNormal(up);
//		painter->addPoint(left);
//		painter->addNormal(up);
//		painter->addPoint(right);
//		painter->addNormal(up);
//		painter->end();

//		glDisable (GL_BLEND);

		if(_activation>0.01){
			painter->setColor(_active_color);
			painter->setLineWidth(3.0f);

		} else {
			painter->setColor(opengl_draw::Color(0, 0, 0));
			painter->setLineWidth(1.0f);
		}

//		painter->setColor(opengl_draw::Color(0, 0, 0));
		painter->beginLineLoop();
		painter->addPoint(btVector3(0, 0, 0));
		painter->addPoint(left);
		painter->addPoint(right);
		painter->end();

//		glLineWidth(3);
//		if(_activation>0.01){
//			painter->setColor(Color(1, 0, 0));
//		} else{
//			painter->setColor(Color(0, 0, 0));
//		}
//		painter->beginLine();
//		painter->addPoint(btVector3(0, 0.01, 0));
//		painter->addPoint(straight);
//		painter->end();
//		glLineWidth(1);

//		glDepthFunc(GL_LESS);
		painter->enableLighting();
		painter->popTransformation();
		painter->setLineWidth(1.0f);
	}
#endif

private:
	btCollisionObject* _parent;
	btVector3 _base_orientation;
	btVector3 _current_orientation;
	btVector3 _modifier;
	btScalar _angle;
	opengl_draw::Color _active_color;
};

}


#endif /* MOD_ROBOT_PIE_SENSOR_HPP_ */
