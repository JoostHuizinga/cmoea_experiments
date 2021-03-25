/*
 * Window.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: joost
 */


#if defined(__APPLE__) && !defined (VMDMESA)
#include <GLUT/glut.h>
#else // defined(__APPLE__) && !defined (VMDMESA)

#include <glut.h>
#include <GL/glut.h>
#endif// defined(__APPLE__) && !defined (VMDMESA)

#include <iostream>
#include <fstream>

#include "Window.h"
#include "pngwriter.h"

using namespace opengl_draw;

//Constants
const double Window::slide = 0.2;


//Globally defined functions to pass to glut.

//The global window pointer used to access the window object from the global functions.
Window* globalWindow;

//The draw callback function
void globalDraw(void){
	globalWindow->draw();
}

//The onMouseClick callback function
void globalOnMouseClick(int button, int state, int x, int y){
	globalWindow->onMouseClick(button, state, x, y);
}

//The onMouseDrag callback function
void globalOnMouseDrag(int x, int y){
	globalWindow->onMouseDrag(x, y);
}

//The onResize callback function
void globalOnResize(GLsizei x, GLsizei y){
	globalWindow->onResize(x, y);
}

//The onMenuEvent callback function
void globalOnMenuEvent(int event){
	globalWindow->onMenuEvent(event);
}

//The onMenuEvent callback function
void globalOnKeyPressEvent(unsigned char key, int x, int y){
	globalWindow->onKeyPressEvent(key, x, y);
}

//The onMenuEvent callback function
void globalOnSpecialKeyPressEvent(int key, int x, int y){
	globalWindow->onSpecialPressEvent(key, x, y);
}


Window::Window(int xpos, int ypos, int width, int height, std::string name):
			width(width),height(height){
	_world = 0;
	_painter = 0;

	viewportWidth = width;
	viewportHeight = height;

	glutInitWindowSize(width,height); // set pixel size of window
	glutInitWindowPosition(xpos,ypos); //set position of top left corner of the window
	glutCreateWindow(name.c_str());

}

void Window::run(){
	globalWindow = this;
	if(_painter && _world){
		glutDisplayFunc(globalDraw);
		glutReshapeFunc(globalOnResize);
		glutMainLoop();
	}
}

void Window::draw(void){
//    _painter->paint();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _world->draw(_painter);
    glFlush();
}

void Window::onResize(size_t x, size_t y){
	width = x;
	height = y;

    if(x*getCamera()->getHeight() > y*getCamera()->getWidth()){
//        glViewport(0, 0, y*getCamera()->getWidth()/getCamera()->getHeight(), y);
        viewportWidth = y*getCamera()->getWidth()/getCamera()->getHeight();
        viewportHeight =y;
    } else if(x*getCamera()->getHeight() < y*getCamera()->getWidth()){
//        glViewport(0, 0, x, x*getCamera()->getHeight()/getCamera()->getWidth());
        viewportWidth = x;
        viewportHeight = x*getCamera()->getHeight()/getCamera()->getWidth();
    } else{
//        glViewport(0, 0, x, y);
        viewportWidth = x;
        viewportHeight = y;
    }
    std::cout << viewportWidth << " " << viewportHeight << std::endl;
    glViewport(0, 0, viewportWidth, viewportHeight);

//    _painter->setScreenSize(viewportWidth, viewportHeight);
}

void Window::enableMouseClick(){
	glutMouseFunc(globalOnMouseClick);
}

void Window::onMouseClick(int button, int state, int x, int y){
	//No mouse click handler needed for this assignment
}

void Window::enableMouseDrag(){
	glutMotionFunc(globalOnMouseDrag);
}

void Window::onMouseDrag(int x, int y){
	//No mouse drag handler needed for this assignment
}

void Window::enableMenu(){
	glutCreateMenu(globalOnMenuEvent);
}

void Window::onMenuEvent(int event){
	//No menu handler needed for this assignment
}

void Window::enableKeyPress(){
	glutKeyboardFunc(globalOnKeyPressEvent);
}

