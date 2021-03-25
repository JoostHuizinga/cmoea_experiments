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

#include <modules/nn2/params.hpp>

#include "phen_hnn_input_functor.hpp"
#include "phen_hnn_ctrnn_two_params.hpp"
#include "phen_hnn_ctrnn_params.hpp"


namespace sferes
{
  namespace phen
  {
    namespace hnn
    {

    struct HnnCtrnnConstants{
        static const size_t weight_output_index = 0;
        static const size_t nb_of_outputs_connection = 1;

        static const size_t bias_output_index = 0;
        static const size_t time_constant_output_index = 1;
        static const size_t nb_of_outputs_neuron = 2;
    };

     // hyperneat-inspired phenotype, based on a cppn
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself>
    class HnnCtrnn : public Indiv<Gen, Fit, Params, typename stc::FindExact<HnnCtrnn<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>
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

        size_t nb_planes() const { return Params::multi_spatial::planes_size(); }
        size_t plane_size(size_t n) const { return Params::multi_spatial::planes(n); }

        size_t get_depth() const { return nb_layers() + 1; }

        size_t connected(size_t source_plane, size_t target_plane){ return Params::multi_spatial::connected(source_plane, target_plane); }



        void develop()
        {
            dbg::trace trace("develop", DBG_HERE);
            //Initializing genotype and clearing vectors.
            int neuronIndex = 0;
            this->gen().init();
            _all_neurons.clear();
            _layers.clear();
            _planes.clear();
            _connected_planes.clear();

            // create the nn
            _nn = nn_t();

            _setPlanes();
            _setNeurons();
            _setConnections();


            dbg::out(dbg::info, "develop") << "Copying coordinates." << std::endl;
            _nn.copy_coords();
            _nn.develop();
        }

        std::vector<std::string> get_weights_vector(){
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
        	//nix
        }

        float compute_length()
        {

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
        	return _nn.compute_optimal_length(threshold, write_svg, filename);
        }

        nn_t simplified_nn()
        {
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
        	_nn.write_dot(ofs);
        }

        void write_dot_cppn(std::ostream& ofs)
        {
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
            _planes.resize(number_of_planes);

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
            size_t current_plane = 0;
            size_t current_neuron = 0;
            dbg::out(dbg::info, "develop") << "Adding input neurons." << std::endl;
            const layer_t inputs = _nn.get_inputs();
            _layers[0].clear();
            BOOST_FOREACH(vertex_desc_t v, inputs){
                _addNode(v, 0, current_neuron, current_plane);
            }

            // Add hidden neurons
            dbg::out(dbg::info, "develop") << "Adding hidden neurons." << std::endl;
            for (size_t i = 1; i < number_of_layers - 1; ++i)
            {
                _layers[i].clear();
                for (size_t j = 0; j < layer_size(i); ++j)
                {
                    vertex_desc_t v = _nn.add_neuron("n" + boost::lexical_cast<std::string>(i) + "-" + boost::lexical_cast<std::string>(j));
                    _addNode(v, i, current_neuron, current_plane);
                }
            }

            // Add output neurons
            dbg::out(dbg::info, "develop") << "Adding output neurons." << std::endl;
            const layer_t outputs = _nn.get_outputs();
            _layers[number_of_layers-1].clear();
            BOOST_FOREACH(vertex_desc_t v, outputs){
                _addNode(v, number_of_layers-1, current_neuron, current_plane);
            }
        }

        /**
         * Sets all connections of the substrate.
         */
        void _setConnections(){
            //Add connections
            dbg::out(dbg::info, "develop") << "Adding connections." << std::endl;
            for (size_t i = 0; i < _connected_planes.size(); i++){
                size_t offset = _connection_offset(i);
                size_t sourcePlaneIndex = _connected_planes[i].first;
                size_t targetPlaneIndex = _connected_planes[i].second;
                layer_t sourcePlane = _planes[sourcePlaneIndex];
                layer_t targetPlane = _planes[targetPlaneIndex];
                _connectPlanes(offset, sourcePlane, targetPlane);
            }
        }


        void _connectPlanes(const size_t& offset, layer_t& sourcePlane, layer_t& targetPlane){
            for(size_t sourceIndex = 0; sourceIndex < sourcePlane.size(); sourceIndex++){
                for(size_t targetIndex = 0; targetIndex < targetPlane.size(); targetIndex++){
                    vertex_desc_t source = sourcePlane[sourceIndex];
                    vertex_desc_t target = targetPlane[targetIndex];
                    _connectVertices(offset, source, target);
                }
            }
        }

