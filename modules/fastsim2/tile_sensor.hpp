#ifndef FASTSIM_TILE_SENSOR_HPP_
#define FASTSIM_TILE_SENSOR_HPP_

#include <boost/shared_ptr.hpp>
#include <utility>
#include <vector>
#include "posture.hpp"
#include "map.hpp"
#include "misc.hpp"

namespace fastsim {
  class TileSensor {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /*
       * Creates a new tile-sensor, with the given angle and range, which detects the amount of unexplored tiles within its FoV.
       * Tile sensors use a pie-slice detection system, but do NOT see through obstacles.
       * Because only one pie slice exists for each sensor, the exact angular range can be supplied as an optional parameter.
       */
      TileSensor(float angle, int range, float angularRange = (M_PI / 8)) :
        angle(angle),
        range(range),
        numTiles(0),
        angularRange(angularRange)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Determines how many unvisited tiles are detected by this sensor. */
      int update(Posture& pos, const boost::shared_ptr<Map>& map);

      /* Gets the angle of this sensor. */
      float getAngle() { return angle; }

      /* Gets the range of this sensor. */
      float getRange() { return range; }

      /* Gets the number of unvisited tiles detected by this sensor. */
  	  int getNumTilesSensed() {return numTiles; }

      /* Gets the angular range of this sensor. */
      float getAngularRange() { return angularRange; }

      /* Gets the vector of unvisited tiles (as coordinate pairs) detected by this sensor. */
      std::vector<std::pair<int, int> > getTilesSensed() { return tilesSensed; }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float angle;
      int range;
      int numTiles;
      float angularRange;
      std::vector<std::pair<int, int> > tilesSensed;
  };
}
#endif
