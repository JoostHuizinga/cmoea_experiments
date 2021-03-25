#ifndef FASTSIM_MISC_HH_
#define FASTSIM_MISC_HH_

#include <vector>
#include <utility>

namespace fastsim {
  template<typename T>
  inline T normalizeAngle(T a) {
    while (a > M_PI)
      a -= 2 * M_PI;
    while (a < -M_PI)
      a += 2 * M_PI;
    return a;
  }  
  
  template<typename T>
  inline T normalizeAngle2PI(T a) {
    while (a > 2 * M_PI)
      a -= 2 * M_PI;
    while (a < 0)
      a += 2 * M_PI;
    return a;
  }  

  /* Determines if a point lies in the defined triangle, using Barycentric coordinates. */
  inline bool pointLiesInTriangle(float targetX, float targetY, float x1, float y1, float x2, float y2, float x3, float y3) {
    float alpha = ((y2 - y3) * (targetX - x3) + (x3 - x2) * (targetY - y3))
      / ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    float beta = ((y3 - y1) * (targetX - x3) + (x1 - x3) * (targetY - y3))
      / ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    float gamma = 1.0f - alpha - beta;

    return (alpha >= 0) && (beta >= 0) && (gamma >= 0) && (gamma <= 1);
  }

  /* Converts the given angle to degrees. */
  inline int angleInDegrees(float angle) {
    return (int) (angle * 180 / M_PI);
  }

  /* Converts the given angle to radians. */
  inline float angleInRadians(int angle) {
    return angle * M_PI / 180.0f;
  }

  /* Gets whether a pair is contained in the given vector. */
  inline bool isPairInVector(std::pair<int, int> toAdd, std::vector<std::pair<int, int> > toTest) {
    for (size_t i = 0; i < toTest.size(); i++) {
      if (toTest.at(i).first == toAdd.first && toTest.at(i).second == toAdd.second) {
        return true;
      }
    }
    return false;
  }
}
#endif /* !MISC_HH_ */
