/*
 * Diamond.h
 *
 *  Created on: Oct 2, 2013
 *      Author: joost
 */

#ifndef DIAMOND_H_
#define DIAMOND_H_


#include "opengl_draw_world_object.hpp"
#include "opengl_draw_face.hpp"
#include <vector>


namespace opengl_draw{
/**
 * A diamond shaped world object.
 */
class Diamond: public WorldObject{
	static const int backright = 0;
	static const int backleft = 1;
	static const int frontleft = 2;
	static const int frontright = 3;
	static const int top = 4;
	static const int bottom = 5;
	static const int nrOfCorners = 6;
public:

	Diamond(btScalar sideLength = 1): sideLength(sideLength){
		halfSideLength = sideLength/2;
		update();
	}

	virtual void update(){
		points[backright] = btVector3(0+halfSideLength, 0, 0-halfSideLength);
		points[backleft] = btVector3(0-halfSideLength, 0, 0-halfSideLength);
		points[frontleft] = btVector3(0-halfSideLength, 0, 0+halfSideLength);
		points[frontright] = btVector3(0+halfSideLength, 0, 0+halfSideLength);
		points[top] = btVector3(0, 0+halfSideLength, 0);
		points[bottom] = btVector3(0, 0-halfSideLength, 0);

		Face* newFace = new Face(points[backright], points[backleft], points[top]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[backright], points[backleft], points[bottom]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[backright], points[frontright], points[top]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[backright], points[frontright], points[bottom]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[frontleft], points[backleft], points[top]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[frontleft], points[backleft], points[bottom]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[frontleft], points[frontright], points[top]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);

		newFace = new Face(points[frontleft], points[frontright], points[bottom]);
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);
	}

private:
	btVector3 points[nrOfCorners];

	btScalar sideLength;
	btScalar halfSideLength;
};

}


#endif /* DIAMOND_H_ */
