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


#ifndef GEN_HNN_WORLD_SEED_HPP_
#define GEN_HNN_WORLD_SEED_HPP_

#include <modules/nn2/neuron.hpp>
#include <modules/nn2/pf.hpp>
#include <modules/nn2/trait.hpp>
#include <modules/datatools/position3.hpp>

#include <sferes/dbg/dbg.hpp>

//#include "af_cppn_extended.hpp"



namespace sferes
{
namespace gen
{

class RandomSeed{
public:
    template<typename CppnType>
    static void set_seed(CppnType& cppn){cppn.random();}
};


template<typename CPPNType,
typename Params,
typename ParamsConns>
class HnnWS
{
public:
    typedef CPPNType nn_conns_t;
    typedef nn_conns_t nn_t;
    typedef typename nn_conns_t::graph_t graph_t;
    typedef typename nn_conns_t::vertex_desc_t vertex_desc_t;
    typedef typename nn_conns_t::edge_desc_t edge_desc_t;
    typedef typename nn_conns_t::pf_t pf_t;
    typedef typename nn_conns_t::af_t af_t;
    typedef typename nn_conns_t::weight_t weight_t;



    //typedef typename nn_conns_t::weight_t weight_t;

    enum input_id { x1 = 0, y1 = 1, z1 = 2,  x2 = 3, y2 = 4, z2 = 5, bias = 6};

    void init(){
        dbg::trace trace("hnnws", DBG_HERE);
        _develop(_cppn);
        _cppn.init();
    }


    void random(){
        dbg::trace trace("hnnws", DBG_HERE);
        Params::cppn::seed_t::set_seed(_cppn);
    }

    void mutate()
    {
        dbg::trace trace("mutate", DBG_HERE);
        //Whenever you are mutated you become a new individual, your past lives have no more meaning to you.
        _world_seeds.clear();
        _cppn.mutate();


#ifdef NO_BLOAT
        _cppn.simplify(true);
#endif
    }

    void cross(const HnnWS& o, HnnWS& c1, HnnWS& c2){
        dbg::trace trace("cross", DBG_HERE);
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

    /**
     * Returns the connection value for a substrate connection.
     */
    float conn_value(size_t output, std::vector<float> in){
        dbg::out(dbg::info, "hnn") << "Output requested: " << output <<
        		" outputs available: " << _cppn.get_nb_outputs() << std::endl;
        dbg::assertion(DBG_ASSERTION(output < _cppn.get_nb_outputs()));
#ifdef DBG_ENABLED
        dbg::out(dbg::info, "hnn") << "Input:" << std::endl;
        for(size_t i=0; i<in.size(); ++i){
        	dbg::out(dbg::info, "hnn") << " - " << in[i] << std::endl;
        }
        for(size_t i=0; i<in.size(); ++i){
        	dbg::assertion(DBG_ASSERTION(!std::isnan(in[i])));
        	dbg::assertion(DBG_ASSERTION(!std::isinf(in[i])));
        }
#endif

        if(in != _last_in){
            for (size_t k = 0; k < _cppn.get_depth(); ++k){
                _cppn.step(in);
            }
            _last_in = in;
        }

        float out = _cppn.get_outf(output);

        dbg::out(dbg::info, "hnn") << "Output: " << output <<
        		" depth: " << _cppn.get_depth() <<
				" nr of outputs " << _cppn.get_nb_outputs() << std::endl;
        assert(!std::isinf(out));
        assert(!std::isnan(out));

#ifdef JHDEBUG
        if (std::isnan(out)){
            std::cerr << "********* Warning: out is nan *********" <<std::endl;
            std::cerr << "********* Result: " << out << "*********" << std::endl;
            std::cerr << "********* Output: " << output << " depth: " << _cppn.get_depth() << " nr of outputs " << _cppn.get_nb_outputs() << "*********" << std::endl;
        }
#endif

        // *(Params::hnn::max - Params::hnn::min) + Params::hnn::min;
        //            assert(out >= Params::hnn::min);
        //            assert(out <= Params::hnn::max);
        //            if(out < 0.0 || out > 1.0){
        //          	  std::cout << "out: " << out << std::endl;
        //            }

//        out = (out+1)*0.5;
//        dbg::assertion(DBG_ASSERTION(out >= 0.0));
//        dbg::assertion(DBG_ASSERTION(out <= 1.0));

        return out;
    }


//    float conn_value(float _x1, float _y1, float _z1,
//            float _x2, float _y2, float _z2,
//            size_t output = 0)
//    {
//        assert(_cppn.get_nb_inputs() == 7);
//        assert(output < _cppn.get_nb_outputs());
//
//        std::vector<float> in(7);
//        in[x1] = _x1;
//        in[y1] = _y1;
//        in[z1] = _z1;
//        in[x2] = _x2;
//        in[y2] = _y2;
//        in[z2] = _z2;
//        in[bias] = 1.0;
//
//        return conn_value(output, in);
//    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::out(dbg::info, "serialize") << "Serializing: _cppn" << std::endl;
        ar& BOOST_SERIALIZATION_NVP(_cppn);
        dbg::out(dbg::info, "serialize") << "Serializing: _world_seeds" << std::endl;
        ar& BOOST_SERIALIZATION_NVP(_world_seeds);
    }

    const nn_conns_t& cppn() const{return _cppn;}
    nn_conns_t& cppn(){return _cppn;}

    const nn_t nn() const{return _cppn;}
    nn_t nn(){return _cppn;}

    const nn_t get_nn() const{return _cppn;}
    nn_t get_nn(){return _cppn;}

    void add_world_seed(size_t world_seed){
        _world_seeds.push_back(world_seed);
    }

    size_t get_world_seed(){
        return _world_seeds.back();
    }

    bool world_seed_set(){
        return !_world_seeds.empty();
    }


protected:
    nn_conns_t _cppn;
    std::vector<size_t> _world_seeds;
    std::vector<float> _last_in;

    float _normalizeConnection(float weight){
        return (weight-ParamsConns::parameters::min)  / (ParamsConns::parameters::max - ParamsConns::parameters::min);
    }

    template<typename NN>
    void _develop(NN& nn)
    {
        dbg::trace trace("hnnws", DBG_HERE);
        nn.develop();
//        BGL_FORALL_EDGES_T(e, nn.get_graph(),
//                typename NN::graph_t)
//                  nn.get_graph()[e].get_weight().develop();
//        BGL_FORALL_VERTICES_T(v, nn.get_graph(),
//                typename NN::graph_t)
//        {
//            nn.get_graph()[v].get_pfparams().develop();
//            nn.get_graph()[v].get_afparams().develop();
//        }
    }
};
}
}

#endif
