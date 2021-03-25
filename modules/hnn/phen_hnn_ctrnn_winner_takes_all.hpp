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

#ifndef PHEN_HNN_CTRNN_WINNER_TAKES_ALL_HPP
#define PHEN_HNN_CTRNN_WINNER_TAKES_ALL_HPP

//Standard includes
#include <map>
#include <utility>

//Sferes includes
#include <sferes/phen/indiv.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/stc.hpp>

//Module includes
#include <modules/nn2/nn.hpp>
#include <modules/nn2/params.hpp>

//Local includes
//#include "gen_hnn.hpp"
#include "phen_hnn_input_functor.hpp"
#include "phen_hnn_ctrnn_two_params.hpp"
#include "phen_hnn_ctrnn_params.hpp"


namespace sferes
{
  namespace phen
  {
    namespace hnn
    {

    struct HnnCtrnnWinnerTakesAllConstants{
        static const size_t weight_output_index = 0;
        static const size_t bias_output_index = 1;
        static const size_t time_constant_output_index = 2;
        static const size_t expression_output_index = 3;
        static const size_t nb_of_cppn_outputs = 4;
        static const size_t nb_of_outputs_connection = 2;
        static const size_t nb_of_outputs_neuron = 2;
    };

     // hyperneat-inspired phenotype, based on a cppn
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself>
    class HnnCtrnnWinnerTakesAll : public Indiv<Gen, Fit, Params, typename stc::FindExact<HnnCtrnnWinnerTakesAll<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>
    {
      public:
    	//Type definitions
        typedef Gen gen_t;
        typedef typename gen_t::graph_t cppn_graph_t;
        typedef typename gen_t::nn_t cppn_nn_t;

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
        typedef HnnCtrnnWinnerTakesAllConstants const_t;

        typedef typename stc::FindExact<HnnCtrnnWinnerTakesAll<Substrate, Gen, Fit, Params, Exact>, Exact>::ret exact_t;

        size_t nb_layers() const { return _nn.nb_layers(); }
        size_t layer_size(size_t n) const { return _nn.layer_size(n); }

        size_t nb_planes() const { return Params::multi_spatial::planes_size(); }
        size_t plane_size(size_t n) const { return Params::multi_spatial::planes(n); }

        size_t get_depth() const { return nb_layers() + 1; }

        size_t connected(size_t source_plane, size_t target_plane){ return Params::multi_spatial::connected(source_plane, target_plane); }
        size_t getNbOfOutputSets(){return Params::cppn::nb_output_sets;}
        float getExpressionThreshold(){return Params::cppn::leo_threshold;}


        void develop()
        {
            dbg::trace trace("develop", DBG_HERE);
            dbg::out(dbg::info, "develop") << "Outputs required: " << (getNbOfOutputSets() * const_t::nb_of_cppn_outputs) << std::endl;
            dbg::out(dbg::info, "develop") << "Outputs present: " << (this->gen().cppn().get_nb_outputs()) << std::endl;
            dbg::assertion(DBG_ASSERTION((getNbOfOutputSets() * const_t::nb_of_cppn_outputs) == this->gen().cppn().get_nb_outputs()));

            //Initializing genotype and clearing vectors.
            this->gen().init();
            _all_neurons.clear();
            _layers.clear();
            _planes.clear();
            _connected_planes.clear();
            _used_output_sets.resize(getNbOfOutputSets());
            for(size_t i=0; i<_used_output_sets.size(); ++i) _used_output_sets[i] = 0;

            // create the nn
            _nn = nn_t();

            _setPlanes();
            _setNeurons();
            _setConnections();

            dbg::out(dbg::info, "develop") << "Copying coordinates." << std::endl;
            _nn.copy_coords();
            _nn.develop();

#ifdef JHDEBUG
            for(size_t i=0; i<_used_output_sets.size(); ++i){
                std::cout << "Output set: " << i << " used in " << _used_output_sets[i] << " cases." << std::endl;
            }
#endif
        }



