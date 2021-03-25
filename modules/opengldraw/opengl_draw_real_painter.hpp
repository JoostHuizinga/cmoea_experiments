/*
 * opengl_draw_real_painter.hpp
 *
 *  Created on: Apr 11, 2017
 *      Author: Joost Huizinga
 */

#ifndef MODULES_OPENGLDRAW_OPENGL_DRAW_REAL_PAINTER_HPP_
#define MODULES_OPENGLDRAW_OPENGL_DRAW_REAL_PAINTER_HPP_

#include <vector>

#ifdef _WIN32//for glut.h
#include <windows.h>
#endif

//think different
#if defined(__APPLE__) && !defined (VMDMESA)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#if BT_USE_FREEGLUT
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif
#else


#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif //_WINDOWS
#endif //APPLE

//#if defined(__APPLE__) && !defined (VMDMESA)
//#include "OpenGL/GlutStuff.h"
//#else// defined(__APPLE__) && !defined (VMDMESA)
//#include "bullet/OpenGL/GlutStuff.h"
//#endif

#if defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btAlignedObjectArray.h"
#else// defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btAlignedObjectArray.h"
#endif

#include "opengl_draw_color.hpp"
#include "opengl_draw_material.hpp"
#include "Light.h"

#define OGLFT_NO_SOLID 1
#define OGLFT_NO_QT 1
#include <modules/oglft/OGLFT.h>

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#define _USE_MATH_DEFINES
#include <math.h>



namespace opengl_draw{
/**
 * This class is responsible for painting and implements my own transformations.
 */
class Painter{

public:

	/**
	 * Default constructor to create a painter instance.
	 */
	Painter(){
		currentMaterial= 0;
		no_mat[0] = 0.0;
		no_mat[1] = 0.0;
		no_mat[2] = 0.0;
		no_mat[3] = 1.0;
		_lineWidth = 1.0;
	}

	virtual ~Painter(){

	}

	/**
	 * Sets the current color.
	 */
	void setColor(Color color){
//		glColor3f(color.r(),color.g(),color.b());
		glColor4f(color.r(),color.g(),color.b(),color.a());
	}

	void setMaterial(Material* material){
//		if(currentMaterial != material && material){
			currentMaterial = material;
//			std::cout << "Ambient: " << currentMaterial->getAmbient() <<std::endl;
//			std::cout << "Diffuse: " << currentMaterial->getDiffuse() <<std::endl;
//			std::cout << "Specular: " << currentMaterial->getSpecular() <<std::endl;

            glMaterialfv(GL_FRONT, GL_AMBIENT, currentMaterial->getAmbient().getVector());
            glMaterialfv(GL_FRONT, GL_DIFFUSE, currentMaterial->getDiffuse().getVector());
            if(currentMaterial->getIlluminationMode() == 2){
                glMaterialfv(GL_FRONT, GL_SPECULAR, currentMaterial->getSpecular().getVector());
            } else {
                glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
            }
            glMaterialf(GL_FRONT, GL_SHININESS, currentMaterial->getShininess());
            glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
        
//            glColor4fv(currentMaterial->getDiffuse().getVector());
//		}
	}

	void beginLine(){
		glBegin(GL_LINES);
	}

	void beginLineLoop(){
		glBegin(GL_LINE_LOOP);
	}

	void beginLineStrip(){
		glBegin(GL_LINE_STRIP);
	}

	void beginTriangleStrip(){
		glBegin(GL_TRIANGLE_STRIP);
	}

	void beginTriangleFan(){
		glBegin(GL_TRIANGLE_FAN);
	}

	void beginPolygon(){
		glBegin(GL_POLYGON);
	}

	void end(){
		glEnd();
	}

	/**
	 * Draws a filled polygon.
	 */
	void drawFilledPolygon(std::vector<btVector3> points) {
		glBegin(GL_POLYGON);
		for(size_t i=0; i<points.size(); i++){
			addPoint(points[i]);
		}
		glEnd();
	}

