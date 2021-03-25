/*
 * opengl_draw_cylinder.hpp
 *
 *  Created on: Mar 10, 2020
 *      Author: Joost Huizinga
 */

#ifndef CYLINDER_H_
#define CYLINDER_H_


#include "opengl_draw_world_object.hpp"
#include "opengl_draw_face.hpp"
#include <vector>


namespace opengl_draw{
/**
 * A diamond shaped world object.
 */
class Cylinder: public WorldObject{
    static const int granularity = 20;
public:

	Cylinder(){
		update();
	}

	virtual void update(){
        std::vector<btVector3> top(granularity);
        std::vector<btVector3> bot(granularity);
        float circum = 0;
        float increment = 1.0 / float(granularity) * 2.0 * M_PI;
        for(int i = 0; i<granularity; i++){
            circum = increment * i;
            float x = sin(circum);
            float z = cos(circum);
            top[i] = btVector3(x, 1, z);
            bot[i] = btVector3(x, -1, z);
        }
        for(int i = 0; i<granularity-1; i++){
            Face* newFace = new Face(top[i], top[i+1], bot[i+1], bot[i]);
            newFace->setNormal(0, top[i] - btVector3(0, 1, 0));
            newFace->setNormal(1, top[i+1] - btVector3(0, 1, 0));
            newFace->setNormal(2, bot[i+1] - btVector3(0, -1, 0));
            newFace->setNormal(3, bot[i] - btVector3(0, -1, 0));
            newFace->setMaterial(this->getDefaultMaterial());
            this->addDrawableChild(newFace);
        }
        
        Face* topFace = new Face(granularity);
        Face* botFace = new Face(granularity);
        for(int i = 0; i<granularity; i++){
            topFace->setVertice(i, top[i]);
            topFace->setNormal(i, btVector3(0, 1, 0));
            botFace->setVertice(i, bot[i]);
            botFace->setNormal(i, btVector3(0, -1, 0));
        }
        topFace->setMaterial(this->getDefaultMaterial());
        botFace->setMaterial(this->getDefaultMaterial());
        this->addDrawableChild(topFace);
        this->addDrawableChild(botFace);
	}
};

}


#endif /* CYLINDER_H_ */