        std::vector<std::string> get_weights_vector(){
            dbg::trace trace("develop", DBG_HERE);
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

        void show_(std::ostream& ofs)
        {
            dbg::trace trace("develop", DBG_HERE);
        	//nix
        }

        float compute_length()
        {
            dbg::trace trace("develop", DBG_HERE);
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

        float compute_optimal_length(float threshold, bool write_svg, std::string filename = "nn_onp.svg")
        {
            dbg::trace trace("develop", DBG_HERE);
        	return _nn.compute_optimal_length(threshold, write_svg, filename);
        }

        nn_t simplified_nn()
        {
            dbg::trace trace("develop", DBG_HERE);
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


        const std::vector<typename nn_t::vertex_desc_t>& all_neurons() const { return _all_neurons; }

        // methods
        void write_dot(std::ostream& ofs)
        {
            dbg::trace trace("develop", DBG_HERE);
        	_nn.write_dot(ofs);
        }

        void write_dot_cppn(std::ostream& ofs)
        {
            dbg::trace trace("develop", DBG_HERE);
        	cppn_graph_t network = this->gen().cppn().get_graph();
        	ofs << "digraph G {" << std::endl;
        	BGL_FORALL_VERTICES_T(v, network, cppn_graph_t)
        	{
        		ofs << network[v].get_id();
        		ofs << " [label=\""<< network[v].get_id();
        		ofs << " "; //<< network[v].get_afparams().param();
      //  		ofs << "| ";
        		switch (network[v].get_afparams().type())
        		{
        		case nn::cppn::sine:
        			ofs << "sin";
        			break;
        		case nn::cppn::sigmoid:
        			ofs << "sig";
        			break;
        		case nn::cppn::gaussian:
        			ofs << "gau";
        			break;
        		case nn::cppn::linear:
        			ofs << "lin";
        			break;
        		default:
        			assert(0);
        		}
        		ofs <<"\"";
//        		ofs << "| pf"<< network[v].get_pfparams()[0] <<"\"";
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
        std::vector<layer_t> _layers;
        std::vector<layer_t> _planes;
        std::vector<std::pair<size_t, size_t> > _connected_planes;
        std::vector<typename nn_t::vertex_desc_t> _all_neurons;
        std::map<vertex_desc_t, pos_t> _coords_map;

        std::vector<int> _used_output_sets;

        /**
         * Builds a vector of connected planes by iterating over all planes,
         * querying whether the two planes are connected,
         * and adding the source-target plane pair to the _connect_planes vector.
         */
        void _setPlanes(){
            dbg::trace trace("develop", DBG_HERE);

            //Setup planes
            dbg::out(dbg::info, "develop") << "Setting up connected planes." << std::endl;
            size_t number_of_planes = nb_planes();
            _planes.resize(1);
//            _planes.resize(number_of_planes);

            for (size_t source_plane = 0; source_plane < number_of_planes; ++source_plane){
                for (size_t target_plane = 0; target_plane < number_of_planes; ++target_plane){
                    dbg::out(dbg::info, "develop") <<
                            "Plane " << source_plane <<
                            " to " << target_plane <<
                            " connected " << connected(source_plane, target_plane) << std::endl;
                    if(connected(source_plane, target_plane)){
                        _connected_planes.push_back(std::pair<size_t, size_t>(source_plane, target_plane));
                    }
                }
            }
        }


        /**
         * Sets the correct bias and time-constant values for the inputs, hidden neurons, and outputs.
         */
        void _setNeurons(){
            dbg::trace trace("develop", DBG_HERE);
            size_t number_of_layers = nb_layers();
            size_t nb_inputs = layer_size(0);
            size_t nb_outputs = layer_size(number_of_layers-1);

            dbg::out(dbg::info, "develop") <<
                    "number_of_layers: " << number_of_layers <<
                    " nb_inputs: " << nb_inputs <<
                    " nb_outputs: " << nb_outputs << std::endl;

            _layers.resize(number_of_layers);
            _nn.set_nb_inputs(nb_inputs);
            _nn.set_nb_outputs(nb_outputs);


            // Add input neurons
            dbg::out(dbg::info, "develop") << "Adding input neurons." << std::endl;
            const layer_t inputs = _nn.get_inputs();
            _layers[0].clear();
            BOOST_FOREACH(vertex_desc_t v, inputs){
                _addNode(v, 0);
            }

            // Add hidden neurons
            dbg::out(dbg::info, "develop") << "Adding hidden neurons." << std::endl;
            for (size_t i = 1; i < number_of_layers - 1; ++i){
                _layers[i].clear();
                for (size_t j = 0; j < layer_size(i); ++j){
                    vertex_desc_t v = _nn.add_neuron("n" + boost::lexical_cast<std::string>(i) + "-" + boost::lexical_cast<std::string>(j));
                    _addNode(v, i);
                }
            }

            // Add output neurons
            dbg::out(dbg::info, "develop") << "Adding output neurons." << std::endl;
            const layer_t outputs = _nn.get_outputs();
            _layers[number_of_layers-1].clear();
            BOOST_FOREACH(vertex_desc_t v, outputs){
                _addNode(v, number_of_layers-1);
            }
        }

        /**
         * Sets all connections of the substrate.
         */
        void _setConnections(){
            dbg::trace trace("develop", DBG_HERE);
            //Add connections
            dbg::out(dbg::info, "develop") << "Adding connections." << std::endl;
            for (size_t i = 0; i < _connected_planes.size(); i++){
//                size_t offset = _connection_offset(i);
                size_t sourcePlaneIndex = _connected_planes[i].first;
                size_t targetPlaneIndex = _connected_planes[i].second;
                layer_t sourcePlane = _planes[sourcePlaneIndex];
                layer_t targetPlane = _planes[targetPlaneIndex];
                _connectPlanes(sourcePlane, targetPlane);
            }
        }


        void _connectPlanes(layer_t& sourcePlane, layer_t& targetPlane){
            dbg::trace trace("develop", DBG_HERE);
            for(size_t sourceIndex = 0; sourceIndex < sourcePlane.size(); sourceIndex++){
                for(size_t targetIndex = 0; targetIndex < targetPlane.size(); targetIndex++){
                    vertex_desc_t source = sourcePlane[sourceIndex];
                    vertex_desc_t target = targetPlane[targetIndex];
                    _connectVertices(source, target);
                }
            }
        }

        void _connectVertices(vertex_desc_t& source, vertex_desc_t& target){
            dbg::trace trace("develop", DBG_HERE);
            dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(source)));
            dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(target)));
            const pos_t& p_s = _coords_map[source];
            const pos_t& p_t = _coords_map[target];
            std::vector<float> in = _inputFunctor.getConnectionInput(p_s, p_t);
            float weight_float = ((exact_t*) this)->_getWeight(in);
            weight_t weight;
            weight.gen().data(0, _scale(weight_float));
            weight.develop();

            dbg::out(dbg::info, "develop") << "Weight before develop: " << weight_float << std::endl;
            dbg::out(dbg::info, "develop") << "Weight after develop: " << nn::trait<weight_t>::single_value(weight) << std::endl;

            if(nn::trait<weight_t>::single_value(weight) != 0.0f){
                dbg::out(dbg::info, "develop") <<
//                        "Adding connection from: " << sourceIndex <<
//                        " plane: " << sourcePlaneIndex <<
                        " address: " << source <<
//                        " to: " << targetIndex <<
//                        " plane: " << targetPlaneIndex <<
                        " address: " << target <<
                        " weight: " << weight_float << std::endl;
                _nn.add_connection(source, target, weight);
            }

        }



        /**
         * Adds a node to the substrate at the correct location.
         */
        void _addNode(vertex_desc_t &v, size_t layer_nr){
            dbg::trace trace("develop", DBG_HERE);
            dbg::out(dbg::info, "develop") << "Adding vertex: " << v << std::endl;

            size_t neuronId = _all_neurons.size();
            pos_t node_position = pos_t(neuronId);
            _coords_map[v] = node_position;
            std::vector<float> input = _inputFunctor.getBiasInput(node_position);
            std::pair<float, float> bias_and_timeconstant = ((exact_t*) this)->_getBiasAndTimeconstant(input);

            dbg::out(dbg::info, "develop") <<
                    "Before develop - bias: " << bias_and_timeconstant.first <<
                    " time-constant: " << bias_and_timeconstant.second << std::endl;
            _nn.get_graph()[v].get_afparams().gen().data(0, _scale(bias_and_timeconstant.first));
            _nn.get_graph()[v].get_afparams().gen().data(1, _scale(bias_and_timeconstant.second));
            _nn.get_graph()[v].get_afparams().develop();
            dbg::out(dbg::info, "develop") <<
                    "After develop - bias: " << _nn.get_graph()[v].get_afparams().gen().data(0) <<
                    " time-constant: " << _nn.get_graph()[v].get_afparams().gen().data(1) << std::endl;
            _all_neurons.push_back(v);

            dbg::assertion(DBG_ASSERTION(layer_nr < _layers.size()));
            _layers[layer_nr].push_back(v);

            size_t current_plane = _planes.size() - 1;
            dbg::assertion(DBG_ASSERTION(current_plane < nb_planes()));
            if(_planes.back().size() >= plane_size(current_plane)){
                _planes.push_back(layer_t());
            }
            _planes.back().push_back(v);
        }



        float _getWeight(std::vector<float>& input){
            dbg::trace trace("develop", DBG_HERE);
            float weight_output = 0;
            size_t nb_of_outputsets = getNbOfOutputSets();
            float max_expression = getExpressionThreshold();

            size_t winner = 0;
            for(size_t output_set = 0; output_set < nb_of_outputsets; ++output_set){
                size_t offset = output_set * const_t::nb_of_cppn_outputs;
                dbg::out(dbg::info, "develop") << "Connection expression output queried: " << const_t::expression_output_index + offset << std::endl;
                float expression = this->gen().conn_value(const_t::expression_output_index + offset, input);
                dbg::out(dbg::info, "develop") << "Connection expression: " << expression << std::endl;
                if(expression > max_expression){
                    max_expression = expression;
                    winner = output_set;
                    dbg::out(dbg::info, "develop") << "Weight output queried: " << const_t::weight_output_index + offset << std::endl;
                    weight_output = this->gen().conn_value(const_t::weight_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Weight output: " << weight_output << std::endl;
#ifdef JHDEBUG
                    if (std::isnan(weight_output)){
                        std::cerr << "********* Warning: weight is nan *********" <<std::endl;
                        std::cerr << "********* Result: " << weight_output << "*********" << std::endl;
                    }
#endif
                }
            }
            _used_output_sets[winner]++;

            return weight_output;
        }



        std::pair<float, float> _getBiasAndTimeconstant(std::vector<float>& input){
            dbg::trace trace("develop", DBG_HERE);
            std::pair<float, float> result(0.0, 0.0);
            size_t nb_of_outputsets = getNbOfOutputSets();
            float max_expression = getExpressionThreshold();

            size_t winner = 0;
            for(size_t output_set = 0; output_set < nb_of_outputsets; ++output_set){
                size_t offset = output_set * const_t::nb_of_cppn_outputs;
                dbg::out(dbg::info, "develop") << "Node expression output queried: " << const_t::expression_output_index + offset << std::endl;
                float expression = this->gen().conn_value(const_t::expression_output_index + offset, input);
                dbg::out(dbg::info, "develop") << "Node expression: " << expression << std::endl;
                if(expression > max_expression){
                    max_expression = expression;
                    winner = output_set;
                    dbg::out(dbg::info, "develop") << "Bias output queried: " << const_t::bias_output_index + offset << std::endl;
                    result.first = this->gen().conn_value(const_t::bias_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Bias output: " << result.first << std::endl;

                    dbg::out(dbg::info, "develop") << "Time constant output queried: " << const_t::time_constant_output_index + offset << std::endl;
                    result.second = this->gen().conn_value(const_t::time_constant_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Time constant output: " << result.second << std::endl;
                }
            }
            _used_output_sets[winner]++;

            return result;
        }


//        size_t _neuron_offset(size_t plane){
//            if(Params::multi_spatial::neuron_offsets_size() == 0){
//                return plane*nb_of_outputs_neuron;
//            } else {
//                dbg::out(dbg::info, "develop") << "offset requested: " << plane
//                        << " offsets available: " << Params::multi_spatial::neuron_offsets_size() << std::endl;
//                dbg::assertion(DBG_ASSERTION(plane < Params::multi_spatial::neuron_offsets_size()));
//                return Params::multi_spatial::neuron_offsets(plane);
//            }
//        }
//
//        size_t _connection_offset(size_t plane){
//            if(Params::multi_spatial::connection_offsets_size() == 0){
//                return nb_layers()*nb_of_outputs_neuron + plane*nb_of_outputs_neuron;
//            } else {
//                return Params::multi_spatial::connection_offsets(plane);
//            }
//        }

        float _scale(float& value){
            return (value + 1) * 0.5;
        }
    };
  }
}
}
#endif //PHEN_HNN_CTRNN_HPP
