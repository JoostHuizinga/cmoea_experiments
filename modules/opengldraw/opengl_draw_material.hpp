/*
 * Material.h
 *
 *  Created on: Nov 4, 2013
 *      Author: joost
 */

#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "opengl_draw_color.hpp"

namespace opengl_draw{

/**
 * Class for holding material values.
 * Consists mainly of getters and setters.
 * Also note that I have stretched the meaning of 'Color' somewhat
 * as everything that has an rgb or rgba value is considered a color for my purposes.
 */
class Material{
	std::string id;
	Color ambient;
	Color diffuse;
	Color specular;
	float alpha;
	float shininess;
	int illumnationMode;

public:
	Material(std::string id):id(id){
		illumnationMode = 1;
		alpha = 0.0;
		shininess = 0.0;
	}

	std::string getId(){
		return id;
	}

	void setAmbient(Color color){
		ambient = color;
	}

	Color& getAmbient(){
		return ambient;
	}

	void setDiffuse(Color color){
		diffuse = color;
	}

	Color& getDiffuse(){
		return diffuse;
	}

	void setSpecular(Color color){
		specular = color;
	}

	Color& getSpecular(){
		return specular;
	}

	void setTransparency(float _alpha){
		alpha = _alpha;
	}

	float getTransparency(){
		return alpha;
	}

	void setShininess(float _shininess){
		shininess = _shininess;
	}

	float getShininess(){
		return shininess;
	}

	void setIlluminationMode(int _illumnationMode){
		illumnationMode = _illumnationMode;
	}

	int getIlluminationMode(){
		return illumnationMode;
	}

};
}

#endif /* MATERIAL_H_ */
