#include <iostream>
#include "linear_camera.hpp"

namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  
  /*
   * Updates the camera by sampling pixels in an even distribution over the camera's angular range (FoV).
   * After calling this function, use getPixels() to get the pixel colors, corresponding to illuminated switches, seen by this camera.
   * Note that the actual range of the camera is theoretically infinite (in actuality, 10000 pixels).
   */
  void LinearCamera :: update(Posture& pos, const boost::shared_ptr<Map>& map) {
    float inc = angularRange / pixels.size();
    float r = -angularRange / 2.0f;
    for (size_t i = 0; i < pixels.size(); i++, r += inc) {
    	float alpha = r + pos.getTheta();
    	float xr = cos(alpha) * 10000 + pos.getX();
    	float yr = sin(alpha) * 10000 + pos.getX();
    	assert(i < pixels.size());
    	pixels[i] = map->isSwitchVisible(pos.getX(), pos.getY(), xr, yr);
    }
  }
}
