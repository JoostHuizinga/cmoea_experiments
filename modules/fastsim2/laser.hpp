#ifndef LASER_HH_
#define LASER_HH_
#include "map.hpp"
#include "posture.hpp"
#include <boost/shared_ptr.hpp>

namespace fastsim {
  class Laser {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a new laser range-finder with the given angle and range. */
      Laser(float angle, float range, float gap_dist = 0.0f, float gap_angle = 0.0f) :
        angle(angle),
        range(range),
        gapDist(gap_dist),
        gapAngle(gap_angle),
        x(0),
        y(0), 
        xPixel(0),
        yPixel(0),
        dist(-1)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Calculates and returns the distance from this laser to the nearest wall. */
      float update(Posture& pos, const boost::shared_ptr<Map>& map);

      /* Gets the angle of this laser. */
      float getAngle() { return angle; }

      /* Gets the range of this laser. */
      float getRange() { return range; }

      /* Gets the gap distance of this laser (its polar distance from the robot's center). */
      float getGapDist() { return gapDist; }

      /* Gets the gap angle of this laser (its polar angle from the robot's center). */
      float getGapAngle() { return gapAngle; }

      /* Gets the x-coordinate where this laser hits a wall, in pixel coordinates. */
      int getXPixel() { return xPixel; }

      /* Gets the y-coordinate where this laser hits a wall, in pixel coordinates. */
      int getYPixel() { return yPixel; }

      /* Gets the most recently calculated distance from this laser to the wall. */
      float getDist() { return dist; }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float angle;
      float range;
      float gapDist; //polar coordinates of the laser
      float gapAngle; //polar coordinates of the laser
      float x;
      float y;
      int xPixel;
      int yPixel;
      float dist;
  };
}
#endif /* !LASER_HH_ */
