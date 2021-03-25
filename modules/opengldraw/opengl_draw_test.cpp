/*
 * opengl_draw_test.cpp
 *
 *  Created on: Mar 09, 2020
 *      Author: Joost Huizinga
 */

// Include standard
#include <iostream>

// Include GLUT
#if defined(__APPLE__) && !defined (VMDMESA)
//#include <GLUT/glut.h>

#include <GL/freeglut.h>
#include "GL/freeglut_ext.h"
#else // defined(__APPLE__) && !defined (VMDMESA)
#include <glut.h>
#include <GL/glut.h>
#endif// defined(__APPLE__) && !defined (VMDMESA)

#include "Window.h"
#include "opengl_draw_color.hpp"
#include "opengl_draw_material.hpp"
#include "opengl_draw_sphere.hpp"
#include "opengl_draw_line.hpp"
#include "OpenGL_Camera.h"
#include "opengl_draw_cylinder.hpp"


void addCylinder(btVector3 pos, btVector3 pos_2, opengl_draw::World* world){
    opengl_draw::Color grey  (0.5, 0.5, 0.5, 1.0);
    btVector3 diff = pos - pos_2;
    float length = diff.length();
    btVector3 mid = pos_2 + 0.5 * diff;
    opengl_draw::Cylinder* cylinder = new opengl_draw::Cylinder();
    cylinder->setPosition(mid);
    diff = diff.normalize();
    btVector3 axis_of_rot = diff.cross(btVector3(0,1,0));
    float angle = -(diff.angle(btVector3(0,1,0))/(2*M_PI)*360);
    cylinder->setRotation(btVector4(axis_of_rot.x(), axis_of_rot.y(), axis_of_rot.z(), angle));
    cylinder->setScale(btVector3(0.02, length/2.0, 0.02));
    cylinder->setDefaultMaterialDiffuseColor(grey);
    cylinder->setDefaultMaterialAmbientColor(grey);
    cylinder->setDefaultMaterialSpecularColor(grey);
    world->addDrawableChild(cylinder);
    
//    opengl_draw::Line* line = new opengl_draw::Line(pos, pos_2);
//    line->setLineWidth(1);
////    line->setDefaultMaterialFlatColor(opengl_draw::Color (.1, 0.1, 0.1, 0.4));
//    line->setDefaultMaterialFlatColor(opengl_draw::Color (.3, 0.3, 0.3, 1));
//    world->addDrawableChild(line);
}

void drawConvConnections(btVector3 pos, int x, int y, int z, int max_y, int max_z, int next_layer_size, int next_layer_start, int conv_size, opengl_draw::World* world, std::vector<float> layer_x_pos){
    int next_layer_end = next_layer_size + next_layer_start;
    for(int y_2=0; y_2<max_y; y_2++){
        for(int z_2=0; z_2<max_z; z_2++){
            if(y_2 >= next_layer_start && y_2 < next_layer_end && z_2 >=next_layer_start && z_2 < next_layer_end && abs(y-y_2) <= conv_size &&  abs(z-z_2) <= conv_size){
                btVector3 pos_2 = btVector3(layer_x_pos[x-1], float(y_2)-float(max_y)/2, float(z_2)*3-(float(max_z)/2)*3);
                addCylinder(pos, pos_2, world);
            }
        }
    }
}

