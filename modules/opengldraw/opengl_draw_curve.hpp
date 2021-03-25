/*
 * opengl_draw_curve.hpp
 *
 *  Created on: May 17, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_OPENGLDRAW_OPENGL_DRAW_CURVE_HPP_
#define MODULES_OPENGLDRAW_OPENGL_DRAW_CURVE_HPP_

#include <sferes/dbg/dbg.hpp>

#include "opengl_draw_drawable_object.hpp"

namespace opengl_draw{

class Curve: public WorldObject{
public:
    Curve(btVector3 startPoint, btVector3 controlPoint1, btVector3 controlPoint2, btVector3 endPoint, btScalar stepSize = 0.1):
        WorldObject(),
        _startPoint(startPoint),
        _controlPoint1(controlPoint1),
        _controlPoint2(controlPoint2),
        _endPoint(endPoint),
        _stepSize(stepSize)
    {
        //Intentionally left blank
      _normal = btVector3(0, 1, 0);
    }

    /**
     * Draw this face. We assume the painter class will know how.
     */
    void draw(Painter* painter){
		dbg::trace trace("opengl_draw", DBG_HERE);
        painter->setMaterial(_materials[0]);
	//GLfloat oldWidth = painter->getLineWidth();
	//painter->setLineWidth(_lineWidth);
        painter->beginTriangleStrip();
	btVector3 prev_point = getPoint(0);
        for(btScalar i=_stepSize; i<=1.0; i+=_stepSize){
            btVector3 point = getPoint(i);
	    btVector3 vector = point - prev_point;
	    btVector3 cross = _normal.cross(vector);
	    cross.normalize();
	    
            painter->addPoint(point + cross*_lineWidth);
	    painter->addPoint(point - cross*_lineWidth);
        }
        btVector3 point = getPoint(1);
	btVector3 vector = point - prev_point;
	btVector3 cross = _normal.cross(vector);
	cross.normalize();
	painter->addPoint(point + cross*_lineWidth);
	painter->addPoint(point - cross*_lineWidth);
        painter->end();
	//painter->setLineWidth(oldWidth);
    }


    inline btVector3 getPoint(const btScalar& t) const{
        btScalar temp1 = (1-t)*(1-t);
        btScalar temp2 = t*t;
        btScalar f1 = temp1*(1-t);
        btScalar f2 = temp1*t*3;
        btScalar f3 = (1-t)*temp2*3;
        btScalar f4 = t*temp2;
        return f1*_startPoint + f2*_controlPoint1 + f3*_controlPoint2 + f4*_endPoint;
    }

protected:
    btVector3 _startPoint;
    btVector3 _controlPoint1;
    btVector3 _controlPoint2;
    btVector3 _endPoint;
    btVector3 _normal;
    btScalar _stepSize;
};

}

#endif /* MODULES_OPENGLDRAW_OPENGL_DRAW_CURVE_HPP_ */
