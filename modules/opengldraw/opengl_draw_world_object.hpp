/*
 * WorldObject.h
 *
 *  Created on: Sep 29, 2013
 *      Author: joost
 */

#ifndef WORLDOBJECT_H_
#define WORLDOBJECT_H_


#if defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
//#include "OpenGL/GlutDemoApplication.h"
#include "LinearMath/btAlignedObjectArray.h"
#else// defined(__APPLE__) && !defined (VMDMESA)
#include "btBulletDynamicsCommon.h"
//#include "bullet/OpenGL/GlutDemoApplication.h"
#include "LinearMath/btAlignedObjectArray.h"
#endif

#include "modules/bullet/GlutDemoApplication.h"

#include "opengl_draw_drawable_object.hpp"
#include "opengl_draw_material.hpp"

namespace opengl_draw{
/**
 * Base class for all objects that have a coordinate and a location somewhere in the world.
 * Its position is of course in world coordinates.
 */
class WorldObject: public DrawableObject{
public:
	WorldObject(btVector3 position = btVector3(0.0, 0.0, 0.0), btVector4 rotation = btVector4(1.0, 0.0, 0.0, 0.0), btVector3 scale = btVector3(1.0, 1.0, 1.0)): _position(position), _rotation(rotation), _scale(scale){
		_createDefaultMaterial();
		_lineWidth = 1.0;
	}

	virtual ~WorldObject(){
		deleteChildren();
		for(int i=0; i<_materials.size(); i++){
			delete _materials[i];
		}
		_materials.clear();
	}

	const btVector3& getPosition() const{
		return _position;
	}

	void setPosition(const btVector3& position){
		_position = position;
	}

	void setRotation(const btVector4& rotation){
		_rotation = rotation;
	}

    void setScale(const btVector3& scale){
        _scale = scale;
    }

    void setLineWidth(const btScalar& lineWidth){
        _lineWidth = lineWidth;
    }

	void move(const btVector3& direction){
		_position+=direction;
	}

	virtual void draw(Painter* painter){
		painter->pushTransformation();
//		std::cout << "Translation: " << _position.x() << " " << _position.y() << " " << _position.z() << std::endl;
//        if (_scale.x() == 0.95){
//            std::cout << "Rotation: "  << _rotation.w() << " "<< _rotation.x() << " " << _rotation.y() << " " << _rotation.z() << std::endl;
//            std::cout << "Scale: " << _scale.x() << " " << _scale.y() << " " << _scale.z() << std::endl;
//        }
		painter->translate(_position);
		painter->rotate(_rotation);
		painter->scale(_scale);
//		GLfloat oldLineWidth = painter->getLineWidth();
		for (std::list<DrawableObject*>::iterator it = drawableObjects.begin(); it != drawableObjects.end(); it++)
			(*it)->draw(painter);
//		painter->setLineWidth(oldLineWidth);
		painter->popTransformation();
	}

	Material* getDefaultMaterial() const{
		return _materials[0];
	}

	void setDefaultMaterialDiffuseColor(Color color){
		Material* mat = _materials[0];
		mat->setDiffuse(color);
	}
    
    void setDefaultMaterialSpecularColor(Color color){
        Material* mat = _materials[0];
        mat->setSpecular(color);
    }
    
    void setDefaultMaterialAmbientColor(Color color){
        Material* mat = _materials[0];
        mat->setAmbient(color);
    }

    void setDefaultMaterialFlatColor(Color color){
        Material* mat = _materials[0];
        mat->setAmbient(color);
        mat->setDiffuse(Color(0.0, 0.0, 0.0, color.a()));
        mat->setSpecular(Color(0.0, 0.0, 0.0, color.a()));
    }

protected:
	btVector3 _position;
	btVector4 _rotation;
	btVector3 _scale;
	btScalar _lineWidth;
	std::vector<Material*> _materials;


	/**
	 * Set the default material if no material file was found.
	 */
	void _createDefaultMaterial(){
		Material* defaultMaterial;
		defaultMaterial = new Material("default");
		defaultMaterial->setAmbient(Color(0.0, 0.0, 0.0));
		defaultMaterial->setDiffuse(Color(1.0, 0.0, 0.0));
		defaultMaterial->setSpecular(Color(1.0, 1.0, 1.0));
		defaultMaterial->setShininess(100.0);
		defaultMaterial->setTransparency(1.0);
		defaultMaterial->setIlluminationMode(2);
		_materials.push_back(defaultMaterial);
	}

};
}

#endif /* WORLDOBJECT_H_ */
