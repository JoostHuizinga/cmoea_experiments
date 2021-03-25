#ifndef ROBOT_HH_
#define ROBOT_HH_

#include <vector>
#include "map.hpp"
#include "posture.hpp"
#include "laser.hpp"
#include "radar.hpp"
#include "light_sensor.hpp"
#include "linear_camera.hpp"
#include "tile_sensor.hpp"

namespace fastsim {
  class Robot {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Structures *//////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      
      /* A bounding box, to be used for collisions and drawing. */
      struct BoundingBox {
        float x;
        float y;
        float w;
        float h;
      };

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Creates a robot centered at (0, 0). */
      Robot(float radius) : 
        radius(radius),
        position(0, 0, 0),
        leftBumper(false),
        rightBumper(false),
        useCamera(false),
        collision(false),
        color(0)
      {
        boundingBox.w = radius * 2 + 8;
        boundingBox.h = radius * 2 + 8;      
        updateBoundingBox();
      }

      /* Creates a robot with the given posture. */
      Robot(float radius, Posture& pos) :
        radius(radius),
        position(pos), 
        leftBumper(false),
        rightBumper(false),
        useCamera(false),
        collision(false),
        color(0)
      {
        boundingBox.w = radius * 2 + 8;
        boundingBox.h = radius * 2 + 8;      
        updateBoundingBox();
      }

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      /* Clears the collision and bumper flags for this robot. */
      void reinit() {
        collision = false;
        leftBumper = false;
        rightBumper = false;
      }

      /* Moves this robot by applying the given forces to its left and right motors. */
      void move(float motorLeft, float motorRight, const boost::shared_ptr<Map>& m);

      /* Gets the posture of this robot. */
      Posture& getPosture() { return position; }

      /* Sets the posture of this robot. */
      void setPosture(const Posture& pos) { position = pos; }

      /* Gets the bounding-box of this robot. */
      BoundingBox& getBoundingBox() { return boundingBox; }

      /* Gets the radius of this robot. */
      float getRadius() { return radius; }

      /* Gets whether the robot has collided with an object. */
      bool isCollision() { return collision;}
      
      /* Gets the state of the left bumper (returns true if the bumper is touching an obstacle). */
      bool getLeftBumper() { return leftBumper; }

      /* Gets the state of the right bumper (returns true if the bumper is touching an obstacle). */
      bool getRightBumper() { return rightBumper; }

      /* Makes the robot use the given camera. */
      void enableCamera(const LinearCamera& c) {
        camera = c;
        useCamera = true;
      }

      /* Gets the camera on this robot, if one exists. */
      LinearCamera& getCamera() { return camera; }

      /* Gets whether this robot currently has a camera. */
      bool isUsingCamera() { return useCamera; }

      /* Adds a laser range-finder to this robot. */
      void addLaser(const Laser& l) { lasers.push_back(l); }

      /* Gets the vector of laser range-finders currently attached to this robot. */
      std::vector<Laser>& getLasers() { return lasers; }

      /* Adds a radar to this robot, which can detect a specific goal color. */
      void addRadar(const Radar& r){ radars.push_back(r); }

      /* Gets the vector of radars currently attached to this robot. */
      std::vector<Radar>& getRadars() { return radars; }

      /* Adds a light sensor to this robot, which can detect illuminated switches. */
      void addLightSensor(const LightSensor& l) { lightSensors.push_back(l); }

      /* Gets the vector of light sensors currently attached to this robot. */
      std::vector<LightSensor>& getLightSensors() { return lightSensors; }

      /* Adds a tile sensor to this robot, to detect unvisited tiles. */
      void addTileSensor(const TileSensor& t) { tileSensors.push_back(t); }

      /* Gets the vector of tile sensors currently attached to this robot. */
      std::vector<TileSensor>& getTileSensors() { return tileSensors; }

      /* Sets the color of this robot, used for drawing purposes. */
      void setColor(unsigned int color) { this->color = color; }

      /* Gets the current color of this robot. */
      unsigned int getColor() { return color; }

    protected:
      /* Checks for a collision, and returns true if one exists. */
      bool checkCollision(const boost::shared_ptr<Map>& m);

      /* Updates the bounding box of this robot. */
      void updateBoundingBox();

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      float radius;
      Posture position;
      BoundingBox boundingBox;
      bool leftBumper;
      bool rightBumper;
      std::vector<Laser> lasers;
      std::vector<Radar> radars;
      std::vector<LightSensor> lightSensors;
      std::vector<TileSensor> tileSensors;
      LinearCamera camera;
      bool useCamera;
      bool collision;
      unsigned int color;
  };
}
#endif /* !ROBOT_HH_ */
