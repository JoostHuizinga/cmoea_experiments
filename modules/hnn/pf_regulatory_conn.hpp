/*
 * pf_regulatory_conn.hpp
 *
 *  Created on: Jul 23, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HNN_PF_REGULATORY_CONN_HPP_
#define MODULES_HNN_PF_REGULATORY_CONN_HPP_

#include <modules/nn2/pf.hpp>
#include <sferes/gen/sampled.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/misc/rand.hpp>
#include <sferes/dbg/dbg.hpp>
#include <limits>

#include "af_cppn.hpp"
#include "regulatory_conn_weight.hpp"

namespace nn
{
SFERES_CLASS(PfRegConnParams)
{
public:
    typedef PfRegConnParams<Params, Exact> this_t;

    void set(size_t t){
        dbg::trace trace("pf", DBG_HERE);
        _type.set_data(0, t);
    }

    void mutate(){
        dbg::trace trace("mutate", DBG_HERE);
        dbg::trace trace2("pf", DBG_HERE);
        _type.mutate();
    }

    void random(){
        dbg::trace trace("pf", DBG_HERE);
        _type.random();
    }

    void develop() {dbg::trace trace("pf", DBG_HERE);}

    int type() const {
        dbg::trace trace("pf", DBG_HERE);
        return _type.data(0);
    }

    template<typename A>
    void serialize(A& ar, unsigned int v)
    {
        dbg::trace trace("pf", DBG_HERE);
        ar& BOOST_SERIALIZATION_NVP(_type);
    }
    bool operator==(const this_t &other) const {
        dbg::trace trace("pf", DBG_HERE);
        return _type == other._type;
    }

protected:
    sferes::gen::Sampled<1, Params> _type;
};

//PfWReg is a subclass of the Type Pf<W> which defines most of its fields
//PfWReg stands for Potential function with Regulatory Activation function,
//and determines the input of a neuron.
template<typename W, typename Params>
struct PfWRegConn : public Pf<W, Params>
{
    typedef W weight_t;

    //Initializes the _w_cache. Note that this only works after the size of weights is set.
    void init(){
        dbg::trace trace("pf", DBG_HERE);
        _w_cache.resize(this->_weights.size());
        _t_cache.resize(this->_weights.size());
        for (size_t i = 0; i < this->_weights.size(); ++i){
            _w_cache[i] = this->_weights[i].weight();
            _t_cache[i] = this->_weights[i].type();
        }
    }

    //The main function.
    //Takes a Eigen::VectorXf of float input values, and if the input size is correct
    //Then (if eigen is enabled, which is should be) returns the dot product of the cache (which should hold the vector of weights)
    //And the input vector. This function is used to calculate the input of a single neuron.
    float operator() (const trait<float>::vector_t & inputs) const{
        dbg::trace trace("pf", DBG_HERE);
        dbg::assertion(DBG_ASSERTION(inputs.size() == _w_cache.size()));
        dbg::assertion(DBG_ASSERTION(inputs.size() == _t_cache.size()));

        if (inputs.size() == 0){
            return 0.0f;
        }

        float result = 0;
        for(size_t i=0; i<_w_cache.size(); ++i){
            float product = _w_cache[i] * inputs[i];
            if(_t_cache[i] == regulatory){
                if(product <= 0.0f){
                    return 0;
                }
            }
            result += product;
        }

//#ifdef EIGEN2_ENABLED
//        float result = _w_cache.dot(inputs);
//#else
//#warning "No eigen2 -> no vectorization of pwfsum"
//        float result = (_w_cache * inputs).sum();
//#endif
        dbg::out(dbg::info, "pfreg") << "Sum: " << result <<std::endl;

        switch (this->_params.type()){
        case nn::cppn::sine:
        {
            result = sin(result);
            dbg::out(dbg::info, "pfreg") << "Sine, result: " << result <<std::endl;

#ifdef JHDEBUG
            if (std::isnan(result)){
                std::cerr << "Warning: result is nan" <<std::endl;
                std::cerr << "result: " << result << " type: " << this->_params.type() << std::endl;
            }
#endif
            return result;
            break;
        }
        case nn::cppn::sigmoid:
        {
            result = ((1.0 / (1.0 + exp(-result))) - 0.5) * 2.0;;
            dbg::out(dbg::info, "pfreg") << "Sigmoid, result: " << result <<std::endl;
#ifdef JHDEBUG
            if (std::isnan(result)){
                std::cerr << "Warning: result is nan" <<std::endl;
                std::cerr << "result: " << result << " type: " << this->_params.type() << std::endl;
            }
#endif
            return result;
            break;
        }
        case nn::cppn::gaussian:
        {
            result = exp(-powf(result, 2));
            dbg::out(dbg::info, "pfreg") << "Gaussian, result: " << result <<std::endl;
#ifdef JHDEBUG
            if (std::isnan(result)){
                std::cerr << "Warning: result is nan" <<std::endl;
                std::cerr << "result: " << result << " type: " << this->_params.type() << std::endl;
            }
#endif
            return result;
            break;
        }
        case nn::cppn::linear:
        {
            result = std::min(std::max(result, -3.0f), 3.0f) / 3.0f;
            dbg::out(dbg::info, "pfreg") << "Linear, result: " << result <<std::endl;
#ifdef JHDEBUG
            if (std::isnan(result)){
                std::cerr << "Warning: result is nan" <<std::endl;
                std::cerr << "result: " << result << " type: " << this->_params.type() << std::endl;
            }
#endif
            return result;
            break;
        }
        default:{
            dbg::sentinel(DBG_HERE);
        }
        }
        dbg::sentinel(DBG_HERE);
        return 0;
    }
protected:
    //Effectively defines something of type Eigen::VectorXf
    trait<float>::vector_t _w_cache;
    trait<reg_conn_type>::vector_t _t_cache;
};


}





#endif /* MODULES_HNN_PF_REGULATORY_CONN_HPP_ */
