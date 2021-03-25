/*
 * position3.hpp
 *
 *  Created on: Dec 9, 2013
 *      Author: joost
 */

#ifndef POSITION3_HPP_
#define POSITION3_HPP_

#include <math.h>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <sferes/dbg/dbg.hpp>

namespace sferes
{
  namespace gen
  {
    namespace spatial
    {
      class Pos
      {
        public:
          Pos() {
          }
          Pos(float x, float y, float z) : _x(x), _y(y), _z(z) {
          }
          float dist(const Pos& p) const
          {
            float x = _x - p._x;
            float y = _y - p._y;
            float z = _z - p._z;
            return sqrt(x * x + y * y + z * z);
          }
          float x() const { return _x; }
          float y() const { return _y; }
          float z() const { return _z; }

          void setX(const float& x){_x = x;};
          void setY(const float& y){_y = y;};
          void setZ(const float& z){_z = z;};

          void moveX(const float& dx){_x += dx;};
          void moveY(const float& dy){_y += dy;};
          void moveZ(const float& dz){_z += dz;};

          void translate(const float& dx, const float& dy, const float& dz){
        	  moveX(dx);
        	  moveY(dy);
        	  moveZ(dz);
          }

          float& operator [] (const size_t& index){
        	  switch(index){
        	  case 0:
        		  return _x;
        	  case 1:
        		  return _y;
        	  case 2:
        		  return _z;
        	  default: dbg::sentinel(DBG_HERE);
        	  }
        	  dbg::sentinel(DBG_HERE);
        	  throw 0;
          }

          Pos& operator *= (const float& scalar){
        	  _x*=scalar;
        	  _y*=scalar;
        	  _z*=scalar;
        	  return *this;
          }

          Pos& operator += (const float& scalar){
        	  _x+=scalar;
        	  _y+=scalar;
        	  _z+=scalar;
        	  return *this;
          }

          Pos& operator += (const Pos& other){
        	  _x+=other.x();
        	  _y+=other.y();
        	  _z+=other.z();
        	  return *this;
          }

          template<class Archive>
          void serialize(Archive& ar, const unsigned int version)
          {
            ar& BOOST_SERIALIZATION_NVP(_x);
            ar& BOOST_SERIALIZATION_NVP(_y);
            ar& BOOST_SERIALIZATION_NVP(_z);
          }
          bool operator == (const Pos &p)
          { return _x == p._x && _y == p._y && _z == p._z; }
        protected:
          float _x, _y, _z;
      };

      Pos operator+(const Pos& lhs, const Pos& rhs){
    	Pos result (lhs);
      	result+=rhs;
      	return result;
      }

      Pos operator*(const Pos& lhs, const float& rhs){
    	Pos result (lhs);
      	result*=rhs;
      	return result;
      }

      Pos operator*(const float& rhs, const Pos& lhs){
      	return lhs*rhs;
      }

      std::ostream& operator<<(std::ostream& is, const Pos& obj){
          is << obj.x() << " " << obj.y() << " " << obj.z();
          return is;
      }
    }
  }
}

#endif /* POSITION3_HPP_ */
