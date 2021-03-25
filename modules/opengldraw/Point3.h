/*
 * Point3.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef POINT3_H_
#define POINT3_H_

#include "Array3.h"
#include "math.h"

class Vector3;

/**
 * Class to store a point
 */
class Point3 : public Array3{

public:

	Point3(const Array3& array):Array3(array){
		setVector(false);
	}

	Point3(const GLfloat& x = 0.0, const GLfloat& y = 0.0, const GLfloat& z = 0.0):
		Array3(x, y, z, false){
	}

	Point3(std::istream& is):Array3(is){
		setVector(false);
	}

	/**
	 * Returns the euclidean distance between this point and an other point.
	 */
	GLfloat distance(const Point3& other){
		GLfloat dx = x() - other.x();
		GLfloat dy = y() - other.y();
		GLfloat dz = z() - other.z();
		return sqrt(dx*dx + dy*dy + dz*dz);
	}

};

std::istream& operator>>(std::istream& is, Point3& obj);
std::ostream& operator<<(std::ostream& os, Point3& obj);
std::ostream& operator<<(std::ostream& os, const Point3& obj);

//Vector3 operator-(const Point3& lhs, const Point3& rhs);
//Point3 operator-(const Point3& lhs, const Vector3& rhs);
//
//
//Point3 operator+(const Point3& lhs, const Vector3& rhs);
//Point3 operator+(const Vector3& lhs, const Point3& rhs);


#endif /* POINT3_H_ */
