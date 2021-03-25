/*
 * opengl_draw_overlay.hpp
 *
 *  Created on: Sep 29, 2013
 *      Author: joost
 */

#ifndef OVERLAY_H_
#define OVERLAY_H_


#if defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
//#include "OpenGL/GlutDemoApplication.h"
#include "LinearMath/btAlignedObjectArray.h"
#else// defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
//#include "bullet/OpenGL/GlutDemoApplication.h"
#include "LinearMath/btAlignedObjectArray.h"
#endif

#include "opengl_draw_drawable_object.hpp"

#define DBO dbg::out(dbg::info, "overlay")

namespace opengl_draw{
/**
 * Base class for all objects that have a coordinate and a location somewhere in the world.
 * Its position is of course in world coordinates.
 */
class Overlay: public DrawableObject{
public:

	Overlay(btVector3 translation = btVector3(0.0, 0.0, 0.0),
			btVector4 rotation = btVector4(1.0, 0.0, 0.0, 0.0),
			btVector3 scale = btVector3(1.0, 1.0, 1.0)):
		_translation(translation), _rotation(rotation), _scale(scale)
	{
	}

	virtual void draw(Painter* painter){
		DBO << "Drawing objects: " << drawableObjects.size() << std::endl;
		painter->pushTransformation();
		painter->loadIdentity();
		painter->clearDepth();
		painter->translate(_translation.x(), _translation.y(), _translation.z());
		painter->rotate(_rotation.w(), _rotation.x(), _rotation.y(), _rotation.z());
		painter->scale(_scale.x(), _scale.y(), _scale.z());

		for (std::list<DrawableObject*>::iterator it = drawableObjects.begin();
				it != drawableObjects.end(); it++)
		{
			(*it)->draw(painter);
		}
		painter->popTransformation();
	}

	void setTranslation(const btVector3& translation){
		_translation = translation;
	}

	void setRotation(const btVector4& rotation){
		_rotation = rotation;
	}

	void setScale(const btVector3& scale){
		_scale = scale;
	}
private:
btVector3 _translation;
btVector4 _rotation;
btVector3 _scale;

};
}

#undef DBO
#endif /* WORLDOBJECT_H_ */
