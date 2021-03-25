/*
 * regularity.hpp
 *
 *  Created on: Dec 30, 2013
 *      Author: joost
 */

#ifndef REGULARITY_HPP_
#define REGULARITY_HPP_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/foreach.hpp>
#include "lzw.hpp"

namespace regularity{

template<typename I>
std::vector<std::string> get_weights_vector(I& indiv){
	std::vector<std::string> result;
	typename I::nn_t::graph_t network = indiv.nn().get_graph();

	BGL_FORALL_VERTICES_T(v, network, typename I::nn_t::graph_t)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(5) << std::showpos;

		ss << nn::trait<typename I::nn_t::neuron_t::af_t::params_t>::single_value(network[v].get_afparams());
		std::string s = ss.str();
		result.push_back(s);
	}
	BGL_FORALL_EDGES_T(e, network, typename I::nn_t::graph_t)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(5) << std::showpos;
		ss << nn::trait<typename I::nn_t::conn_t::weight_t>::single_value(network[e].get_weight());
		std::string s = ss.str();
		result.push_back(s);
	}
	return result;
}



long _compressed_difference(std::string weights){
	LZWEncoding codec;
	long originalLength = weights.size()+1;
	uint length = weights.size()+1;

	uchar* data = new uchar[length];
	uchar* encodedResult;

	weights.copy((char*)data, length);
	data[length-1] = '\0';

	encodedResult = codec.Encode(data, length);

	delete[] data;
	delete[] encodedResult;

	return (originalLength*8 - codec.getTotalBits());
}

/**
 *
 */
template<typename I>
float analysis_regularity(I& indiv, int nr_of_permutation = 500){
	long totalCompression = 0;



	for(int i=0; i < nr_of_permutation; i++){
		std::vector<std::string> weightsVector = get_weights_vector(indiv);
		//std::cout << weightsVector.size() <<std::endl;
		std::random_shuffle ( weightsVector.begin(), weightsVector.end() );
		std::string weightsString = "";

		BOOST_FOREACH(std::string weight, weightsVector){
			weightsString.append(weight);
		}

		//std::cout << weightsString << std::endl;
		//std::cout << i;
		totalCompression += _compressed_difference(weightsString);
		//std::cout << totalCompression << std::endl;
	}


	return float(totalCompression)/nr_of_permutation;
}

template<typename I>
float get_original_length(I& indiv){
	std::vector<std::string> weightsVector = get_weights_vector(indiv);
	std::string weightsString = "";
	BOOST_FOREACH(std::string weight, weightsVector){
		weightsString.append(weight);
	}
	return (weightsString.size()+1)*8;
}
}

#endif /* REGULARITY_HPP_ */
