/*
 * mod_robot_object_group.hpp
 *
 *  Created on: Sep 17, 2014
 *      Author: joost
 */

#ifndef MOD_ROBOT_OBJECT_GROUP_HPP_
#define MOD_ROBOT_OBJECT_GROUP_HPP_


#include <iostream>
#include "mod_robot_object.hpp"

namespace mod_robot{
class ObjectGroup{

public:
	ObjectGroup(size_t type): _type(type){

	}

	inline size_t size() const{
		return _objects.size();
	}


	inline size_t type() const{
		return _type;
	}

	inline const btVector3& getPosition(const size_t& index) const{
		return _objects[index]->getPosition();
	}

	Object* &operator[](size_t i){
		return _objects[i];
	}

	Object* const &operator[](size_t i) const{
//		const WorldObject* temp = _objects[i];
		return _objects[i];
	}


	inline void remove(Object* object){
		for(std::vector<Object*>::iterator it = _objects.begin(); it != _objects.end(); ++it){
			if((*it) == object){
				delete object;
				_objects.erase(it, it+1);
				return;
			}
		}
	}

	inline void add(Object* object){
		_objects.push_back(object);
	}

	inline void clear(){
		for(std::vector<Object*>::iterator it = _objects.begin(); it != _objects.end(); ++it){
			delete (*it);
		}
		_objects.clear();
	}

protected:
	std::vector<Object*> _objects;
	size_t _type;

};

std::ostream& operator<< (std::ostream &o, const ObjectGroup& object){
  o << object.type() << " : ";
  for(size_t i=0; i<object.size(); ++i){
	  btVector3 pos = object.getPosition(i);
	  o << pos.x() << " " << pos.y()<< " " << pos.z() << ", ";
  }
  return o;
}
}



#endif /* MOD_ROBOT_OBJECT_GROUP_HPP_ */
