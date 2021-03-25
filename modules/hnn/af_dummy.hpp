/*
 * af_dummy.hpp
 *
 *  Created on: Jul 23, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HNN_AF_DUMMY_HPP_
#define MODULES_HNN_AF_DUMMY_HPP_

#include <sferes/stc.hpp>
#include <sferes/dbg/dbg.hpp>

#include <modules/nn2/trait.hpp>
#include <modules/nn2/af.hpp>

namespace nn{

SFERES_CLASS(AfDummyParams){
  public:
    typedef AfDummyParams<Params, Exact> this_t;

    /**
     * Sets the activation function.
     *
     * @param t A float value representing the activation function.
     */
//    void set(float t){_type.set_data(0, t);}

    /**
     * Does nothing
     */
    void mutate(){}

    /**
     * Does nothing
     */
    void random(){}

    /**
     * Does nothing
     */
    void develop(){}

    /**
     * Returns 0.
     */
    int type() const {return 0;}

    /**
     * Returns 0.
     *
     * @return Returns 0.0,
     */
//        float param() const {return 0.0;}

    /**
     * Does nothing.
     */
    template<typename A>
    void serialize(A& ar, unsigned int v){}


    /**
     * Returns true;
     */
    bool operator==(const this_t &other) const {
      return true;
    }
};

// A dummy activation function to go with the regulator potential function
// It is a dummy function because the activation function is actually applied in the potential function.
template<typename P = AfDummyParams<> >
struct AfDummy : public Af<P>
{
    typedef P params_t;
    float operator() (float p) const{
        dbg::out(dbg::info, "af") << "Dummy, result: " << p <<std::endl;
#ifdef JHDEBUG
            if (std::isnan(p)){
                std::cerr << "Warning: p is nan" <<std::endl;
                std::cerr << "result: " << p << std::endl;
            }
#endif
        return p;
    }

    /**
     * Normalize function that guarantees that,
     * if the input value was the result of the () operator,
     * the output will be scaled between 0 and 1,
     * with 0 being reserved for the lowest possible value the () operator could return,
     * and 1 being reserved for the highest possible value the () operator could return.
     *
     * @param value Value returned by the () operator.
     * @return Float, scaled to lie within [0, 1].
     */
    static float normalize(float value){
        float result = (value + 1) * 0.5;
        dbg::assertion(DBG_ASSERTION(result >= 0.0f));
        dbg::assertion(DBG_ASSERTION(result <= 1.0f));
        return result;
    }
};

}

namespace sferes{

template<typename Params, typename Exact>
std::ostream& operator<< (std::ostream &out, const nn::AfDummyParams<Params, Exact> &params){
    out << "dummy params";
    return out;
}

}
#endif /* MODULES_HNN_AF_DUMMY_HPP_ */
