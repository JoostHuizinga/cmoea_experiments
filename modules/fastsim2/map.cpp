#include <iostream>
#include "map.hpp"

namespace fastsim {
    const int Map::collision = -1;
    const int Map::noCollision = 0;
  
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /* Reads map data from the given file. */
  void Map::readFile(std::string& mapFile) {
    std::ifstream mapInput(mapFile.c_str());
    if (mapInput.is_open()) {
      std::string input;

      /* Check that the file is valid. */
      mapInput >> input; //get the file type of this image
      if (input != "P1") {
        std::cout << "Error: " << mapFile << " must be an ASCII-formatted, grayscale Portable Bitmap File (P1)!\n";
        exit(1);
      }

      /* Ensure that the file is not commented, and get the image's width and height. */
      int tempWidth = 0;
      mapInput >> tempWidth;
      if (tempWidth == 0) { //this will occur if the token is actually a string (i.e. a comment)
        std::cout << "Error: " << mapFile << " should not be commented (remove lines starting with '#' from this file before proceeding)!\n";
        exit(1);
      } else {
        width = tempWidth;
      }
      mapInput >> height;

      _checkDimensions();

      /* Create the data grid. */
      data.resize(width, std::vector<int>(height, Map::noCollision));

      /* Read all pixels from the image into a vector. */
      std::vector<int> pixelStates(width * height);
      char c;
      for (int i = 0; i < pixelStates.size(); i++) {
        mapInput.get(c);
        /* CASE 1: pixel is black */
        if (c == '1') {
          pixelStates.at(i) = Map::collision;
        /* CASE 2: pixel is white */
        } else if (c == '0') {
          pixelStates.at(i) = Map::noCollision;
        /* CASE 3: not a pixel (e.g. a line delimiter) */
        } else {
          i--; //not a valid pixel, so don't advance i
        } 
      }

      /* Read each pixel into the corresponding tile-grid location. */
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          data.at(x).at(y) = pixelStates.at(y * width + x);
        }
      }
    } else {
      std::cout << "Error: could not open " << mapFile << "!\n";
      exit(1);
    }
    std::cout << "Successfully loaded " << mapFile << ": width " << width << " and height " << height << "\n";
  }
  
  /*
   * Checks whether the given real-valued line intersects the given circle.
   * Uses the "triangle method" for fast approximation.
   */
  bool Map::isLineInCircle(float x1, float y1, float x2, float y2, float xr, float yr, float radius) {
    // check if the object is on the right side of the camera
    float dot = (x2 - x1) * (xr - x1) + (y2 - y1) * (yr - y1);
    if (dot < 0) {
      return false;
    }
    float area2 = fabs((x2 - x1) * (yr - y1) -  (y2 - y1) * (xr - x1));
    float dx = x2 - x1;
    float dy = y2 - y1;
    float lab = sqrt(dx * dx + dy * dy);
    float h = area2 / lab;
    if (h < radius) {
      return true;
    }
    return false;
  }
  
  /*
   * Gets the color of the closest illuminated switch that intersects the real-valued line.
   * If no switch intersects this line-of-sight, or if all intersecting switches are off, returns -1.
   */
  int Map::isSwitchVisible(float x1, float y1, float x2, float y2) {
    // list intersections with rays
    std::vector<illuminated_switch> res;
    for (size_t i = 0; i < illuminatedSwitches.size(); i++) {
    	illuminated_switch isv = illuminatedSwitches[i];
    	float xtmp, ytmp;
    	if (isv->isOn()
    	    && isLineInCircle(x1, y1, x2, y2, isv->getX(), isv->getY(), isv->getRadius())
    	    && !isRealLineOfSightObscured(x1, y1, isv->getX(), isv->getY(), xtmp, ytmp)) {
    	  res.push_back(isv);	    
      }
    }
    if (res.empty()) {
      return -1;
    }
    // return the closest
    std::sort(res.begin(), res.end(), ClosestSwitch(x1, y1));
    return res[0]->getColor();
  }

  /*
   * Checks whether the given line-of-sight is obstructed by a collision, where the line is given in pixel coordinates.
   * Uses a Bresenham-based supercover algorithm (http://lifc.univ-fcomte.fr/~dedu/projects/bresenham/index.html).
   * Returns true if the line is obstructed, and sets the value of result variables to the obstructing pixel.
   * If no obstruction was detected, the result variables will still be altered (typically to very large numbers).
   * Be sure to check the return value of this expression before using y_result or x_result.
   */
  bool Map::isLineOfSightObscured(int y1, int x1, int y2, int x2, int& y_result, int& x_result) {
    int i;               // loop counter
    int ystep, xstep;    // the step on y and x axis
    int error;           // the error accumulated during the increment
    int errorprev;       // *vision the previous value of the error variable
    int y = y1, x = x1;  // the line points
    int ddy, ddx;        // compulsory variables: the double values of dy and dx
    int dx = x2 - x1;
    int dy = y2 - y1;
    bool inter = (getPixel(y1, x1) != Map::noCollision);
    if (dy < 0) {
      ystep = -1;
      dy = -dy;
    } else {
      ystep = 1;
    }
    if (dx < 0) {
      xstep = -1;
      dx = -dx;
    } else {
      xstep = 1;
    }
    ddy = dy * 2;
    ddx = dx * 2;
    if (ddx >= ddy) { // first octant (0 <= slope <= 1)  
    	errorprev = error = dx;
    	for (i = 0 ; i < dx; i++) {  // do not use the first point (already done)
    	  x += xstep;
    	  error += ddy;
    	  if (error > ddx) {
      		y += ystep;
      		error -= ddx;
      		if (error + errorprev < ddx) {  // bottom square also
      		  inter = inter || (getPixel(y - ystep, x) != Map::noCollision);
      		} else if (error + errorprev > ddx) { // left square also
      		  inter = inter || (getPixel(y, x - xstep) != Map::noCollision);
      		} else { // corner: bottom and left squares also
      		  inter = inter || (getPixel(y - ystep, x) != Map::noCollision);
      		  inter = inter || (getPixel(y, x - xstep) != Map::noCollision);
      		}
    	  }
    	  inter = inter || (getPixel(y, x) != Map::noCollision);
    	  errorprev = error;
    	  if (inter) {
    		  x_result = x;
    		  y_result = y;
    		  return true;
    	  }
    	}
    } else { // the same as above
  	  errorprev = error = dy;
    	for (i = 0 ; i < dy; i++) {
    	  y += ystep;
    	  error += ddx;
    	  if (error > ddy) {
    	    x += xstep;
    	    error -= ddy;
      	  if (error + errorprev < ddy) {
      		  inter = inter || (getPixel(y, x - xstep) != Map::noCollision);
      	  } else if (error + errorprev > ddy) {
      		  inter = inter || (getPixel(y - ystep, x) != Map::noCollision);
      	  } else {
      		  inter = inter || (getPixel(y, x - xstep) != Map::noCollision);
      		  inter = inter || (getPixel(y -  ystep, x) != Map::noCollision);
      		}
    	  }
    	  inter = inter || (getPixel(y, x) != Map::noCollision);
    	  errorprev = error;
    	  if (inter) {
      		x_result = x;
      		y_result = y;
      		return true;
    	  }
    	}
    }
    return false;
  }

  /* Draws a region of pixels at the given coordinates with the given width and height. */
  void Map::drawRegion(int x, int y, int lx, int ly, int color = Map::collision) {
    for (int i = 0; i < lx; i++) {
      for (int j = 0; j < ly; j++) {
      	if ((x + i) >= 0 && (y + j) >= 0 && (x + i) < getWidth()  && (y + j) < getHeight()) {
      	  setPixel(x + i, y + j, color);
        }
      }
    }
  }

  /* Initializes a tile grid for this map using the current grid size. */
  void Map::initGrid() {
    tileGrid.resize(width / gridSize, std::vector<bool>(height / gridSize, false));
  }

  /* Gets whether the given pixel has been visited. */
  bool Map::isPixelVisited(int x, int y) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
      std::cout << "Error: given pixel coordinates (" << x << ", " << y << ") are out of bounds!\n";
      return false;
    }
    return tileGrid.at(x / gridSize).at(y / gridSize);
  }

  /* Gets whether the given tile has been visited. */
  bool Map::isTileVisited(int x, int y) {
    if (x < 0 || y < 0 || x >= (width / gridSize) || y >= (height / gridSize)) {
      std::cout << "Error: given tile coordinates (" << x << ", " << y << ") are out of bounds!\n";
      return false;
    }
    return tileGrid.at(x).at(y);
  }

  /* Marks the appropriate tile as visited, given a (robot's) posture. */
  void Map::visitTile(Posture& posture) {
    int xPixel = realToPixel(posture.getX());
    int yPixel = realToPixel(posture.getY());
    tileGrid.at(xPixel / gridSize).at(yPixel / gridSize) = true;
  }

  /* Record the number of tiles visited by this robot (for stat collection). */
  int Map::getNumTilesVisited() const{
      int result=0;
      for(size_t i = 0; i < tileGrid.size(); ++i){
          for(size_t j = 0; j < tileGrid.size(); ++j){
              if(tileGrid[i][j]){
                  ++result;
              }
          }
      }
      return result;
  }

  /* Store the grid of tiles visited by this robot (for stat collection). */
  int Map::getGridWidth() const{
      return (width / gridSize);
  }
  int Map::getGridHeight() const{
      return (height / gridSize);
  }
  const std::vector<std::vector<bool> >& Map::getTilesVisited() const{
      return tileGrid;
  }

  void Map::_checkDimensions(){
      /* Check for illegal width or height. */
      if (width % 8 != 0) {
        std::cout << "Error: illegal width " << width << "; width must be a multiple of 8!\n";
        exit(1);
      } else if (height % 8 != 0) {
        std::cout << "Error: illegal height " << height << "; height must be a multiple of 8!\n";
        exit(1);
      } else if (width % gridSize != 0) {
        std::cout << "Error: width must be divisible by grid size! " << gridSize << " (" << width << " %% " << gridSize << " != 0)\n";
        exit(1);
      } else if (height % gridSize != 0) {
        std::cout << "Error: height must be divisible by grid size! " << gridSize << " (" << height << " %% " << gridSize << " != 0)\n";
        exit(1);
      } else if (gridSize == 0) {
        std::cout << "Error: grid size cannot be 0!\n";
        exit(1);
      }

  }

  std::ostream& operator<<(std::ostream& is, const Map& obj){
      /* Read each pixel into the corresponding tile-grid location. */
      for (unsigned int y = 0; y < obj.getHeight(); y++) {
        for (unsigned int x = 0; x < obj.getWidth(); x++) {
            if (obj.getPixel(x, y) == Map::collision){
                is << 'x';
            } else {
                is << '-';
            }
            //is << obj.getPixel(x, y);
        }
        is << "\n";
      }

      return is;
  }
}