/* Sets up and runs the experiment. */
int main(int argc, char **argv) {
    std::cout << "Entering main: " << std::endl;
    typedef btVector3 Vector_t;
    
    //Initializing glut
    glutInit(&argc, argv);
    //glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGB); //GLUT_SINGLE means use one frame buffer and use color mode RGB {red,green,blue}
    glutSetOption(GLUT_MULTISAMPLE, 8);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH  | GLUT_RGBA | GLUT_MULTISAMPLE);
    
    //Define constants
    static const double speedOfLightGlass = 200000000.0;
    static const double speedOfLightAir = 299705000.0;
    static const int screenWidth = 1004;
    static const int screenHeight = 1004;
    double aspect = (double)screenWidth/ (double)screenHeight;
    
    //Define colors
    opengl_draw::Color white  (1.0, 1.0, 1.0, 1.0);
    opengl_draw::Color grey  (0.5, 0.5, 0.5, 1.0);
    opengl_draw::Color dark_grey  (0.2, 0.2, 0.2, 1.0);
    opengl_draw::Color black  (0.0, 0.0, 0.0, 1.0);
    opengl_draw::Color globalAmbientColor (0.25f, 0.05f, 0.05f, 1.0f);
    opengl_draw::Color pointLightColor (0.65f, 0.65f, 0.65f, 1.0f);
    opengl_draw::Color blueSplotColor (0.4f, 0.4f, 0.7f, 1.0f);
    opengl_draw::Color yellowDirectColor (0.8f, 0.8f, 0.6f, 1.0f);
//    opengl_draw::Color cmoea_red (0.8f, 0.0f, 0.0f, 1.0f);
    
    //inbetween small sphere
    opengl_draw::Color mat_ambient1 (0.5f, 0.5f, 0.5f, 1.0f);
    opengl_draw::Color mat_diffuse1 (0.7f, 0.7f, 0.7f, 1.0f);
    opengl_draw::Color mat_specular1 (0.9f, 0.9f, 0.9f, 1.0f);
    
    //matte medium sphere
    opengl_draw::Color mat_ambient2 (0.5f, 0.5f, 0.5f, 1.0f);
    opengl_draw::Color mat_diffuse2 (0.9f, 0.9f, 0.9f, 1.0f);
    opengl_draw::Color mat_specular2 (0.001f, 0.001f, 0.001f, 1.0f);
    
    //shiny large sphere
    opengl_draw::Color mat_ambient3 (0.23125f, 0.23125f, 0.23125f, 1.0f);
    opengl_draw::Color mat_diffuse3 (0.2775f, 0.2775f, 0.2775f, 1.0f);
    opengl_draw::Color mat_specular3 (0.7f, 0.7f, 0.7f, 1.0f);
    
    //Pixmap texture
//    opengl_draw::Pixmap* pixmap = new opengl_draw::Pixmap(5, 5);
//    pixmap->clear();
//    pixmap->setPixel(0, 0, white);
//    pixmap->setPixel(2, 0, white);
//    pixmap->setPixel(4, 0, white);
//    pixmap->setPixel(1, 1, white);
//    pixmap->setPixel(3, 1, white);
//    pixmap->setPixel(0, 2, white);
//    pixmap->setPixel(2, 2, white);
//    pixmap->setPixel(4, 2, white);
//    pixmap->setPixel(1, 3, white);
//    pixmap->setPixel(3, 3, white);
//    pixmap->setPixel(0, 4, white);
//    pixmap->setPixel(2, 4, white);
//    pixmap->setPixel(4, 4, white);
    
    //Create textures
//    CheckerboardTexture* checkerboardTexture = new CheckerboardTexture(black, white, 0.2);
//    FlatToSphereTexture* flatTexture = new FlatToSphereTexture(pixmap);
    
    //The 'world material' has only the background color and 'speed-of-light'. All other components will not be used as the world will never get 'hit'.
