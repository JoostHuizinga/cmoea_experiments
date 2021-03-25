#include <iostream>
#include <algorithm>
#include "tile_sensor.hpp"

namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  
  /* Determines how many unvisited tiles are detected by this sensor. */
  int TileSensor::update(Posture& pos, const boost::shared_ptr<Map>& map) {
    std::vector<std::pair<int, int> > unvisitedTiles;
    std::vector<std::pair<int, int> > pointsToTest;

    /* Get point 1 of the triangle. */
    int xx1 = map->realToPixel((cosf(pos.getTheta() + angle + angularRange) * range / cosf(angularRange)) + pos.getX());
    int yy1 = map->realToPixel((sinf(pos.getTheta() + angle + angularRange) * range / cosf(angularRange)) + pos.getY());
    /* Get point 2 of the triangle. */
    int xx2 = map->realToPixel((cosf(pos.getTheta() + angle - angularRange) * range / cosf(angularRange)) + pos.getX());
    int yy2 = map->realToPixel((sinf(pos.getTheta() + angle - angularRange) * range / cosf(angularRange)) + pos.getY());
    /* Get point 3 of the triangle. */
    int x3 = map->realToPixel(pos.getX());
    int y3 = map->realToPixel(pos.getY());
    /* Adjust for overflow and underflow. */
    if (xx1 < 0) { xx1 = 0; }
    if (xx1 >= map->getWidth()) { xx1 = map->getWidth() - 1; }
    if (yy1 < 0) { yy1 = 0; }
    if (yy1 >= map->getHeight()) { yy1 = map->getHeight() - 1; }
    if (xx2 < 0) { xx2 = 0; }
    if (xx2 >= map->getWidth()) { xx2 = map->getWidth() - 1; }
    if (yy2 < 0) { yy2 = 0; }
    if (yy2 >= map->getHeight()) { yy2 = map->getHeight() - 1; }

    /* Update points based on collision. */
    int x1, y1, x2, y2;
    if (!map->isLineOfSightObscured(x3, y3, xx1, yy1, x1, y1)) {
      x1 = xx1; //if there was no collision, revert to original point, as isLineOfSightObscured() will modify the result variable either way
      y1 = yy1;
    }
    if (!map->isLineOfSightObscured(x3, y3, xx2, yy2, x2, y2)) {
      x2 = xx2; //if there was no collision, revert to original point, as isLineOfSightObscured() will modify the result variable either way
      y2 = yy2;
    }

    /* Get the bounding box of the triangle */
    int xMin = std::min(x3, std::min(x1, x2));
    int yMin = std::min(y3, std::min(y1, y2));
    int xMax = std::max(x3, std::max(x1, x2));
    int yMax = std::max(y3, std::max(y1, y2));

    /* Add points from the bounding box into the testing set, if they lie in the triangle we have defined. */
    for (size_t i = xMin; i <= xMax; i++) {
      for (size_t j = yMin; j <= yMax; j++) {
        if (pointLiesInTriangle(i, j, x1, y1, x2, y2, x3, y3)) {
          std::pair<int, int> newPair(i, j);
          pointsToTest.push_back(newPair);
        }
      }
    }

    /* Transform each sample point into a tile, check whether that tile has *not* been visited, and add it to the set. */
    for (size_t i = 0; i < pointsToTest.size(); i++) {
      int xTile = pointsToTest.at(i).first / map->getGridSize();
      int yTile = pointsToTest.at(i).second / map->getGridSize();
      // std::cout << "Point (" << pointsToTest.at(i).first << ", " << pointsToTest.at(i).second << ") maps to tile (" << xTile << ", " << yTile << ")\n";
      if (!map->isPixelVisited(pointsToTest.at(i).first, pointsToTest.at(i).second)) {
        std::pair<int, int> newPair(xTile, yTile);
        if (!isPairInVector(newPair, unvisitedTiles)) { //don't add the same tile more than once
          unvisitedTiles.push_back(newPair);
        }
      }
    }


        // /* Determine the number of unique tiles visited. */
    // for (size_t i = 0; i < pointsToTest.size(); i++) {
    //   int xTile = pointsToTest.at(i).first / map->getGridSize();
    //   int yTile = pointsToTest.at(i).second / map->getGridSize();

    //   if (!map->isPixelVisited(pointsToTest.at(i).first, pointsToTest.at(i).second)) {
    //     std::pair<int, int> newPair(xTile, yTile);
    //     if (!isPairInVector(newPair, unvisitedTiles)) {
    //       unvisitedTiles.push_back(newPair);
    //     }
    //   }
    // }

    /*
     * TODO: redo this check to use Barycentric rasterization
     * Determine the bounding box of the created triangle (calculated x1/y1 and truncate using realLineOfSightObscured first)
     * Sample each point in the bounding box and check if it belongs -- if yes, add to list
     * Select tiles as usual
     */

    // /* Sample points within the pie-slice, to be used for testing tiles. */
    // for (size_t i = 0; i <= range; i++) {
    //   /* Sample points along the left side. */
    //   for (size_t j = 0; j < angleInDegrees(angularRange) / 2; j++) {
    //     float x1 = (cosf(pos.getTheta() + angle + angleInRadians(j)) * i / cosf(angleInRadians(j))) + pos.getX();
    //     float y1 = (sinf(pos.getTheta() + angle + angleInRadians(j)) * i / cosf(angleInRadians(j))) + pos.getY();
    //     int xx1 = fabs(map->realToPixel(x1));
    //     int yy1 = fabs(map->realToPixel(y1));
    //     float x1Modified = fabs(map->realToPixel(x1));
    //     float y1Modified = fabs(map->realToPixel(y1));

    //     map->isRealLineOfSightObscured(posX, posY, xx1, yy1, x1Modified, y1Modified); //this will update x1Modified and y1Modified, if they obscured by an obstacle
    //     // std::cout << "x1: " << x1 << " and y1: " << y1 << "\n";
    //     // std::cout << x1Modified << ", " << y1Modified << "\n";
    //     std::pair<int, int> newPair(map->realToPixel(x1Modified), map->realToPixel(y1Modified));
    //     if (!isPairInVector(newPair, pointsToTest)) {
    //       pointsToTest.push_back(newPair); //add the (potentially modified) points to the set of test points
    //     }
    //   }

    //   // /* Sample points along the right side. */
    //   // for (size_t j = 0; j < angleInDegrees(angularRange) / 2; j++) {
    //   //   float x1 = (cosf(pos.getTheta() + angle - angleInRadians(j)) * i / cosf(angleInRadians(j))) + pos.getX();
    //   //   float y1 = (sinf(pos.getTheta() + angle - angleInRadians(j)) * i / cosf(angleInRadians(j))) + pos.getY();
    //   //   float x1Modified = x1;
    //   //   float y1Modified = y1;

    //   //   map->isRealLineOfSightObscured(pos.getX(), pos.getY(), x1, y1, x1Modified, y1Modified); //this will update x1Modified and y1Modified, if they obscured by an obstacle
    //   //   std::pair<int, int> newPair(map->realToPixel(x1Modified), map->realToPixel(y1Modified));
    //   //   if (!isPairInVector(newPair, pointsToTest)) {
    //   //     pointsToTest.push_back(newPair); //add the (potentially modified) points to the set of test points
    //   //   }
    //   // }
    // }

    // /* Determine the number of unique tiles visited. */
    // for (size_t i = 0; i < pointsToTest.size(); i++) {
    //   int xTile = pointsToTest.at(i).first / map->getGridSize();
    //   int yTile = pointsToTest.at(i).second / map->getGridSize();

    //   if (!map->isPixelVisited(pointsToTest.at(i).first, pointsToTest.at(i).second)) {
    //     std::pair<int, int> newPair(xTile, yTile);
    //     if (!isPairInVector(newPair, unvisitedTiles)) {
    //       unvisitedTiles.push_back(newPair);
    //     }
    //   }
    // }

    // std::cout << pointsToTest.size() << " and " << unvisitedTiles.size() << "\n";

    numTiles = unvisitedTiles.size();
    tilesSensed = unvisitedTiles;
    return numTiles;
  }
}
