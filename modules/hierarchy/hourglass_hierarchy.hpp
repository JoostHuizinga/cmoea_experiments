/*
 * hourglass_hierarchy.hpp
 *
 *  Created on: Apr 9, 2017
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HIERARCHY_HOURGLASS_HIERARCHY_HPP_
#define MODULES_HIERARCHY_HOURGLASS_HIERARCHY_HPP_

// Boost includes
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/foreach.hpp>

// Sferes includes
#include <sferes/dbg/dbg.hpp>

// Debug macro to make debug statements more concise
#define DBO dbg::out(dbg::info, "hierarchy")

// Import name spaces
using namespace boost;

namespace hierar{
struct VertexProperties{
	VertexProperties(){
		lowfront = -1;
		lowvine = -1;
		id = -1;
		on_stack = false;
		ancestor = false;
	}

	inline void set(int id_){
		lowfront = id_;
		lowvine = id_;
		id = id_;
		on_stack = true;
		ancestor = true;
	}

	inline bool isSet(){
		return id != -1;
	}

	inline void updateLowfront(int number){
		lowfront = std::min(lowfront, number);
	}

	inline void updateLowvine(int number){
		lowvine = std::min(lowvine, number);
	}

	inline bool isRoot(){
		return (lowfront == id) && (lowvine == id);
	}

	int lowfront;
	int lowvine;
	int id;
	bool on_stack;
	bool ancestor;
};

template<typename graph_t>
class HourglassHierarchyCalculator{
public:
	typedef typename graph_traits<graph_t>::vertex_descriptor vert_t;
	typedef typename graph_traits<graph_t>::out_edge_iterator out_edge_it_t;
	typedef typename graph_traits<graph_t>::in_edge_iterator in_edge_it_t;
	typedef typename graph_traits<graph_t>::adjacency_iterator adj_it;
	typedef typename std::pair<in_edge_it_t, in_edge_it_t> in_pair_t;
	typedef typename std::pair<out_edge_it_t, out_edge_it_t> out_pair_t;

	/**
	 * Basic constructor.
	 *
	 * Does not set a graph for which to calculate the hierarchy.
	 */
	HourglassHierarchyCalculator(){
		_id = 0;
		_hierarchy = -1;
		_graph_set = false;
		_dag_set = false;
	}

	/**
	 * Constructor which sets the graph for which to calculate the hierarchy.
	 *
	 * @param graph: The graph for which to calculate the hierarchy.
	 */
	HourglassHierarchyCalculator(graph_t& graph):
		HourglassHierarchyCalculator()
	{
		setGraph(graph);
	}

	/**
	 * Calculates the hierarchy of the current graph.
	 *
	 * Prints and error, and returns a hierarchy of -1, if no graph was set.
	 *
	 * @return The hierarchy of the current graph.
	 */
	inline float getHierarchy(){
		if(_hierarchy < 0){
			_calculatePathCentrality();
		}
		return _hierarchy;
	}

	/**
	 * Set the graph for which to calculate the hierarchy.
	 *
	 * Should not be called more than once, because the hierarchy calculator
	 * keeps internal state regarding the current graph, and never clears it.
	 * Construct a new hierarchy calculator for new graphs.
	 *
	 * @param graph: The graph for which to calculate the hierarchy.
	 */
	void setGraph(graph_t& graph){
		if(_graph_set){
			std::cerr << "ERROR: Graph already set!" << std::endl;
			return;
		}
		_graph_set = true;
		_graph = graph;
	}


	/**
	 * Returns the directed acyclic graph (DAG) constructed out of the set
	 * graph.
	 *
	 * Usually not necessary, but useful for debugging purpose (or if you
	 * need a DAG for a different algorithm). Prints an error and returns an
	 * empty graph if no graph was set.
	 *
	 * @return A DAG constructed from the set graph.
	 */
	graph_t& getDag(){
		if(!_dag_set && _graph_set){
			_createDag(_graph);
		} else if(!_dag_set && !_graph_set){
			std::cerr << "ERROR: Graph not set!" << std::endl;
		}
		return _dag;
	}

