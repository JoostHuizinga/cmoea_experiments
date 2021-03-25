/*
 * Point3.cpp
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#include "Point3.h"
#include "Vector3.h"


/**
 * Convenience operator to read this point
 */
std::istream& operator>>(std::istream& is, Point3& obj)
{
	obj = Point3(is);
	return is;
}

/**
 * Convenience operator to print points.
 */
std::ostream& operator<<(std::ostream& os, Point3& obj)
{
	os << obj.x() << " " << obj.y() << " " << obj.z();
	return os;
}

/**
 * Convenience operator to print points, now also for constant objects.
 */
std::ostream& operator<<(std::ostream& os, const Point3& obj)
{
	os << obj.x() << " " << obj.y() << " " << obj.z();
	return os;
}

