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




#ifndef DNN_FF_HPP_
#define DNN_FF_HPP_

#include <modules/nn2/gen_dnn.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <sferes/dbg/dbg.hpp>
#include <modules/datatools/position3.hpp>

namespace sferes
{
  namespace gen
  {
  template<typename N, typename C, typename Params>
  class DnnFF : public Dnn<N, C, Params>
  {
  public:
	  typedef nn::NN<N, C> nn_t;
	  typedef N neuron_t;
	  typedef C conn_t;
	  typedef typename nn_t::io_t io_t;
	  typedef typename nn_t::vertex_desc_t vertex_desc_t;
	  typedef typename nn_t::edge_desc_t edge_desc_t;
	  typedef typename nn_t::graph_t graph_t;
	  typedef typename nn_t::weight_t weight_t;
	  typedef typename nn_t::pf_t pf_t;
	  typedef typename nn_t::af_t af_t;

	  DnnFF() {} // @suppress("Class members should be properly initialized")
	  DnnFF& operator=(const DnnFF& o)
	  {
		  static_cast<nn::NN<N, C>& >(*this)
			  = static_cast<const nn::NN<N, C>& >(o);
		  return *this;
	  }

	  DnnFF(const DnnFF& o)
	  { *this = o; }
	  void init()
	  {
		  Dnn<N, C, Params>::init();
		  _compute_depth();
	  }

	  void random()
	  {
		  assert(Params::dnn::init == dnn::ff);
		  this->_random_ff(Params::dnn::nb_inputs, Params::dnn::nb_outputs);
//		  _make_all_vertices();
	  }

	  void mutate()
	  {
	      dbg::trace trace("mutate", DBG_HERE);

		  _change_conns();
		  this->_change_neurons();

		  if (misc::rand<float>() < Params::dnn::m_rate_add_conn)
			  _add_conn();

		  if (misc::rand<float>() < Params::dnn::m_rate_del_conn)
			  this->_del_conn();

		  if (misc::rand<float>() < Params::dnn::m_rate_add_neuron)
			  this->_add_neuron_on_conn();

		  if (misc::rand<float>() < Params::dnn::m_rate_del_neuron)
			  this->_del_neuron();
	  }

	  void cross(const DnnFF& o, DnnFF& c1, DnnFF& c2)
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
	  size_t get_depth() const { return _depth; }

	    /**
	     * Get layered coordinates map
	     */
	    std::map<vertex_desc_t, spatial::Pos> get_coordsmap(){
	        typename boost::graph_traits<graph_t>::in_edge_iterator in_edge_it, in_edge_end;
	        typename boost::graph_traits<graph_t>::out_edge_iterator out_edge_it, out_edge_end;
	        std::map<vertex_desc_t, spatial::Pos> result;
	        std::set<edge_desc_t> edges;
	        std::set<vertex_desc_t> placed_nodes;
	        std::vector<vertex_desc_t> remaining_nodes;
	        std::vector<std::vector<vertex_desc_t> > layers;
	        std::vector<vertex_desc_t> outputs;

	        layers.push_back(std::vector<vertex_desc_t>());
	        BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t){
	            if(this->is_output(v)){
	                outputs.push_back(v);
	                placed_nodes.insert(v);
	            } else if (this->is_input(v)){
	                layers.back().push_back(v);
	                placed_nodes.insert(v);
	            } else{
	                remaining_nodes.push_back(v);
	            }
	        }

	        while(remaining_nodes.size() != 0){
	            layers.push_back(std::vector<vertex_desc_t>());
	            for(size_t i=0; i<remaining_nodes.size(); ++i){
	                vertex_desc_t v = remaining_nodes[i];

	                //Count the number of nodes that need to be placed before this node can be placed
	                int remaining_nodes = 0;
	                boost::tie(in_edge_it, in_edge_end) = boost::in_edges(v, this->get_graph());
	                for(; in_edge_it != in_edge_end; ++in_edge_it) {
	                    if(placed_nodes.count(boost::source(*in_edge_it, this->get_graph())) == 0){
	                        ++remaining_nodes;
	                    }
	                }

	                //If the node has no dependencies, place this node
	                if(remaining_nodes == 0){
	                    layers.back().push_back(v);
	                }
	            }

	            //If no new nodes where added to the current layer, we are dealing with a cycle. Abort!
	            dbg::assertion(DBG_ASSERTION(layers.back().size() > 0));
	            for(size_t i=0;i<layers.back().size(); ++i){
	                placed_nodes.insert(layers.back()[i]);
	            }

	            //Re-populate the remaining nodes
	            remaining_nodes.clear();
	            BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t){
	                if(placed_nodes.count(v) == 0){
	                    remaining_nodes.push_back(v);
	                }
	            }
	        }
	        layers.push_back(outputs);

	        //Build the coordsmap
	        for(size_t i=0; i<layers.size(); ++i){
	            for(size_t j=0; j<layers[i].size(); j++){
	                float x_pos = 0.5;
	                float y_pos = 0.5;
	                if(layers[i].size() > 1){
	                    x_pos = float(j)/float(layers[i].size()-1);
	                }
                    if(layers.size() > 1){
                        y_pos = float(i)/float(layers.size()-1);
                    }
	                result[layers[i][j]] = spatial::Pos(x_pos, y_pos, 0);
	            }
	        }

