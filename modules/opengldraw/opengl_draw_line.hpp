/*
 * opengl_draw_line.hpp
 *
 *  Created on: May 17, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_OPENGLDRAW_OPENGL_DRAW_LINE_HPP_
#define MODULES_OPENGLDRAW_OPENGL_DRAW_LINE_HPP_

#include <sferes/dbg/dbg.hpp>

#include "opengl_draw_drawable_object.hpp"

namespace opengl_draw{

class Line: public WorldObject{
public:
    Line(btVector3 start, btVector3 end):
        WorldObject(),
        _start(start),
        _end(end)
    {
        //Intentionally left blank
    }

    /**
     * Draw this face. We assume the painter class will know how.
     */
    void draw(Painter* painter){
    	dbg::trace trace("opengl_draw", DBG_HERE);
        painter->setMaterial(_materials[0]);
	GLfloat oldWidth = painter->getLineWidth();
	painter->setLineWidth(_lineWidth);
        painter->beginLine();
        painter->addPoint(_start);
        painter->addPoint(_end);
        painter->end();
	painter->setLineWidth(oldWidth);
    }

protected:
    btVector3 _start;
    btVector3 _end;

};

}

#endif /* MODULES_OPENGLDRAW_OPENGL_DRAW_LINE_HPP_ */
