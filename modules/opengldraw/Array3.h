/*
 * Array3.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef ARRAY3_H_
#define ARRAY3_H_

#if defined(__APPLE__) && !defined (VMDMESA)
#include <GLUT/glut.h>
#else // defined(__APPLE__) && !defined (VMDMESA)

#include <glut.h>
#include <GL/glut.h>
#endif// defined(__APPLE__) && !defined (VMDMESA)

#include <iostream>
#include <math.h>

/**
 * An array containing exactly three values (and an implicit fourth one).
 * This will be the base class for both Point3 and Vector3.
 */
class Array3{
public:

	/**
	 * Create an array3 from a stream.
	 */
	Array3(std::istream& is){
		is >> values[x_index];
		is >> values[y_index];
		is >> values[z_index];
	}

	/**
	 * Constructs an array with the indicated x, y and z values.
	 */
	Array3(const GLfloat& x = 0.0, const GLfloat& y = 0.0, const GLfloat& z = 0.0, bool isVector = false){
		setX(x);
		setY(y);
		setZ(z);
		setVector(isVector);
	}

	/**
	 * Fast way to set the x, y, z values.
	 */
	inline void set(const GLfloat& x = 0.0, const GLfloat& y = 0.0, const GLfloat& z = 0.0, bool isVector = false){
		setX(x);
		setY(y);
		setZ(z);
		setVector(isVector);
	}

	/**
	 * Returns the x value of this array.
	 */
	inline GLfloat x() const{
		return values[x_index];
	}

	/**
	 * Returns the y value of this array.
	 */
	inline GLfloat y() const{
		return values[y_index];
	}

	/**
	 * Returns the z value of this array.
	 */
	inline GLfloat z() const{
		return values[z_index];
	}

	/**
	 * Returns the z value of this array.
	 */
	inline GLfloat vectorIndicator() const{
		return values[vector_index_or_length];
	}

	/**
	 * Test whether this array actually holds a vector.
	 */
	inline bool isVector(){
		return !((bool) values[vector_index_or_length]);
	}

	/**
	 * Turn this array into either a vector or a point.
	 */
	inline void setVector(bool isVector){
		if (isVector){
			values[vector_index_or_length] = 0;
		} else {
			values[vector_index_or_length] = 1;
		}
	}

	/**
	 * Set the x value of this array.
	 */
	inline void setX(const GLfloat& x){
		values[x_index] = x;
	}


	/**
	 * Subtracts the elements of the other array from this array.
	 */
	Array3 & operator-=(const Array3 &rhs){
		values[x_index] -= rhs.x();
		values[y_index] -= rhs.y();
		values[z_index] -= rhs.z();
		values[vector_index_or_length] -= rhs.vectorIndicator();
		return *this;
	}

	/**
	 * Adds the elements of the other array to this array.
	 */
	Array3 & operator+=(const Array3 &rhs){
		values[x_index] += rhs.x();
		values[y_index] += rhs.y();
		values[z_index] += rhs.z();
		values[vector_index_or_length] += rhs.vectorIndicator();
		return *this;
	}


	/**
	 * Adds the elements of the other array to this array.
	 */
	Array3 & operator*=(const Array3 &rhs){
		values[x_index] *= rhs.x();
		values[y_index] *= rhs.y();
		values[z_index] *= rhs.z();
		values[vector_index_or_length] *= rhs.vectorIndicator();
		return *this;
	}

	/**
	 * Adds the elements of the other array to this array.
	 */
	Array3 & operator/=(const Array3 &rhs){
		values[x_index] /= rhs.x();
		values[y_index] /= rhs.y();
		values[z_index] /= rhs.z();
		return *this;
	}

	/**
	 * Adds the elements of the other array to this array.
	 */
	Array3 & operator*=(const float &rhs){
		values[x_index] *= rhs;
		values[y_index] *= rhs;
		values[z_index] *= rhs;
		return *this;
	}


	/**
	 * Adds the elements of the other array to this array.
	 */
	Array3 & operator/=(const float &rhs){
		values[x_index] /= rhs;
		values[y_index] /= rhs;
		values[z_index] /= rhs;
		return *this;
	}

	/**
	 * Adds the scalar to this array.
	 */
	Array3 & operator+=(const float& rhs){
		values[x_index] += rhs;
		values[y_index] += rhs;
		values[z_index] += rhs;
		return *this;
	}

	/**
	 * Subtracts the scalar from this array.
	 */
	Array3 & operator-=(const float& rhs){
		values[x_index] -= rhs;
		values[y_index] -= rhs;
		values[z_index] -= rhs;
		return *this;
	}

	/**
	 * Set the y value of this array.
	 */
	inline void setY(const GLfloat& y){
		values[y_index] = y;
	}

	/**
	 * Set the z value of this array.
	 */
	inline void setZ(const GLfloat& z){
		values[z_index] = z;
	}

	/**
	 * Returns the length of this array (which is always 3).
	 */
	inline size_t size() const{
		return vector_index_or_length;
	}


	/**
	 * Returns a pointer to an array for matrix multiplication.
	 * The vector may contain the mandatory 1 at the end.
	 */
	inline const GLfloat* getArray() const{
		return values;
	}

	/**
	 * Normalizes this vector.
	 */
	inline Array3& normalize(){
		return (*this)/=length();
	}

	/**
	 * Returns the length (if it was a line from the origin), or magnitude of this vector.
	 */
	inline float length() const{
		return sqrt(x()*x() + y()*y() + z()*z());
	}

	/**
	 * Returns the dot product of this array and the other array.
	 */
	float dot(const Array3& other) const{
		return x()*other.x() + y()*other.y() + z()*other.z();
	}

	/**
	 * Returns the sum of this array.
	 */
	inline float sum() const{
		return x()+y()+z();
	}


protected:
	static const size_t x_index = 0;
	static const size_t y_index = 1;
	static const size_t z_index = 2;
	static const size_t vector_index_or_length = 3;

	//Make the array one longer to add the 0 or 1 for the generic vector notation.
	static const size_t real_length = 4;

	GLfloat values[real_length];
};

std::ostream& operator<<(std::ostream& os, const Array3& obj);

/**
 * Divide operators
 */
Array3 operator/(const Array3& lhs, const Array3& rhs);
Array3 operator/(const Array3& lhs, const GLfloat& rhs);


/**
 * Multiplication operators
 */
Array3 operator*(const Array3& lhs, const Array3& rhs);
Array3 operator*(const Array3& lhs, const GLfloat& rhs);
Array3 operator*(const GLfloat& lhs, const Array3& rhs);

/**
 * Subtraction operators
 */
Array3 operator-(const Array3& lhs, const Array3& rhs);
Array3 operator-(const Array3& lhs, const GLfloat& rhs);


/**
 * Addition operators
 */
Array3 operator+(const Array3& lhs, const Array3& rhs);
Array3 operator+(const Array3& lhs, const GLfloat& rhs);

#endif /* ARRAY3_H_ */
