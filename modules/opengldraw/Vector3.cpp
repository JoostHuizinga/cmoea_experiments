/*
 * Vector3.cpp
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#include "Vector3.h"
#include "Point3.h"



/**
 * Convenience operator to read vectors from a stream.
 */
std::istream& operator>>(std::istream& is, Vector3& obj)
{
	obj = Vector3(is);
	return is;
}

/**
 * Convenience operator to print points.
 */
std::ostream& operator<<(std::ostream& os, Vector3& obj)
{
	os << obj.x() << " " << obj.y() << " " << obj.z();
	return os;
}

/**
 * Convenience operator to print points, now also for constant objects.
 */
std::ostream& operator<<(std::ostream& os, const Vector3& obj)
{
	os << obj.x() << " " << obj.y() << " " << obj.z();
	return os;
}

