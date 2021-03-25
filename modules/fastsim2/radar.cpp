#include <iostream>
#include "radar.hpp"

namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /* Updates the radar, activating any slice that sees a matching goal. */
  int Radar::update(Posture& pos, const boost::shared_ptr<Map>& map) {
    Goal& g = map->getGoals()[color];
    float angle = normalizeAngle2PI(atan2(g.getY() - pos.getY(), g.getX() - pos.getX()) - pos.getTheta());
    float x;
    float y;
    bool not_visible = throughWalls ? false : map->isRealLineOfSightObscured(pos.getX(), pos.getY(), g.getX(), g.getY(), x, y);
    if (not_visible && !throughWalls) {
      activatedSlice = -1;
      return -1;
    }
    float xi = sliceSize;
    for (size_t i = 0; i < numSlices + 1; i++) {
    	if (angle < xi) {
    	  return (activatedSlice = i % numSlices);
      }
    	xi += sliceSize;
    }
    return -1;
  }
}
