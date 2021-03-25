/*
 * mod_robot_control_sensor.hpp
 *
 *  Created on: Jan 29, 2015
 *      Author: Joost Huizinga
 *
 * A sensor that, rather than respond to the environment, is controlled directly.
 */

#ifndef EXP_MODULARITY_MOD_ROBOT_CONTROL_SENSOR_HPP_
#define EXP_MODULARITY_MOD_ROBOT_CONTROL_SENSOR_HPP_



#include <set>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <sferes/dbg/dbg.hpp>
#include "mod_robot_sensor.hpp"

namespace mod_robot{

class ControlSensor: public Sensor{

public:
    ControlSensor(int sensorIndex, btScalar inactiveValue):
        _sensorIndex(sensorIndex),
        _inactiveValue(inactiveValue)
	{
        dbg::trace trace("sensor", DBG_HERE);
        _activation = _inactiveValue;
    }

    virtual ~ControlSensor(){
        dbg::trace trace("sensor", DBG_HERE);
        //Intentionally left blank
    }

    virtual void activate(Object* object){
        dbg::trace trace("sensor", DBG_HERE);
        if(object->getSensorIndex()==_sensorIndex){
            _activation = 1.0f;
        } else {
            _activation = _inactiveValue;
        }
    }

    virtual void update(){
        dbg::trace trace("sensor", DBG_HERE);
        //Intentionally left blank
    }

    inline void setModifier(const btVector3& modifier){
        dbg::trace trace("sensor", DBG_HERE);
        //Intentionally left blank
    }

    template<typename Painter>
    void draw(Painter painter){
        dbg::trace trace("sensor", DBG_HERE);
        //Intentionally left blank
    }

    template<typename Color>
    void setActiveColor(Color active_color){
        dbg::trace trace("sensor", DBG_HERE);
        //Intentionally left blank
    }

private:
    int _sensorIndex;
    btScalar _inactiveValue;
};

}



#endif /* EXP_MODULARITY_MOD_ROBOT_CONTROL_SENSOR_HPP_ */
