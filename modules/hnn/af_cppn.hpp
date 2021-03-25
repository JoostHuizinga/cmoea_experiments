//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
//|
//| This software is a computer program whose purpose is to facilitate
//| experiments in evolutionary computation and evolutionary robotics.
//|
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.




#ifndef AF_CPPN_HPP_
#define AF_CPPN_HPP_

#include <sferes/gen/sampled.hpp>
#include <sferes/gen/evo_float.hpp>
#include <iostream>
#include <fstream>


// classic activation functions
namespace nn
{
  namespace cppn
  {
    enum func_e { sine = 0, sigmoid, gaussian, linear, usine, usigmoid, ugaussian, ulinear };
    static const size_t nb_functions = 8;
    SFERES_CLASS(AfParams){
      public:
    	typedef AfParams<Params, Exact> this_t;

    	AfParams(){}

    	/**
    	 * Sets the activation function.
    	 *
    	 * @param t A float value representing the activation function.
    	 */
        void set(float t){dbg::trace trace("afparams", DBG_HERE); _type.data(0, t);}

        /**
         * Mutates the activation function.
         *
         * Randomly selects from:
         * - sine
         * - sigmoid
         * - gaussian
         * - linear
         */
        void mutate(){_type.mutate();}

        /**
         * Randomly selects an activation function.
         *
         * Effectively the same as mutate().
         */
        void random(){_type.random();}

        /**
         * Does nothing, the AfParams do not need to be developed.
         */
        void develop() {}

        /**
         * Returns the current activation function as an integer.
         *
         * @return An integer representing the activation function.
         */
        int type() const {return _type.data(0);}

        /**
         * Returns 0.
         *
         * @return Returns 0.0,
         */
//        float param() const {return 0.0;}

        /**
         * Serializes the object, storing the activation function as a float.
         */
        template<typename A>
        void serialize(A& ar, unsigned int v){
          ar& BOOST_SERIALIZATION_NVP(_type);
#ifdef AF_HAS_PARAM
          sferes::gen::EvoFloat<1, Params> param;//todo What is its function?
          ar& BOOST_SERIALIZATION_NVP(param);
#endif

        }

        /**
         * Compares this object with an other object.
         *
         * Two objects are the same if, and only if, they represent the same activation function.
         *
         * @return True if the objects are the same, false otherwise.
         */
        bool operator==(const this_t &other) const {
          return _type == other._type;
        }

      protected:
        sferes::gen::Sampled<1, Params> _type;
    };

  }

   // Activation function for Compositional Pattern Producing Networks
  template<typename P>
  struct AfCppn : public Af<P>{
	  typedef P params_t;

	  /**
	   * Applies the activation function, as indicated by _params.type(),
	   * to the supplied value.
	   *
	   * @param The supplied value, the result of the potential function.
	   * @return The result of the indicated activation function.
	   */
	  float operator() (float p) const
	  {
		  //float s = p > 0 ? 1 : -1;
		  //std::cout<<"type:"<<this->_params.type()<<" p:"<<p<<" this:"<<this
		  //<< " out:"<< p * exp(-powf(p * 10/ this->_params.param(), 2))<<std::endl;
		  float out;
		  switch (this->_params.type())
		  {
		  case cppn::sine:
			  out = sin(p);
			  break;
		  case cppn::sigmoid:
			  out = ((1.0 / (1.0 + exp(-p))) - 0.5) * 2.0;
			  break;
		  case cppn::gaussian:
			  out = exp(-powf(p, 2));
			  break;
		  case cppn::linear:
			  out = std::min(std::max(p, -3.0f), 3.0f) / 3.0f;
			  break;
          case cppn::usine:
              out = (sin(p) + 1) * 0.5;
              break;
          case cppn::usigmoid:
              out = (1.0 / (1.0 + exp(-p)));
              break;
          case cppn::ugaussian:
              out = exp(-powf(p, 2));
              break;
          case cppn::ulinear:
              out = std::min(std::max(p, 0.0f), 3.0f) / 3.0f;
              break;
		  default:
			  out = 0.0f;
			  assert(0);
		  }
		  return out;
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

	  static std::string actToString(int type){
	      switch (type)
	      {
	      case cppn::sine:
	          return "sin";
	          break;
	      case cppn::sigmoid:
	          return "sig";
	          break;
	      case cppn::gaussian:
	          return "gua";
	          break;
	      case cppn::linear:
	          return "lin";
	          break;
	      case cppn::usine:
	          return "usin";
	          break;
	      case cppn::usigmoid:
	          return "usig";
	          break;
	      case cppn::ugaussian:
	          return "ugua";
	          break;
	      case cppn::ulinear:
	          return "ulin";
	          break;
	      default:
	          assert(0);
	      }
	      return "";
	  }
  };

  //template<typename Params = stc::_Params, typename Exact = stc::Itself> class AfParams : public stc::Any<Exact>
}

template<typename Params, typename Exact>
std::ostream& operator<< (std::ostream &out, nn::cppn::AfParams<Params, Exact> &params)
{
    out << "type: " << params.type(); // << ", bias: " << params.param() ;
    return out;
}


#endif
