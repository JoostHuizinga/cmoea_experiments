/*
 * mod_robot_visualize_network.hpp
 *
 *  Created on: Mar 17, 2018
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_MOD_ROBOT_VISUALIZE_NETWORK_HPP_
#define EXP_MODULARITY_MOD_ROBOT_VISUALIZE_NETWORK_HPP_

#include "svg_writer.hpp"
#include "nn_conn_to_key.hpp"

template<typename Params, typename robot_t, typename ctrnn_nn_t>
void visualizeNetworkBullet(robot_t* spider, const bool asp, const bool asg) {
    std::cout << "Entering main" <<std::endl;
    //Add options

    std::cout << "Retrieving world" << std::endl;
    btDynamicsWorld* world = Params::simulator->getWorld();

    std::cout << "Resetting simulator" << std::endl;
    Params::simulator->reset();

    std::cout << "Creating neural network" << std::endl;
    //Create the neural network
    ctrnn_nn_t* dnn = new ctrnn_nn_t(Params::dnn::nb_inputs, Params::dnn::nb_of_hidden, Params::dnn::nb_outputs);
    for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
        dnn->getNeuron(i)->setBias(0);
        dnn->getNeuron(i)->setTimeConstant(2);
    }
    float transparancy = 0.2;
    if(asp){
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            //uint start = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            //uint end = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*(i+1);
            for(size_t j=0; j<Params::dnn::nb_of_hidden; ++j){
                int imod = i;
                int kmod = j / 5;
                std::string key = inputToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                }
                //            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, 0.5), i, j);
            }
        }
        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            //uint node = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            for(size_t j=0; j<Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; ++j){
                int imod = i / 5;
                int kmod = j / 5;
                if( imod == kmod || j >= Params::dnn::nb_of_hidden){
                    std::string key = hiddenToHiddenKey(Params::dnn::nb_of_hidden, i, j);
                    dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                    if(asg){
                        switch(imod){
                        case 0:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                            break;
                        case 1:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                            break;
                        case 2:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                            break;
                        case 3:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                            break;
                        case 4:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                            break;
                        case 5:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                            break;
                        }
                    } else {
                        if(j < Params::dnn::nb_of_hidden ){
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        } else if(j >= Params::dnn::nb_of_hidden && j < Params::dnn::nb_of_hidden + 6){
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        } else if(j >= Params::dnn::nb_of_hidden+6 && j < Params::dnn::nb_of_hidden + 12){
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        } else {
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        }
                    }

                    //                if(k >= Params::dnn::nb_of_hidden){
                    //                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, 0.5), i, k);
                    //                } else {
                    //                    Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, 0.5), i, k);
                    //                }
                }
            }
        }
    } else {
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            for(size_t j=0; j<Params::dnn::nb_of_hidden; ++j){
                int imod = i;
                int kmod = j / 5;
                std::string key = inputToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                }
            }
        }
        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            //uint node = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            for(size_t j=0; j<Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; ++j){
                int imod = i / 5;
                int kmod = j / 5;

                std::string key = hiddenToHiddenKey(Params::dnn::nb_of_hidden, i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    if(j < Params::dnn::nb_of_hidden ){
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                    } else if(j >= Params::dnn::nb_of_hidden && j < Params::dnn::nb_of_hidden + 6){
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                    } else if(j >= Params::dnn::nb_of_hidden+6 && j < Params::dnn::nb_of_hidden + 12){
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                    } else {
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                    }
                }
            }
        }

    }
    dnn->reset();

    std::cout << "Setting visualization colors" << std::endl;
    //Set the visualization of the network
    std::vector<sferes::SvgColor> colors;
    size_t j=0;
     //for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
     //   colors.push_back(SvgColor(125, 0, 0));
     //}
    for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
        int imod;
        if(asg){
            imod = i % 6;
            if(i > 5 && i <= (5 + 6*5)){
                imod = (i-6) / 5;
            } else if (i > (5 + 6*5)){
                imod = 6;
            }
        } else {
            if(i < 6){
                imod = 0;
            } else if(i < (6*6)){
                imod = 1;
            } else if(i < (6*7)){
                imod = 2;
            } else if(i < (6*8)){
                imod = 3;
            } else {
                imod = 4;
            }
        }
        //double r = ((double)colors[i].r)/255.0;
        //  double g = ((double)colors[i].g)/255.0;
        //  double b = ((double)colors[i].b)/255.0;
        switch(imod){
        case 0:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .8,.0,.0,
                    i);
            break;
        case 1:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .0,.8,.0,
                    i);
            break;
        case 2:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .0,.0,.8,
                    i);
            break;
        case 3:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .8,.8,.0,
                    i);
            break;
        case 4:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .8,.0,.8,
                    i);
            break;
        case 5:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .0,.8,.8,
                    i);
            break;
        case 6:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .4,.4,1.0,
                    i);
            break;
        default:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    .8,.8,.8,
                    i);
            break;
        }

        //                if(k >= Params::dnn::nb_of_hidden){
        //                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, 0.5), i, k);
        //                } else {
        //                    Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, 0.5), i, k);
        //                }
    }
     Params::simulator->setOverlayOffset(btVector3(0,0,-5));


     std::cout << "Setting dnn" << std::endl;
     spider->setNN(dnn);

     std::cout << "Setting individual" << std::endl;
     Params::simulator->setIndividual(spider);
     //Params::simulator->addItem(new mod_robot::Control(neuron_index));
     std::cout << "Stepping the simulator" << std::endl;
     Params::simulator->stepsVisual(10000);
}


#endif /* EXP_MODULARITY_MOD_ROBOT_VISUALIZE_NETWORK_HPP_ */
