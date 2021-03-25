/*
 * Global.h
 *
 *  Created on: Oct 10, 2013
 *      Author: joost
 */

#ifndef MOD_ROBOT_GLOBAL_HPP_
#define MOD_ROBOT_GLOBAL_HPP_

#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>

namespace mod_robot{

void printPosition(btVector3 position){
	std::cout << "x: " << position.getX()
	<< " y: " << position.getY()
	<< " z: " << position.getZ();
}

template <typename Type>
inline std::string toString(Type value){
	std::ostringstream stream;
	stream << value;
	return stream.str();
}

static const int spider_index = 1;
static const int target_index = 2;
}

#endif /* MOD_ROBOT_GLOBAL_HPP_ */
