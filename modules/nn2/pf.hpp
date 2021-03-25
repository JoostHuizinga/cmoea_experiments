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


#ifndef NN_PF_HPP
#define NN_PF_HPP

#include <cstdlib>
#include <sferes/misc/rand.hpp>
#include <sferes/dbg/dbg.hpp>

#include "params.hpp"
#include "trait.hpp"

// potential functions (weighted sum, leaky integrator, etc.)
namespace nn
{
  template<typename W = float, typename P = params::Dummy>
  class Pf
  {
    public:
      typedef P params_t;
      typedef W weight_t;

      //Getters and setters for parameters in both constant and non-constant versions.
      const params_t& get_params() const { return _params; }
      params_t& get_params() { return _params; }
      void set_params(const params_t& params) { _params = params; }

      //Init function to be overwritten (does nothing)
      void init() {}

      // weights are stored in the pf function for efficiency reasons

      //Sets the size of weights to n
      void set_nb_weights(size_t n) { _weights.resize(n); }

      //Sets the weight at index i to value w.
      void set_weight(size_t i, const W& w) {
    	  assert(i < _weights.size());
    	  _weights[i] = w;
      }

      //Returns the _weights valarray.
      const std::valarray<W>& get_weights() const { return _weights; }

       // main function, to be overwritten
      //Takes a vector of inputs (type is usually valarray<IO>) and returns 0.
      template<typename IO>
      float operator() (const typename trait<IO>::vector_t & inputs) const { return 0.0f; }


    protected:
      params_t _params;
       // cache weights
      std::valarray<W> _weights;
  };

  //PfWSum is a subclass of the Type Pf<W> which defines most of its fields
  //PfWSum stands for Potential function Weighted Sum, and determines the input of a neuron.
  template<typename W = float>
  struct PfWSum : public Pf<W>
  {
    typedef params::Dummy params_t;
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
      assert(inputs.size() == _w_cache.size());
       //std::cout<<"in:"<<inputs.transpose()<<" w:"<<_w_cache.transpose()<<"=>"<<
       //_w_cache.dot(inputs)<<std::endl;
      if (inputs.size() == 0)
        return 0.0f;
#ifdef EIGEN3_ENABLED
//      float result = _w_cache.dot(inputs);
//      dbg::out(dbg::info, "pfsum") << "In: " << eigen_to_hex(inputs) << " out: " << type_to_hex(result) << std::endl;
      return _w_cache.dot(inputs);
#else
 #warning "No eigen2 -> no vectorization of pwfsum"
      return (_w_cache * inputs).sum();
#endif
    }
    protected:
    	//Effectively defines something of type Eigen::VectorXf
    	trait<float>::vector_t _w_cache;
  };



   // Ijsspert's coupled non-linear oscillators
   // see : Learning to Move in Modular Robots using Central Pattern
   // Generators and Online Optimization, 2008
   // Main parameters:
   // - phi_i: phase lag
   // - r_i: amplitude
   // - x_i: offset
   // - _omega: frequency
  template<typename P>
  struct PfIjspeert : public Pf<std::pair<float, float>, P>
  {
    typedef std::pair<float, float> weight_t;
    typedef P params_t;
    BOOST_STATIC_CONSTEXPR float dt = 0.01;
    BOOST_STATIC_CONSTEXPR float a_r = 20.0f;
    BOOST_STATIC_CONSTEXPR float a_x = 20.0f;
    void set_r(float r) { _r = r; }
    void set_x(float x) { _x = x; }
    void set_omega(float o) { _omega = o; }
    float get_theta_i() const { return _theta_i; }
    void init()
    {
      _phi_i = 0; //sferes::misc::rand<float>();
      _r_i = 0; //sferes::misc::rand<float>();
      _x_i = 0; //sferes::misc::rand<float>();
      _theta_i = 0;
      _phi_i_d = 0; _r_i_d = 0; _x_i_d = 0;
    }
     // depends on r_j and phi_j
    weight_t operator() (const trait<weight_t>::vector_t & inputs)
    {
      _phi_i_d = _omega;
      for (size_t j = 0; j < inputs.size(); ++j)
      {
        float r_j = inputs[j].first;
        float phi_j = inputs[j].second;
        float w_ij = this->_weights[j].first;
        float phi_ij = this->_weights[j].second;
        std::cout << "phi_ij:" << phi_ij << " " << inputs.size() << std::endl;
        _phi_i_d += w_ij * r_j * sin(phi_j - phi_ij - _phi_i);
      }
      float r_i_dd = a_r * (a_r / 4 * (_r - _r_i) - _r_i_d);
      float x_i_dd = a_r * (a_r / 4 * (_x - _x_i) - _x_i_d);

       // integrate
      _r_i_d += r_i_dd * dt;
      _r_i += _r_i_d * dt;
      _x_i_d += x_i_dd * dt;
      _x_i += _x_i_d * dt;
      _phi_i += _phi_i_d * dt;

       // result
      _theta_i = _x_i + _r_i *cos(_phi_i);
      return std::make_pair(_r_i, _phi_i);
    }
    private:
       // states
      float _phi_i, _r_i, _theta_i, _x_i;
       // states dot
      float _phi_i_d, _r_i_d, _x_i_d;
       // parameters
      float _r, _x, _omega;
  };

}

#endif
