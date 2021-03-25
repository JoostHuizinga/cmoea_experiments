#ifndef MAP_HH_
#define MAP_HH_

#include <vector>
#include <fstream>
#include <cmath>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>
#include "misc.hpp"
#include "goal.hpp"
#include "illuminated_switch.hpp"
#include "door.hpp"
#include "posture.hpp"

namespace fastsim {
  class Door; //forward declaration
  class Map {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Constants *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      static const int collision;
      static const int noCollision;

      typedef boost::shared_ptr<IlluminatedSwitch> illuminated_switch;

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /*
       * Creates a map using the given file and grid size.
       * By convention, all maps use (0, 0) as the top-left corner.
       */
      Map(std::string mapFile, int gridSize) :
        gridSize(gridSize),
        scalingFactor(1.0f)
      {
        readFile(mapFile);
        initGrid();
      }


      /*
       * Creates a map of the given size.
       */
      Map(int width_, int height_, int gridSize) :
        gridSize(gridSize),
        scalingFactor(1.0f)
      {
        width = width_;
        height = height_;
        _checkDimensions();
        /* Create the data grid. */
        data.resize(width, std::vector<int>(height, Map::noCollision));

        initGrid();
      }

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Gets the state of the pixel at the given (pixel) coordinates. */
      int getPixel(unsigned x, unsigned y) const{
        if (x >= width || y >= height) {
          return Map::collision; //out-of-bounds counts as a collision
        }
        return data.at(x).at(y);
      }

      /* Sets the state of the pixel at the given (pixel) coordinates. */
      void setPixel(unsigned x, unsigned y, int state) {
        if (x >=width || y >= height) {
          std::cout << "Error: tried to set a pixel at out-of-bounds location (" << x << ", " << y << ")!\n";
          return;
        }
        data.at(x).at(y) = state;
      }

      /* Gets the state of the pixel at the given (real) coordinates. */
      int getReal(float x, float y) const{ return getPixel(realToPixel(x), realToPixel(y)); }

      /* Converts the real-valued coordinate to a pixel coordinate. */
      int realToPixel(float real) const{ return (unsigned) roundf(real * scalingFactor); }

      /* Converts the pixel coordinate to a real-valued coordinate. */
      float pixelToReal(unsigned pixel) const{ return pixel / scalingFactor; }

      /* Gets the real-valued width of this map. */
      float getRealWidth() const{ return width * 1.0f; }

      /* Gets the real-value height of this map. */
      float getRealHeight() const{ return height * 1.0f; }

      /* Gets the width of this map in pixels. */
      unsigned getWidth() const{ return width; }

      /* Gets the height of this map in pixels. */
      unsigned getHeight() const{ return height; }

      /* Gets the vector of goals in this map. */
      std::vector<Goal>& getGoals() { return goals; }

      /* Adds a goal to this map. */
      void addGoal(const Goal& g) { goals.push_back(g); }

      /* Gets the vector of doors in this map. */
      std::vector<Door>& getDoors() { return doors; }

      /* Adds a door to this map. */
      void addDoor(const Door& door) { doors.push_back(door); }

      /* Gets the vector of illuminated switches in this map. */
      std::vector<illuminated_switch>& getIlluminatedSwitches() { return illuminatedSwitches; }

      /* Adds an illuminated switch to this map. */
      void addIlluminatedSwitch(illuminated_switch ill_switch) { illuminatedSwitches.push_back(ill_switch); }

      /*
       * Checks whether the given line-of-sight is obstructed by a collision, where the line is given in pixel coordinates.
       * Uses a Bresenham-based supercover algorithm (http://lifc.univ-fcomte.fr/~dedu/projects/bresenham/index.html).
       * Returns true if the line is obstructed, and sets the value of result variables to the obstructing pixel.
       * If no obstruction was detected, the result variables will still be altered (typically to very large numbers).
       * Be sure to check the return value of this expression before using y_result or x_result.
       */
      bool isLineOfSightObscured(int x1, int y1, int x2, int y2, int& x_result, int& y_result);