	        return result;
	    }
//	  void make_all_vertices(){
//		  _make_all_vertices();
//	  }
  protected:
//	  std::set<vertex_desc_t> _all_vertices;
	  size_t _depth;

//	  void _make_all_vertices()
//	  {
//		  _all_vertices.clear();
//		  BGL_FORALL_VERTICES_T(v, this->_g, graph_t)
//		  	  _all_vertices.insert(v);
//	  }

	  void _change_conns(){
	      dbg::trace trace("mutate", DBG_HERE);
		  BGL_FORALL_EDGES_T(e, this->_g, graph_t){
#if defined(DBG_ENABLED)
			  weight_t weight = this->_g[e].get_weight();
#endif
			  this->_g[e].get_weight().mutate();
#if defined(DBG_ENABLED)
	            vertex_desc_t src = boost::source(e, this->_g);
	              vertex_desc_t tgt = boost::target(e, this->_g);
			  this->_g[e].get_weight().develop();
			  weight.develop();

			  if(nn::trait<weight_t>::single_value(this->_g[e].get_weight()) != nn::trait<weight_t>::single_value(weight)){
				  dbg::out(dbg::info, "random") << "Weight conn changed - from: " << this->_g[src].get_id()
						  << " to: " << this->_g[tgt].get_id()
						  << " weight: " << nn::trait<weight_t>::single_value(this->_g[e].get_weight()) << std::endl;
			  }
#endif
		  }
	  }


	  // add only feed-forward connections
	  void _add_conn()
	  {
	      dbg::trace trace("mutate", DBG_HERE);
		  using namespace boost;

		  dbg::out(dbg::info, "addcon") << "Random source" << std::endl;
		  vertex_desc_t v = this->_random_src();

		  std::vector<vertex_desc_t> avail;
		  std::set<vertex_desc_t> in;

		  //Get the set of all predecessors of this randomly selected node
		  std::set<vertex_desc_t> preds;
		  nn::bfs_pred_visitor<vertex_desc_t> vis(preds);
		  dbg::out(dbg::info, "addcon") << "Breadth first search" << std::endl;
		  breadth_first_search(make_reverse_graph(this->_g), v, color_map(get(&N::_color, this->_g)).visitor(vis));

		  // avoid to connect to inputs
		  dbg::out(dbg::info, "addcon") << "List all inputs" << std::endl;
		  BOOST_FOREACH(vertex_desc_t v, this->_inputs){
			  in.insert(v);
		  }

		  //Add all vertices that are not inputs and that are not predecessors
		  dbg::out(dbg::info, "addcon") << "Add all descendants" << std::endl;
		  BGL_FORALL_VERTICES_T(v, this->_g, graph_t){
			  if(preds.count(v) == 0 && in.count(v) == 0){
				  avail.push_back(v);
			  }
		  }

		  if (avail.empty())
			  return;

		  dbg::out(dbg::info, "addcon") << "Add connection" << std::endl;
		  vertex_desc_t tgt = *misc::rand_l(avail);

		  weight_t w;
		  w.random();
		  this->add_connection(v, tgt, w);

#if defined(DBG_ENABLED)
		  for (typename std::vector<vertex_desc_t>::iterator it = avail.begin(); it != avail.end(); it++){
			  dbg::out(dbg::info, "random") << this->_g[*it].get_id() << " ";
		  }
		  dbg::out(dbg::info, "random") << std::endl;
		  w.develop();
		  dbg::out(dbg::info, "random") << "conn added - from: " << this->_g[v].get_id()
				  << " to: " << this->_g[tgt].get_id()
				  << " weight: " << nn::trait<weight_t>::single_value(w) << std::endl;
#endif

	  }

	  // useful to make the right number of steps
	  void _compute_depth()
	  {
		  using namespace boost;
		  typedef std::map<vertex_desc_t, size_t> int_map_t;
		  typedef std::map<vertex_desc_t, vertex_desc_t> vertex_map_t;
		  typedef std::map<vertex_desc_t, default_color_type> color_map_t;
		  typedef std::map<edge_desc_t, int> edge_map_t;

		  typedef associative_property_map<int_map_t> a_map_t;
		  typedef associative_property_map<color_map_t> c_map_t;
		  typedef associative_property_map<vertex_map_t> v_map_t;
		  typedef associative_property_map<edge_map_t> e_map_t;

		  color_map_t cm; c_map_t cmap(cm);
		  vertex_map_t vm; v_map_t pmap(vm);
		  edge_map_t em;
		  BGL_FORALL_EDGES_T(e, this->_g, graph_t)
		  em[e] = 1;
		  e_map_t wmap(em);
		  _depth = 0;
		  // we compute the longest path between inputs and outputs
		  BOOST_FOREACH(vertex_desc_t s, this->_inputs)
		  {
			  int_map_t im; a_map_t dmap(im);
			  dag_shortest_paths
			  (this->_g, s, dmap, wmap, cmap, pmap,
					  dijkstra_visitor<null_visitor>(),
					  std::greater<int>(),
					  closed_plus<int>(),
					  std::numeric_limits<int>::min(), 0);

			  BGL_FORALL_VERTICES_T(v, this->_g, graph_t)
			  {
				  size_t d = get(dmap, v);
				  if (this->_g[v].get_out() != -1 && d <= num_vertices(this->_g))
					  _depth = std::max(_depth, d);
			  }
		  }
		  // add one to be sure
		  _depth ++;
	  }

  };

  }
}

#endif