        void _connectVertices(const size_t& offset, vertex_desc_t& source, vertex_desc_t& target){
            dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(source)));
            dbg::assertion(DBG_ASSERTION(_nn.vertex_is_member(target)));
            const pos_t& p_s = _coords_map[source];
            const pos_t& p_t = _coords_map[target];
            std::vector<float> in = _inputFunctor.getConnectionInput(p_s, p_t);
            float weight_float = _getWeight(offset, in);
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

        float _getWeight(const size_t& offset, std::vector<float>& in){
            size_t wo_index = offset + HnnCtrnnConstants::weight_output_index;
            float weight_output = this->gen().conn_value(wo_index, in);
#ifdef JHDEBUG
            if (std::isnan(weight_output)){
                std::cerr << "********* Warning: weight is nan *********" <<std::endl;
                std::cerr << "********* Result: " << weight_output << "*********" << std::endl;
            }
#endif

            return weight_output;
        }

        /**
         * Adds a node to the substrate at the correct location.
         */
        void _addNode(vertex_desc_t &v, size_t layer_nr, size_t& current_neuron, size_t& current_plane){
        	assert(current_plane < nb_planes());
        	dbg::out(dbg::info, "develop") << "Adding vertex: " << v << std::endl;

        	size_t offset = _neuron_offset(current_plane);
        	size_t neuronId = _all_neurons.size();
        	_all_neurons.push_back(v);
            _coords_map[v] = pos_t(neuronId);
            std::vector<float> input = _inputFunctor.getBiasInput(_coords_map[v]);

            dbg::out(dbg::info, "develop") <<
                    "Output requested: " << HnnCtrnnConstants::time_constant_output_index + offset <<
                    " outputs available: " << this->gen().cppn().get_nb_outputs() << std::endl;
            assert(HnnCtrnnConstants::time_constant_output_index + offset < this->gen().cppn().get_nb_outputs());

            float bias = this->gen().conn_value(HnnCtrnnConstants::bias_output_index + offset, input);
            float time_constant = this->gen().conn_value(HnnCtrnnConstants::time_constant_output_index + offset, input);

            dbg::out(dbg::info, "develop") << "bias: " << bias << std::endl;
            dbg::out(dbg::info, "develop") << "time_constant: " << time_constant << std::endl;
            assert(!std::isinf(bias));
            assert(!std::isnan(bias));
            assert(!std::isinf(time_constant));
            assert(!std::isnan(time_constant));

            _nn.get_graph()[v].get_afparams().gen().data(0, _scale(bias));
            _nn.get_graph()[v].get_afparams().gen().data(1, _scale(time_constant));
            _nn.get_graph()[v].get_afparams().develop();

//            std::cout << "Bias: " << _nn.get_graph()[v].get_afparams().data(0) << std::endl;
//            std::cout << "Time-constant: " << _nn.get_graph()[v].get_afparams().data(1) << std::endl;

            _layers[layer_nr].push_back(v);
            _planes[current_plane].push_back(v);


            //Switch to next layer if necessary
            current_neuron++;
            if (current_neuron >= plane_size(current_plane)){
            	current_neuron -= plane_size(current_plane);
            	current_plane++;
            }
        }


        size_t _neuron_offset(size_t plane){
            if(Params::multi_spatial::neuron_offsets_size() == 0){
                return plane*HnnCtrnnConstants::nb_of_outputs_neuron;
            } else {
                dbg::out(dbg::info, "develop") << "offset requested: " << plane
                        << " offsets available: " << Params::multi_spatial::neuron_offsets_size() << std::endl;
                dbg::assertion(DBG_ASSERTION(plane < Params::multi_spatial::neuron_offsets_size()));
                return Params::multi_spatial::neuron_offsets(plane);
            }
        }

        size_t _connection_offset(size_t plane){
            if(Params::multi_spatial::connection_offsets_size() == 0){
                return nb_layers()*HnnCtrnnConstants::nb_of_outputs_neuron + plane*HnnCtrnnConstants::nb_of_outputs_connection;
            } else {
                return Params::multi_spatial::connection_offsets(plane);
            }
        }

        float _scale(float& value){
            return af_t::normalize(value); //(value + 1) * 0.5;
        }
    };
  }
}
}
#endif //PHEN_HNN_CTRNN_HPP
