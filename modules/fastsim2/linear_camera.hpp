#ifndef FASTSIM_LINEAR_CAMERA_HPP_
#define FASTSIM_LINEAR_CAMERA_HPP_

#include <boost/shared_ptr.hpp>
#include "posture.hpp"
#include "map.hpp"

namespace fastsim {
  class LinearCamera {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a camera with the default angular range (FoV) and pixel detection threshold. */
      LinearCamera() :
        angularRange(M_PI / 2),
        pixels(12)
      {
        std::fill(pixels.begin(), pixels.end(), -1);
      }

      /* Creates a camera with the given angular range (FoV) and pixel detection threshold. */
      LinearCamera(float angular_range, int nb_pixels) :
        angularRange(angular_range),
        pixels(nb_pixels)
      {
        std::fill(pixels.begin(), pixels.end(), -1);
      }

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
      void update(Posture& pos, const boost::shared_ptr<Map>& map);

      /* Gets the pixel values seen by this camera. */
      std::vector<int>& getPixels() { return pixels; }

      /* Gets the angular range of the camera. */
      float getAngularRange() { return angularRange; }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float angularRange;
      std::vector<int> pixels;
  };
}
#endif
