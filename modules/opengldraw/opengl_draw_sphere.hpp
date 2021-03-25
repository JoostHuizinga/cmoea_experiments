/*
 * opengl_draw_sphere.hpp
 *
 *  Created on: Sep 29, 2013
 *      Author: joost
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include "opengl_draw_world_object.hpp"
#include "opengl_draw_face.hpp"
#include <vector>
#include <boost/config.hpp>
#include <sferes/stc.hpp>


namespace opengl_draw{
/**
 * A sphere shaped world object.
 */
class Sphere: public WorldObject{
	static const int left = 0;
	static const int right = 1;
	static const int front = 2;
	static const int back = 3;
	static const int top = 4;
	static const int bottom = 5;

	static const int granularity = 20;

	SFERES_CONST btScalar halfSideLength = 1;
public:

	Sphere():WorldObject(){
		_update();
	}

	void makePolygons(btVector3 a, btVector3 b, btVector3 c){
		btVector3 delta1 = b - a;
		btVector3 delta2 = c - a;

		delta1/=granularity;
		delta2/=granularity;

		int sub = granularity;
		btVector3 base1 = a;

		for(int i =0; i<granularity; i++){
			btVector3 base2 = base1;
			for(int i =0; i<sub; i++){
				btVector3 newTopPoint = base2 + delta1;
				btVector3 newLeftPoint = base2 + delta2;
				btVector3 newTopLeftPoint = newLeftPoint + delta1;

				addPolygon(base2, newTopPoint, newLeftPoint);

				if(i<sub-1){
					addPolygon(newTopPoint, newLeftPoint, newTopLeftPoint);
				}
				base2 = base2 + delta2;
			}
			sub--;
			base1 = base1 + delta1;
		}
	}


	void addPolygon(btVector3 point1, btVector3 point2, btVector3 point3){
		Face* newFace = new Face(3);
		newFace->setVertice(0, point1.normalized());
		newFace->setVertice(1, point2.normalized());
		newFace->setVertice(2, point3.normalized());
		newFace->setNormal(0, point1.normalized());
		newFace->setNormal(1, point2.normalized());
		newFace->setNormal(2, point3.normalized());
		newFace->setMaterial(this->getDefaultMaterial());
		this->addDrawableChild(newFace);
	}

	//This function is almost a straight copy from stack overflow: http://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
	btVector3 normalize(btVector3 a, btVector3 b, GLdouble length){
		btScalar dx = b.x() - a.x();
		btScalar dy = b.y() - a.y();
		btScalar dz = b.z() - a.z();

	    dx = dx * length / a.distance(b);
	    dy = dy * length / a.distance(b);
	    dz = dz * length / a.distance(b);
	    btVector3 c(a.x() + dx, a.y() + dy, a.z() + dz);
	    return c;
	}

	//This function is almost a straight copy from stack overflow: http://stackoverflow.com/questions/7687148/drawing-sphere-in-opengl-without-using-glusphere
	btVector3 normalize2(btVector3 b){
		btScalar dx = b.x();
		btScalar dy = b.y();
		btScalar dz = b.z();

	    dx = dx / b.length();
	    dy = dy / b.length();
	    dz = dz / b.length();
	    btVector3 c(dx, dy, dz);
	    return c;
	}

private:
	void _update(){
		btScalar _halfSideLength = halfSideLength;
		points[left] = btVector3(_halfSideLength, 0, 0);
		points[right] = btVector3(-_halfSideLength, 0, 0);
		points[front] = btVector3(0, 0, -_halfSideLength);
		points[back] = btVector3(0, 0, _halfSideLength);
		points[top] = btVector3(0, _halfSideLength, 0);
		points[bottom] = btVector3(0, -_halfSideLength, 0);

		makePolygons(points[left], points[front], points[top]);
		makePolygons(points[right], points[front], points[top]);
		makePolygons(points[left], points[front], points[bottom]);
		makePolygons(points[right], points[front], points[bottom]);

		makePolygons(points[left], points[back], points[top]);
		makePolygons(points[right], points[back], points[top]);
		makePolygons(points[left], points[back], points[bottom]);
		makePolygons(points[right], points[back], points[bottom]);
	}

	btVector3 points[6];
};
}

#endif /* PIRAMID_H_ */
