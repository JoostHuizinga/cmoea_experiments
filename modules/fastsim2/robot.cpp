#include <iostream>
#include <list>
#include <cmath>
#include "robot.hpp"

namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /* Moves this robot by applying the given forces to its left and right motors. */
  void Robot::move(float v1, float v2, const boost::shared_ptr<Map>& m) {
    m->visitTile(position); //update the robot's visitations on the map
    Posture prev = position;
    position.move(v1, v2, radius * 2);
    updateBoundingBox();
    // update bumpers & go back if there is a collision
    if(checkCollision(m)) {
    	float theta = position.getTheta();
    	position = prev;
    	position.setTheta(theta);
    	collision = true;
    }
    for (size_t i = 0; i < lasers.size(); i++) {
      lasers[i].update(position, m);
    }
    for (size_t i = 0; i < radars.size(); i++) {
      radars[i].update(position, m);
    }
    for (size_t i = 0; i < lightSensors.size(); i++) {
      lightSensors[i].update(position, m);
    }
    for (size_t i = 0; i < m->getDoors().size(); i++) {
      m->getDoors()[i].update(m);
    }
    if (useCamera) {
      camera.update(position, m);
    }
    for (size_t i = 0; i < tileSensors.size(); i++) {
      tileSensors[i].update(position, m);
    }
  }

  /* Checks for a collision, and returns true if one exists. */
  bool Robot::checkCollision(const boost::shared_ptr<Map>& m) {
    // pixel wise
    int rp = m->realToPixel(radius);
    int r = rp * rp;
    int x = m->realToPixel(position.getX());
    int y = m->realToPixel(position.getY());
    int bbx = m->realToPixel(boundingBox.x);
    int bby = m->realToPixel(boundingBox.y);
    int bbw = m->realToPixel(boundingBox.x + boundingBox.w);
    int bbh = m->realToPixel(boundingBox.y + boundingBox.h);
    
    typedef std::pair<int, int> p_t;
    std::list<p_t > coll_points;
    for (int i = bbx; i < bbw; i++) {
      for (int j = bby; j < bbh; j++) {
      	if (m->getPixel(i, j) != Map::noCollision) {
      	  float d1 = (i - x);
      	  float d2 = (j - y);
      	  if (d1 * d1 + d2 * d2 <= r) {
      	    coll_points.push_back(p_t(i, j));
          }
      	}
      }
    }
    leftBumper = false;
    rightBumper = false;
    if (coll_points.empty()) {
      return false;
    } else {
    	for (std::list<p_t>::const_iterator it = coll_points.begin(); it != coll_points.end(); ++it) {
    	    float a = normalizeAngle(atan2(it->second - y, it->first - x) - position.getTheta());
    	    if (a > 0 && a < M_PI / 2.0) {
    	      leftBumper = true;
          }
    	    if (a < 0 && a > -M_PI / 2.0) {
    	      rightBumper = true;
          }
    	}
    	return true;
    }
  }

  /* Updates the bounding box of this robot. */
  void Robot::updateBoundingBox() {
    boundingBox.x = position.getX() - radius - 4;
    boundingBox.y = position.getY() - radius - 4;
  }
}
