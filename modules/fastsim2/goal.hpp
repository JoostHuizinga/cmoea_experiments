#ifndef FASTSIM_GOAL_HPP_
#define FASTSIM_GOAL_HPP_

namespace fastsim {
  class Goal { 
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a new goal at the given location. */
      Goal(float x, float y, float diameter, int color) :
        x(x),
        y(y),
        diameter(diameter),
        color(color)
      {}

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Gets the x-position of this goal. */
      float getX() { return x; }

      /* Gets the y-position of this goal. */
      float getY() { return y; }

      /* Gets the diameter of this goal. */
      float getDiameter() { return diameter; }

      /* Gets the color of this goal. */
      int getColor() { return color; }

    private:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float x;
      float y;
      float diameter;
      int color; 
    };
}
#endif