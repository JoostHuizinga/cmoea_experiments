/*
 * mod_robot_sensor_group.hpp
 *
 *  Created on: Sep 17, 2014
 *      Author: joost
 */

#ifndef MOD_ROBOT_SENSOR_GROUP_HPP_
#define MOD_ROBOT_SENSOR_GROUP_HPP_

#include <set>
#include <vector>

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <sferes/dbg/dbg.hpp>

#include "mod_robot_sensor.hpp"

namespace mod_robot{
class SensorGroup {
public:
	SensorGroup(btCollisionObject* parent): _parent(parent)
	{
		dbg::trace trace("sensor", DBG_HERE);
	}

	~SensorGroup(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	inline void update(const std::vector<ObjectGroup>& object_groups){
		dbg::trace trace("sensor", DBG_HERE);
		for(size_t i=0; i<_sensors.size(); ++i){
			_sensors[i]->update();
		}

		for(size_t i=0; i<object_groups.size(); ++i){
			dbg::out(dbg::info, "sensor") << "Object group: " << i << " type: "<< object_groups[i].type() << std::endl;
			if(_detects.count(object_groups[i].type())){
				_update(object_groups[i]);
			}
		}
	}

	inline size_t size() const{
		dbg::trace trace("sensor", DBG_HERE);
		return _sensors.size();
	}

	Sensor* operator[](size_t i){
		dbg::trace trace("sensor", DBG_HERE);
		return _sensors[i];
	}

	inline void add(Sensor* sensor){
		dbg::trace trace("sensor", DBG_HERE);
		_sensors.push_back(sensor);
	}

	void addDetects(size_t id){
		dbg::trace trace("sensor", DBG_HERE);
		_detects.insert(id);
	}

#ifndef NO_OPEN_GL
	void draw(opengl_draw::Painter* painter){
		dbg::trace trace("sensor", DBG_HERE);
		for(size_t i=0; i<_sensors.size(); ++i){
			_sensors[i]->draw(painter);
		}
	}
#else
	template<typename Painter>
	void draw(Painter painter){}
#endif

	void clear(){
		dbg::trace trace("sensor", DBG_HERE);
		for(size_t i=0; i<_sensors.size(); ++i){
			delete _sensors[i];
		}
		_sensors.clear();
	}

private:
	inline void _update(const ObjectGroup& object_group){
		dbg::trace trace("sensor", DBG_HERE);
		for(size_t i=0; i<object_group.size(); ++i){
			for(size_t j=0; j<_sensors.size(); ++j){
				_sensors[j]->activate(object_group[i]);
			}
		}
	}

	btCollisionObject* _parent;
	std::vector<Sensor*> _sensors;
	std::set<size_t> _detects;
};
}

#endif /* MOD_ROBOT_SENSOR_GROUP_HPP_ */
