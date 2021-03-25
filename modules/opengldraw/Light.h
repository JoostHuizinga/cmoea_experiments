/*
 * Light.h
 *
 *  Created on: Nov 4, 2013
 *      Author: joost
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include <math.h>

#include "opengl_draw_color.hpp"
#include "Vector3.h"
#include "Point3.h"

namespace opengl_draw{

/**
 * This is the base class to represent a light in the environment.
 * Mainly consists of getters and setters for all different properties a light can have.
 */
class Light{
	//This static variable hold the current_id to be assigned to the next light that gets constructed.
//    static size_t current_id;
    size_t id;
    
	bool spotlight;
	bool directional;
	Color ambient;
	Color diffuse;
	Color specular;
	Array3 position;

	float spotExponent; //Only values in the range [0, 128] are accepted.
	float spotCutoff;   //Only values in the range [0, 90] and the special value 180 are accepted.
	float cosCutoff;
	Array3 spotDirection; //Not that this returns 4 values, not 3. Lets try it and see if it crashes.

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;

	bool dirty;
	bool enabled;

public:
	Light(size_t _id){
        //Set id
        id = _id;
//        current_id++;

        
		//Default is no spotlight
		spotlight = false;
		dirty = true;
		enabled = true;
		directional = false;
		spotCutoff = 0.0;
		cosCutoff = 0.0;
		spotExponent = 0.0;

		//Default for no attenuation
		constantAttenuation = 1.0;
		linearAttenuation = 0.0;
		quadraticAttenuation = 0.0;
	}
    
    float getConstantAttenuation(){
        return constantAttenuation;
    }
    
    float getLinearAttenuation(){
        return linearAttenuation;
    }
    
    float getQuadraticAttenuation(){
        return quadraticAttenuation;
    }
    
    bool isEnabled(){
        return enabled;
    }
    
    size_t getId(){
        return id;
    }

	void setAmbient(Color ambientColor){
		ambient = ambientColor;
		dirty = true;
	}

	Color getAmbient(){
		return ambient;
	}

	void setDiffuse(Color diffuseColor){
		diffuse = diffuseColor;
		dirty = true;
	}

	Color getDiffuse(){
		return diffuse;
	}

	void setSpecular(Color specularColor){
		specular = specularColor;
		dirty = true;
	}

	Color getSpecular(){
		return specular;
	}

	void setPosition(Array3 _position){
		position = _position;
		dirty = true;
	}

	const Array3& getPosition() const{
		return position;
	}


	void setDirectional(bool _directional){
		directional = _directional;
		dirty = true;
	}

	inline const bool& isDirectional() const {
		return directional;
	}

	void setSpotLight(bool _spotlight){
		spotlight = _spotlight;
		dirty = true;
	}

	void setSpotDirection(Array3 direction){
		spotDirection = direction;
		dirty = true;
	}

	Array3 getSpotDirection(){
		return spotDirection;
	}

	void setSpotExponent(GLfloat exponent){
		spotExponent = exponent;
		dirty = true;
	}

	GLfloat getExponent(){
		return spotExponent;
	}

	void setSpotCutoff(GLfloat cutoff){
		spotCutoff = cutoff;
		cosCutoff = cos((M_PI/180.0) * spotCutoff);
		dirty = true;
	}

	GLfloat getCutoff(){
		return spotCutoff;
	}

	GLfloat getCosCutoff(){
		return cosCutoff;
	}

	bool isSpotLight(){
		return spotlight;
	}
    
    bool isDirty(){
        return dirty;
    }
    
    void setDirty(bool dirty_){
        dirty = dirty_;
    }

	/**
	 * "Enables" the light, meaning it will set all properties that OpenGL needs to know about.
	 */
	void enable(){
		//If this light is directional position should be a vector.
		position.setVector(directional);
		dirty = true;
	}
};
}
    


#endif /* LIGHT_H_ */
