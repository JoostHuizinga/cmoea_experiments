#include "display.hpp"
#include <bitset>
// #include "SDL_timer.h"

#ifdef USE_SDL
  namespace fastsim {
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////* Constructors/Destructors *////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    /*
     * Creates the Display.
     * A framerate must be specified; the display will attempt to render at this number of frames per second.
     */
    Display::Display(const boost::shared_ptr<Map>& m, Robot& r, float frameRate) :
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
      fps(1000.0f / frameRate),
      map(m),
      robot(r),
      dCamera(r.isUsingCamera())
    {
      /* Create the screen. */
      width = map->getWidth();
      height = map->getHeight();
      if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        std::cout << "Error: could not initialize SDL!\n";
        exit(1);
      }
      screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
      if (!screen) {
        std::cout << "Error: could not initialize Display!\n";
        exit(1);
      }

      /* Set up color information. */
      Uint32 rmask, gmask, bmask, amask;
      #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
      #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
      #endif

      /* Create the map. */
      mapBMP = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    }

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////* Functions *///////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////

    /* Updates the display (sensors, robot position, etc.). */
    void Display::update()
    {
      /* Initialize the framerate capper. */
      if (startRender == 0.0f) { startRender = SDL_GetTicks(); }
      frameCount++;

//      std::cout << "Processing events" << std::endl;
      events(); //process SDL events

      /* Get robot coordinates. */
      unsigned x = map->realToPixel(robot.getPosture().getX());
      unsigned y = map->realToPixel(robot.getPosture().getY());
      unsigned r = map->realToPixel(robot.getRadius());
      float theta = robot.getPosture().getTheta();
      
      /* Erase everything. */
      SDL_BlitSurface(mapBMP, 0, screen, 0);

      /* Redraw the map */
//      std::cout << "Redrawing map" << std::endl;
      putAllPixels();
      if(dRobot){
          //      std::cout << "Drawing grid" << std::endl;
          if (dGrid) { drawGrid(); }
          //      std::cout << "Drawing tiles" << std::endl;
          if (dTiles) { drawTiles(); }

          /* Display sensors, switches, and goals. */
          //      std::cout << "Drawing sensors" << std::endl;
          if (dTileSensors) { drawTileSensors(); }
          //      std::cout << "Drawing goals" << std::endl;
          if (dGoals) { drawGoals(); }
          //      std::cout << "Drawing switches" << std::endl;
          if (dSwitches) { drawSwitches(); }
          //      std::cout << "Drawing radars" << std::endl;
          if (dRadars) { drawRadars(); }
          //      std::cout << "Drawing lasers" << std::endl;
          if (dLasers) { drawLasers(); }
          //      std::cout << "Drawing light sensors" << std::endl;
          if (dLightSensors) { drawLightSensors(); }
          //      std::cout << "Drawing camera" << std::endl;
          if (dCamera) { drawCamera(); }

          /* Redraw robot. */
          //      std::cout << "Drawing robot" << std::endl;
          unsigned int robotColor = robot.getColor();
          drawDisc(screen, x, y, r, getColorFromID(screen, robotColor));
          drawCircle(screen, x, y, r, 255, 0, 0);
      }

      /* Draw robot's heading. */
      Uint32 headingColor = SDL_MapRGB(screen->format, 0, 255, 0);
      drawLine(screen, x, y, (int) (r * cosf(theta) + x), (int) (r * sinf(theta) + y), headingColor);

      /* Draw robot's bumpers. */
//      std::cout << "Drawing bumpers" << std::endl;
      drawBumpers();
      
      /* Draw bounding box. */
      //drawBoundingBox();

      /* Update screen. */
      SDL_UpdateRect(screen, 0, 0, width, height);
      SDL_UpdateRect(mapBMP, 0, 0, width, height);

      /* Perform basic framerate capping (a quick & dirty algorithm; could be improved). */
      Uint32 targetTime = startRender + frameCount * fps; //the time the frame should have taken to render
      Uint32 actualTime = SDL_GetTicks(); //the time the frame actually took the render
      /* CASE 1: rendered faster than expected; delay for the remaining time. */
      if (actualTime <= targetTime) {
        SDL_Delay(targetTime- actualTime); //note: SDL_Delay is not guaranteed to wait for this long, but is a decent approximation on most machines
      /* CASE 2: rendered slower than expected; restart the frameCount and render timer. */
      } else {
        frameCount = 0;
        startRender = SDL_GetTicks();
      }
    }

    /* Processes SDL events on this display. */
    void Display::events() {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
  	     switch (event.type) {
  	       case SDL_QUIT: //close the display
  	         quit();
             break;
  	       case SDL_KEYDOWN: //pressed the escape key; close the display
  	         if (event.key.keysym.sym == SDLK_ESCAPE) { quit(); }
  	         break;
  	     }
      }
    }

    /*
     * Places all pixels in the map onto the display.
     * Called whenever the map topology changes.
     */
    void Display::putAllPixels()
    {
      for (unsigned i = 0; i < map->getWidth(); i++) {
        for (unsigned j = 0; j < map->getHeight(); j++) {
        	if (map->getPixel(i, j) == Map::collision) {
        	  putPixel(screen, i, j, 0, 0, 0);
          } else if (map->getPixel(i, j) == Map::noCollision) {
        	  putPixel(screen, i, j, 255, 255, 255);
          } else {
            putPixel(screen, getColorFromID(screen, map->getPixel(i, j)), i, j);
          }
        }
      }
    }

    /* Draws the points of an ellipse, with the given center and x-radius and y-radius. */
    void Display::drawCirclePoints(SDL_Surface* surf, int cx, int cy, int x, int y, Uint32 color) {
      if (x == 0) {
        putPixel(surf, color, cx, cy + y);
        putPixel(surf, color, cx, cy - y);
        putPixel(surf, color, cx + y, cy);
        putPixel(surf, color, cx - y, cy);
      } else if (x == y) {
        putPixel(surf, color, cx + x, cy + y);
        putPixel(surf, color, cx - x, cy + y);
        putPixel(surf, color, cx + x, cy - y);
        putPixel(surf, color, cx - x, cy - y);
      } else if (x < y) {
        putPixel(surf, color, cx + x, cy + y);
        putPixel(surf, color, cx - x, cy + y);
        putPixel(surf, color, cx + x, cy - y);
        putPixel(surf, color, cx - x, cy - y);
        putPixel(surf, color, cx + y, cy + x);
        putPixel(surf, color, cx - y, cy + x);
        putPixel(surf, color, cx + y, cy - x);
        putPixel(surf, color, cx - y, cy - x);
      }
    }

    /* Draws a circle with the given color, position, and radius. */
    void Display::drawCircle(SDL_Surface *surf, int x_center, int y_center, int radius, Uint32 color) {    
      int x = 0;
      int y = radius;
      int p = (5 - radius * 4) / 4;
      drawCirclePoints(surf, x_center, y_center, x, y, color);
      while (x < y) {
        x++;
        if (p < 0) {
          p += 2 * x + 1;
        } else {
          y--;
          p += 2 * (x - y) + 1;
        }
        drawCirclePoints(surf, x_center, y_center, x, y, color);
      }
    }

    /* Draws a filled circle with the given color, position, and radius. */
    void Display::drawDisc(SDL_Surface *surf, int x_center, int y_center, int radius, Uint32 color)
    {
      int f = 1 - radius;
      int ddF_x = 1;
      int ddF_y = -2 * radius;
      int x = 0;
      int y = radius;
      int x0 = x_center;
      int y0 = y_center;

      drawLine(surf, x0, y0 - radius, x0, y0 + radius, color);
      drawLine(surf, x0 - radius, y0, x0 + radius, y0, color);    
      while(x < y) {
        if(f >= 0) {
          y--;
          ddF_y += 2;
          f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;   
        drawLine(surf, x0 - x, y0 + y, x0 + x, y0 + y, color);
        drawLine(surf, x0 - x, y0 - y, x0 + x, y0 - y, color);
        drawLine(surf, x0 - y, y0 + x, x0 + y, y0 + x, color);
        drawLine(surf, x0 - y, y0 - x, x0 + y, y0 - x, color);
      }
    }

    /* Draws a line using the given coordinates and color. */
    void Display::drawLine(SDL_Surface* surf, int x0, int y0, int x1, int y1, Uint32 color) {
      x0 = std::max(x0, 0); x0 = std::min(x0, surf->w - 1);
      x1 = std::max(x1, 0); x1 = std::min(x1, surf->w - 1);
      y0 = std::max(y0, 0); y0 = std::min(y0, surf->h - 1);
      y1 = std::max(y1, 0); y1 = std::min(y1, surf->h - 1);
      int dy = y1 - y0;
      int dx = x1 - x0;
      int stepx, stepy;

      if (dy < 0) {
        dy = -dy;  stepy = -1;
      } else {
        stepy = 1;
      }
      if (dx < 0) {
        dx = -dx;  stepx = -1;
      } else {
        stepx = 1;
      }
      dy <<= 1; //dy is now 2*dy
      dx <<= 1; //dx is now 2*dx

      putPixel(surf, color, x0, y0);
      if (dx > dy) {
      	int fraction = dy - (dx >> 1); //same as 2*dy - dx
      	while (x0 != x1) {
      	  if (fraction >= 0) {
        		y0 += stepy;
        		fraction -= dx; //-= 2*dx
      	  }
      	  x0 += stepx;
      	  fraction += dy; //-= 2*dy
      	  putPixel(surf, color, x0, y0);
      	}
      } else {
      	int fraction = dx - (dy >> 1);
      	while (y0 != y1) {
      	  if (fraction >= 0) {
        		x0 += stepx;
        		fraction -= dy;
      	  }
      	  y0 += stepy;
      	  fraction += dx;
      	  putPixel(surf, color, x0, y0);
      	}
      }
    }

    /* Using the given integer ID, gets the resulting color. */
    Uint32 Display::getColorFromID(SDL_Surface* surf, int x) {
      std::bitset<4> bs = x;  
      Uint8 k = 1 - bs[4];
      Uint8 r = bs[0] * 128 + 127 * k;
      Uint8 g = bs[1] * 128 + 127 * k;
      Uint8 b = bs[2] * 128 + 127 * k;
      return SDL_MapRGB(surf->format, r, g, b);
    }

    /* Displays the robot's bounding box. */
    void Display::drawBoundingBox() {
      unsigned x = map->realToPixel(robot.getBoundingBox().x);
      unsigned y = map->realToPixel(robot.getBoundingBox().y);
      unsigned w = map->realToPixel(robot.getBoundingBox().w);
      unsigned h = map->realToPixel(robot.getBoundingBox().h);
      
      assert(x >= 0);
      assert(y >= 0);
      assert(x + w < screen->w);
      assert(y + h < screen->h);

      drawLine(screen, x, y, x + w, y, 0);
      drawLine(screen, x + w, y, x + w, y + h, 0);
      drawLine(screen, x + w, y + h, x, y + h, 0);
      drawLine(screen, x, y + h, x, y, 0);    
    }

    /* Displays any map goals. */
    void Display::drawGoals() {
      for (size_t i = 0; i < map->getGoals().size(); i++) {
      	Goal& goal = map->getGoals()[i];
      	unsigned x = map->realToPixel(goal.getX());
      	unsigned y = map->realToPixel(goal.getY());
      	unsigned diam = map->realToPixel(goal.getDiameter());
      	Uint8 r = 0, g = 0, b = 0;
      	switch (goal.getColor()) {
      	  case 0:
      	    r = 255;
      	    break;
      	  case 1:
      	    g = 255;
      	    break;
      	  case 2:
      	    b = 255;
      	    break;
      	  default:
      	    assert(0);
      	}
      	drawCircle(screen, x, y, diam, r, g, b);
      }    
    }

    /* Displays any illuminated switches. */
    void Display::drawSwitches() {
      for (size_t i = 0; i < map->getIlluminatedSwitches().size(); i++) {
        IlluminatedSwitch& sw = *map->getIlluminatedSwitches()[i];
        if (!sw.isVisible()) { continue; }
        unsigned x = map->realToPixel(sw.getX());
        unsigned y = map->realToPixel(sw.getY());
        unsigned rad = map->realToPixel(sw.getRadius());
        Uint32 color = getColorFromID(screen, sw.getColor());

        drawCircle(screen, x, y, rad, color);
        if (sw.isOn()) {
          drawDisc(screen, x, y, rad, color);
        }
      }
    }

    /* Displays the robot's radars. */
    void Display::drawRadars() {
      unsigned r = map->realToPixel(robot.getRadius())  / 2;
      unsigned x = map->realToPixel(robot.getPosture().getX());
      unsigned y = map->realToPixel(robot.getPosture().getY());
      for (size_t i = 0; i < robot.getRadars().size(); i++) {
      	Radar& radar = robot.getRadars()[i];
      	if (radar.getActivatedSlice() != -1) {
      	  float a1 = robot.getPosture().getTheta() + radar.getSliceSize() * radar.getActivatedSlice();
      	  float a2 = robot.getPosture().getTheta() + radar.getSliceSize() * (radar.getActivatedSlice() + 1);
      	  drawLine(screen, cos(a1) * r + x, sin(a1) * r + y, cos(a2) * r + x, sin(a2) * r + y, 0x0000FF);
      	  Goal& g = map->getGoals()[radar.getColor()];
      	  unsigned gx = map->realToPixel(g.getX());
      	  unsigned gy = map->realToPixel(g.getY());	    
      	  drawLine(screen, x, y, gx, gy, 0x0000FF);
      	}
      }   
    }

    /* Displays the robot's bumpers. */
    void Display::drawBumpers() {
      unsigned x = map->realToPixel(robot.getPosture().getX());
      unsigned y = map->realToPixel(robot.getPosture().getY());
      unsigned r = map->realToPixel(robot.getRadius());
      float theta = robot.getPosture().getTheta();
      Uint32 cb_left = SDL_MapRGB(screen->format, robot.getLeftBumper() ? 255 : 0, 0, 0);
      Uint32 cb_right = SDL_MapRGB(screen->format, robot.getRightBumper() ? 255 : 0, 0, 0);
      drawLine(screen,
    	  (int) (r * cosf(theta + M_PI / 2.0f) + x),
    	  (int) (r * sinf(theta + M_PI / 2.0f) + y),
    	  (int) (r * cosf(theta) + x),
    	  (int) (r * sinf(theta) + y),
    	  cb_left);
      drawLine(screen,
    	  (int) (r * cosf(theta - M_PI / 2.0f) + x),
    	  (int) (r * sinf(theta - M_PI / 2.0f) + y),
    	  (int) (r * cosf(theta) + x),
    	  (int) (r * sinf(theta) + y),
    	  cb_right);
    }

    /* Displays the robot's lasers (range-finders). */
    void Display::drawLasers() {
      for (size_t i = 0; i < robot.getLasers().size(); i++) {
      	unsigned x_laser = map->realToPixel(robot.getPosture().getX() 
      		+ robot.getLasers()[i].getGapDist() * cosf(robot.getPosture().getTheta() 
      		+ robot.getLasers()[i].getGapAngle()));

      	unsigned y_laser = map->realToPixel(robot.getPosture().getY() 
      		+ robot.getLasers()[i].getGapDist() * sinf(robot.getPosture().getTheta()
      		+ robot.getLasers()[i].getGapAngle()));

      	drawLine(screen, x_laser, y_laser,
      	  robot.getLasers()[i].getXPixel(),
      	  robot.getLasers()[i].getYPixel(),
      	  0xFF00000);
      }
    }

    /* Displays the robot's light sensors. */
    void Display::drawLightSensors() {
      for (size_t i = 0; i < robot.getLightSensors().size(); i++) {
      	LightSensor& ls = robot.getLightSensors()[i];
        if (!ls.isActivated()) { continue; } //don't display light sensors unless they see something

      	unsigned x = map->realToPixel(robot.getPosture().getX());
      	unsigned y = map->realToPixel(robot.getPosture().getY());

        int x1 = map->realToPixel((cosf(robot.getPosture().getTheta() + ls.getAngle() + ls.getAngularRange()) * ls.getRange() / cosf(ls.getAngularRange())) + robot.getPosture().getX());
        int y1 = map->realToPixel((sinf(robot.getPosture().getTheta() + ls.getAngle() + ls.getAngularRange()) * ls.getRange() / cosf(ls.getAngularRange())) + robot.getPosture().getY());

        int x2 = map->realToPixel((cosf(robot.getPosture().getTheta() + ls.getAngle() - ls.getAngularRange()) * ls.getRange() / cosf(ls.getAngularRange())) + robot.getPosture().getX());
        int y2 = map->realToPixel((sinf(robot.getPosture().getTheta() + ls.getAngle() - ls.getAngularRange()) * ls.getRange() / cosf(ls.getAngularRange())) + robot.getPosture().getY());

        drawLine(screen, x, y, x1, y1, getColorFromID(screen, ls.getColor()));
      	drawLine(screen, x, y, x2, y2, getColorFromID(screen, ls.getColor()));
      	drawLine(screen, x1, y1, x2, y2, getColorFromID(screen, ls.getColor()));

      	IlluminatedSwitch& is = *map->getIlluminatedSwitches()[ls.getNum()];
      	unsigned x_is = map->realToPixel(is.getX());
      	unsigned y_is = map->realToPixel(is.getY());
      	drawLine(screen, x, y, x_is, y_is, getColorFromID(screen, is.getColor()));
      }
    }

    /* Displays the robot's tile sensors. */
    void Display::drawTileSensors() {
      /* For each sensor... */
      for (size_t i = 0; i < robot.getTileSensors().size(); i++) {
        TileSensor& ts = robot.getTileSensors()[i];
        /* For each tile sensed by the sensor... */
        for (size_t j = 0; j < ts.getNumTilesSensed(); j++) {
          /* Draw the pixel coordinates of the sensed tile onto the map. */
          int startPixelX = ts.getTilesSensed().at(j).first * map->getGridSize();
          int startPixelY = ts.getTilesSensed().at(j).second * map->getGridSize();
          int endPixelX = (ts.getTilesSensed().at(j).first + 1) * map->getGridSize();
          int endPixelY = (ts.getTilesSensed().at(j).second + 1) * map->getGridSize();
          for (size_t pixelX = startPixelX; pixelX < endPixelX; pixelX++) {
            for (size_t pixelY = startPixelY; pixelY < endPixelY; pixelY++) {
                if(testPixel(screen, pixelX, pixelY, 10, 10, 10)) continue;
                if(i==0){
                    putPixel(screen, pixelX, pixelY, 255, 200, 200, 150);
                }else if(i==1){
                    putPixel(screen, pixelX, pixelY, 200, 255, 200, 150);
                }else if(i==2){
                    putPixel(screen, pixelX, pixelY, 200, 200, 255, 150);
                }else if(i==3){
                    putPixel(screen, pixelX, pixelY, 255, 255, 200, 150);
                }else if(i==4){
                    putPixel(screen, pixelX, pixelY, 255, 200, 255, 150);
                }else if(i==5){
                    putPixel(screen, pixelX, pixelY, 200, 255, 255, 150);
                }else if(i>5){
                    putPixel(screen, pixelX, pixelY, 200, 200, 200, 150);
                }
            }
          }
        }
      }
    }

    /* Displays the robot's camera. */
    void Display::drawCamera() {
      static const int pw = 20;
      unsigned x_ls = map->realToPixel(robot.getPosture().getX());
      unsigned y_ls = map->realToPixel(robot.getPosture().getY());

      float a1 = robot.getPosture().getTheta() + robot.getCamera().getAngularRange() / 2.0;
      drawLine(screen, x_ls, y_ls, cos(a1) * 200 + x_ls, sin(a1) * 200 + y_ls, 0x0000ff);

      float a2 = robot.getPosture().getTheta() - robot.getCamera().getAngularRange() / 2.0;
      drawLine(screen, x_ls, y_ls, cos(a2) * 200 + x_ls,  sin(a2) * 200 + y_ls, 0x0000ff);

      for (size_t i = 0; i < robot.getCamera().getPixels().size(); i++) {
      	int pix = robot.getCamera().getPixels()[i];	
      	Uint32 color = pix == -1 ? 0xffffff : getColorFromID(screen, pix);
      	SDL_Rect r;
        r.x = i * pw;
        r.y = 0;
        r.w = pw;
        r.h = pw;
      	SDL_FillRect(screen, &r, color);
      }
    }

    /* Draws the tile grid. */
    void Display::drawGrid() {
      /* Draw vertical lines */
      for (size_t i = 1; i < map->getWidth(); i++) {
        drawLine(screen, i * map->getGridSize(), 0, i * map->getGridSize(), map->getHeight() - 1, 200, 200, 200);
      }

      /* Draw horizontal lines */
      for (size_t i = 1; i < map->getHeight(); i++) {
        drawLine(screen, 0, i * map->getGridSize(), map->getWidth() - 1, i * map->getGridSize(), 200, 200, 200);
      }
    }

    /* Draws the tiles explored by the robot. */
    void Display::drawTiles() {
      for (size_t i = 0; i < map->getWidth(); i++) {
        for (size_t j = 0; j < map->getHeight(); j++) {
          if (map->isPixelVisited(i, j)) {
            putPixel(screen, i, j, 150, 150, 150, 150);
          }
        }
      }
    }
  }
#endif
