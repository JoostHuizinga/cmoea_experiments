/*
 * pf_regulator.hpp
 *
 *  Created on: Aug 21, 2014
 *      Author: joost
 */

#include <modules/nn2/pf.hpp>

#ifndef PF_REGULATOR_HPP_
#define PF_REGULATOR_HPP_

#include <sferes/gen/sampled.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/misc/rand.hpp>
#include <sferes/dbg/dbg.hpp>
#include <limits>

namespace nn
{

enum reg_func { sine = 0, sigmoid, gaussian, linear, regulator };
SFERES_CLASS(PfRegParams)
{
public:
    typedef PfRegParams<Params, Exact> this_t;

    void set(size_t t, size_t i)
    {
        _type.set_data(0, t);
        _index = size_t(i);
    }

    void mutate()
    {
        dbg::trace trace("mutate", DBG_HERE);
        _type.mutate();
        _index=sferes::misc::rand<unsigned long>(std::numeric_limits<unsigned long>::max());
    }
    void random()
    {
        _type.random();
        _index=sferes::misc::rand<unsigned long>(std::numeric_limits<unsigned long>::max());
    }
    void develop() {
    }
    int type() const {
        return _type.data(0);
    }
    size_t index() const {
        return _index;
    }
    template<typename A>
    void serialize(A& ar, unsigned int v)
    {
        ar& BOOST_SERIALIZATION_NVP(_type);
        ar& BOOST_SERIALIZATION_NVP(_index);
    }
    bool operator==(const this_t &other) const {
        return _type == other._type && _index == other._index;
    }

protected:
    sferes::gen::Sampled<1, Params> _type;
    size_t _index;
};

//PfWReg is a subclass of the Type Pf<W> which defines most of its fields
//PfWReg stands for Potential function with Regulatory Activation function,
//and determines the input of a neuron.
template<typename W, typename Params>
struct PfWReg : public Pf<W, Params>
{
    typedef W weight_t;

    //Initializes the _w_cache. Note that this only works after the size of weights is set.
    void init()
    {
        _w_cache.resize(this->_weights.size());
        for (size_t i = 0; i < this->_weights.size(); ++i)
            _w_cache[i] = trait<weight_t>::single_value(this->_weights[i]);
    }

    //The main function.
    //Takes a Eigen::VectorXf of float input values, and if the input size is correct
    //Then (if eigen is enabled, which is should be) returns the dot product of the cache (which should hold the vector of weights)
    //And the input vector. This function is used to calculate the input of a single neuron.
    float operator() (const trait<float>::vector_t & inputs) const
    {
        dbg::assertion(DBG_ASSERTION(inputs.size() == _w_cache.size()));

        if (inputs.size() == 0)
            return 0.0f;

#ifdef EIGEN2_ENABLED
        float result = _w_cache.dot(inputs);
#else
#warning "No eigen2 -> no vectorization of pwfsum"
        float result = (_w_cache * inputs).sum();
#endif
        dbg::out(dbg::info, "pfreg") << "Sum: " << result <<std::endl;

        switch (this->_params.type()){
        case nn::sine:
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
        case nn::sigmoid:
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
        case nn::gaussian:
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
        case nn::linear:
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
        case nn::regulator:
        {
            size_t index = this->_params.index() % inputs.size();
            float regulated_input = _w_cache[index] * inputs[index];
            float other_inputs  = result - regulated_input;
            float bounded_regulated_input = std::min(std::max(regulated_input, -3.0f), 3.0f) / 3.0f;

            dbg::out(dbg::info, "pfreg") << "Type regulator, result: " << result << " regulated_input: " << bounded_regulated_input << " other_inputs: " << other_inputs <<std::endl;
            if(std::abs(other_inputs) < 0.5){
#ifdef JHDEBUG
            if (std::isnan(bounded_regulated_input)){
                std::cerr << "Warning: result is nan" <<std::endl;
                std::cerr << "result: " << bounded_regulated_input << " type: " << this->_params.type() << std::endl;
            }
#endif
                return bounded_regulated_input;
            } else {
                return 0;
            }
            break;
        }
        default:
        {
            dbg::sentinel(DBG_HERE);
        }
        }
        dbg::sentinel(DBG_HERE);
        return 0;
    }
protected:
    //Effectively defines something of type Eigen::VectorXf
    trait<float>::vector_t _w_cache;
};


// A dummy activation function to go with the regulator potential function
// It is a dummy function because the activation function is actually applied in the potential function.
template<typename P>
struct AfRegDummy : public Af<P>
{
    typedef P params_t;
    float operator() (float p) const
    {
        dbg::out(dbg::info, "pfreg") << "Dummy, result: " << p <<std::endl;
#ifdef JHDEBUG
            if (std::isnan(p)){
                std::cerr << "Warning: p is nan" <<std::endl;
                std::cerr << "result: " << p << std::endl;
            }
#endif
        return p;
    }
};
}

template<typename Params, typename Exact>
std::ostream& operator<< (std::ostream &out, nn::PfRegParams<Params, Exact> &params)
{
    out << "type: " << params.type() << ", index: " << params.index() ;
    return out;
}

#endif /* PF_REGULATOR_HPP_ */
