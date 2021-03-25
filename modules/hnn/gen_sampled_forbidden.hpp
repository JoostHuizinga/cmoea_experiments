/*
 * gen_sampled_forbidden.hpp
 *
 *  Created on: Oct 10, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HNN_GEN_SAMPLED_FORBIDDEN_HPP_
#define MODULES_HNN_GEN_SAMPLED_FORBIDDEN_HPP_

#include <sferes/gen/sampled.hpp>

namespace sferes
{
namespace gen
{
template<int Size, typename Params, typename Exact = stc::Itself>
class SampledForbidden : public Sampled<Size, Params, typename stc::FindExact<SampledForbidden<Size, Params, Exact>, Exact>::ret>
{
public:
    typedef Params params_t;
    typedef SampledForbidden<Size, Params, Exact> this_t;
    typedef typename Params::sampled::values_t values_t;

    void mutate(){
        BOOST_FOREACH(size_t & v, this->_data){
            if (misc::rand<float>() < Params::sampled::mutation_rate){
                v = Params::sampled::allowed_values(misc::rand<size_t>(0, Params::sampled::allowed_values_size()));
            }
        }
        this->_check_invariant();
    }

    void random()
    {
        BOOST_FOREACH(size_t & v, this->_data){
                v = Params::sampled::allowed_values(misc::rand<size_t>(0, Params::sampled::allowed_values_size()));
        }
        this->_check_invariant();
    }

    bool operator==(const this_t &other) const {
        for(size_t i =0; i<this->_data.size(); i++){
            if(!(this->_data[i] == other._data[i])) return false;
        }
        for(size_t i =0; i<this->_possible_values.size(); i++){
            if(!(this->_possible_values[i] == other._possible_values[i])) return false;
        }
        return true;
    }
};

} // gen
} // sferes


#endif /* MODULES_HNN_GEN_SAMPLED_FORBIDDEN_HPP_ */
