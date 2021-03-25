/*
 * opengl_draw_text.hpp
 *
 *  Created on: Aug 25, 2014
 *      Author: joost
 */

#ifndef OPENGL_DRAW_TEXT_HPP_
#define OPENGL_DRAW_TEXT_HPP_

#include "opengl_draw_world_object.hpp"
#include <vector>
#include <string>

#define OGLFT_NO_SOLID 1
#define OGLFT_NO_QT 1
#include <modules/oglft/OGLFT.h>

namespace opengl_draw{
/**
 * A pyramid shaped world object.
 */
class Text: public WorldObject{

public:

	Text(std::string text, btScalar x, btScalar y):WorldObject(btVector3(x, y, 0)){
		_text = text;
		_font = 0;
		_outline = 0;
		_shadow = 0;
	}

	Text(std::string text, btVector3 position = btVector3()):WorldObject(position){
		_text = text;
		_font = 0;
		_outline = 0;
		_shadow = 0;
	}


	void draw(Painter* painter){
	    if(_font){
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
	        painter->push2D();
	        //Presumably required to draw texture based text
	        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	        glEnable( GL_BLEND );
	        glEnable( GL_TEXTURE_2D );
            if(_shadow){
                _shadow->draw(_position.x(), _position.y(), _text.c_str());
            }
            glDisable( GL_TEXTURE_2D );
            glDisable( GL_BLEND );
	        _font->draw(_position.x(), _position.y(), _text.c_str());
	        if(_outline){
	            _outline->draw(_position.x(), _position.y(), _text.c_str());
	        }
	        painter->pop2D();
	    } else{
	        painter->setColor(_color);
	        painter->drawString(_text, GLint(_position.x()), GLint(_position.y()));
	    }
	}

	inline void setText(const std::string text){
		_text = text;
	}

	void setFont(OGLFT::Face* font){
	    _font = font;
	}

    void setOutline(OGLFT::Outline* outline){
        _outline = outline;
    }

    void setShadow(OGLFT::TranslucentTexture* shadow){
        _shadow = shadow;
    }

private:
	OGLFT::Face* _font;
	OGLFT::Outline* _outline;
    OGLFT::TranslucentTexture* _shadow;
	std::string _text;
	Color _color;
};
}



#endif /* OPENGL_DRAW_TEXT_HPP_ */