private:
	int _id;
	std::map<vert_t, VertexProperties> _vert_prop_m;
	std::vector<std::vector<vert_t> > _components;
	std::stack<vert_t> _points;
	bool _graph_set;
	graph_t _graph;
	bool _dag_set;
	graph_t _dag;
	double _hierarchy;

	inline void _calculatePathCentrality(){
		getDag();
		if(!_dag_set) return;
		std::vector<vert_t> front;
		std::set<vert_t> visited;
		std::map<vert_t, int> in_path_map;
		std::map<vert_t, int> out_path_map;
		std::map<vert_t, double> path_map;
		size_t nb_vertices = num_vertices(_dag);

		// Trivial case
		if(nb_vertices <= 1){
			_hierarchy = 0;
			return;
		}

		// Calculate incoming paths
		for(int layer = 0; visited.size() != nb_vertices; ++layer){
			DBO << "Layer: " << layer << std::endl;
			front = _getSources(_dag, visited);
			if(front.empty()) break;
			for(size_t i=0; i<front.size(); ++i){
				vert_t current = front[i];
				DBO << "Vertex: (" << _dag[current]._id << ")" << std::endl;
				visited.insert(current);
				int in_paths = 0;
				in_pair_t in_pair = in_edges(current, _dag);
				if(in_pair.first==in_pair.second){
					out_pair_t out_pair = out_edges(current, _dag);
					if(out_pair.first != out_pair.second){
						in_paths = 1;
					}
				}
				for(; in_pair.first!=in_pair.second; in_pair.first++){
					vert_t in_vert = source(*in_pair.first, _dag);
					DBO << "src: (" << _dag[in_vert]._id << ")" << std::endl;
					if(in_path_map.count(in_vert) > 0){
						DBO << "src paths: " <<
								in_path_map[in_vert] << std::endl;
						in_paths += in_path_map[in_vert];
					} else {
						std::cerr << "ERROR: in-edges unknown" << std::endl;
						return;
					}
				}
				DBO << "Total in-paths: " << in_paths << std::endl;
				in_path_map[current] = in_paths;
			}
		}

		DBO << "Incoming paths" << std::endl;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			DBO << "(" << _dag[v]._id << "): " << in_path_map[v] << std::endl;
		}
		DBO << std::endl;


		// Calculate outgoing paths
		visited.clear();
		for(int layer = 0; visited.size() != nb_vertices; ++layer){
			DBO << "Layer: " << layer << std::endl;
			front = _getOutputs(_dag, visited);
			if(front.empty()) break;
			for(size_t i=0; i<front.size(); ++i){
				vert_t current = front[i];
				DBO << "Vertex: (" << _dag[current]._id << ")" << std::endl;
				visited.insert(current);
				int out_paths = 0;
				out_pair_t out_pair = out_edges(current, _dag);
				if(out_pair.first==out_pair.second){
					in_pair_t in_pair = in_edges(current, _dag);
					if(in_pair.first != in_pair.second){
						out_paths = 1;
					}
				}
				for(; out_pair.first!=out_pair.second; out_pair.first++){
					vert_t out_vert = target(*out_pair.first, _dag);
					DBO << "trg: (" << _dag[out_vert]._id << ")" << std::endl;
					if(out_path_map.count(out_vert) > 0){
						DBO << "trg paths: " <<
								out_path_map[out_vert] << std::endl;
						out_paths += out_path_map[out_vert];
					} else {
						std::cerr << "ERROR: in-edges unknown" << std::endl;
						return;
					}
				}
				DBO << "Total in-paths: " << out_paths << std::endl;
				out_path_map[current] = out_paths;
			}
		}

		DBO << "Outgoing paths" << std::endl;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			DBO << "(" << _dag[v]._id << "): " << out_path_map[v] << std::endl;
		}
		DBO << std::endl;

		// Calculate combined paths
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			path_map[v] = in_path_map[v] * out_path_map[v];
		}

		DBO << "Total paths per node" << std::endl;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			DBO << "(" << _dag[v]._id << "): " << path_map[v] << std::endl;
		}
		DBO << std::endl;

		// Calculate total number of paths
		double total_paths = 0;
		visited.clear();
		front = _getSources(_dag, visited);
		for(size_t i=0; i<front.size(); ++i){
			total_paths += path_map[front[i]];
		}
		DBO << "Total paths: " << total_paths << std::endl;

		// Normalize path centrality
		double max_path_centrality = 0;
		double path_centrality;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			if(total_paths == 0){
				path_centrality = 0;
			} else {
				path_centrality = path_map[v] / total_paths;
			}
			if(path_centrality > max_path_centrality){
				max_path_centrality = path_centrality;
			}
			path_map[v] = path_centrality;
		}

		DBO << "Path centrality per node" << std::endl;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			DBO << "(" << _dag[v]._id << "): " << path_map[v] << std::endl;
		}
		DBO << std::endl;

		// Calculate hourglass hierarchy
		_hierarchy = 0;
		BGL_FORALL_VERTICES_T(v, _dag, graph_t){
			_hierarchy += max_path_centrality - path_map[v];
		}
		DBO << "Total path centrality: " << _hierarchy << std::endl;
		_hierarchy /= double(nb_vertices-1);
		DBO << "Hierarchy: " << _hierarchy << std::endl;
	}

	std::vector<vert_t> _getSources(const graph_t& g,
			const std::set<vert_t>& visited)
	{
		std::vector<vert_t> result;
		BGL_FORALL_VERTICES_T(v, g, graph_t){
			if(visited.count(v) > 0) continue;
			bool add = true;
			in_pair_t in_edge_p = in_edges(v, g);
			for(; in_edge_p.first!=in_edge_p.second; in_edge_p.first++){
				vert_t in_vert = source(*in_edge_p.first, g);
				if(visited.count(in_vert) == 0){
					add = false;
				}
			}
			if(add){
				result.push_back(v);
			}
		}
		return result;
	}

	std::vector<vert_t> _getOutputs(const graph_t& g,
			const std::set<vert_t>& visited)
	{
		std::vector<vert_t> result;
		BGL_FORALL_VERTICES_T(v, g, graph_t){
			if(visited.count(v) > 0) continue;
			bool add = true;
			out_pair_t out_edge_p = out_edges(v, g);
			for(; out_edge_p.first!=out_edge_p.second; out_edge_p.first++){
				vert_t out_vert = target(*out_edge_p.first, g);
				if(visited.count(out_vert) == 0){
					add = false;
				}
			}
			if(add){
				result.push_back(v);
			}
		}
		return result;
	}


	void _createDag(graph_t& graph){
		DBO << "Assigning vertex properties to each vertex" <<std::endl;
		BGL_FORALL_VERTICES_T(v, graph, graph_t){
			_vert_prop_m[v] = VertexProperties();
		}

		DBO << "Strongly connected components: start" <<std::endl;
		BGL_FORALL_VERTICES_T(v, graph, graph_t){
			_strongconnect(v, graph);
		}
		DBO << "Strongly connected components: end" <<std::endl;

		//graph_t new_graph;
		_dag = graph_t();
		std::map<vert_t, int> vertex_component_m;
		std::map<vert_t, vert_t> vertex_vertex_m;
		for(size_t i=0; i<_components.size(); ++i){
			DBO << "Adding vertex" << std::endl;
			vert_t v = add_vertex(_dag);
			(_dag)[v]._id = boost::lexical_cast<std::string>(i) +": ";
			for(size_t j=0; j<_components[i].size(); ++j){
				vert_t& vertex = _components[i][j];
				(_dag)[v]._id += graph[vertex]._id + " ";
				vertex_component_m[vertex] = i;
				vertex_vertex_m[vertex] = v;
			}
		}

		for(size_t i=0; i<_components.size(); ++i){
			for(size_t j=0; j<_components[i].size(); ++j){
				vert_t& source = vertex_vertex_m[_components[i][j]];
				std::pair<adj_it, adj_it> adj_pair;
				adj_pair = adjacent_vertices(_components[i][j], graph);
				for(; adj_pair.first!=adj_pair.second; adj_pair.first++){
					vert_t& target = vertex_vertex_m[*adj_pair.first];
					if(target == source) continue;
					if(edge(source, target, _dag).second) continue;
					DBO << "Adding edge" << std::endl;
					add_edge(source, target, _dag);
				}
			}
		}

		_dag_set= true;
	}

	void _strongconnect(vert_t v, graph_t& graph){
		VertexProperties& vert_prop = _vert_prop_m[v];
		if(vert_prop.isSet()) return;
		vert_prop.set(_id);
		_id++;
		_points.push(v);

		DBO << "Processing vertex: " << vert_prop.id <<std::endl;
		std::pair<adj_it, adj_it> adj_pair = adjacent_vertices(v, graph);

		DBO << "Visiting neighbors" << std::endl;
		for(; adj_pair.first!=adj_pair.second; adj_pair.first++){
			vert_t w = *adj_pair.first;
			VertexProperties& neigh_prop = _vert_prop_m[w];
			DBO << "  visiting neighbor: " << neigh_prop.id << std::endl;
			if(!neigh_prop.isSet()){
				_strongconnect(w, graph);
				vert_prop.updateLowfront(neigh_prop.lowfront);
				vert_prop.updateLowvine(neigh_prop.lowvine);
			} else if(neigh_prop.ancestor){
				vert_prop.updateLowfront(neigh_prop.id);
			} else if(neigh_prop.id < vert_prop.id){
				if(neigh_prop.on_stack){
					vert_prop.updateLowvine(neigh_prop.id);
				}
			}
		}
		vert_prop.ancestor = false;
		DBO << "Done visiting neighbors" << std::endl;

		if(vert_prop.isRoot()){
			DBO << "Adding components for: " << vert_prop.id << std::endl;
			std::vector<vert_t> component;
			while(!_points.empty()){
				VertexProperties& top_prop = _vert_prop_m[_points.top()];
				if(top_prop.id < vert_prop.id) break;
				DBO << "Adding vertex: " << top_prop.id << std::endl;
				component.push_back(_points.top());
				top_prop.on_stack = false;
				_points.pop();
			}
			_components.push_back(component);
		}
		DBO << "  strong connect: end" <<std::endl;
	}
};

template<typename graph_t>
double getHourglassHierarchy(graph_t& graph){
	HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
	return hierarchy_calculator.getHierarchy();
}

} // namespace hierar


#undef DBO
#endif /* MODULES_HIERARCHY_HOURGLASS_HIERARCHY_HPP_ */