//    opengl_draw::Material* worldMaterial = new opengl_draw::Material("default");
//    opengl_draw::Material* sphere1Material = new opengl_draw::Material("mat_1");
//    opengl_draw::Material* sphere2Material = new opengl_draw::Material("mat_2");
//    opengl_draw::Material* sphere3Material = new opengl_draw::Material("mat_3");
    
    
//    //SMALL SPHERE
//    opengl_draw::Sphere* sphere1 = new opengl_draw::Sphere();
//    sphere1->setScale(Vector_t(0.05, .05, .05));
//    sphere1->setPosition(Vector_t(0.125, -0.25, -1.0));
//    sphere1->setDefaultMaterialDiffuseColor(cmoea_red);
////    sphere1->setMaterial(sphere1Material);
//
//    //MEDIUM SPHERE
//    opengl_draw::Sphere* sphere2 = new opengl_draw::Sphere();
//    sphere2->setScale(Vector_t(0.375, 0.375, 0.375));
//    sphere2->setPosition(Vector_t(0.5, 0.5, -1.75));
//    sphere2->setDefaultMaterialDiffuseColor(cmoea_red);
////    sphere2->setMaterial(sphere2Material);
//
//    //LARGE SPHERE
//    opengl_draw::Sphere* sphere3 = new opengl_draw::Sphere();
//    sphere3->setScale(Vector_t(0.75, 0.75, 0.75));
//    sphere3->setPosition(Vector_t(-0.5, 0, -2.5));
//    sphere3->setDefaultMaterialDiffuseColor(cmoea_red);
//    sphere3->setMaterial(sphere3Material);
    
//    //POINT LIGHT
//    opengl_draw::Light* pointLight = new opengl_draw::Light();
//    pointLight->setAmbiant(pointLightColor);
//    pointLight->setPosition(Vector_t(200.0, 100.0, 50.0));
//    pointLight->setDirectional(false);
//    pointLight->setSpotLight(false);
//    pointLight->enable();
//
//    //BLUE SPOT LIGHT
//    opengl_draw::Light* spotLight = new opengl_draw::Light();
//    spotLight->setSpecular(blueSplotColor);
//    spotLight->setPosition(Vector_t(-1.0, 0.0, 1.0));
//    spotLight->setSpotDirection(Vector_t(0.0, 0.0, -1.0));
//    spotLight->setDirectional(false);
//    spotLight->setSpotLight(true);
//    spotLight->setSpotCutoff(30.0f);
//    spotLight->setSpotExponent(0.5);
//    spotLight->enable();
//
    //YELLOW DIRECTIONAL LIGHT
    opengl_draw::Light* directLight = new opengl_draw::Light(0);
    directLight->setDiffuse(opengl_draw::Color(0.5, 0.5, 0.5));
    directLight->setSpecular(opengl_draw::Color(0.2, 0.2, 0.2));
    directLight->setAmbient(opengl_draw::Color(0.5, 0.5, 0.5));
//    directLight->setPosition(Array3(1, 10, -20.0));
    directLight->setPosition(Array3(0.5, 1, 0));
    directLight->setDirectional(true);
    directLight->setSpotLight(false);
    directLight->enable();
    
    opengl_draw::Light* directLight_2 = new opengl_draw::Light(1);
    directLight_2->setDiffuse(white);
    directLight_2->setSpecular(white);
    directLight_2->setAmbient(dark_grey);
    directLight_2->setPosition(Array3(-1, -10, 0));
    directLight_2->setDirectional(true);
    directLight_2->setSpotLight(false);
    directLight_2->enable();
    
    //Create window
    //The window has to be created before the painter as certain opengl commands will crash
    //if no window is present
    opengl_draw::Window* window = new opengl_draw::Window(0, 0, screenWidth, screenHeight);
    
    glEnable( GL_MULTISAMPLE );
