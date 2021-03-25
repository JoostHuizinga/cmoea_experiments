/*
 * Color.h
 *
 *  Created on: Oct 2, 2013
 *      Author: joost
 */

#ifndef COLOR_H_
#define COLOR_H_

#include <iostream>

namespace opengl_draw{

/**
 * A class for storing rgba colors.
 */
class Color{
	static const int colorLength=4;
	static const int rIndex=0;
	static const int gIndex=1;
	static const int bIndex=2;
	static const int alphaIndex=3;
public:

	/**
	 * Copy constructor
	 */
	Color(const Color& other){
		colorVector[rIndex] = other.r();
		colorVector[gIndex] = other.g();
		colorVector[bIndex] = other.b();
		colorVector[alphaIndex] = other.a();
	}

	/**
	 * Creates a new color object based on the provided rgb values.
	 */
	Color(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0){
		colorVector[rIndex] = r;
		colorVector[gIndex] = g;
		colorVector[bIndex] = b;
		colorVector[alphaIndex] = a;
	}

	/**
	 * Fast way of setting r, g, b and a values.
	 */
	void set(float r = 0.0, float g = 0.0, float b = 0.0, float a = 1.0){
		colorVector[rIndex] = r;
		colorVector[gIndex] = g;
		colorVector[bIndex] = b;
		colorVector[alphaIndex] = a;
	}

	/**
	 * Implemented the assignment operator to guarantee correct copying.
	 */
	Color & operator=(const Color &rhs){
		colorVector[rIndex] = rhs.r();
		colorVector[gIndex] = rhs.g();
		colorVector[bIndex] = rhs.b();
		colorVector[alphaIndex] = rhs.a();
		return *this;
	}

    /**
     * Implemented the comparison operator
     */
    bool operator==(const Color &rhs){
        if(colorVector[rIndex] != rhs.r()) return false;
        if(colorVector[gIndex] != rhs.g()) return false;
        if(colorVector[bIndex] != rhs.b()) return false;
        if(colorVector[alphaIndex] != rhs.a()) return false;
        return true;
    }

	/**
	 * Returns the r value of this color.
	 */
    float r() const{
		return colorVector[rIndex];
	}

	/**
	 * Returns the g value of this color.
	 */
    float g() const{
		return colorVector[gIndex];
	}

	/**
	 * Returns the b value of this color.
	 */
    float b() const{
		return colorVector[bIndex];
	}

	/**
	 * Returns the alpha value of this color.
	 */
    float a() const{
		return colorVector[alphaIndex];
	}

    /**
     * Returns the r value of this color.
     */
	void setR(const float& value){
        colorVector[rIndex] = value;
    }

    /**
     * Returns the g value of this color.
     */
	void setG(const float& value){
        colorVector[gIndex] = value;
    }

    /**
     * Returns the b value of this color.
     */
	void setB(const float& value){
        colorVector[bIndex] = value;
    }

    /**
     * Returns the alpha value of this color.
     */
	void setA(const float& value){
        colorVector[alphaIndex] = value;
    }

	/**
	 * Returns a vector usable by various openGL operations.
	 */
	float* getVector(){
		return colorVector;
	}

	/**
	 * Prints this color for debug purposes.
	 */
	void print() const{
		std::cout << "r: " << r() << " g: " << g() << " b: " << b() << " a: " << a() << std::endl;
	}



private:
	float colorVector[colorLength];
};

}

///**
// * Convenience operator for writing colors.
// */
//std::ostream& operator<<(std::ostream& is, opengl_draw::Color& obj){
//    is << "r: " << obj.r() << " g: " << obj.g() << " b: " << obj.b() << " a: " << obj.a();
//    return is;
//}

#endif /* COLOR_H_ */
