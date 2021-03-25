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




#ifndef PHEN_HNN_HPP
#define PHEN_HNN_HPP

#include <map>
#include <sferes/phen/indiv.hpp>
#include <modules/nn2/nn.hpp>

#include <modules/nn2/params.hpp>
#include "gen_hnn.hpp"
#include "phen_hnn_input_functor.hpp"

namespace sferes
{
  namespace phen
  {
    namespace hnn
    {

     // hyperneat-inspired phenotype, based on a cppn
    //SFERES_INDIV(Hnn, Indiv)
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself> \
    	      class Hnn : public Indiv<Gen, Fit, Params, typename stc::FindExact<Hnn<Gen, Fit, Params, Exact>, Exact>::ret>
    {
      public:
        typedef Gen gen_t;
        typedef Substrate nn_t;
        typedef typename Substrate::neuron_t neuron_t;
        typedef typename Substrate::conn_t connection_t;
        typedef typename Params::dnn::input_t input_t;
        typedef typename connection_t::weight_t con_weight_t;
        typedef typename neuron_t::af_t::params_t neu_bais_t;
        typedef typename input_t::pos_t pos_t;
        typedef typename nn_t::vertex_desc_t vertex_desc_t;
        typedef typename nn_t::graph_t graph_t;
        typedef typename std::vector<vertex_desc_t> layer_t;
        typedef typename Gen::graph_t cppn_graph_t;

        size_t nb_layers() const { return _nn.nb_layers(); }
        size_t layer_size(size_t n) const { return _nn.layer_size(n); }
        size_t get_depth() const { return nb_layers() + 1; }

        void develop()
        {
          //Initializing genotype and clearing vectors.
          int neuronIndex = 0;
          this->gen().init();
          _all_neurons.clear();
          _layers.clear();

          // create the nn
          _nn = nn_t();
          size_t number_of_layers = nb_layers();
          size_t nb_inputs = layer_size(0);
          size_t nb_outputs = layer_size(number_of_layers-1);

          _layers.resize(number_of_layers);
          _nn.set_nb_inputs(nb_inputs);
          _nn.set_nb_outputs(nb_outputs);

          // Add input neurons
          const layer_t inputs = _nn.get_inputs();
          _layers[0].clear();
          BOOST_FOREACH(vertex_desc_t v, inputs){
        	  addNode(v, 0);
          }

           // Add hidden neurons
          for (size_t i = 1; i < number_of_layers - 1; ++i)
          {
        	  _layers[i].clear();
        	  for (size_t j = 0; j < layer_size(i); ++j)
        	  {
        		  vertex_desc_t v = _nn.add_neuron("n" + boost::lexical_cast<std::string>(i) + "-" + boost::lexical_cast<std::string>(j));
        		  addNode(v, i);
        	  }
          }

          // Add output neurons
          const layer_t outputs = _nn.get_outputs();
          _layers[number_of_layers-1].clear();
          BOOST_FOREACH(vertex_desc_t v, outputs){
        	  addNode(v, number_of_layers-1);
          }

          //Add connections

          //For each layer except the last
          for (size_t i = 0; i < _layers.size() - 1; ++i){
        	  //For each neuron in the current layer
        	  for (size_t s = 0; s < _layers[i].size(); ++s){
        		  //For each neuron in the next layer
        		  for (size_t t = 0; t < _layers[i + 1].size(); ++t)
        		  {
        			  typename connection_t::weight_t weight = typename connection_t::weight_t();
        			  const pos_t& p_s = _coords_map[_layers[i][s]];
        			  const pos_t& p_t = _coords_map[_layers[i + 1][t]];
        			  std::vector<float> in = _inputFunctor.getConnectionInput(p_s, p_t);

        			  float link_expression_output = this->gen().conn_value(1, in);
        			  float weight_output = this->gen().conn_value(0, in);

//        			  std::cout << link_expression_output << " : " << link_expression_output*4-2 << std::endl;

#ifdef LEGACY
        			  if(link_expression_output*(4)-2 > 0){
        				  link_expression_output = Params::cppn::leo_threshold + 1.0;
        			  } else {
        				  link_expression_output = Params::cppn::leo_threshold - 1.0;
        			  }
#endif

        			  if (link_expression_output >= Params::cppn::leo_threshold){
        				  weight.gen().data(0, weight_output);
        				  weight.develop();
        				  if(weight.data(0) != 0) _nn.add_connection(_layers[i][s], _layers[i + 1][t], weight);
        			  }
        		  }
        	  }
          }

          _nn.copy_coords();
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

        const std::vector<typename nn_t::vertex_desc_t>& all_neurons() const { return _all_neurons; }

        // methods
        void write_dot(std::ostream& ofs)
        {
        	_nn.write_dot(ofs);
        }

        void write_dot_cppn(std::ostream& ofs)
        {
        	cppn_graph_t network = this->gen().get_cppn().get_graph();
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
        std::vector<typename nn_t::vertex_desc_t> _all_neurons;
        std::map<vertex_desc_t, pos_t> _coords_map;

        /**
         * Adds a node to the substrate at the correct location.
         */
        void addNode(vertex_desc_t &v, size_t layer_nr){
        	size_t neuronId = _all_neurons.size();
        	_all_neurons.push_back(v);
            _coords_map[v] = pos_t(neuronId);
            std::vector<float> input = _inputFunctor.getBiasInput(_coords_map[v]);
            float bias = this->gen().conn_value(2, input);

//            _nn.get_graph()[v].get_afparams()[0] = bias;
            _nn.get_graph()[v].get_afparams().gen().data(0, bias);
            _nn.get_graph()[v].get_afparams().develop();
            _layers[layer_nr].push_back(v);
        }
    };
  }
}
}
#endif
