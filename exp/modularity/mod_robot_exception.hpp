/*
 * mod_robot_exception.hpp
 *
 *  Created on: Nov 5, 2013
 *      Author: joost
 */

#ifndef MOD_ROBOT_EXCEPTION_HPP_
#define MOD_ROBOT_EXCEPTION_HPP_

#include <exception>
#include <string>

namespace mod_robot{
class mod_robot_exception: public std::exception
{
	//	char message[256];
	std::string message;
public:
	virtual ~mod_robot_exception() throw (){

	}

	mod_robot_exception	(std::string message):
		message(message){

	}

	virtual const char* what() const throw(){
		return message.c_str();
	}
};
}

#endif /* MOD_ROBOT_EXCEPTION_HPP_ */
