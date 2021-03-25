/*
 * Painter.h
 *
 *  Created on: Oct 2, 2013
 *      Author: joost
 */

#ifndef PAINTER_H_
#define PAINTER_H_

#if defined(NO_OPEN_GL)
#include "opengl_draw_dummy_painter.hpp"
#else
#include "opengl_draw_real_painter.hpp"
#endif

#endif /* PAINTER_H_ */
