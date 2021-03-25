#ifndef FASTSIM_LIGHT_SENSOR_HPP_
#define FASTSIM_LIGHT_SENSOR_HPP_

#include <boost/shared_ptr.hpp>
#include "posture.hpp"
#include "map.hpp"

namespace fastsim {
  class LightSensor {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /*
       * Creates a new light-sensor, with the given angle and range, which detects the given color switch.
       * Light sensors use a pie-slice detection system, but do NOT see through obstacles.
       * Because only one pie slice exists for each sensor, the exact angular range can be supplied as an optional parameter.
       * Light sensors are typically not rendered unless they detect a corresponding illuminated switch, which must be on.
       */
      LightSensor(int color, float angle, float range, float angularRange = (M_PI / 8)) :
        color(color),
        angle(angle),
        range(range),
        activated(false),
        num(0),
        angularRange(angularRange)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Determines whether this sensor detects a colored switch. */
      bool update(Posture& pos, const boost::shared_ptr<Map>& map);

      /* Gets the color that this sensor can detect. */
      int	getColor() { return color; }

      /* Gets the angle of this sensor. */
      float getAngle() { return angle; }

      /* Gets the range of this sensor. */
      float getRange() { return range; }

      /* Gets whether this sensor has detected a colored switch. */
      bool isActivated() { return activated; }

      /* Gets the number of this sensor (used for display purposes only). */
  	  unsigned int getNum() {return num; }

      /* Gets the angular range of this sensor. */
      float getAngularRange() { return angularRange; }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      int color;
      float angle;
      float range;
      bool activated;
    	unsigned int num; //used only for display
      float angularRange;
  };
}
#endif
