#ifndef FASTSIM_ILLUMINATED_HHP_
#define FASTSIM_ILLUMINATED_HHP_

#include <vector>
#include "posture.hpp"

namespace fastsim {
  class IlluminatedSwitch {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a new illuminated switch at the given position (in world coordinates). */
      IlluminatedSwitch(int color, float radius, float x, float y, bool on) :
        color(color),
        radius(radius),
        x(x),
        y(y),
        on(on),
        activated(false),
        visible(true)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /*
       * Attempts to activate the switch.
       * Switches cannot be activated unless they are on and the robot is within their illumination radius.
       */
      void tryToActivate(Posture& robotPos) {
        float d = robotPos.distTo(x, y);
        if (on && d < radius) {
          activate();
        }
      }

      /*
       * Activates the switch.
       * If any switches have been linked to this switch, they will be turned on (but not activated).
       */
      void activate() {
        activated = true;
        for (size_t i = 0; i < linkedSwitches.size(); i++) {
          linkedSwitches[i]->turnOn();
        }
      }

      /*
       * Deactivates the switch.
       * If any switches have been linked to this switch, they will be turned off (but not deactivated).
       */
      void deactivate() {
        activated = false;
        for (size_t i = 0; i < linkedSwitches.size(); i++) {
          linkedSwitches[i]->turnOff();
        }
      }

      /* Sets whether this switch should be drawn to the screen, without affecting its functionality. */
      void setVisible(bool isVisible) { visible = isVisible; }

      /* Gets whether this switch is visible (for drawing purposes only). */
      bool isVisible() { return visible; }

      /* Turns on (but does not activate) the switch. */
      void turnOn() { on = true; }

      /* Turns off (but does not deactivate) the switch. */
      void turnOff() { on = false; }

      /* Gets whether the switch is on. */
      bool isOn() { return on; }

      /* Gets whether the switch is off. */
      bool isOff() { return !on; }

      /* Gets the color of this switch. */
      int  getColor() { return color; }

      /* Gets the radius of this switch (a robot must be within this radius to activate the switch).*/
      float getRadius() { return radius; }

      /* Gets the x-coordinate of this switch. */
      float getX() { return x; }

      /* Gets the y-coordinate of this switch. */
      float getY() { return y; }

      /* Gets whether this switch has been activated. */
      bool isActivated() { return activated; }

      /* Sets a new position for the switch. */
      void setPos(float newX, float newY) {
        x = newX;
        y = newY;
      }

      /*
       * Links this switch with another switch.
       * All linked switches will turn on if this switch is activated, and will turn off if this switch is deactivated.
       */
      void link(boost::shared_ptr<IlluminatedSwitch> o) {
        linkedSwitches.push_back(o);
      }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      int color;
      float radius;
      float x;
      float y;
      bool on;
      bool activated;
      std::vector<boost::shared_ptr<IlluminatedSwitch> > linkedSwitches;
      bool visible;
  };

  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  /////////////////////* Structs *////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  
  /* A structure for finding the closest switch to a set of coordinates. */
  struct ClosestSwitch {
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////* Constructors/Destructors *////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    
    /* Creates a closest-switch structure centered on the given coordinates, against which comparisons will be made. */
    ClosestSwitch(float x, float y) :
      sourceX(x),
      sourceY(y)
    {}

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////* Functions *///////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    /*
     * Compares the distances of two switches to the point specified during this structure's creation.
     * If the first switch is closer to the fixed point, this function returns true; otherwise, it returns false.
     */
    bool operator() (const boost::shared_ptr<IlluminatedSwitch> i1, const boost::shared_ptr<IlluminatedSwitch> i2) {
      float x1 = i1->getX() - sourceX;
      float y1 = i1->getY() - sourceY;
      float x2 = i2->getX() - sourceX;
      float y2 = i2->getY() - sourceY;
      float d1 = sqrtf(x1 * x1 + y1 * y1);
      float d2 = sqrtf(x2 * x2 + y2 * y2);
      return d1 < d2;
    }

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    //////////////////////* Fields *////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    float sourceX;
    float sourceY;
  };
}
#endif
