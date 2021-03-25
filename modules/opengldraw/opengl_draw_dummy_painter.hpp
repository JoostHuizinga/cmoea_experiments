/*
 * opengl_draw_dummy_painter.hpp
 *
 *  Created on: Apr 11, 2017
 *      Author: Joost Huizinga
 */

#ifndef MODULES_OPENGLDRAW_OPENGL_DRAW_DUMMY_PAINTER_HPP_
#define MODULES_OPENGLDRAW_OPENGL_DRAW_DUMMY_PAINTER_HPP_

// Standard includes
#include <vector>

// Bullet includes
#if defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btAlignedObjectArray.h"
#else// defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
#include "LinearMath/btAlignedObjectArray.h"
#endif

// Local includes
#include "opengl_draw_color.hpp"
#include "opengl_draw_material.hpp"

namespace opengl_draw{
/**
 * This class is responsible for painting and implements my own transformations.
 */
class Painter{

public:

	/**
	 * Default constructor to create a painter instance.
	 */
	Painter(){}

	virtual ~Painter(){}

	/**
	 * Sets the current color.
	 */
	void setColor(Color color){}

	void setMaterial(Material* material){}

	void beginLine(){}

	void beginLineLoop(){}

	void beginLineStrip(){}

	void beginTriangleStrip(){}

	void beginTriangleFan(){}

	void beginPolygon(){}

	void end(){}

	/**
	 * Draws a filled polygon.
	 */
	void drawFilledPolygon(std::vector<btVector3> points) {}

	/**
	 * Draws only the lines of a polygon.
	 */
	void drawEmptyPolygon(std::vector<btVector3> points) {}

	void loadIdentity(){}

	void clearDepth(){}

	void push2D(){}

	void pop2D(){}

	void createFont(){}

	void drawString(std::string string, int x, int y){}

	/**
	 * Pushes a point into the pipeline, either through openGl transformations, or our own transformations.
	 */
	void addPoint(const btVector3& point){}

	/**
	 * Pushes a point into the pipeline
	 */
	virtual void addNormal(const btVector3& vector){}

	/**
	 * Alters the translation of the current transformation matrix.
	 */
	inline void translate(btScalar dx, btScalar dy, btScalar dz){}

	/**
	 * Alters the translation of the current transformation matrix.
	 */
	void translate(btVector3 translation_vector){}

	/**
	 * Alters the rotation of the current transformation matrix.
	 */
	inline void rotate(btScalar angle, btScalar ux, btScalar uy, btScalar uz){}

	/**
	 * Alters the rotation of the current transformation matrix.
	 */
	void rotate(btVector4 rotation_vector){}

	/**
	 * Alters the scale of the current transformation matrix.
	 */
	inline void scale(btScalar sx, btScalar sy, btScalar sz){}

	/**
	 * Alters the scale of the current transformation matrix.
	 */
	void scale(btVector3 scale_vector){}

	/**
	 * Clears all stacks and transformations.
	 */
	void reset(){}

	/**
	 * Pushes a new transformation matrix on the stack that is a copy of the previous one.
	 */
	void pushTransformation(){}

	/**
	 * Pops the current transformation matrix from the stack.
	 */
	void popTransformation(){}

	void enableLighting(){}

	void disableLighting(){}

	void setLineWidth(float width = 1.0f){}

	float getLineWidth() const{return 0;}

	void enableTransparancy(){}



};
}


#endif /* MODULES_OPENGLDRAW_OPENGL_DRAW_DUMMY_PAINTER_HPP_ */
