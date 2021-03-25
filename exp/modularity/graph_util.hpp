/*
 * graph_util.hpp
 *
 *  Created on: Apr 21, 2017
 *      Author: joost
 */

#ifndef EXP_MODULARITY_GRAPH_UTIL_HPP_
#define EXP_MODULARITY_GRAPH_UTIL_HPP_

#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_utility.hpp>

template<typename G>
inline G extractHiddenLayer(const G& g, std::vector<bool>& removed){
	G hiddenLayers = g;
	typedef typename boost::graph_traits<G>::vertex_descriptor vertex_d_t;
	typedef typename G::vertex_property_type neuron_t;
	std::vector<vertex_d_t> to_remove;

	BGL_FORALL_VERTICES_T(v, hiddenLayers, G){
		neuron_t neuron = hiddenLayers[v];
		if(neuron.is_input() || neuron.is_output()){
			to_remove.push_back(v);
			removed.push_back(true);
		} else {
			removed.push_back(false);
		}
	}

	for (size_t i = 0; i < to_remove.size(); ++i){
		clear_vertex(to_remove[i], hiddenLayers);
		remove_vertex(to_remove[i], hiddenLayers);
	}

	to_remove.clear();
	size_t i = 0;
	BGL_FORALL_VERTICES_T(v, hiddenLayers, G){
		//Skip neurons that are removed
		while(i < removed.size() && removed[i]){
			++i;
		}
		if (boost::in_degree(v, hiddenLayers) +
				boost::out_degree(v, hiddenLayers) == 0){
			to_remove.push_back(v);
			removed[i] = true;
		}
		++i;
	}
	for (size_t i = 0; i < to_remove.size(); ++i){
		remove_vertex(to_remove[i], hiddenLayers);
	}

	return hiddenLayers;
}



#endif /* EXP_MODULARITY_GRAPH_UTIL_HPP_ */
