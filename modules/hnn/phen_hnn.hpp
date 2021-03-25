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




#ifndef PHEN_HNN_CTRNN_HPP
#define PHEN_HNN_CTRNN_HPP

#include <map>
#include <utility>
#include <sferes/phen/indiv.hpp>
#include <modules/nn2/nn.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

#include <modules/nn2/params.hpp>

#include "neuron_groups.hpp"
#include "phen_hnn_input_functor.hpp"
#include "phen_hnn_ctrnn_two_params.hpp"
#include "phen_hnn_ctrnn_params.hpp"


namespace stc{
    template <typename Gen, typename Fit, typename Params, typename Exact>
    Exact& exact(sferes::phen::Indiv<Gen, Fit, Params, Exact>& ref)
    {
      return *(Exact*)(void*)(&ref);
    }

    template <typename Gen, typename Fit, typename Params, typename Exact>
    const Exact& exact(const sferes::phen::Indiv<Gen, Fit, Params, Exact>& cref)
    {
      return *(const Exact*)(const void*)(&cref);
    }

    template <typename Gen, typename Fit, typename Params, typename Exact>
    Exact* exact(sferes::phen::Indiv<Gen, Fit, Params, Exact>* ptr)
    {
      return (Exact*)(void*)(ptr);
    }

    template <typename Gen, typename Fit, typename Params, typename Exact>
    const Exact* exact(const sferes::phen::Indiv<Gen, Fit, Params, Exact>* cptr)
    {
      return (const Exact*)(const void*)(cptr);
    }
}

namespace sferes
{
  namespace phen
  {
    namespace hnn
    {

    struct HnnConstants{
        static const size_t weight_output_index = 0;
        static const size_t nb_of_outputs_connection = 1;

        static const size_t bias_output_index = 0;
        static const size_t nb_of_outputs_neuron = 1;
    };

     // hyperneat-inspired phenotype, based on a cppn
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself>
    class Hnn : public Indiv<Gen, Fit, Params, typename stc::FindExact<Hnn<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>
    {
      public:
    	//Type definitions
        typedef Gen gen_t;
        typedef typename gen_t::graph_t cppn_graph_t;
        typedef typename gen_t::nn_t cppn_nn_t;
        typedef typename cppn_nn_t::af_t af_t;

        typedef Substrate nn_t;
        typedef typename nn_t::neuron_t neuron_t;
        typedef typename neuron_t::af_t::params_t neu_bais_t;
        typedef typename nn_t::conn_t connection_t;
        typedef typename connection_t::weight_t con_weight_t;
        typedef typename connection_t::weight_t weight_t;
        typedef typename nn_t::vertex_desc_t vertex_desc_t;
        typedef typename std::vector<vertex_desc_t> layer_t;
        typedef typename nn_t::graph_t graph_t;

        typedef typename Params::dnn::input_t input_t;
        typedef typename input_t::pos_t pos_t;

        size_t nb_layers() const { return _nn.nb_layers(); }
        size_t layer_size(size_t n) const { return _nn.layer_size(n); }

//        size_t nb_planes() const { return Params::multi_spatial::planes_size(); }
//        size_t plane_size(size_t n) const { return Params::multi_spatial::planes(n); }

        size_t get_depth() const { return nb_layers() + 1; }

//        size_t connected(size_t source_plane, size_t target_plane){ return Params::multi_spatial::connected(source_plane, target_plane); }

        static const NeuronGroups& getNeuronGroups(){ return *Params::multi_spatial::substrate_structure;}

        void develop(){
            dbg::trace trace("develop", DBG_HERE);
            dbg::trace trace2("hnn", DBG_HERE);
            //Initializing genotype and clearing vectors.
            int neuronIndex = 0;
            this->gen().init();
            _groups.clear();


            // create the nn
            _nn = nn_t();

            _setNeurons();
            _setConnections();

            dbg::out(dbg::info, "develop") << "Copying coordinates." << std::endl;
            _nn.copy_coords();
            _nn.develop();
        }

