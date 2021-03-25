/*
 * mod_robot_sensor.hpp
 *
 *  Created on: Sep 17, 2014
 *      Author: joost
 */

#ifndef MOD_ROBOT_SENSOR_HPP_
#define MOD_ROBOT_SENSOR_HPP_

#include <modules/opengldraw/opengl_draw_painter.hpp>

namespace mod_robot{
/**
 * Convenience operator for writing btVector3 objects.
 */
std::ostream& operator<<(std::ostream& is, const btVector3& obj){
	is << "x: " << obj.x() << " y: " << obj.y() << " z: " << obj.z();
	return is;
}


class Sensor{
public:
	virtual ~Sensor(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void update(){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void activate(Object* object){
		dbg::trace trace("sensor", DBG_HERE);
	}

	virtual void draw(opengl_draw::Painter* painter){
	    dbg::trace trace("sensor", DBG_HERE);
	}

	inline btScalar getActivation(){
		dbg::trace trace("sensor", DBG_HERE);
		return _activation;
	}

	inline void setNeuronPosition(const btVector3& neuron_position){
		dbg::trace trace("sensor", DBG_HERE);
		_neuron_position = neuron_position;
	}

	inline const btVector3& getNeuronPosition(){
		dbg::trace trace("sensor", DBG_HERE);
		return _neuron_position;
	}

protected:
	btScalar _activation;
	btVector3 _neuron_position;
};

}

#endif /* MOD_ROBOT_SENSOR_HPP_ */
