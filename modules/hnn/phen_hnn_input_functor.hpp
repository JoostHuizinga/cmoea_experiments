/*
 * phen_hnn_input_functor.hpp
 *
 *  Created on: Nov 6, 2013
 *      Author: joost
 */

#ifndef PHEN_HNN_INPUT_FUNCTOR_HPP_
#define PHEN_HNN_INPUT_FUNCTOR_HPP_

namespace sferes
{
  namespace phen
  {
    namespace hnn
    {
    template<typename Params>
      class Pos
      {
        public:
          Pos()
          {
          }
          Pos(float x, float y, float z) : _v(x, y, z)
          {
          }
          Pos(size_t neuronId){
          	float x = Params::spatial::x(neuronId);
          	float y = Params::spatial::y(neuronId);
          	float z = Params::spatial::z(neuronId);
            assert(!std::isinf(x));
            assert(!std::isnan(x));
            assert(!std::isinf(y));
            assert(!std::isnan(y));
            assert(!std::isinf(z));
            assert(!std::isnan(z));
          	_v = Eigen::Vector3f(x, y, z);
          }

          float x() const { return _v.x(); }
          float y() const { return _v.y(); }
          float z() const { return _v.z(); }
          float dist(const Pos& p) const { return (_v - p._v).norm(); }
        protected:
          Eigen::Vector3f _v;
      };

      /*
      template <typename P = nn::params::Dummy>
      class HnnInput
      {
      public:
    	  typedef P params_t;
    	  std::vector<float> getBiasInput(hnn::Pos& p_s) const {return this->getBiasInput(p_s);}
    	  std::vector<float> getConnectionInput(size_t neuronId) const {return this->getConnectionInput(neuronId);}
    	  const params_t& get_params() const  { return _params; }
    	  params_t& get_params() { return _params; }
    	  void set_params(const params_t& params) { _params = params; }
    	  void init() {}
    	  HnnInput() {}
      protected:
    	  params_t _params;
      };*/



      template<typename P>
      struct ThreeDInput
      {
     // public:
    	  SFERES_CONST float bias = 1.0;
    	  SFERES_CONST size_t inputSize = 7;
    	  typedef P params_t;
    	  typedef hnn::Pos<P> pos_t;

    	  ThreeDInput(){}

    	  std::vector<float> getBiasInput(pos_t& p_s) {
    		  std::vector<float> in(inputSize);
    		  in[0] = p_s.x();
    		  in[1] = p_s.y();
    		  in[2] = p_s.z();
    		  in[3] = 0;
    		  in[4] = 0;
    		  in[5] = 0;
    		  in[6] = bias;
    		  return in;
    	  }

    	  std::vector<float> getConnectionInput(const pos_t& p_s, const pos_t& p_t) {
    		  std::vector<float> in(inputSize);
    		  in[0] = p_s.x();
    		  in[1] = p_s.y();
    		  in[2] = p_s.z();
    		  in[3] = p_t.x();
    		  in[4] = p_t.y();
    		  in[5] = p_t.z();
    		  in[6] = bias;
    		  return in;
    	  }
      };



      template<typename P>
      struct TwoDInput
      {
     // public:
          SFERES_CONST float bias = 1.0;
          SFERES_CONST size_t inputSize = 5;
          typedef P params_t;
          typedef hnn::Pos<P> pos_t;

          TwoDInput(){}

          std::vector<float> getBiasInput(pos_t& p_s) {
              std::vector<float> in(inputSize);
              in[0] = p_s.x();
              in[1] = p_s.y();
              in[2] = 0;
              in[3] = 0;
              in[4] = bias;
              return in;
          }

          std::vector<float> getConnectionInput(const pos_t& p_s, const pos_t& p_t) {
              std::vector<float> in(inputSize);
              in[0] = p_s.x();
              in[1] = p_s.y();
              in[2] = p_t.x();
              in[3] = p_t.y();
              in[4] = bias;
              return in;
          }
      };

      /**
       * This struct specifies how the HNN class determines what the inputs will be to the CPPN.
       * For example, when the HNN class want to query the CPPN for the connection between neuron A and neuron B
       * the getConnectionInput function will take the positions for neuron A and B, and it expects a vector containing
       * (in this case) 8 values. Usually these values are just the coordinates of the neurons, but it is possible to, for example,
       * use the distance between these neurons as an input.
       *
       * This function also allows you to do things like ignoring the z coordinates or, if you use a different position type,
       * handle 4 dimensional cppn's.
       */
      template<typename P>
      struct DistanceInput
      {
      // public:
      	  SFERES_CONST float bias = 1.0;
      	  SFERES_CONST size_t inputSize = 8;
      	  typedef phen::hnn::Pos<P> pos_t;
      	  typedef P params_t;

      	  DistanceInput(){}

      	  std::vector<float> getBiasInput(const pos_t& p_s) {
      		  std::vector<float> in(inputSize);
      		  in[0] = p_s.x();
      		  in[1] = p_s.y();
      		  in[2] = p_s.z();
      		  in[3] = 0;
      		  in[4] = 0;
      		  in[5] = 0;
      		  in[6] = bias;
      		  in[7] = 0;
      		  return in;
      	  }

      	  std::vector<float> getConnectionInput(const pos_t& p_s, const pos_t& p_t) {
      		  std::vector<float> in(inputSize);
      		  in[0] = p_s.x();
      		  in[1] = p_s.y();
      		  in[2] = p_s.z();
      		  in[3] = p_t.x();
      		  in[4] = p_t.y();
      		  in[5] = p_t.z();
      		  in[6] = bias;
      		  in[7] = p_s.dist(p_t);
      		  return in;
      	  }
      };

    }
  }
}


#endif /* PHEN_HNN_INPUT_FUNCTOR_HPP_ */