        std::vector<std::string> get_weights_vector(){
            dbg::trace trace("hnn", DBG_HERE);
        	std::vector<std::string> result;

        	graph_t network = _nn.get_graph();

        	BGL_FORALL_VERTICES_T(v, network, graph_t)
        	{
        		std::stringstream ss;
        		ss << std::fixed << std::setprecision(5) << std::showpos;

        		ss << nn::trait<neu_bais_t>::single_value(network[v].get_afparams());
        		std::string s = ss.str();
        		result.push_back(s);
        	}
        	BGL_FORALL_EDGES_T(e, network, graph_t)
        	{
        		std::stringstream ss;
        		ss << std::fixed << std::setprecision(5) << std::showpos;
        		ss << nn::trait<con_weight_t>::single_value(network[e].get_weight());
        		std::string s = ss.str();
        		result.push_back(s);
        	}
        	return result;
        }

        void show_(std::ostream& ofs){
            dbg::trace trace("hnn", DBG_HERE);
        	//nix
        }

        float compute_length(){
            dbg::trace trace("hnn", DBG_HERE);

          float length = 0;
          BGL_FORALL_EDGES_T(e, _nn.get_graph(), graph_t)
          {
            vertex_desc_t src = boost::source(e, _nn.get_graph());
            vertex_desc_t tgt = boost::target(e, _nn.get_graph());
            float d = _coords_map[src].dist(_coords_map[tgt]);
            length += d * d;
          }
          return length;
        }

        float compute_optimal_length(float threshold, bool write_svg, std::string filename = "nn_onp.svg"){
            dbg::trace trace("hnn", DBG_HERE);
        	return _nn.compute_optimal_length(threshold, write_svg, filename);
        }

        nn_t simplified_nn(){
            dbg::trace trace("hnn", DBG_HERE);
        	nn_t nn = _nn;
        	// remove not-connected vertices
        	std::vector<vertex_desc_t> to_remove;
        	nn.simplify(); //WARNING: suppress some subgraphs !
        	BGL_FORALL_VERTICES_T(v, nn.get_graph(), graph_t)
        	if (boost::in_degree(v, nn.get_graph())
        	+ boost::out_degree(v, nn.get_graph()) == 0)
        		to_remove.push_back(v);
        	for (size_t i = 0; i < to_remove.size(); ++i)
        		remove_vertex(to_remove[i], nn.get_graph());
        	return nn;
        }

        nn_t& nn() { return _nn; }
        const nn_t& nn() const { return _nn; }

        cppn_nn_t& cppn() { return this->gen().cppn(); }
        const cppn_nn_t& cppn() const { return this->gen().cppn(); }


        // methods
        void write_dot(std::ostream& ofs){
            dbg::trace trace("hnn", DBG_HERE);
        	_nn.write_dot(ofs);
        }

        void write_dot_cppn(std::ostream& ofs){
            dbg::trace trace("hnn", DBG_HERE);
        	cppn_graph_t network = this->gen().cppn().get_graph();
        	ofs << "digraph G {" << std::endl;
        	BGL_FORALL_VERTICES_T(v, network, cppn_graph_t)
        	{
        		ofs << network[v].get_id();
        		ofs << " [label=\""<< network[v].get_id();
        		ofs << " ";
        		ofs << _actToString(network[v].get_afparams().type());
        		ofs <<"\"";
        		if (network[v].is_input() || network[v].is_output())
        			ofs<<" shape=doublecircle";

        		ofs <<"]"<< std::endl;
        	}
        	BGL_FORALL_EDGES_T(e, network, cppn_graph_t)
        	{
        		ofs << network[source(e, network)].get_id()
            		 << " -> " << network[target(e, network)].get_id()
            		 << "[label=\"" << network[e].get_weight() << "\"]" << std::endl;
        	}
        	ofs << "}" << std::endl;
        }

      protected:
        input_t _inputFunctor;
        nn_t _nn;
        std::vector<layer_t> _groups;
        std::map<vertex_desc_t, pos_t> _coords_map;


        /**
         * Adds a node to the substrate at the correct location.
         */
        void _addNode(vertex_desc_t &v, const NeuronGroup& neuronGroup, const std::vector<float>& input){
            dbg::trace trace("hnn", DBG_HERE);
            dbg::out(dbg::info, "develop") << "Adding vertex: " << v << std::endl;

            //Retrieve the bias
            float bias = this->gen().conn_value(HnnConstants::bias_output_index + neuronGroup.offset(), input);

            //Set the data
            _nn.get_graph()[v].get_afparams().gen().data(0, _scale(bias));
            _nn.get_graph()[v].get_afparams().develop();
        }


