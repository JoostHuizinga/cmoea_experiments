#include <iostream>
#include "light_sensor.hpp"

namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  
  /* Determines whether this sensor detects a colored switch. */
  bool LightSensor::update(Posture& pos, const boost::shared_ptr<Map>& map) {
    std::vector<Map::illuminated_switch>& isv = map->getIlluminatedSwitches();
    activated = false;
    for (size_t i = 0; i < isv.size(); i++) {
      if (isv[i]->getColor() == getColor() && isv[i]->isOn()) {
        float x1 = (cosf(pos.getTheta() + angle + angularRange) * range / cosf(angularRange)) + pos.getX();
        float y1 = (sinf(pos.getTheta() + angle + angularRange) * range / cosf(angularRange)) + pos.getY();

        float x2 = (cosf(pos.getTheta() + angle - angularRange) * range / cosf(angularRange)) + pos.getX();
        float y2 = (sinf(pos.getTheta() + angle - angularRange) * range / cosf(angularRange)) + pos.getY();

        if (pointLiesInTriangle(isv[i]->getX(), isv[i]->getY(), pos.getX(), pos.getY(), x1, y1, x2, y2)) {
          float x_result = 0; //unused
          float y_result = 0; //unused
          if (!map->isRealLineOfSightObscured(isv[i]->getX(), isv[i]->getY(), pos.getX(), pos.getY(), x_result, y_result)) {
            activated = true;
            num = i;
          }
        }  
	    }
	  }
    return activated;
  }
}
