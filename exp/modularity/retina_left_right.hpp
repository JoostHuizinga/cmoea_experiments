/*
 * retina_left_right.hpp
 *
 *  Created on: Dec 23, 2013
 *      Author: joost
 */

#ifndef RETINA_LEFT_RIGHT_HPP_
#define RETINA_LEFT_RIGHT_HPP_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/foreach.hpp>

#include <queue>

template<typename Params, typename Indivdual>
class LeftRightAnalysis{
	//Type declarations
	typedef Indivdual individual_t;
	typedef typename Indivdual::nn_t::graph_t graph_t;
	typedef typename Indivdual::nn_t::vertex_desc_t vertex_desc_t;
	typedef typename Indivdual::nn_t::edge_desc_t edge_desc_t;
	typedef typename Indivdual::nn_t::con_weight_t con_weight_t;
	typedef typename boost::graph_traits<graph_t>::in_edge_iterator in_edge_iterator_t;
	typedef std::map<std::pair<vertex_desc_t, size_t>, float > map_t;
	typedef std::map<std::string, size_t > solve_map_t;

	//Data
	map_t _neuron_activation_map;
	solve_map_t _solve_map;
	std::vector<bool> _is_left_object;
	std::vector<bool> _is_right_object;
	std::vector<bool> _result;
	std::vector<int> _neurons_involved;

public:
	LeftRightAnalysis(const std::vector<size_t>& _left_objects, const std::vector<size_t>& _right_objects):
		_is_left_object(Params::nr_of_input_patterns, false),
		_is_right_object(Params::nr_of_input_patterns, false),
		_result(Params::nr_of_subtasks, false),
		_neurons_involved(Params::nr_of_subtasks, 0)
	{
		for (size_t s = 0; s < Params::nr_of_input_patterns; ++s) {
			std::bitset<8> in(s);
			bool left = std::find(_left_objects.begin(), _left_objects.end(), (s & 0xF)) != _left_objects.end();
			bool right = std::find(_right_objects.begin(), _right_objects.end(), (s & 0xF0)) != _right_objects.end();
			if(left) _is_left_object[s] = true;
			if(right) _is_right_object[s] = true;
		}
	}

	bool one_solved(int maximum_neurons_used){
		for (int i=0; i<Params::nr_of_subtasks; i++){
			if(_result[i] && _neurons_involved[i]<=maximum_neurons_used){
				return true;
			}
		}
		return false;
	}

	bool two_solved(int maximum_neurons_used){
		for (int i=0; i<Params::nr_of_subtasks; i++){
			if(!(_result[i] && _neurons_involved[i]<=maximum_neurons_used)){
				return false;
			}
		}
		return true;
	}

	solve_map_t& getSolvedMap(){
		return _solve_map;
	}

	bool analysis_left_right_cases(Indivdual& indiv) {
//		std::cout << "Start left-right analysis" << std::endl;
		std::vector<edge_desc_t> incomming_connections;
		in_edge_iterator_t e_start, e_end, current;

		typename Indivdual::nn_t& nnsimp = indiv.nn();
//		nnsimp.init();


		//Fill the neuron activation map
//		std::cout << "Filling neuron activation map" << std::endl;
		std::vector<float> inputs(8);
		for (size_t s = 0; s < 256; ++s) {
			std::bitset<8> in(s);
			for (size_t i = 0; i < in.size(); ++i) inputs[i] = in[i] ? 1 : -1;

			//Activate
			for (size_t i = 0; i < nnsimp.get_depth(); ++i) nnsimp.step(inputs);

			//Store all neuron activations
			BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t) {
				std::pair<vertex_desc_t, size_t> vertex_object_id_pair(v, s);
				_neuron_activation_map[vertex_object_id_pair] = nnsimp.get_graph()[v].get_current_output();
			}
		}


		//Get the maximum number of incoming edges
		size_t max_in_edges = 0;
		size_t in_edges;
		BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t) {
            boost::tie(e_start, e_end) = boost::in_edges(v, nnsimp.get_graph());

            in_edges=0;
            for(current = e_start; current != e_end; current++){
            	in_edges++;
            }

            if(in_edges > max_in_edges) max_in_edges = in_edges;
		}



		for(size_t edges = 1; edges <= max_in_edges; edges++){
		//Get all incoming connections for each neuron and check whether they solve one of the sub-tasks.
			BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t) {
				boost::tie(e_start, e_end) = boost::in_edges(v, nnsimp.get_graph());
				incomming_connections.clear();

				for(current = e_start; current != e_end; current++){
					incomming_connections.push_back(*current);
				}

				_check_solve(incomming_connections, nnsimp.get_graph(), edges);
				if(_cumulative_and(_result)) return true;
			}
		}

		return false;
	}

