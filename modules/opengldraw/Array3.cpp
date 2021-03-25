/*
 * Array3.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: joost
 */
#include "Array3.h"

/**
 * Convenience operator to print Array3 objects.
 */
std::ostream& operator<<(std::ostream& os, const Array3& obj)
{
	os << obj.x() << " " << obj.y() << " " << obj.z();
	return os;
}

/**
 * Divide operators
 */
Array3 operator/(const Array3& lhs, const Array3& rhs){
	Array3 result (lhs);
	result/=rhs;
	return result;
}

Array3 operator/(const Array3& lhs, const GLfloat& rhs){
	Array3 result (lhs);
	result/=rhs;
	return result;
}


/**
 * Multiplication operators
 */
Array3 operator*(const Array3& lhs, const Array3& rhs){
	Array3 result (lhs);
	result*=rhs;
	return result;
}

Array3 operator*(const Array3& lhs, const GLfloat& rhs){
	Array3 result (lhs);
	result*=rhs;
	return result;
}

Array3 operator*(const GLfloat& lhs, const Array3& rhs){
	return rhs * lhs;
}

/**
 * Subtraction operators
 */
Array3 operator-(const Array3& lhs, const Array3& rhs){
	Array3 result (lhs);
	result-=rhs;
	return result;
}

Array3 operator-(const Array3& lhs, const GLfloat& rhs){
	Array3 result (lhs);
	result-=rhs;
	return result;
}


/**
 * Addition operators
 */
Array3 operator+(const Array3& lhs, const Array3& rhs){
	Array3 result (lhs);
	result+=rhs;
	return result;
}

Array3 operator+(const Array3& lhs, const GLfloat& rhs){
	Array3 result (lhs);
	result+=rhs;
	return result;
}

