/*
 * Face.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef FACE_H_
#define FACE_H_


#include "opengl_draw_drawable_object.hpp"



namespace opengl_draw{
/**
 * Class for storing faces.
 * To speed-up the process the face will store direct pointers to the vertices, normals and faces associated with it.
 */
class Face: public DrawableObject{
	btVector3* vertices;
	btVector3* normals;
	size_t nrOfVertices;
	Material* material;

public:
	Face():vertices(0), normals(0), nrOfVertices(0), material(0){
		//nix
	}

	Face(size_t nrOfVertices){
		init(nrOfVertices);
	}

	Face(const btVector3& first, const btVector3& second, const btVector3& third, bool invert_normal=false){
		init(3);
		btVector3 vector1 = second - first;
		btVector3 vector2 = third - first;

		btVector3 normal = vector2.cross(vector1);
		normal.normalize();
		if(invert_normal) normal *= -1;
		setVertice(0, first);
		setVertice(1, second);
		setVertice(2, third);
		setNormal(0, normal);
		setNormal(1, normal);
		setNormal(2, normal);
	}

	Face(const btVector3& first, const btVector3& second, const btVector3& third, const btVector3& fourth){
		init(4);
		btVector3 vector1 = second - first;
		btVector3 vector2 = third - first;

		btVector3 normal = vector1.cross(vector2);
		normal.normalize();
		setVertice(0, first);
		setVertice(1, second);
		setVertice(2, third);
		setVertice(3, fourth);
		setNormal(0, normal);
		setNormal(1, normal);
		setNormal(2, normal);
		setNormal(3, normal);
	}


	virtual ~Face(){
		delete [] vertices;
		delete [] normals;
	}

	/**
	 * Initializes the face with space for the provided number of vertices.
	 */
	void init(size_t _nrOfVertices){
		nrOfVertices = _nrOfVertices;
		vertices = new btVector3[nrOfVertices];
		normals = new btVector3[nrOfVertices];
		material = 0;
	}

	/**
	 * Sets a pointer to a vertice at the provided index.
	 */
	inline void setVertice(size_t index, const btVector3& vertex){
		vertices[index] = vertex;
//		std::cout << "set point "<< vertices[index] << " " << vertex << " index: " << index << " " << vertices[index].x() << " " << vertices[index].y() << " " << vertices[index].z() <<std::endl;
	}

	/**
	 * Sets a pointer to a normal at the provided index.
	 */
	inline void setNormal(size_t index, const btVector3& normal){
		normals[index] = normal;
	}


	/**
	 * Returns the point at the provided index
	 */
	inline const btVector3& getPoint(size_t index) const{
//		std::cout << "get point " << vertices[index] << " index: " << index << " " << vertices[index].x() << " " << vertices[index].y() << " " << vertices[index].z() <<std::endl;
		return vertices[index];
	}

	/**
	 * Returns the normal at the provided index
	 */
	inline const btVector3& getNormal(size_t index) const{
//		std::cout << normals[index].x() << " " << normals[index].y() << " " << normals[index].z() <<std::endl;
		return normals[index];
	}

	/**
	 * Returns the number of vertices
	 */
	inline size_t getNrOfVertices() const{
		return nrOfVertices;
	}

	/**
	 * Returns the material
	 */
	inline Material* getMaterial() const{
		return material;
	}

	/**
	 * Set the material
	 */
	void setMaterial(Material* _material){
		material = _material;
	}

	/**
	 * Draw this face. We assume the painter class will know how.
	 */
	void draw(Painter* painter){
//		std::cout << "Drawing Face" <<std::endl;
		//If this light is directional position should be a vector.


//	    glEnable (GL_DEPTH_TEST);
//
//		GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
//		GLfloat local_view[] = { 0.0 };
////
////		//Global ambient light
//		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
//		glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
//
////		std::cout << "Enabling light" << std::endl;
//		glLightfv(GL_LIGHT0+10, GL_AMBIENT, Color(0,0,0).getVector());
//		glLightfv(GL_LIGHT0+10, GL_DIFFUSE, Color(1,1,1).getVector());
//		glLightfv(GL_LIGHT0+10, GL_SPECULAR, Color(0,0,0).getVector());
//
//		GLfloat constantAttenuation;
//		GLfloat linearAttenuation;
//		GLfloat quadraticAttenuation;
//
//		//Default for no attenuation
//		constantAttenuation = 1.0;
//		linearAttenuation = 0.0;
//		quadraticAttenuation = 0.0;
//
//		glLightf(GL_LIGHT0+10, GL_CONSTANT_ATTENUATION, constantAttenuation);
//		glLightf(GL_LIGHT0+10, GL_LINEAR_ATTENUATION, linearAttenuation);
//		glLightf(GL_LIGHT0+10, GL_QUADRATIC_ATTENUATION, quadraticAttenuation);
//
//		glLightfv(GL_LIGHT0+10, GL_POSITION, Color(0, -1, 0, 0).getVector());
//		glDisable(GL_LIGHT0+10);
//
//		glDisable(GL_LIGHT0);
//		glDisable(GL_LIGHT1);
//




		painter->setMaterial(material);
//		painter->setColor(Color(1,1,1));
		painter->beginPolygon();
		for(size_t i=0; i<getNrOfVertices(); i++){
			painter->addNormal(getNormal(i));
			painter->addPoint(getPoint(i));
		}
		painter->end();
	}
};
}

#endif /* FACE_H_ */
