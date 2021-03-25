#ifndef POSTURE_HH_
#define POSTURE_HH_
#include <cmath>
#include "misc.hpp"

namespace fastsim {
  class Posture {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a posture with the given coordinates and rotation. */
      Posture(float x, float y, float theta) : 
        x(x),
        y(y),
        theta(theta)
      {}

      /* Creates a non-initialized posture. */
      Posture(){}

      /* Creates a posture with the same parameters as the given posture. */
      Posture(const Posture& p) : 
        x(p.x), 
        y(p.y), 
        theta(p.theta)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////* Operator Overloads *//////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Overloads the assignment operator to work on postures. */
      Posture& operator=(const Posture& o) {
        x = o.x;
        y = o.y;
        theta = o.theta;
        return *this;
      }

      /* 
       * Overloads the addition operator to work on postures.
       * Creates a new posture that is the addition of this posture and the given posture.
       */
      Posture operator+(const Posture& o) {
        Posture p;
        p.x = x + o.x;
        p.y = y + o.y;
        p.theta = normalizeAngle(theta + o.theta);
        return p;
      }

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Gets the rotation of this posture. */
      float getTheta() { return theta;}

      /* Sets the rotation of this posture. */
      void setTheta(float theta) { this->theta = theta; }

      /* Gets the x-coordinate of this posture. */
      float getX() { return x; }

      /* Gets the y-coordinate of this posture. */
      float getY() { return y; }

      /* Sets the x-coordinate of this posture. */
      void setX(float x) { this->x = x; }
      
      /* Sets the y-coordinate of this posture. */
      void setY(float y) { this->y = y; }

      /* Gets the Euclidian distance from this posture to the given point (x, y). */
      float distTo(float x, float y) {
        float xx = this->x - x;
        float yy = this->y - y;
        return sqrtf(xx * xx + yy * yy);
      }

      /* Gets the Euclidian distance from this posture to the given posture. */
      float distTo(const Posture& p) { return distTo(p.x, p.y); }

      /* Rotates this posture by the given angle. */
//      Posture& rotate(float theta) {
//        x = cos(theta) * x - sin(theta) * y;
//        y = cos(theta) * y + sin(theta) * x;
//        std::cout << "x: " << x << " y: " << y << " theta: " << theta << " length: " << distTo(0,0) << std::endl;
//        this->theta = theta;
//        return *this;
//      }

      const Posture& rotate(float theta)
      {
        float x_ = cos(theta) * x - sin(theta) * y;
        float y_ = cos(theta) * y + sin(theta) * x;
        x = x_;
        y = y_;
        this->theta  += theta;
        return *this;
      }

      /* Moves this posture by the given amount. */
      Posture& move(float d_l, float d_r, float wheels_dist) {
        Posture old_pos = *this;
        float alpha = (d_r - d_l) / wheels_dist;
        Posture p;
        if (fabs(alpha) > 1e-10) {
      	  float	r = (d_l / alpha) +  wheels_dist / 2;
      	  float	d_x = (cos(alpha) - 1) * r;
      	  float	d_y = sin(alpha) * r;
      	  p = Posture(d_x, d_y, alpha);
      	  p.rotate(old_pos.getTheta() - M_PI / 2);
      	  p.setTheta(normalizeAngle(alpha));
  	    } else {
  	      p = Posture(d_l * cos(old_pos.getTheta()), d_l * sin(old_pos.getTheta()), 0);
        }
        *this = p + old_pos;
        return *this;
      }

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float x;
      float y;    
      float theta;
  };
}


#endif /* !POSTURE_HH_ */
