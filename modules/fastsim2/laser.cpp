#include <iostream>
#include "laser.hpp"

namespace fastsim
{
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /* Calculates and returns the distance from this laser to the nearest wall. */
  float Laser::update(Posture& pos, const boost::shared_ptr<Map>& m) {
    float x2 = cosf(angle + pos.getTheta()) * range + pos.getX() + gapDist * cosf(gapAngle + pos.getTheta());
    float y2 = sinf(angle + pos.getTheta()) * range + pos.getY() + gapDist * sinf(gapAngle + pos.getTheta());

    //convert to pixels
    int xx1 = m->realToPixel(pos.getX() + gapDist * cosf(gapAngle + pos.getTheta()));
    int yy1 = m->realToPixel(pos.getY() + gapDist * sinf(gapAngle + pos.getTheta()));
    int xx2 = m->realToPixel(x2);
    int yy2 = m->realToPixel(y2);

    //check intersection
    xPixel = m->realToPixel(x2);
    yPixel = m->realToPixel(y2);
    bool inter = m->isLineOfSightObscured(xx1, yy1, xx2, yy2, xPixel, yPixel);
    assert(xPixel >= 0);
    assert(yPixel >= 0);

    //convert back to real
    x = m->pixelToReal(xPixel);
    y = m->pixelToReal(yPixel);

    if (!inter) {
      dist = -1;
    } else {
    	float px = pos.getX() + gapDist * cosf(gapAngle + pos.getTheta()) - x;
    	float py = pos.getY() + gapDist * sinf(gapAngle + pos.getTheta()) - y;    
    	dist = sqrtf(px * px  + py * py);
    }
    return dist;
  } 
}