//    glEnable(GL_MULTISAMPLE_ARB);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
//    glEnable(GL_LINE_SMOOTH); //Antialiasing
//    glEnable(GL_LINE_STIPPLE);
//    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    
//    glEnable(GL_POINT_SMOOTH); //This is important so that the joins are round
    
    //background color
    glClearColor(1.0,1.0,1.0,0.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Global ambient light
    GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat local_view[] = { 0.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
    
    // Enable depth testing
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
//    glEnable(GL_COLOR_MATERIAL);
    
    //Create the world
    opengl_draw::World* world = new opengl_draw::World();
//    world->setMaterial(worldMaterial);
    world->setGlobalAmbientLight(globalAmbientColor);
//    world->addDrawableChild(sphere1);
//    world->addDrawableChild(sphere2);
//    world->addDrawableChild(sphere3);
//    world->addLight(pointLight);
//    world->addLight(directLight_2);
    world->addLight(directLight);
    
//    {
//        opengl_draw::Sphere* sphere = new opengl_draw::Sphere();
//        float size = 0.1;
//        sphere->setScale(Vector_t(size, size, size));
//        sphere->setPosition(Vector_t(0, 0, 0));
//        opengl_draw::Color cmoea_red (0.8f, 0.0f, 0.0f, 1.0f);
//        sphere->setDefaultMaterialDiffuseColor(cmoea_red);
//        world->addDrawableChild(sphere);
//    }
//
//    {
//        opengl_draw::Sphere* sphere = new opengl_draw::Sphere();
//        float size = 0.1;
//        sphere->setScale(Vector_t(size, size, size));
//        sphere->setPosition(Vector_t(1, 0, 0));
//        opengl_draw::Color cmoea_red (0.8f, 0.0f, 0.0f, 1.0f);
//        sphere->setDefaultMaterialDiffuseColor(cmoea_red);
//        world->addDrawableChild(sphere);
//    }
    
//    int y=2;
//    int z=0;
    int layer_1_neurons = 5;
    int layer_2_neurons = 3;
    int layer_3_neurons = 3;
    bool layer_3_conv = false;
    int layer_4_neurons = 3;
    int layer_5_neurons = 3;
    int nb_layers = 5;
    int starting_layer = 0;
    
    std::vector<float> layer_x_pos;
    
    // Larger variation
    layer_x_pos.push_back(4-3*2 - 1.5*2);
    layer_x_pos.push_back(4-3*2 - 1.5);
    layer_x_pos.push_back(4-3*2);
    layer_x_pos.push_back(4-3);
    layer_x_pos.push_back(4);
    
    // Larger variation
//    layer_x_pos.push_back(4-2.5*2 - 1.5*2);
//    layer_x_pos.push_back(4-2.5*2 - 1.5);
//    layer_x_pos.push_back(4-2.5*2);
//    layer_x_pos.push_back(4-2.5);
//    layer_x_pos.push_back(4);
    
    // Small variation
//    layer_x_pos.push_back(4-2.3*2 - 1.8*2);
//    layer_x_pos.push_back(4-2.3*2 - 1.8);
//    layer_x_pos.push_back(4-2.3*2);
//    layer_x_pos.push_back(4-2.3);
//    layer_x_pos.push_back(4);
    
    // Equi-distant
//    layer_x_pos.push_back(-4);
//    layer_x_pos.push_back(-2);
//    layer_x_pos.push_back(0);
//    layer_x_pos.push_back(2);
//    layer_x_pos.push_back(4);
    
    int max_y = 0;
    int max_z = 0;
    for(int x=starting_layer; x<nb_layers+starting_layer; x++){
        if( x == 4){
            max_y = layer_1_neurons;
        } else if(x == 3) {
            max_y = layer_2_neurons;
        } else if(x == 2 && layer_3_conv){
            max_y = layer_3_neurons;
        } else {
            max_y = 1;
        }
        if( x == 4){
            max_z = layer_1_neurons;
        } else if(x == 3) {
            max_z = layer_2_neurons;
        } else if(x == 2){
            max_z = layer_3_neurons;
        } else if(x == 1){
            max_z = layer_4_neurons;
        } else if(x == 0){
            max_z = layer_5_neurons;
        }
        for(int y=0; y<max_y; y++){
            for(int z=0; z<max_z; z++){
                opengl_draw::Sphere* sphere = new opengl_draw::Sphere();
                float size = 0.2;
                sphere->setScale(Vector_t(size, size, size));
                Vector_t pos = Vector_t(layer_x_pos[x], float(y)-float(max_y)/2, float(z)*3-(float(max_z)/2)*3);
                sphere->setPosition(pos);
                opengl_draw::Color cmoea_red (0.0f, 0.0f, 0.8f, 1.0f);
                sphere->setDefaultMaterialDiffuseColor(opengl_draw::Color((1.0/255) * 20.0, (1.0/255) * 100.0, (1.0/255) * 128.0, 1.0f));
                sphere->setDefaultMaterialSpecularColor(opengl_draw::Color(1.0f, 1.0f, 1.0f, 1.0f));
                sphere->setDefaultMaterialAmbientColor(opengl_draw::Color((1.0/255) * 20.0, (1.0/255) * 100.0, (1.0/255) * 128.0, 1.0f));
                world->addDrawableChild(sphere);
                
                if(x==4){
                    drawConvConnections(pos, x, y, z, max_y, max_z, layer_2_neurons, (layer_1_neurons - layer_2_neurons) / 2, 1, world, layer_x_pos);
                    
                } else if(x==3 && starting_layer < 3) {
                    if(layer_3_conv){
                        drawConvConnections(pos, x, y, z, max_y, max_z, layer_3_neurons, (layer_2_neurons - layer_3_neurons) / 2, 1, world, layer_x_pos);
                    } else {
                        for(int z_2=0; z_2<layer_3_neurons; z_2++){
                            Vector_t pos_2 = Vector_t(layer_x_pos[x-1], -0.5, float(z_2)*3-(float(layer_3_neurons)/2.0)*3);
                            addCylinder(pos, pos_2, world);
                        }
                    }
                } else if(x==2 && starting_layer < 2) {
                    for(int z_2=0; z_2<layer_4_neurons; z_2++){
                        Vector_t pos_2 = Vector_t(layer_x_pos[x-1], -0.5, float(z_2)*3-(float(layer_4_neurons)/2.0)*3);
                        addCylinder(pos, pos_2, world);
                        
                    }
                } else if(x==1 && starting_layer < 1) {
                    for(int z_2=0; z_2<layer_5_neurons; z_2++){
                        Vector_t pos_2 = Vector_t(layer_x_pos[x-1], -0.5, float(z_2)*3-(float(layer_5_neurons)/2.0)*3);
                        addCylinder(pos, pos_2, world);
                    }
                }
            }
        }
    }
    
    
    world->setScale(btVector3(1.1, 1.1, 1.1));
    
    //Create the painter
    opengl_draw::Painter* painter = new opengl_draw::Painter();
//    painter->getCamera()->set(0.0, 0.0, 1.0, 0.0, 0.0 ,0.0 ,0.0 ,1.0 ,0.0);
//    painter->getCamera()->setShape(30.0, (float)screenWidth/(float)screenHeight, 1.0, 8000);
//    painter->setWorld(world);
    
    opengl_draw::OpenGlCamera* camera = new opengl_draw::OpenGlCamera();
    camera->setProjection(opengl_draw::OpenGlCamera::perspective);
    Point3 look_at(0, 0, 0);
    Vector3 up(0, 1, 0);
    if(camera->getProjection() == opengl_draw::OpenGlCamera::orthographic){
        camera->setShape(-6, 6,-6,6, -30, 30);
        Point3 camera_pos(0.0, 0.0, -1.0);
        camera->set(camera_pos, look_at, up);
        world->setRotation(btVector4(-0.5, 0.75, 0, 10));
    } else if(camera->getProjection() == opengl_draw::OpenGlCamera::perspective){
        camera->setShape(30.0, (float)screenWidth/(float)screenHeight, 1.0, 16000);
        Point3 camera_pos(0.0, 0.0, -30.0);
        camera->set(camera_pos, look_at, up);
        world->setRotation(btVector4(-0.25, 0.4, 0, 20));
    } else if(camera->getProjection() == opengl_draw::OpenGlCamera::frustrum){
        camera->setShape(-0.24*aspect, 0.24*aspect, -0.30, 0.22, 1, 100);
        Point3 camera_pos(0.0, 0.0, -1.0);
        camera->set(camera_pos, look_at, up);
        world->setRotation(btVector4(-0.5, 0.75, 0, 10));
    }
    
    
    //Set the window
    window->enableKeyPress();
    window->enableSpecialPress();
    window->setPainter(painter);
    window->setCamera(camera);
    window->setWorld(world);
    
    //Start running
    window->run();
}