      /*
       * Checks whether the given line-of-sight is obstructed by a collision, where the line is given in real-valued coordinates.
       * Uses a Bresenham-based supercover algorithm (http://lifc.univ-fcomte.fr/~dedu/projects/bresenham/index.html).
       * Returns true if the line is obstructed, and sets the value of result variables to the obstructing pixel.
       */
      bool isRealLineOfSightObscured(float x1, float y1, float x2, float y2, float& x_result, float &y_result) {
        int px = 0;
        int py = 0;
        bool result = isLineOfSightObscured(realToPixel(x1), realToPixel(y1), realToPixel(x2), realToPixel(y2), px, py);
        x_result = pixelToReal(px);
        y_result = pixelToReal(py);
        return result;
      }

      /*
       * Gets the color of the closest illuminated switch that intersects the real-valued line.
       * If no switch intersects this line-of-sight, or if all intersecting switches are off, returns -1.
       */
      int isSwitchVisible(float x1, float y1, float x2, float y2);
      
      /* Gets the first illuminated switch matching the given color, or an empty illuminated_switch if no switch is found. */
      illuminated_switch getSwitchByColor(int color)
      {
        for (size_t i = 0; i < illuminatedSwitches.size(); i++) {
          if (illuminatedSwitches[i]->getColor() == color) {
            return illuminatedSwitches[i];
          }
        }
        return illuminated_switch();
      }

      /* Looks for any nearby switches and tries to activate them. */
      void activateSwitches(Posture& robotPos) {
        for (size_t i = 0; i < illuminatedSwitches.size(); i++) {
  	      illuminatedSwitches[i]->tryToActivate(robotPos);
        }
      }

      /* Looks for any nearby switches of the given color and tries to activate them. */
      void activateSwitches(Posture& robotPos, int color) {
        for (size_t i = 0; i < illuminatedSwitches.size(); i++) {
          if (illuminatedSwitches[i]->getColor() == color) {
            illuminatedSwitches[i]->tryToActivate(robotPos);
          }
        }
      }

      /* Gets the width of each tile in the grid. */
      int getGridSize() { return gridSize; }

      /* Reinitializes the map to use the given file. */
      void reinit(std::string &mapFile) { readFile(mapFile); }

      /* Draws a region of pixels at the given coordinates with the given width and height. */
      void drawRegion(int x, int y, int rWidth, int rHeight, int color);

      /* Gets whether the given pixel has been visited. */
      bool isPixelVisited(int x, int y);

      /* Gets whether the given tile has been visited. */
      bool isTileVisited(int x, int y);

      /* Marks the appropriate tile as visited, given a (robot's) posture. */
      void visitTile(Posture& posture);

      /* Record the number of tiles visited by this robot (for stat collection). */
      int getNumTilesVisited() const;

      /* Store the grid of tiles visited by this robot (for stat collection). */
      int getGridWidth() const;
      int getGridHeight() const;
      const std::vector<std::vector<bool> >& getTilesVisited() const;

    protected:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      std::vector<std::vector<int> > data;
      std::vector<Goal> goals;
      std::vector<illuminated_switch> illuminatedSwitches;
      std::vector<Door> doors;
      int width;
      int height;
      int gridSize;
      std::vector<std::vector<bool> > tileGrid;

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      
      /* Reads map data from the given file. */
      void readFile(std::string &mapFile);
      
      /* Initializes a tile grid for this map using the current grid size. */
      void initGrid();

      /*
       * Checks whether the given real-valued line intersects the given circle.
       * Uses the "triangle method" for fast approximation.
       */
      bool isLineInCircle(float x1, float y1, float x2, float y2, float xr, float yr, float radius);


      void _checkDimensions();
    private:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float scalingFactor; //can be used to adjust the relative drawing size of the robot (typically not necessary)
  };


  std::ostream& operator<<(std::ostream& is, const Map& obj);
}
#endif /* !MAP_HH_ */
