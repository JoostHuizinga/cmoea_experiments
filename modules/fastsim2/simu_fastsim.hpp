#ifndef SIMU_FASTSIM_HPP_
#define SIMU_FASTSIM_HPP_

#include <iostream>
#include <boost/config.hpp> //BOOST_STATIC_CONSTEXPR
#include <sferes/simu/simu.hpp>
#include "fastsim.hpp"

namespace sferes {
  namespace simu {
    SFERES_SIMU(Fastsim, Simu) {
      public:
      	    ////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////
    		////////////* Constructors/Destructors *////////////////
    		////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////

        /*
         * Creates a new simulator with the given map file, grid size, and robot radius.
         * If visualization is desired, a frame rate (given in frames per second) may also be provided; the default is 60 FPS.
         */
        Fastsim(std::string mapFile, int gridSize, bool useVisualization, float robotRadius, float frameRate = 60.0f) :
        	mapFile(mapFile),
        	robot(robotRadius)
        {
        	map = boost::shared_ptr<fastsim::Map>(new fastsim::Map(mapFile, gridSize));
            if (useVisualization) {
                initDisplay(frameRate);
                //display = boost::shared_ptr<fastsim::Display>(new fastsim::Display(map, robot, frameRate));
            }
        }

        /*
         * Creates a new simulator with the map file, grid size, and robot radius.
         * If visualization is desired, a frame rate (given in frames per second) may also be provided; the default is 60 FPS.
         */
        Fastsim(boost::shared_ptr<fastsim::Map> map_, int gridSize, bool useVisualization, float robotRadius, float frameRate = 60.0f) :
            mapFile(""),
            robot(robotRadius)
        {
            map = map_;
            if (useVisualization) {
                initDisplay(frameRate);
                //display = boost::shared_ptr<fastsim::Display>(new fastsim::Display(map, robot, frameRate));
            }
        }

        /* Does nothing. */
        ~Fastsim(){}

        ////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////
    		////////////////////* Functions *///////////////////////
    		////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////

        /* Initialize the display after the simulator has already been created */
        void initDisplay(float frameRate = 60.0f){
            if(!display){
                display = boost::shared_ptr<fastsim::Display>(new fastsim::Display(map, robot, frameRate));
            }
        }

        /* Controls which map elements (mostly sensors) are drawn by the display. */
        void adjustVisibility(bool goals, bool switches, bool radars, bool lasers, bool lightSensors, bool grid, bool tiles, bool tileSensors) {
          display->dGoals = goals;
          display->dSwitches = switches;
          display->dRadars = radars;
          display->dLasers = lasers;
          display->dLightSensors = lightSensors;
          display->dGrid = grid;
          display->dTiles = tiles;
          display->dTileSensors = tileSensors;
        }

       	/*
       	 * Refreshes the simulator's display.
       	 * Should be called once every timestep IF AND ONLY IF visualization is enabled.
       	 */
       	void updateView() {
          if (display.get()) {
          	display->update();
          } else {
          	std::cout << "Error: cannot call updateView() on a simulator in which visualization has not been enabled!\n";
          }
        }

        /*
         * Forces the simulator to use the given map.
         * If visualization is enabled, be sure to called refreshViewMap() or refreshViewAll() after calling this function.
         */
        void setMap(const boost::shared_ptr<fastsim::Map>& map) {
  			this->map = map;
        }
        
        /*
         * Forces the simulator to load the given map file.
         * If visualization is enabled, be sure to called refreshViewMap() or refreshViewAll() after calling this function.
    	   */
        void switchMap(std::string altMapFile) {
          map->reinit(altMapFile);
        }

        /*
    		 * Forces the simulator to reload the original map.
    		 * If visualization is enabled, be sure to called refreshViewMap() or refreshViewAll() after calling this function.
         */
        void resetMap() {
          if (mapFile != "") {
          	map->reinit(mapFile);
          } else {
          	std::cout << "Error: this simulator has not specified a map file (call setMapFile() or use an alternate constructor to fix this)!\n";
          }
        }

        /* Sets the map file, as would be used during a call to resetMap(). */
        void setMapFile(std::string mapFile) {
        	this->mapFile = mapFile;
        }

        /* Moves the robot attached to this simulator by setting its motors to the given values. */
        void moveRobot(float motorLeft, float motorRight) {
        	robot.move(motorLeft, motorRight, map);
       	}

       	/* Gets a reference to this simulator's current map. */
        boost::shared_ptr<fastsim::Map> getMap() {
        	return map;
        }

        /* Gets a reference to this simulator's current robot. */
        fastsim::Robot& getRobot() {
        	return robot;
        }

        /* Gets a reference to this simulator's current display. */
        fastsim::Display& getDisplay() {
        	return *display;
        }

      protected:
      	////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////
    		//////////////////////* Fields *////////////////////////
    		////////////////////////////////////////////////////////
    		////////////////////////////////////////////////////////

      	std::string mapFile;
        fastsim::Robot robot;
        boost::shared_ptr<fastsim::Map> map;
        boost::shared_ptr<fastsim::Display> display;
    };
  }
}
#endif
