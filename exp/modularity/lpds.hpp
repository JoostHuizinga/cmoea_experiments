#ifndef PF_LPDS_HPP_
#define PF_LPDS_HPP_

#include <modules/nn2/params.hpp>
#include <sferes/misc/rand.hpp>

namespace nn
{
  //see Girard et al., Contracting model of the basal ganglia (2008)
  template<typename Params = nn::params::Vectorf<4> >
  class AfLpds : public Af<Params>
  {
  public:
    static const float dt = 1e-3;
    SFERES_ARRAY(float, tau, 5e-3, 10e-3, 20e-3, 40e-3);
    typedef Params params_t;

    AfLpds() : _a(0) {}
   
    void set_params(const params_t& v) 
    { 
      assert(v.size() == 4);
      this->_params = v;
      float t = this->_params.data(0) == 1 ? 1-1e-5 : this->_params.data(0);
      _tau = tau((int)(t * tau_size()));
      _threshold = (this->_params.data(1) - 0.5) * 10.0f;
      _inhib = this->_params.data(2) > 0.5;
      _alone = this->_params.data(3) > 0.5;
      assert(_tau != 0);
    }
    float operator()(float p)
    { 
      //float noised = p + sferes::misc::rand<float>()*0.1f; 
      assert(_tau != 0);
      _a = std::max(0.0f, std::min(1.0f, _a + (p - _a + _threshold) * dt / _tau));
      return (_inhib ? -_a : _a);
    }
    bool alone() const { return _alone; }
  protected:
    float _a;
    float _tau;
    float _threshold;
    bool _inhib;
    bool _alone;
  };

}

#endif
 
