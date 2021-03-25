/*
 * Piramid.h
 *
 *  Created on: Sep 29, 2013
 *      Author: joost
 */

#ifndef PIRAMID_H_
#define PIRAMID_H_

#include "opengl_draw_world_object.hpp"
#include "opengl_draw_face.hpp"
#include <vector>


namespace opengl_draw{
/**
 * A pyramid shaped world object.
 */
class Pyramid: public WorldObject{
	static const int front = 0;
	static const int back = 1;
	static const int left = 2;
	static const int right = 3;
	static const int top = 4;
	static const int nrOfCorners = 5;
public:

	Pyramid(btScalar sideLength = 1):sideLength(sideLength){
		halfSideLength = sideLength/2;
		update();
	}

	void update(){
		points[front] = btVector3(0, 0, 0-halfSideLength);
		points[back] = btVector3(0, 0, 0+halfSideLength);
		points[left] = btVector3(0-halfSideLength, 0, 0);
		points[right] = btVector3(0+halfSideLength, 0, 0);
		points[top] = btVector3(0, 0+halfSideLength, 0);

		Face* newFace = new Face(points[front], points[left], points[top], true);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[right], points[front], points[top], true);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[left], points[back], points[top], true);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[back], points[right], points[top], true);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[left], points[back], points[right], points[front]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);
	}

private:
	btVector3 points[nrOfCorners];


	btScalar sideLength;
	btScalar halfSideLength;
};
}

#endif /* PIRAMID_H_ */
