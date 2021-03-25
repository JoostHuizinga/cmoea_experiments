/*
 * mod_robot_object.hpp
 *
 *  Created on: Jan 29, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_MOD_ROBOT_OBJECT_HPP_
#define EXP_MODULARITY_MOD_ROBOT_OBJECT_HPP_

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <sferes/dbg/dbg.hpp>

namespace mod_robot{
class Object{
public:
    Object(): _body(0), _shape(0), _sensorIndex(-1), _type(-1), _userIndex(0){

    }

    Object(int sensorIndex): _body(0), _shape(0), _sensorIndex(sensorIndex), _type(-1), _userIndex(0){

    }

    virtual ~Object(){
        if(_body){
            delete _body->getMotionState();
            delete _body;
            delete _shape;
        }
    }

    inline btRigidBody* getRigidBody() const{
        return _body;
    }

    inline btCollisionShape* getShape() const{
        return _shape;
    }

    inline int getSensorIndex() const{
        return _sensorIndex;
    }

    inline void setSensorIndex(const int& sensorIndex){
        _sensorIndex = sensorIndex;
    }

    inline const btVector3 &getPosition() const{
        return _body->getCenterOfMassPosition();
    }

    inline int getType() const{
        return _type;
    }

    inline void setPosition(const btVector3& position){
        btTransform transformation;
        transformation.setIdentity();
        transformation.setOrigin(position);
        _body->setWorldTransform(transformation);
    }

    inline void move(const btVector3& direction){
        btTransform newTrans;
        _body->getMotionState()->getWorldTransform(newTrans);
        newTrans.getOrigin() += direction;
        _body->getMotionState()->setWorldTransform(newTrans);
    }

    virtual inline void init(const btVector3& position){
        //nix
    }


    inline int getUserIndex(){
    	return _userIndex;
    }

    inline void setUserIndex(int i){
    	_userIndex = i;
    }

    inline opengl_draw::Color getColor() const{
    	return _color;
    }

    inline void setColor(opengl_draw::Color color){
    	_color = color;
    }

    virtual void step(){
    	// nix
    }

protected:
    btRigidBody* _body;
    btCollisionShape* _shape;
    opengl_draw::Color _color;
    int _sensorIndex;
    int _type;
    int _userIndex;
};
}


#endif /* EXP_MODULARITY_MOD_ROBOT_OBJECT_HPP_ */