protected:

	/**
	 * Get the activation of the specified neuron and the specified input from the activation map.
	 */
	float _get_activation(vertex_desc_t vertex, size_t input_pattern_id){
		std::pair<vertex_desc_t, size_t> vertex_input_pair (vertex, input_pattern_id);
		return _neuron_activation_map[vertex_input_pair];
	}

	/**
	 * Returns true if the sub-task number is associated with an object and false otherwise
	 */
	bool _is_object(size_t object_nr, size_t subtask_nr){
		if(subtask_nr == 0){
			return _is_left_object[object_nr];
		} else {
			return _is_right_object[object_nr];
		}
	}

	/**
	 * Calculates whether the provided connections and their source neurons together solve one or more of the sub-tasks.
	 */
	void _solves(const std::vector<edge_desc_t>& connections, const graph_t& graph){
//		std::cout << "Checking solve for single set of connections: " << connections.size() << std::endl;
		std::vector<float> is_object_max(Params::nr_of_subtasks, -std::numeric_limits<float>::max());
		std::vector<float> is_object_min(Params::nr_of_subtasks, std::numeric_limits<float>::max());
		std::vector<float> not_object_max(Params::nr_of_subtasks, -std::numeric_limits<float>::max());
		std::vector<float> not_object_min(Params::nr_of_subtasks, std::numeric_limits<float>::max());
		float value = 0;

		//If there are no connection those connection will certainly not solve any problems
		if(connections.size() == 0) return;

		//Try ever input pattern and record the minimum and maximum values for those patterns
		for (size_t s = 0; s < 256; ++s) {
			float value = 0;
			for(int i=0; i<connections.size(); i++){
//				std::cout << "Activation: " << _get_activation(boost::source(connections[i], graph), s) << " weight: " << nn::trait<con_weight_t>::single_value(graph[connections[i]].get_weight()) << std::endl;
				value += _get_activation(boost::source(connections[i], graph), s) *
						nn::trait<con_weight_t>::single_value(graph[connections[i]].get_weight());
			}
//			std::cout << "Con-size: " << connections.size() << " value: " << value << " is object left: " << _is_object(s, 0) << " is object right: " << _is_object(s, 1) << std::endl;
			for(size_t subtask_nr=0; subtask_nr < Params::nr_of_subtasks; subtask_nr++){
				if(_is_object(s, subtask_nr)){
					if(value > is_object_max[subtask_nr]) is_object_max[subtask_nr] = value;
					if(value < is_object_min[subtask_nr]) is_object_min[subtask_nr] = value;
				} else {
					if(value > not_object_max[subtask_nr]) not_object_max[subtask_nr] = value;
					if(value < not_object_min[subtask_nr]) not_object_min[subtask_nr] = value;
				}
			}
		}

//		std::cout << "First objective: is_object_max: " << is_object_max[0] << " is_object_min: " << is_object_min[0]
//				<< " not_object_max: " << not_object_max[0] << " not_object_min: " << not_object_min[0] <<std::endl;
//
//		std::cout << "Second objective: is_object_max: " << is_object_max[1] << " is_object_min: " << is_object_min[1]
//				<< " not_object_max: " << not_object_max[1] << " not_object_min: " << not_object_min[1] <<std::endl;

		//Check whether the recorded values are linearly separable
		for(size_t subtask_nr=0; subtask_nr < Params::nr_of_subtasks; subtask_nr++){
			if((is_object_max[subtask_nr] < not_object_min[subtask_nr] || not_object_max[subtask_nr] < is_object_min[subtask_nr]) && !_result[subtask_nr]){
//				std::cout << "####################Sub-problem solved#########################" << std::endl;
//				std::cout << "####################Sub-problem solved#########################" << std::endl;
//				std::cout << "####################Sub-problem solved#########################" << std::endl;
				_result[subtask_nr] = true;
				_neurons_involved[subtask_nr] = connections.size();
				for(int i=0; i<connections.size(); i++){
//					std::cout << "Id of solving neuron: " << graph[boost::source(connections[i], graph)].get_id() << std::endl;
					_solve_map[graph[boost::source(connections[i], graph)].get_id()] += (subtask_nr+1);
				}


			}
		}
	}

//	std::vector<bool>& _pairwise_or(std::vector<bool>& a, const std::vector<bool>& b){
//		for(int i=0; i<a.size(); i++){
//			a[i] = a[i] || b[i];
//		}
//		return a;
//	}

	bool _cumulative_and(const std::vector<bool>& a){
		for(int i=0; i<a.size(); i++){
			if(a[i] == false) return false;
		}
		return true;
	}

	/**
	 * Uses breadth first search to find the minimal number of connections that together solve one or more of the subproblems.
	 * Returns an array that indicates which subproblems were solved.
	 */
	void _check_solve(const std::vector<edge_desc_t>& connections, const graph_t& this_nn, const size_t& edges){
//		std::cout << "Checking solve for " << edges << " connections" << std::endl;
		typedef std::pair<std::vector<edge_desc_t>, size_t> element_t;

		if(connections.size() < edges) return;

		std::queue<element_t> frontier;
		element_t current_element(std::vector<edge_desc_t>(), 0);
		frontier.push(current_element);

		while(!frontier.empty()){
			//Get the first element from the frontier
			current_element = frontier.front();
			frontier.pop();

			//Check if the connections and neurons in the current frontier solves one of the sub-tasks and 'add' it to the current result
			if(current_element.first.size() == edges){
				_solves(current_element.first, this_nn);

				//If all sub-problems are solved, stop searching
				if(_cumulative_and(_result)) return;
				continue;
			}

			//If the not all problems are solved, add the second layer of connections to the back of the queue
			for(size_t connection_index = current_element.second; connection_index < connections.size(); connection_index++){
				edge_desc_t connection_to_add = connections[connection_index];
				element_t new_element(current_element.first, connection_index+1);
				new_element.first.push_back(connection_to_add);
				frontier.push(new_element);
			}
		}
	}

};


#endif /* RETINA_LEFT_RIGHT_HPP_ */
