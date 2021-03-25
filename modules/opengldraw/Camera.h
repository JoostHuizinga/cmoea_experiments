/*
 * Camera.h
 *
 *  Created on: Nov 3, 2013
 *      Author: joost
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Point3.h"
#include "Vector3.h"

#include <sstream>

float const e = 2.718281828f;
float const pi = 3.141592653f;

namespace opengl_draw {

/**
 * The camera class, mostly copied from the example, but adapted to work with my vectors and points.
 * Note that this is a base class and that it does not assume that we are using openGL
 */
class Camera{


public:
    static const int orthographic = 0;
    static const int perspective = 1;
    static const int frustrum = 2;
    
	Camera():height(0), width(0), viewAngle(0), aspect(0), nearDist(0), farDist(0), left(0), bot(0), projection(0)
	{}

	virtual ~Camera(){
		//nix
	}


	/**
	 * Sets the camera. Tries to imitate gluLookAt as good as possible.
	 */
	virtual void set(Point3& _eye_point, Point3& _lookat_point, Vector3& _up_vector){
		eye_point = _eye_point;
		lookat_point = _lookat_point;
		up_vector = _up_vector;
		camera_forward = eye_point - lookat_point;
		camera_forward.normalize();
		camera_left = up_vector.cross(camera_forward);
		camera_left.normalize();
		camera_up = camera_forward.cross(camera_left);
		camera_up.normalize();
		_update();
	}

	/**
	 * Overloaded function to set the camera
	 */
	virtual void set(float ex, float ey, float ez, float lx, float ly, float lz, float ux, float uy, float uz){
		Point3 _eye_point(ex, ey, ez);
		Point3 _lookat_point(lx, ly, lz);
		Vector3 _up_vector(ux, uy, uz);
		set(_eye_point, _lookat_point, _up_vector);
		_update();
	}

	/**
	 * Get the width (in world coordinates) of the near plane
	 */
	virtual float getWidth(){
		return width;
	}

	/**
	 * Get the height (in world coordinates) of the near plane
	 */
	virtual float getHeight(){
		return height;
	}

	/**
	 * Return the aspect ratio of the screen
	 */
	virtual const float& getAspect() const{
		return aspect;
	}

	/**
	 * Returns the eye-point
	 */
	virtual Point3 getEyePoint(){
		return eye_point;
	}

	/**
	 * Returns the N vector
	 */
	virtual Vector3 getN(){
		return camera_forward;
	}

	/**
	 * Returns the U vector
	 */
	virtual Vector3 getU(){
		return camera_left;
	}

	/**
	 * Returns the V vector
	 */
	virtual Vector3 getV(){
		return camera_up;
	}

	/**
	 * Returns all four corners of the near plane.
	 * In reality we use only the bottom left corner, but I did not think about that when writing this function.
	 */
	virtual std::vector<Point3> getNearPlane(){
		std::vector<Point3> result;
		result.resize(4);
		float halfWidth = width/2;
		float halfHeight = height/2;
		Point3 center = eye_point - camera_forward*nearDist;
		Point3 topLeft = center + camera_left*halfWidth + camera_up*halfHeight;
		Point3 topRight = center + camera_left*halfWidth - camera_up*halfHeight;
		Point3 bottomLeft = center - camera_left*halfWidth + camera_up*halfHeight;
		Point3 bottomRight = center - camera_left*halfWidth - camera_up*halfHeight;
		result[0] = topLeft;
		result[1] = topRight;
		result[2] = bottomRight;
		result[3] = bottomLeft;
		return result;
	}

	/**
	 * Set the shape of the viewing volume according to perspective projection
	 */
	virtual void setShape(float vAng, float asp, float nearD, float farD){
		float degrees;
		viewAngle = vAng;
		aspect = asp;
		nearDist = nearD;
		farDist = farD;

		degrees = ((viewAngle)*pi)/180;
		height = nearDist*tan(degrees);
		width = height*aspect;
        
        _update();
	}
    
    /**
     * Set the shape of the viewing volume for the orthographic projection
     */
    virtual void setShape(float _left, float right, float _bot, float top, float nearD, float farD){
        nearDist = nearD;
        farDist = farD;
        height = top - _bot;
        width = right - _left;
        left = _left;
        bot = _bot;
        
        aspect = width/height;
        
        _update();
    }
    
//    virtual void setShapeFrustum(-0.24*aspect, 0.24*aspect, -0.30, 0.22, 1, 100){
//
//    }

