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


#ifndef GEN_HNN_HPP_
#define GEN_HNN_HPP_

#include <modules/nn2/neuron.hpp>
#include <modules/nn2/pf.hpp>

#include <exp/modularity/dnn_ff_alt_mut.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
//#include <modules/nn2/af_cppn.hpp>
#include "af_cppn.hpp"
#include <modules/nn2/trait.hpp>

#include <sferes/dbg/dbg.hpp>

namespace sferes
{
  namespace gen
  {
    template<typename W,
             typename Params,
             typename ParamsConns>
    class Hnn
    {
      public:
    	typedef nn::cppn::AfParams<typename Params::cppn> af_params_t;
    	typedef nn::PfWSum<W> pf_params_t;

#ifdef ALT_MUT
    	typedef DnnFFAltMut<nn::Neuron<pf_params_t, nn::AfCppn<af_params_t> >,
    	                      nn::Connection<W>,
    	                      Params> nn_conns_t;
#else
        typedef DnnFF<nn::Neuron<pf_params_t, nn::AfCppn<af_params_t> >,
                      nn::Connection<W>,
                      Params> nn_conns_t;
#endif
        typedef nn_conns_t nn_t;
        typedef typename nn_conns_t::graph_t graph_t;
        typedef typename nn_conns_t::vertex_desc_t vertex_desc_t;
        typedef typename nn_conns_t::pf_t pf_t;
        typedef typename nn_conns_t::af_t af_t;

        //typedef typename nn_conns_t::weight_t weight_t;

        enum input_id { x1 = 0, y1 = 1, z1 = 2,  x2 = 3, y2 = 4, z2 = 5, bias = 6};

        void init()
        {
          _develop(_cppn);
          _cppn.init();
        }


#ifdef LEO
        void random()
        {
        	//Add normal connections
        	_cppn.set_nb_inputs(Params::dnn::nb_inputs);
        	_cppn.set_nb_outputs(Params::dnn::nb_outputs);
        	std::vector<vertex_desc_t> inputs = _cppn.get_inputs();
        	std::vector<vertex_desc_t> outputs = _cppn.get_outputs();
        	BOOST_FOREACH(vertex_desc_t& i, inputs){
        		for (int j=0; j< outputs.size(); j++){
        		//BOOST_FOREACH(vertex_desc_t& o, outputs){
        			if(j != 1){
        				W weight;
        				weight.random();
        				_cppn.add_connection(i, outputs[j], weight);
        			}
        		}
        	}

        	//Add leo bias
        	_cppn.random_neuron_params();
        	typename pf_t::params_t empty;
        	typename af_t::params_t params1;
        	params1.set(nn::cppn::gaussian, 0);
        	vertex_desc_t leo_bias = _cppn.add_neuron("leo_bias", empty, params1);

			W weight1;
			weight1.gen().data(0, _normalizeConnection(Params::leo::input_to_seed1));
        	_cppn.add_connection(inputs[x1], leo_bias, weight1);

        	W weight2;
        	weight2.gen().data(0, _normalizeConnection(Params::leo::input_to_seed2));
        	_cppn.add_connection(inputs[x2], leo_bias, weight2);

        	W weight3;
        	weight3.random();
        	_cppn.add_connection(leo_bias, outputs[1], weight3);

        	W weight4;
        	weight4.gen().data(0, _normalizeConnection(Params::leo::bias_to_leo));
        	_cppn.add_connection(inputs[bias], outputs[1], weight4);

        	typename af_t::params_t params2;
        	params2.set(nn::cppn::sigmoid, 0);
        	_cppn.get_neuron_by_vertex(outputs[1]).set_afparams(params2);
//        	_cppn.make_all_vertices();
        }

#else
        void random()
        {
          _cppn.random();
/*
          for (size_t i = 0; i < Params::hnn::nb_rand_mutations; ++i)
          {
            _cppn.mutate();
          }
          assert(_cppn.get_nb_inputs() == 7);
*/
        }
#endif
        void mutate()
        {
          _cppn.mutate();


#ifdef NO_BLOAT
          _cppn.simplify(true);
#endif
        }

        void cross(const Hnn& o, Hnn& c1, Hnn& c2)
        {
          if (misc::flip_coin())
          {
            c1 = *this;
            c2 = o;
          }
          else
          {
            c2 = *this;
            c1 = o;
          }
        }

        float conn_value(size_t output, std::vector<float> in){
        	dbg::assertion(DBG_ASSERTION(output < _cppn.get_nb_outputs()));

        	if(in != _last_in){
        	    for (size_t k = 0; k < _cppn.get_depth(); ++k){
        	        _cppn.step(in);
        	    }
        	    _last_in = in;
        	}

            float out = _cppn.get_outf(output);
            out = (out+1)*0.5; //*(Params::hnn::max - Params::hnn::min) + Params::hnn::min;

//            assert(out >= Params::hnn::min);
//            assert(out <= Params::hnn::max);
//            if(out < 0.0 || out > 1.0){
//          	  std::cout << "out: " << out << std::endl;
//            }


            assert(out >= 0.0);
            assert(out <= 1.0);
            assert(!std::isinf(out));
            assert(!std::isnan(out));

            return out;
        }

//        float conn_value(float _x1, float _y1, float _z1,
//                         float _x2, float _y2, float _z2,
//                         size_t output = 0)
//        {
//          assert(_cppn.get_nb_inputs() == 7);
//          assert(output < _cppn.get_nb_outputs());
//
//          std::vector<float> in(7);
//          in[x1] = _x1;
//          in[y1] = _y1;
//          in[z1] = _z1;
//          in[x2] = _x2;
//          in[y2] = _y2;
//          in[z2] = _z2;
//          in[bias] = 1.0;
//
//          return conn_value(output, in);
//        }

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
          ar& BOOST_SERIALIZATION_NVP(_cppn);
        }

        const nn_conns_t& get_cppn() const
        {
          return _cppn;
        }

        nn_conns_t& get_cppn()
        {
          return _cppn;
        }

        const nn_conns_t& cppn() const{return _cppn;}
        nn_conns_t& cppn(){return _cppn;}

        const nn_t get_nn() const{
            dbg::trace trace("gen_hnn", DBG_HERE);
        	return _cppn;
        }

        nn_t get_nn(){
            dbg::trace trace("gen_hnn", DBG_HERE);
        	return _cppn;
        }

      protected:
        nn_conns_t _cppn;
        std::vector<float> _last_in;

        float _normalizeConnection(float weight){
        	return (weight-ParamsConns::parameters::min)  / (ParamsConns::parameters::max - ParamsConns::parameters::min);
        }

        template<typename NN>
        void _develop(NN& nn)
        {
            nn.develop();
//          BGL_FORALL_EDGES_T(e, nn.get_graph(),
//                             typename NN::graph_t)
//          nn.get_graph()[e].get_weight().develop();
//          BGL_FORALL_VERTICES_T(v, nn.get_graph(),
//                                typename NN::graph_t)
//          {
//            nn.get_graph()[v].get_pfparams().develop();
//            nn.get_graph()[v].get_afparams().develop();
//          }
        }
    };
  }
}

#endif
