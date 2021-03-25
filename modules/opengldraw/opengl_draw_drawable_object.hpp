/*
 * DrawableObject.h
 *
 *  Created on: Sep 14, 2013
 *      Author: joost
 */

#ifndef DRAWABLEOBJECT_H_
#define DRAWABLEOBJECT_H_

#include <list>
#include "opengl_draw_painter.hpp"
#include "opengl_draw_material.hpp"

namespace opengl_draw{
/**
 * Base class for all objects that implement a draw() function.
 */
class DrawableObject {
public:
//	DrawableObject();
	virtual ~DrawableObject(){
		deleteChildren();
	}

	virtual void draw(Painter* painter){
		for (std::list<DrawableObject*>::iterator it = drawableObjects.begin();
				it != drawableObjects.end(); it++)
		{
			(*it)->draw(painter);
		}
	}

	void addDrawableChild(DrawableObject* child){
		drawableObjects.push_back(child);
	}

	void deleteChildren(){
		for (std::list<DrawableObject*>::iterator it = drawableObjects.begin();
				it != drawableObjects.end(); it++)
		{
			(*it)->deleteChildren();
			delete (*it);
		}
		drawableObjects.clear();
	}

	size_t getNrOfChildren(){
		return drawableObjects.size();
	}

	std::list<DrawableObject*> getChildren(){
		return drawableObjects;
	}


	void removeChild(DrawableObject* child){
		drawableObjects.remove(child);
		delete child;
	}

protected:
	std::list<DrawableObject*> drawableObjects;
};
}

#endif /* DRAWABLEOBJECT_H_ */
