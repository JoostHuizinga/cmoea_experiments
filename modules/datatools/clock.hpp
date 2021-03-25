/*
 * clock.hpp
 *
 *  Created on: Jun 17, 2014
 *      Author: joost
 */

#ifndef CLOCK_HPP_
#define CLOCK_HPP_

#include <sferes/dbg/dbg.hpp>
#include <sferes/ea/ea.hpp>
//#include <boost/archive/xml_oarchive.hpp>

#ifndef TIME_DISABLED
class Clock{
public:
	Clock(std::string name = ""){
		_time = 0;
		_start = 0;
		_name = name;
	}

	inline void reset(){
		_time = 0;
	}

	inline void start(){
		_start = std::clock();
	}

	inline void resetAndStart(){
		reset();
		start();
	}

	inline void stop(){
		_time += 1000.0 * (std::clock() - _start) / CLOCKS_PER_SEC;
	}
	inline double time() const{
		 return _time;
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(_time);
	}
private:
	dbg::dbgclock_t _start;
	double _time;
	std::string _name;
};
#else
class Clock{
public:
	Clock(){};
	inline void start(){};
	inline void stop(){};
	inline void reset(){};
	inline void resetAndStart(){};
	inline double time(){return 0;};
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		//nix
	}
};

#endif

#endif /* CLOCK_HPP_ */