void Window::onKeyPressEvent(unsigned char key, int x, int y){
	Vector3 lookAtVector;
	switch(key){
	case 's':
            std::cout << "Pressed s" << std::endl;
//        if(_painter->shadowsEnabled()){
//            _painter->drawText("Shadows off - reverting to original directional light");
//            _world->getLights().back()->setPosition(Array3(-400.0, 692.0, 0.0));
//        } else {
//            _painter->drawText("Shadows on - changing directional light");
//            _world->getLights().back()->setPosition(Array3(1.0, 0.0, 1.0));
//        }
//        _painter->flush();
//        _painter->toggleShadows();

		refresh();
		break;
	case 'q':
		exit(0);
		break;
	case '>':
	case '.':
//        _painter->drawText("Increasing resolution");
//        _painter->flush();
//        _painter->setResolution(_painter->getXResolution()*2, _painter->getYResolution()*2);
		refresh();
		break;
	case '<':
	case ',':
//        _painter->drawText("Decreasing resolution");
//        _painter->flush();
//        _painter->setResolution(_painter->getXResolution()/2, _painter->getYResolution()/2);
		refresh();
		break;
	case '?':
	case '/':
//        _painter->drawText("Setting perfect resolution");
//        _painter->flush();
//        _painter->setResolution(viewportWidth, viewportHeight);
		refresh();
		break;
	case '_':
	case '-':
//        _painter->drawText("Max depth reduced");
//        _painter->flush();
//        _painter->decrementRecusionDepth();
		refresh();
		break;
	case '+':
	case '=':
//        _painter->drawText("Max depth incremented");
//        _painter->flush();
//        _painter->incrementRecusionDepth();
		refresh();
		break;
	case 'r':
	case 'R':
//        if(_painter->reflectionsEnabled()){
//            _painter->drawText("Reflections off");
//        } else {
//            _painter->drawText("Reflections on");
//        }
//        _painter->flush();
//        _painter->toggleReflections();
		refresh();
		break;
	case 't':
	case 'T':
        {
            int width = viewportWidth;
            int height = viewportHeight;
            
            
            int nPixels = width * height * 3;
            GLfloat* image = new GLfloat[nPixels];
//            display();
            glReadPixels(0, 0, width, height,GL_RGB, GL_FLOAT, image);
            
            pngwriter PNG(width, height, 1.0, "test.png");
            std::cout << "test.png written" << std::endl;
            size_t x = 1;   // start the top and leftmost point of the window
            size_t y = 1;
            double R, G, B;
            for(size_t i=0; i<nPixels; i++){
                switch(i%3){
                    case 2:{
                        B = (double) image[i];
                        PNG.plot(x, y, R, G, B);
                        if( x == width ){
                            x=1;
                            y++;
                        }
                        else{
                            x++;
                        }
                        break;
                    }
                    case 1:{
                        G = (double) image[i];
                        break;
                    }
                    case 0:{
                        R = (double) image[i];
                        break;
                    }
                }
            }
            PNG.close();
            
            delete[] image;
            refresh();
            break;
        }
	case 'y':
	case 'Y':
//        if(_painter->texturesEnabled()){
//            _painter->drawText("Textures off");
//        } else {
//            _painter->drawText("Textures on");
//        }
//        _painter->flush();
//        _painter->toggleTextures();
		refresh();
		break;
	}
}

void Window::enableSpecialPress(){
	glutSpecialFunc(globalOnSpecialKeyPressEvent);
}

void Window::onSpecialPressEvent(int key, int x, int y){
	Vector3 lookAtVector;
	switch(key){
	case GLUT_KEY_UP:
//        _painter->drawText("Moving forward");
//        _painter->flush();
		getCamera()->slide(0.0, 0.0, -slide);
		refresh();
		break;
	case GLUT_KEY_DOWN:
//        _painter->drawText("Moving backward");
//        _painter->flush();
		getCamera()->slide(0.0, 0.0, slide);
		refresh();
		break;
	case GLUT_KEY_LEFT:
//        _painter->drawText("Moving left");
//        _painter->flush();
		getCamera()->slide(-slide, 0.0, 0.0);
		refresh();
		break;
	case GLUT_KEY_RIGHT:
//        _painter->drawText("Moving right");
//        _painter->flush();
		getCamera()->slide(slide, 0.0, 0.0);
		refresh();
		break;
	case GLUT_KEY_F1:
	case GLUT_KEY_PAGE_UP:
//        _painter->drawText("Moving up");
//        _painter->flush();
		getCamera()->slide(0.0, slide, 0.0);
		refresh();
		break;
	case GLUT_KEY_F2:
	case GLUT_KEY_PAGE_DOWN:
//        _painter->drawText("Moving down");
//        _painter->flush();
		getCamera()->slide(0.0, -slide, 0.0);
		refresh();
		break;
	}
}

void Window::refresh(){
	glutPostRedisplay();
}