        /**
         * Sets the correct bias and time-constant values for the inputs, hidden neurons, and outputs.
         */
        void _setNeurons(){
            dbg::trace trace("hnn", DBG_HERE);
            const NeuronGroups& groups = getNeuronGroups();
            _groups.resize(groups.getNbOfNeuronGroups());

            _nn.set_nb_inputs(groups.getNbOfInputs());
            _nn.set_nb_hidden(groups.getNbOfHidden());
            _nn.set_nb_outputs(groups.getNbOfOutputs());

            graph_t& graph = _nn.get_graph();
            NeuronGroupIterator it = groups.getGroupIterator();
            BGL_FORALL_VERTICES_T(v, graph, graph_t){
                pos_t position = pos_t(it.currentNeuron());
                _coords_map[v] = position;
                std::vector<float> input = _inputFunctor.getBiasInput(position);
                _addNode(v, it.next(), input);
                _groups[it.currentGroup()].push_back(v);
            }
        }

        /**
         * Sets all connections of the substrate.
         */
        void _setConnections(){
            dbg::trace trace("hnn", DBG_HERE);
            //Add connections
            dbg::out(dbg::info, "develop") << "Adding connections." << std::endl;
            const NeuronGroups& groups = getNeuronGroups();
            for(size_t i=0; i<groups.getNbOfConnectionGroups(); ++i){
                ConnectionGroup con_group = groups.getConnectionGroup(i);
                layer_t& sourceGroup = _groups[con_group.source()];
                layer_t& targetGroup = _groups[con_group.target()];
                _connectPlanes(con_group.offset(), sourceGroup, targetGroup);
            }
        }


        void _connectPlanes(const size_t& offset, layer_t& sourcePlane, layer_t& targetPlane){
            dbg::trace trace("hnn", DBG_HERE);
            for(size_t sourceIndex = 0; sourceIndex < sourcePlane.size(); sourceIndex++){
                vertex_desc_t source = sourcePlane[sourceIndex];
                for(size_t targetIndex = 0; targetIndex < targetPlane.size(); targetIndex++){
                    vertex_desc_t target = targetPlane[targetIndex];
                    dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(source)));
                    dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(target)));
                    dbg::out(dbg::info, "develop") << "Connecting: " << _nn.get_graph()[source].get_id() <<
                            " with: " << _nn.get_graph()[target].get_id() << std::endl;
                    const pos_t& p_s = _coords_map[source];
                    const pos_t& p_t = _coords_map[target];
                    std::vector<float> in = _inputFunctor.getConnectionInput(p_s, p_t);
                    stc::exact(this)->_connectVertices(offset, source, target, in);
                }
            }
        }

        void _connectVertices(const size_t& offset, vertex_desc_t& source, vertex_desc_t& target, std::vector<float>& in){
            dbg::trace trace("hnn", DBG_HERE);
            float weight_float = _getWeight(offset, in);
            weight_t weight;
            weight.gen().data(0, _scale(weight_float));
            weight.develop();

            dbg::out(dbg::info, "develop") << " weight: " << weight_float << " (raw) " <<
                    nn::trait<weight_t>::single_value(weight) << " (developed)" << std::endl;

            if(fabs(nn::trait<weight_t>::single_value(weight)) > Params::cppn::con_threshold){
                _nn.add_connection(source, target, weight);
            }
        }

        float _getWeight(const size_t& offset, std::vector<float>& in){
            dbg::trace trace("hnn", DBG_HERE);
            size_t wo_index = offset + HnnConstants::weight_output_index;
            float weight_output = this->gen().conn_value(wo_index, in);
#ifdef JHDEBUG
            if (std::isnan(weight_output)){
                std::cerr << "********* Warning: weight is nan *********" <<std::endl;
                std::cerr << "********* Result: " << weight_output << "*********" << std::endl;
            }
#endif
            return weight_output;
        }

        float _scale(float& value){
            dbg::trace trace("hnn", DBG_HERE);
            return af_t::normalize(value); //(value + 1) * 0.5;
        }

        std::string _actToString(int type){
            dbg::trace trace("hnn", DBG_HERE);
            return af_t::actToString(type);
        }
    };
  }
}

}
#endif //PHEN_HNN_CTRNN_HPP
