/*
 * Window.h
 *
 *  Created on: Sep 13, 2013
 *      Author: joost
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <list>
#include <string>

#include "opengl_draw_painter.hpp"
#include "World.h"
#include "Camera.h"

namespace opengl_draw{
/**
 * This class is responsible for the window, the viewport, and currently the camera.
 * It is also the root of all drawable objects.
 */
class Window {
public:

	/**
	 * Creates a window with the indicated size and height.
	 * Notice that this size and height will also define the aspect ratio of the viewport when the window gets resize.
	 */
	Window(int xpos, int ypos, int width, int height, std::string name = "a");

	//Set this window as the global window and enter the glutMainLoop.
	void run();

	//Initial draw.
	void draw(void);

	//On resize event
	void onResize(size_t x, size_t y);

	/**
	 * Enables mouse clicks
	 */
	void enableMouseClick();

	/**
	 * Triggers on a mouse-click event if mouse-click is enabled
	 */
	void onMouseClick(int button, int state, int x, int y);

	/**
	 * Enables mouse drag
	 */
	void enableMouseDrag();

	/**
	 * Triggers on a mouse-drag event if mouse-drag is enabled
	 */
	void onMouseDrag(int x, int y);

	/**
	 * Enables menus
	 */
	void enableMenu();

	/**
	 * Triggers on a menu event if menu is enabled
	 */
	void onMenuEvent(int option);

	/**
	 * Enables regular key-presses
	 */
	void enableKeyPress();

	/**
	 * Triggers on a key-press event if key-press is enabled
	 */
	void onKeyPressEvent(unsigned char key, int x, int y);

	/**
	 * Enables key-presses for 'special' keys
	 */
	void enableSpecialPress();

	/**
	 * Triggers on a special key-press if special key-presses are enabled
	 */
	void onSpecialPressEvent(int key, int x, int y);

    /**
     * Returns the camera associated with this window.
     */
    Camera* getCamera(){
        return _camera;
    }
    
    void setCamera(Camera* camera){
        _camera = camera;
    }

	/**
	 * Returns the painter associated with this window.
	 */
	Painter* getPainter(){
		return _painter;
	}

	void setPainter(Painter* painter){
		_painter = painter;
	}

	World* getWorld(){
		return _world;
	}

	void setWorld(World* world){
		_world = world;
	}

	void refresh();

protected:
	int width;
	int height;
	int viewportWidth;
	int viewportHeight;

	static const double slide;

	Painter* _painter;
    Camera* _camera;
	World* _world;
}   ;
}

#endif /* WINDOW_H_ */
