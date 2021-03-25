/*
 * Vector3.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef VECTOR3_H_
#define VECTOR3_H_

#include "Array3.h"
#include "math.h"

class Point3;

/**
 * Class to store a vector
 */
class Vector3 : public Array3{

public:

	Vector3(const Array3& array):Array3(array){
		setVector(true);
	}

	Vector3(const GLfloat& x = 0.0, const GLfloat& y = 0.0, const GLfloat& z = 0.0):
		Array3(x, y, z, true){
		//nix
	}

	Vector3(std::istream& is):Array3(is){
		setVector(true);
	}


	/**
	 * Returns the cross product of this vector and the other vector.
	 */
	Vector3 cross(const Vector3& other){
		return Vector3(y()*other.z() - z()*other.y(), z()*other.x() - x()*other.z(), x()*other.y() - y()*other.x());
	}

	/**
	 * Flips this vector.
	 */
	void flip(){
		setX(-x());
		setY(-y());
		setZ(-z());
	}

	Vector3 flipped() const{
		Vector3 result(*this);
		result.flip();
		return result;
	}
};

std::istream& operator>>(std::istream& is, Vector3& obj);
std::ostream& operator<<(std::ostream& os, Vector3& obj);
std::ostream& operator<<(std::ostream& os, const Vector3& obj);

#endif /* VECTOR3_H_ */
