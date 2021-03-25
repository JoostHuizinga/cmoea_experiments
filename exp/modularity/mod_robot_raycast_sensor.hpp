/*
 * mod_robot_raycast_sensor.hpp
 *
 *  Created on: Apr 11, 2017
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_MOD_ROBOT_RAYCAST_SENSOR_HPP_
#define EXP_MODULARITY_MOD_ROBOT_RAYCAST_SENSOR_HPP_

#include <set>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <modules/opengldraw/opengl_draw_color.hpp>
#include <sferes/dbg/dbg.hpp>
#include "mod_robot_sensor.hpp"
#include "mod_robot_global.hpp"

#define DBO dbg::out(dbg::info, "raysensor")

namespace mod_robot{

class RaycastSensorCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
	RaycastSensorCallback () :
		btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0),
				btVector3(0.0, 0.0, 0.0))
	{
	}

	virtual btScalar addSingleResult(
			btCollisionWorld::LocalRayResult& rayResult,
			bool normalInWorldSpace)
	{
#ifndef BULLET_2_81
		if (rayResult.m_collisionObject->getUserIndex() == spider_index)
			return 1.0;
#endif
		return ClosestRayResultCallback::addSingleResult(rayResult,
				normalInWorldSpace);
	}
};


class RaycastSensor: public Sensor{

public:
	RaycastSensor(btCollisionObject* parent,
			btDynamicsWorld* world,
			btMatrix3x3 base_rotation):
		_parent(parent),
		_base_rotation(base_rotation)
{
		dbg::trace trace("sensor", DBG_HERE);
		_world = world;
		_max_activation = 3.0f;
		_object_hit = 0;
		_range = 10000;
		update();
	}

	virtual ~RaycastSensor(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void activate(Object* object){
		dbg::trace trace("sensor", DBG_HERE);
		DBO << "Object hit: " << _object_hit << std::endl;
		DBO << "Object testing: " << object->getRigidBody() << std::endl;
		if(_object_hit && object->getRigidBody() == _object_hit){
			_activation = _max_activation;
		}
	}

	virtual void update(){
		dbg::trace trace("sensor", DBG_HERE);
		btMatrix3x3 parent_rotation = _parent->getWorldTransform().getBasis();
		_current_rotation = parent_rotation*_base_rotation;
		_start = _parent->getWorldTransform().getOrigin();
		_end = _start + (_current_rotation*btVector3(_range, 0, 0));
		_activation = 0;
		_object_hit = 0;

		DBO << "Parent: " << _parent << std::endl;
		if(_world){
			RaycastSensorCallback callback;
			_world->rayTest(_start, _end, callback);
			if(callback.hasHit()){
				DBO << "Hit object: "
						<< callback.m_collisionObject << std::endl;
				DBO << "Hit dist: "
						<< callback.m_closestHitFraction << std::endl;
#ifndef BULLET_2_81
				DBO << "Hit index: "
						<< callback.m_collisionObject->getUserIndex()
						<< std::endl;
#endif
				_object_hit = callback.m_collisionObject;
			}
		}
	}

	void setActiveColor(opengl_draw::Color active_color){
		_active_color = active_color;
	}

#ifndef NO_OPEN_GL
	virtual void draw(opengl_draw::Painter* painter){
		dbg::trace trace("sensor", DBG_HERE);

		painter->disableLighting();
		painter->pushTransformation();
		//painter->translate(_parent->getWorldTransform().getOrigin());

		if(_activation>0.01){
			painter->setColor(_active_color);
			painter->setLineWidth(3.0f);

		} else {
			painter->setColor(opengl_draw::Color(0, 0, 0));
			painter->setLineWidth(1.0f);
		}

		painter->beginLine();
		painter->addPoint(_start);
		painter->addPoint(_end);
		painter->end();

		painter->popTransformation();
		painter->enableLighting();
		painter->setLineWidth(1.0f);
	}
#endif

private:
	btCollisionObject* _parent;
	const btCollisionObject* _object_hit;
	btDynamicsWorld* _world;
	btMatrix3x3 _base_rotation;
	btMatrix3x3 _current_rotation;
	btVector3 _start;
	btVector3 _end;
	btScalar _max_activation;
	btScalar _range;
	opengl_draw::Color _active_color;
};

}

#undef DBO
#endif /* EXP_MODULARITY_MOD_ROBOT_RAYCAST_SENSOR_HPP_ */
