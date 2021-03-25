/*
 * regulatory_conn_weight.hpp
 *
 *  Created on: Jul 23, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HNN_REGULATORY_CONN_WEIGHT_HPP_
#define MODULES_HNN_REGULATORY_CONN_WEIGHT_HPP_

#include <sferes/gen/sampled.hpp>
#include <sferes/gen/evo_float.hpp>

#include <modules/nn2/trait.hpp>

namespace nn
{

enum reg_conn_type { normal = 0, regulatory, unknown};

// go with eigen with float (TODO : double)
template<>
struct trait<reg_conn_type>
{
  typedef std::valarray<reg_conn_type> vector_t;
  static reg_conn_type zero() { return normal; }
  static reg_conn_type zero(size_t k) { return zero(); }
  static reg_conn_type single_value(const reg_conn_type& t) { return t; }
  static size_t size(const reg_conn_type& t) { return 1; }
};

SFERES_CLASS(RegConnWeight)
{
public:
    typedef RegConnWeight<Params, Exact> this_t;
    typedef float type_t;

    void set(float weight, float type){
        _weight.set_data(0, weight);
        _type.set_data(0, type);
    }

    void mutate(){
        dbg::trace trace("mutate", DBG_HERE);
        _weight.mutate();
        _type.mutate();
    }

    void random(){
        _weight.random();
        _type.random();
    }

    void develop() {}

    reg_conn_type type() const {
        switch((int)_type.data(0)){
        case normal:
            return normal;
        case regulatory:
            return regulatory;
        default:
            dbg::sentinel(DBG_HERE);
        }
        return unknown;
    }

    float weight() const {
        return _weight.data(0);
    }

    float data(const size_t& index) const{
        return _weight.data(index);
    }

    template<typename A>
    void serialize(A& ar, unsigned int v){
        ar& BOOST_SERIALIZATION_NVP(_weight);
        ar& BOOST_SERIALIZATION_NVP(_type);
    }

    bool operator==(const this_t &other) const {
        return _weight== other._weight && _type == other._type;
    }

protected:
    sferes::gen::EvoFloat<1, Params> _weight;
    sferes::gen::Sampled<1, Params> _type;
};

}

template<typename Params, typename Exact>
std::ostream& operator<< (std::ostream &out, nn::RegConnWeight<Params, Exact> &params)
{
    out << "weight: " << params.weight() << ", type: " << params.type() ;
    return out;
}


#endif /* MODULES_HNN_REGULATORY_CONN_WEIGHT_HPP_ */
