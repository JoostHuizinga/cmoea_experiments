/*
 * RayCast_Camera.h
 *
 *  Created on: Nov 13, 2013
 *      Author: joost
 */

#ifndef OPENGL_CAMERA_H_
#define OPENGL_CAMERA_H_

namespace opengl_draw{
/**
 * The Camera used for raycasting.
 * Is only here so the normal 'OpenGL_Camera' can be added as well.
 */
class OpenGlCamera: public Camera{
	void _update(){
        GLdouble m[16];
        Vector3 eVec(eye_point.x(), eye_point.y(), eye_point.z());
        
        m[0] = camera_left.x();
        m[1] = camera_up.x();
        m[2] = camera_forward.x();
        m[3] = 0;
        m[4] = camera_left.y();
        m[5] = camera_up.y();
        m[6] = camera_forward.y();
        m[7] = 0;
        m[8] = camera_left.z();
        m[9] = camera_up.z();
        m[10] = camera_forward.z();
        m[11] = 0;
        m[12] = -eVec.dot(camera_left);
        m[13] = -eVec.dot(camera_up);
        m[14] = -eVec.dot(camera_forward);
        m[15] = 1.0;
        
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(m);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
//        gluPerspective(viewAngle, aspect, nearDist, farDist);
        
        if(projection == orthographic){
            glOrtho(left, left+width, bot, bot+height, nearDist, farDist);
        } else if(projection == perspective){
            gluPerspective(viewAngle, aspect, nearDist, farDist);
        } else if(projection == frustrum){
            glFrustum(left, left+width, bot, bot+height, nearDist, farDist);
        } else {
            // TODO: Raise error
        }
        
        glutPostRedisplay();
	}

public:
	OpenGlCamera()
	{}



};
}

#endif /* OPENGL_CAMERA_H_ */
