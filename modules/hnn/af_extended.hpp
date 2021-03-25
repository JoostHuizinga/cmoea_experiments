/*
 * af_extended.hpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_AF_EXTENDED_HPP_
#define MODULES_MISC_AF_EXTENDED_HPP_

#include <modules/nn2/af.hpp>

namespace nn
{
  // -1 to +1 sigmoid
  template <typename P, typename Params>
  struct AfTanhLambda : public Af<P>
  {
    typedef P params_t;
    SFERES_CONST float lambda = Params::lambda;
    AfTanhLambda() { assert(trait<P>::size(this->_params) == 1); }
    float operator()(float p) const
    {
      return tanh(p * lambda + trait<P>::single_value(this->_params));
    }
  protected:
  };

}
#endif /* MODULES_MISC_AF_EXTENDED_HPP_ */
