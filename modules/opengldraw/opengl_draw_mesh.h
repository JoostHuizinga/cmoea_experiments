/*
 * Mesh.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef MESH_H_
#define MESH_H_

#include <iostream>
#include <istream>
#include <fstream>
#include <limits>
#include <map>
#include "opengl_draw_material.hpp"
#include "opengl_draw_face.hpp"
#include "opengl_draw_world_object.hpp"


namespace opengl_draw{


/**
 * This class stores a mesh as a list of vertices, faces and normals. It is called static as it can not be moved.
 */
class Mesh: public WorldObject{
	Material* defaultMaterial;

public:
	virtual ~Mesh(){
		delete defaultMaterial;
	}


	/**
	 * Create an uninitialized static mesh.
	 */
	Mesh(): faceList(0){
		createDefaultMaterial();
	}


	/**
	 * Set the default material if no material file was found.
	 */
	void createDefaultMaterial(){
		defaultMaterial = new Material("default");
		defaultMaterial->setAmbient(Color(0.2, 0.0, 0.0));
		defaultMaterial->setDiffuse(Color(1.0, 0.5, 0.5));
		defaultMaterial->setSpecular(Color(1.0, 1.0, 1.0));
		defaultMaterial->setShininess(100.0);
		defaultMaterial->setTransparency(1.0);
		defaultMaterial->setIlluminationMode(2);
	}

};
}

#endif /* MESH_H_ */
