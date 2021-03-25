#include "SDL.h" //this is the only SDL file you will need to include
				 //if your compiler can't find it, either your SDL is not correctly installed or your link flags are incorrect
#include "simu_fastsim.hpp"


//Module includes
#include <modules/datatools/params.hpp>

//Include namespaces
using namespace sferes;

/**************************************************************
 ************************* PARAMETERS *************************
 **************************************************************/
SFERES_PARAMS(
    struct Params {
        static long seed;
        struct simu {
            SFERES_CONST float dt = 0.1f;
        };
    };
)


/* Adds sensors to the robot. */
template<typename Simu>
void initRobot(Simu& simulator) {
    float angle = 1;
    float sensorRange = 5;
    float colorToDetect = 0;
    float robotStartingX = 20;
    float robotStartingY = 20;
    float robotStartingRotation = 0;
	simulator.getRobot().addLaser(fastsim::Laser(angle, sensorRange)); //define these parameters before compiling
	simulator.getRobot().addLightSensor(fastsim::LightSensor(colorToDetect, angle, sensorRange)); //define these parameters before compiling
	simulator.getRobot().setPosture(fastsim::Posture(robotStartingX, robotStartingY, robotStartingRotation)); //define these parameters before compiling
}

/* Runs the simulation. */
void runSimulator() {
  /* Create the simulator. */
    std::string mapFile("cuisine.pbm");
    int gridSize = 50;
    bool useVisualization = true;
    float robotRadius = 10;

    simu::Fastsim<Params> simulator(mapFile, gridSize, useVisualization, robotRadius); //define these parameters before compiling
  	//note: Fastsim is designed to integrate with Sferes; to use it outside of Sferes, simu_fastsim.hpp and the above constructor will need to be changed
    initRobot(simulator);

    bool drawGoals = true;
    bool drawSwitches = true;
    bool drawRadars = true;
    bool drawLasers = true;
    bool drawLightSensors = true;
    bool drawGrid = true;
    bool drawTiles = true;
    bool drawTileSensors = true;

    simulator.adjustVisibility(drawGoals, drawSwitches, drawRadars, drawLasers, drawLightSensors, drawGrid, drawTiles, drawTileSensors); //define these booleans before compiling

    float lastMotorLeft = 0.0;
    float lastMotorRight = 0.0;

    /* Run the simulator forever. */
    while (true) {
        simulator.updateView(); //update the display

        /* Check for manual input. */
        SDL_Event keyPressed;
        SDL_PollEvent(&keyPressed); //check for key presses
        /* CASE 1a: Pressed a command. */
        if (keyPressed.key.state == SDL_PRESSED) {
            int keyCode = keyPressed.key.keysym.sym;
            switch (keyCode) {
                case SDLK_LEFT: //turn left
                    lastMotorLeft += 0.01;
                    lastMotorRight -= 0.01;
                    break;
                case SDLK_RIGHT: //turn right
                    lastMotorLeft -= 0.01;
                    lastMotorRight += 0.01;
                    break;
                case SDLK_UP: //move forward
                    lastMotorLeft += 0.03;
                    lastMotorRight += 0.03;
                    break;
                case SDLK_DOWN: //move backward
                    lastMotorLeft -= 0.03;
                    lastMotorRight -= 0.03;
                    break;
                case SDLK_ESCAPE: //quit program
                    SDL_Quit();
                    exit(0);
                    break;
            }
            /* CASE 1b: no command given; stop the robot. */
        } else {
            lastMotorLeft = 0.0;
            lastMotorRight = 0.0;
        }
        if (lastMotorLeft > 1.5) { lastMotorLeft = 1.5; } //clamp the left motor to prevent infinite acceleration
        if (lastMotorLeft < -1.5) { lastMotorLeft = -1.5; } //clamp the left motor to prevent infinite deceleration
        if (lastMotorRight > 1.5) { lastMotorRight = 1.5; } //clamp the right motor to prevent infinite acceleration
        if (lastMotorRight < -1.5) { lastMotorRight = -1.5; } //clamp the right motor to prevent infinite deceleration

        /* Move the robot. */
        simulator.moveRobot(lastMotorLeft, lastMotorRight);
    }
}

/* Sets up and runs the simulation. */
extern "C" int main(int argc, char **argv) {
    /*
     * Load a configuration file, initialize parameters, etc.
     */
    runSimulator();
    return 0;
}