	/**
	 * Draws only the lines of a polygon.
	 */
	void drawEmptyPolygon(std::vector<btVector3> points) {
		glBegin(GL_LINE_LOOP);
		for(size_t i=0; i<points.size(); i++){
			addPoint(points[i]);
		}
		glEnd();
	}

	void loadIdentity(){
		glLoadIdentity();
	}

	void clearDepth(){
		glClear(GL_DEPTH_BUFFER_BIT);
	}


	void push2D(){
	    glMatrixMode(GL_PROJECTION);
	    glPushMatrix();
	    glLoadIdentity();
	    int w = glutGet( GLUT_WINDOW_WIDTH );
	    int h = glutGet( GLUT_WINDOW_HEIGHT );
	    glOrtho( 0, w, 0, h, -1, 1 );

		pushTransformation();
	    glLoadIdentity();
	    glDisable( GL_DEPTH_TEST );
	    glDisable( GL_LIGHTING );
	}

	void pop2D(){
	    glEnable( GL_LIGHTING );
	    glEnable (GL_DEPTH_TEST);

	    popTransformation();

	    glMatrixMode(GL_PROJECTION);
	    glPopMatrix();
	}

	void createFont(){
	    fonts.push_back(new OGLFT::Filled("/Library/Fonts/HelveticaCY.dfont", 36, 200, 2 ));
	    fonts.back()->setHorizontalJustification( OGLFT::Face::CENTER );
	    fonts.back()->setForegroundColor( 0.2, 0.2, 0.2, 1. );
	}

	void drawString(std::string string, GLint x, GLint y)
	{
		push2D();
	    glRasterPos2i(x, y);
	    void *font = GLUT_BITMAP_HELVETICA_18;

	    for (size_t i=0; i<string.size(); i++){
	    	glutBitmapCharacter(font, int(string[i]));
	    }

	    pop2D();
	}

	/**
	 * Pushes a point into the pipeline, either through openGl transformations, or our own transformations.
	 */
	void addPoint(const btVector3& point){
		glVertex3f(point.x(), point.y(), point.z());
	}


	/**
	 * Pushes a point into the pipeline
	 */
	virtual void addNormal(const btVector3& vector){
		glNormal3f(vector.x(), vector.y(), vector.z());
	}

	/**
	 * Alters the translation of the current transformation matrix.
	 */
	inline void translate(btScalar dx, btScalar dy, btScalar dz){
		glMatrixMode (GL_MODELVIEW);
//		std::cout << "Translation: " << dx << " " << dy << " " << dz << std::endl;
		glTranslated (dx, dy, dz);
	}

	/**
	 * Alters the translation of the current transformation matrix.
	 */
	void translate(btVector3 translation_vector){
		glMatrixMode (GL_MODELVIEW);
		translate(translation_vector.x(), translation_vector.y(), translation_vector.z());
//		glTranslated (translation.x(), translation.y(), translation.z());
	}

	/**
	 * Alters the rotation of the current transformation matrix.
	 */
	inline void rotate(btScalar angle, btScalar ux, btScalar uy, btScalar uz){
		glMatrixMode (GL_MODELVIEW);
//		std::cout << "Rotation: " << angle <<  " "<< ux << " " << uy << " " << uz << std::endl;
		glRotated (angle, ux, uy, uz);
	}

	/**
	 * Alters the rotation of the current transformation matrix.
	 */
	void rotate(btVector4 rotation_vector){
		rotate (rotation_vector.w(), rotation_vector.x(), rotation_vector.y(), rotation_vector.z());
	}

	/**
	 * Alters the rotation of the current transformation matrix.
	 */
	void rotate(btQuaternion rot_q){
		btVector3 v = rot_q.getAxis();
		rotate ((rot_q.getAngle()/(2*M_PI))*360, v.x(), v.y(), v.z());
	}

	/**
	 * Alters the scale of the current transformation matrix.
	 */
	inline void scale(btScalar sx, btScalar sy, btScalar sz){
		glMatrixMode (GL_MODELVIEW);
//		std::cout << "Scale: " << sx << " " << sy << " " << sz << std::endl;
		glScaled(sx, sy, sz);
	}

