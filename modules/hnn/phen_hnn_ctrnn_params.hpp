/*
 * phen_hnn_ctrnn_params.hpp
 *
 *  Created on: Nov 6, 2013
 *      Author: joost
 */

#ifndef PHEN_HNN_CTRNN_PARAMS_HPP_
#define PHEN_HNN_CTRNN_PARAMS_HPP_

#include <vector>
#include <sferes/phen/indiv.hpp>
#include <boost/foreach.hpp>



namespace sferes
{

  namespace ctrnn_phen
  {
    SFERES_INDIV(ParametersCTRNN, phen::Indiv){

    template<typename G, typename F, typename P, typename E>
      friend std::ostream& operator<<(std::ostream& output, const ParametersCTRNN< G, F, P, E >& e);

    public:
      typedef ParametersCTRNN<Gen, Fit, Params, Exact> this_t;

#ifdef EIGEN_CORE_H
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW
#endif
      //The constructor, sets the size of params
      //The size is passed by the genotype class, set in the mains experiment.
      ParametersCTRNN() : _params((*this)._gen.size()) { }

      //Type is float
      typedef float type_t;

      //Develop function
      void develop()
      {
    	  this->_gen.develop();
    	  for (unsigned i = 0; i < _params.size(); ++i)
    		  _params[i] = this->_gen.data(i);
      }

      //Returns the value at index i
      float data(size_t i) const {
    	  assert(i < size());
    	  return _params[i];
      }

      //Returns the length of the _params vector.
      size_t size() const { return _params.size(); }

      //Returns the _params vector.
      const std::vector<float>& data() const { return _params; }

      // squared Euclidean distance between these parameters and the parameters in params.
      float dist(const ParametersCTRNN& params) const
      {
    	  assert(params.size() == size());
    	  float d = 0.0f;
    	  for (size_t i = 0; i < _params.size(); ++i)
    	  {
    		  float x = _params[i] - params._params[i];
    		  d += x * x;
    	  }
    	  return d;
      }

      //Print the values in _params to the output stream os
      void show(std::ostream& os) const
      {
    	  BOOST_FOREACH(float p, _params)
			  os<<p<<" ";
    	  os<<std::endl;
      }

      bool operator==(const this_t &other) const {
      	for(size_t i =0; i<_params.size(); i++){
      		if(!(_params[i] == other._params[i])) return false;
      	}
      	return true;
      }

    protected:
      std::vector<float> _params;
    };

    template<typename G, typename F, typename P, typename E>
    std::ostream& operator<<(std::ostream& output, const ParametersCTRNN< G, F, P, E >& e) {
      for (size_t i = 0; i < e.size();++i)
        output <<" "<<e.data(i) ;
      return output;
    }
  }

}



#endif /* PHEN_HNN_CTRNN_PARAMS_HPP_ */
