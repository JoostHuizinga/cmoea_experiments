/*
 * World.h
 *
 *  Created on: Nov 29, 2013
 *      Author: joost
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "opengl_draw_world_object.hpp"
#include "Light.h"


namespace opengl_draw{

/**
 * The world in which objects reside.
 * The world can be extended, transformed and has a base material for if some of its children have none.
 *
 * The world also contains all lights and background color.
 */
class World: public WorldObject {
public:
	World(float width = 0, float height = 0): _width(width), _height(height){
		//Do nothing
	}

	virtual ~World(){
		for (std::list<Light*>::iterator it = _lights.begin(); it != _lights.end(); it++)
			delete (*it);
		_lights.clear();
	}
    
    virtual void draw(Painter* painter){
        painter->pushTransformation();
        //        std::cout << "Translation: " << _position.x() << " " << _position.y() << " " << _position.z() << std::endl;
//        std::cout << "Rotation: "  << _rotation.w() << " "<< _rotation.x() << " " << _rotation.y() << " " << _rotation.z() << std::endl;
        //        std::cout << "Scale: " << _scale.x() << " " << _scale.y() << " " << _scale.z() << std::endl;
        painter->translate(_position);
        painter->rotate(_rotation);
        painter->scale(_scale);
        
        for (std::list<Light*>::iterator it = _lights.begin(); it != _lights.end(); it++){
//            std::cout << "Drawing light" << std::endl;
            painter->drawLight(*it);
        }
        
        //        GLfloat oldLineWidth = painter->getLineWidth();
        for (std::list<DrawableObject*>::iterator it = drawableObjects.begin(); it != drawableObjects.end(); it++)
            (*it)->draw(painter);
        //        painter->setLineWidth(oldLineWidth);
        painter->popTransformation();
    }

	/**
	 * Adds a light as a drawable child.
	 */
	inline void addLight(Light* light){
		_lights.push_back(light);
	}

	inline const std::list<Light*>& getLights() const{
		return _lights;
	}

	inline const float& getWidth() const{
		return _width;
	}

	inline const float& getHeight() const{
		return _height;
	}

	inline void setWidth(const float& width){
		_width = width;
	}

	inline void setHeight(const float& height){
		_height = height;
	}

	inline const Color& getGlobalAmbientLight() const{
		return _globalAmbientLight;
	}

	inline void setGlobalAmbientLight(const Color& globalAmbientLight){
		_globalAmbientLight = globalAmbientLight;
	}

protected:
	std::list<Light*> _lights;
	Color _globalAmbientLight;
	float _width;
	float _height;
};
    
}

#endif /* WORLD_H_ */