	/**
	 * Alters the scale of the current transformation matrix.
	 */
	void scale(btVector3 scale_vector){
		glMatrixMode (GL_MODELVIEW);
		scale(scale_vector.x(), scale_vector.y(), scale_vector.z());
//		glScaled(scale.x(), scale.y(),scale.z());
	}

	void mult(btScalar m[16]){
#ifdef BT_USE_DOUBLE_PRECISION
        float f[16];
        for(int i=0; i<16; ++i){
            f[i] = m[i];
        }
        glMultMatrixf(f);
#else
		glMultMatrixf(m);
#endif
	}

	/**
	 * Clears all stacks and transformations.
	 */
	void reset(){
		resetOpenGl();
	}

	/**
	 * Pushes a new transformation matrix on the stack that is a copy of the previous one.
	 */
	void pushTransformation(){
		glMatrixMode (GL_MODELVIEW);
		glPushMatrix();

		GLint depth2;
		glGetIntegerv (GL_MODELVIEW_STACK_DEPTH, &depth2);
	}

	/**
	 * Pops the current transformation matrix from the stack.
	 */
	void popTransformation(){

		glMatrixMode (GL_MODELVIEW);
		GLint depth;
		glGetIntegerv (GL_MODELVIEW_STACK_DEPTH, &depth);
		if(depth > 1)
			glPopMatrix();

		GLint depth2;
		glGetIntegerv (GL_MODELVIEW_STACK_DEPTH, &depth2);
	}

	void enableLighting(){
		glEnable(GL_LIGHTING);
	}

	void disableLighting(){
		glDisable(GL_LIGHTING);
	}

	void setLineWidth(GLfloat width = 1.0f){
	    _lineWidth = width;
		glLineWidth(width);
	}

	GLfloat getLineWidth() const{
	    return _lineWidth;
	}


	void enableTransparancy(){
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

    void drawLight(Light* light){
        if(light->isDirty()){
//            std::cout << "Enabling light " << std::endl;
            size_t id = light->getId();
            glLightfv(GL_LIGHT0 + id, GL_AMBIENT, light->getAmbient().getVector());
            glLightfv(GL_LIGHT0 + id, GL_DIFFUSE, light->getDiffuse().getVector());
            glLightfv(GL_LIGHT0 + id, GL_SPECULAR, light->getSpecular().getVector());
            
            if(light->isSpotLight()){
                glLightfv(GL_LIGHT0 + id, GL_SPOT_DIRECTION, light->getSpotDirection().getArray());
                glLightf(GL_LIGHT0 + id, GL_SPOT_EXPONENT, light->getExponent());
                glLightf(GL_LIGHT0 + id, GL_SPOT_CUTOFF, light->getCutoff());
            }
            
//            glLightf(GL_LIGHT0 + id, GL_CONSTANT_ATTENUATION, light->getConstantAttenuation());
//            glLightf(GL_LIGHT0 + id, GL_LINEAR_ATTENUATION, light->getLinearAttenuation());
//            glLightf(GL_LIGHT0 + id, GL_QUADRATIC_ATTENUATION, light->getQuadraticAttenuation());
            
            glLightfv(GL_LIGHT0 + id, GL_POSITION, light->getPosition().getArray());
            
            if(light->isEnabled()){
                glEnable(GL_LIGHT0 + id);
            } else {
                glDisable(GL_LIGHT0 + id);
            }
        }
        light->setDirty(false);
    }

private:
	Material* currentMaterial;
	GLfloat no_mat[4];
	GLfloat _lineWidth;
	std::vector<OGLFT::Face*> fonts;
	/**
	 * Resets the openGl Model View stack and loads the identity matrix.
	 */
	void resetOpenGl(){
		glMatrixMode (GL_MODELVIEW);
		GLint depth;
		glGetIntegerv (GL_MODELVIEW_STACK_DEPTH, &depth);

		for(int i=1; i< depth;i++){
			glPopMatrix();
		}

		glLoadIdentity();
	}

};
}

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#endif
#endif
//#pragma GCC diagnostic pop



#endif /* MODULES_OPENGLDRAW_OPENGL_DRAW_REAL_PAINTER_HPP_ */
