#ifndef FASTSIM_RADAR_HPP_
#define FASTSIM_RADAR_HPP_

#include <boost/shared_ptr.hpp>
#include "posture.hpp"
#include "map.hpp"

namespace fastsim {
  class Radar {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /*
       * Creates a new radar with "n" pie-slices arranged in a circle, each of which can detect the given color.
       * By default, radars are able to see through walls, and have an infinite range.
       */
      Radar(int color, int numSlices, bool throughWalls = true) :
        color(color),
        numSlices(numSlices),
        sliceSize(2 * M_PI / numSlices),
        activatedSlice(0),
        throughWalls(throughWalls)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Updates the radar, activating any slice that sees a matching goal. */
      int update(Posture& pos, const boost::shared_ptr<Map>& map);

      /* Gets the currently activated slice, or -1 if no slice can see the goal. */
      int getActivatedSlice() { return activatedSlice; }

      /* Gets the number of slices. */
      int getNumSlices() { return numSlices; }

      /* Gets the color detected by this radar. */
      int getColor() { return color; }

      /* Gets the size of each slice, in radians. */
      float getSliceSize() { return sliceSize; }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      int color;
      int numSlices;
      float sliceSize;
      int activatedSlice;
      bool throughWalls;
  };
}
#endif