	// slide used in the book "Computer Graphics using OpenGL" Hill/Kelley page 337
	// the book had typos that have been corrected here
	// move camera and look point according to delta values
	void slide(float delU, float delV, float delN){
		float dx = delU * camera_left.x() + delV * camera_up.x() + delN * camera_forward.x();
		float dy = delU * camera_left.y() + delV * camera_up.y() + delN * camera_forward.y();
		float dz = delU * camera_left.z() + delV * camera_up.z() + delN * camera_forward.z();

		eye_point.setX(eye_point.x() + dx);
		eye_point.setY(eye_point.y() + dy);
		eye_point.setZ(eye_point.z() + dz);

		lookat_point.setX(lookat_point.x() + dx);
		lookat_point.setY(lookat_point.y() + dy);
		lookat_point.setZ(lookat_point.z() + dz);

		_update();
	}




	// roll used in the book "Computer Graphics using OpenGL" Hill/Kelley page 338
	// rotate around n vector
	void roll(float theta){
		float cs = cos(M_PI/180.0 * theta);
		float sn = sin(M_PI/180.0 * theta);
		Vector3 t(camera_left);
		camera_left.set(cs*t.x() + sn*camera_up.x(), cs*t.y() + sn*camera_up.y(), cs*t.z() + sn*camera_up.z());
		camera_up.set(-sn*t.x() + cs*camera_up.x(), -sn*t.y() + cs*camera_up.y(), -sn*t.z() + cs*camera_up.z());
		camera_left.normalize();
		camera_up.normalize();
		_update();
	}


	// rotate camera around camera_left vector
	void pitch(float theta){
		float cs = cos(M_PI/180.0 * theta);
		float sn = sin(M_PI/180.0 * theta);
		Vector3 t(camera_up);
		camera_up.set(cs*t.x() + sn*camera_forward.x(), cs*t.y() + sn*camera_forward.y(), cs*t.z() + sn*camera_forward.z());
		camera_forward.set(-sn*t.x() + cs*camera_forward.x(), -sn*t.y() + cs*camera_forward.y(), -sn*t.z() + cs*camera_forward.z());
		camera_up.normalize();
		camera_forward.normalize();
		_update();
	}

	//rotate camera around camera_up vector
	void yaw(float theta){
		float cs = cos(M_PI/180.0 * theta);
		float sn = sin(M_PI/180.0 * theta);
		Vector3 t(camera_forward);
		camera_forward.set(cs*t.x() + sn*camera_left.x(), cs*t.y() + sn*camera_left.y(), cs*t.z() + sn*camera_left.z());
		camera_left.set(-sn*t.x() + cs*camera_left.x(), -sn*t.y() + cs*camera_left.y(), -sn*t.z() + cs*camera_left.z());
		camera_forward.normalize();
		camera_left.normalize();
		_update();
	}


	// rotate camera around a fixed point such that the camera
	// is always looking at that point (0, 0, 0);
	void FocusedPitch(float theta){
		Point3 e, l;
		Vector3 eVec(eye_point.x(), eye_point.y(), eye_point.z());

		float cs = cos(M_PI/180.0 * theta);
		float sn = sin(M_PI/180.0 * theta);
		float mag = eVec.length();

		Vector3 t(camera_up);

		camera_up.set(
				cs*t.x() + sn*camera_forward.x(),
				cs*t.y() + sn*camera_forward.y(),
				cs*t.z() + sn*camera_forward.z());
		camera_forward.set(
				-sn*t.x() + cs*camera_forward.x(),
				-sn*t.y() + cs*camera_forward.y(),
				-sn*t.z() + cs*camera_forward.z());
		camera_up.normalize();
		camera_forward.normalize();

		eye_point.set(mag*camera_forward.x(), mag*camera_forward.y(), mag*camera_forward.z());

		_update();
	}



	// rotate camera around a fixed point such that the camera
	// is always looking at that point (0, 0, 0);
	void FocusedYaw(float theta){
		Point3 e, l;
		Vector3 eVec(eye_point.x(), eye_point.y(), eye_point.z());

		float cs = cos(pi/180 * theta);
		float sn = sin(pi/180 * theta);
		float mag = eVec.length();

		Vector3 t(camera_forward);

		camera_forward.set(
				cs*t.x() + sn*camera_left.x(),
				cs*t.y() + sn*camera_left.y(),
				cs*t.z() + sn*camera_left.z());
		camera_left.set(
				-sn*t.x() + cs*camera_left.x(),
				-sn*t.y() + cs*camera_left.y(),
				-sn*t.z() + cs*camera_left.z());
		camera_forward.normalize();
		camera_left.normalize();

		eye_point.set(mag*camera_forward.x(), mag*camera_forward.y(), mag*camera_forward.z());

		_update();
	}


	void CenterFocus(){
		Point3 center;
		set(eye_point, center, up_vector);
	}
    
    int getProjection(){
        return projection;
    }
    
    void setProjection(int _projection){
        projection = _projection;
    }

protected:
	Point3 eye_point, lookat_point;
	Vector3 camera_left, camera_up, camera_forward, up_vector;
	float height, width, viewAngle, aspect, nearDist, farDist, left, bot;
    int projection;

	virtual void _update() = 0;

};
}

#endif /* CAMERA_H_ */
