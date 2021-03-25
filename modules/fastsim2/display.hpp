#ifndef SDL_HH_
#define SDL_HH_

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <valarray>
#include <stdio.h>
#include <unistd.h>
#include "map.hpp"
#include "robot.hpp"
#include <boost/shared_ptr.hpp>

#ifdef USE_SDL
#include <SDL.h>
#endif

namespace fastsim {
  class Display {
    public:
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////* Constructors/Destructors *////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      #ifdef USE_SDL
        /*
         * Creates the Display.
         * A framerate must be specified; the display will attempt to render at this number of frames per second.
         */
        Display(const boost::shared_ptr<Map>& m, Robot& r, float frameRate);
        
        /* Destroys the Display by freeing all SDL resources. */
        ~Display() {
          SDL_FreeSurface(mapBMP);
          SDL_FreeSurface(screen);
          SDL_Quit();
        }
      #else
        /* Creates the Display (dummy method; doesn't use SDL). */
        Display(const boost::shared_ptr<Map>& m, Robot& r, float frameRate) :
          dGoals(true),
          dSwitches(true),
          dRadars(true),
          dLasers(true),
          dLightSensors(true),
          dGrid(false),
          dTiles(false),
          dTileSensors(false),
          dRobot(true),
          startRender(0.0f),
          frameCount(0),
          fps(0),
          map(m),
          robot(r),
          dCamera(false),
          width(0),
          height(0)
        {}
        
        /* Destroys the Display (dummy method; doesn't use SDL). */
        ~Display() {}
      #endif

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

#ifdef USE_SDL
        /* Updates the display (sensors, robot position, etc.). */
        void update();
#else
        /* Updates the display (dummy method; doesn't use SDL). */
        void update(){}
#endif

#ifdef USE_SDL
        SDL_Surface* getScreen(){
        	return screen;
        }
#endif
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      //////////////////////* Fields *////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      bool dGoals;
      bool dSwitches;
      bool dRadars;
      bool dLasers;
      bool dLightSensors;
      bool dGrid;
      bool dTiles;
      bool dTileSensors;
      bool dRobot;

    protected:
      float startRender;
      int frameCount;
      float fps;
      const boost::shared_ptr<Map>& map;
      Robot& robot;
      bool dCamera;
      int width;
      int height;

#ifdef USE_SDL
      SDL_Surface* screen;
      SDL_Surface* mapBMP;
#endif

      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////* Functions *///////////////////////
      ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////

      #ifdef USE_SDL
        /* Processes SDL events on this display. */
        void events();

        /* Closes the display. */
        void quit() {
          SDL_Quit();
          exit(0);
        }

        /* Places a colored pixel at the given location on a SDL surface. */
        void putPixel(SDL_Surface* surf, Uint32 color, unsigned x, unsigned y)
        {
          if (x >= surf->w || y >= surf->h ) { return; } //pixel is out of bounds
          Uint32 *bufp = (Uint32*)surf->pixels + y * surf->pitch / 4 + x;
          *bufp = color;
        }

        /* Places a colored pixel at the given location on a SDL surface. */
        void putPixel(SDL_Surface* surf, unsigned x, unsigned y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
        {
          if (x >= surf->w || y >= surf->h ) { return; } //pixel is out of bounds
          float alpha = ((float)a) / 255.0;
          Uint32 *bufp = (Uint32*)surf->pixels + y * surf->pitch / 4 + x;
          Uint8 cr;
          Uint8 cg;
          Uint8 cb;
          SDL_GetRGB(*bufp, surf->format, &cr, &cg, &cb);
          cr = cr*(1-alpha) + r*alpha;
          cg = cg*(1-alpha) + g*alpha;
          cb = cb*(1-alpha) + b*alpha;
          putPixel(surf, x, y, cr, cg, cb);
        }

        /* Places a colored pixel at the given location on a SDL surface. */
        void putPixel(SDL_Surface* surf, unsigned x, unsigned y, Uint8 r, Uint8 g, Uint8 b) { 
          putPixel(surf, SDL_MapRGB(surf->format, r, g, b), x, y);
        }
        
        bool testPixel(SDL_Surface* surf, unsigned x, unsigned y, Uint8 r, Uint8 g, Uint8 b){
            if (x >= surf->w || y >= surf->h ) { return false; } //pixel is out of bounds
            Uint32 *bufp = (Uint32*)surf->pixels + y * surf->pitch / 4 + x;
            Uint8 cr;
            Uint8 cg;
            Uint8 cb;
            SDL_GetRGB(*bufp, surf->format, &cr, &cg, &cb);
            return (cr < r) && (cg < g) && (cb < b);
        }

        /*
         * Places all pixels in the map onto the display.
         * Called whenever the map topology changes.
         */
        void putAllPixels();

        /* Draws the points of an ellipse, with the given center and x-radius and y-radius. */
        void drawCirclePoints(SDL_Surface* surf, int cx, int cy, int x, int y, Uint32 color);

        /* Draws a circle with the given color, position, and radius. */
        void drawCircle(SDL_Surface* surf, int x_center, int y_center, int radius, Uint32 color);

        /* Draws a circle with the given color, position, and radius. */
        void drawCircle(SDL_Surface* surf, int x_center, int y_center, int radius, Uint8 r, Uint8 g, Uint8 b) {
          drawCircle(surf, x_center, y_center, radius, SDL_MapRGB(surf->format, r, g, b));
        }

        /* Draws a filled circle with the given color, position, and radius. */
        void drawDisc(SDL_Surface* surf, int x_center, int y_center, int radius, Uint32 color);

        /* Draws a filled circle with the given color, position, and radius. */
        void drawDisc(SDL_Surface* surf, int x_center, int y_center, int radius, Uint8 r, Uint8 g, Uint8 b) {
          drawDisc(surf, x_center, y_center, radius, SDL_MapRGB(surf->format, r, g, b));
        }

        /* Draws a line using the given coordinates and color. */
        void drawLine(SDL_Surface* surf, int x0, int y0, int x1, int y1, Uint32 color);

        /* Draws a line using the given coordinates and color. */
        void drawLine(SDL_Surface* surf, int x0, int y0, int x1, int y1, Uint8 r, Uint8 g, Uint8 b) { 
          drawLine(surf, x0, y0, x1, y1, SDL_MapRGB(surf->format, r, g, b));
        }

        /* Using the given integer ID, gets the resulting color. */
        Uint32 getColorFromID(SDL_Surface* surf, int id);

        /* Displays the robot's bounding box. */
        void drawBoundingBox();

        /* Displays any map goals. */
        void drawGoals();

        /* Displays any illuminated switches. */
        void drawSwitches();

        /* Displays the robot's radars. */
        void drawRadars();

        /* Displays the robot's bumpers. */
        void drawBumpers();

        /* Displays the robot's lasers (range-finders). */
        void drawLasers();

        /* Displays the robot's light sensors. */
        void drawLightSensors();

        /* Displays the robot's tile sensors. */
        void drawTileSensors();

        /* Displays the robot's camera. */
        void drawCamera();

        /* Draws the tile grid. */
        void drawGrid();

        /* Draws the tiles explored by the robot. */
        void drawTiles();
      #endif
  };
}
#endif /* !SDL_HH_ */
