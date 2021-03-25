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




#ifndef DNN_FF_ALT_MUT_HPP_
#define DNN_FF_ALT_MUT_HPP_

#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/gen_dnn.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/property_map/vector_property_map.hpp>

namespace sferes
{
  namespace gen
  {
  template<typename N, typename C, typename Params>
  class DnnFFAltMut : public DnnFF<N, C, Params>
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

	  DnnFFAltMut() {}

	  DnnFFAltMut(const DnnFFAltMut& o)
	  { *this = o; }

	  void init()
	  {
		  DnnFF<N, C, Params>::init();
	  }

	  void mutate()
	  {
		  this->_change_conns();
		  this->_change_neurons();

		  _add_conn();
		  _edge_based_mutations();
		  _del_neuron();
	  }

  protected:


	  void _edge_based_mutations(){
		  std::vector<edge_desc_t> to_remove;
		  std::vector<edge_desc_t> to_add;

		  BGL_FORALL_EDGES_T(e, this->_g, graph_t){
			  if (misc::rand<float>() < Params::dnn::m_rate_del_conn){
				  to_remove.push_back(e);
			  }
			  else if (misc::rand<float>() < Params::dnn::m_rate_add_neuron){
				  to_add.push_back(e);
			  }
		  }

		  BOOST_FOREACH(edge_desc_t e, to_remove){
			  _del_con(e);
		  }
		  BOOST_FOREACH(edge_desc_t e, to_add){
			  _add_neuron_on_conn(e);
		  }
	  }

	  void _add_conn(vertex_desc_t src, vertex_desc_t tgt){
		  if (this->get_nb_connections() >= Params::dnn::max_nb_conns) return;
		  typename nn_t::weight_t w;
		  w.random();
		  this->add_connection(src, tgt, w);
	  }


	  void _del_con(edge_desc_t e){
		  if (!this->get_nb_connections()) return;
		  if(this->get_nb_connections() <= Params::dnn::min_nb_conns) return;
		  remove_edge(e, this->_g);
	  }

	  void _add_neuron_on_conn(edge_desc_t e){
		  if (!this->get_nb_connections()) return;
		  if (this->get_nb_neurons() >= Params::dnn::max_nb_neurons) return;
		  if (this->get_nb_connections() >= Params::dnn::max_nb_conns) return;

		  vertex_desc_t src = source(e, this->_g);
		  vertex_desc_t tgt = target(e, this->_g);
		  typename nn_t::weight_t w = this->_g[e].get_weight();
		  vertex_desc_t n = this->add_neuron("n");
		  this->_g[n].get_pfparams().random();
		  this->_g[n].get_afparams().random();
		  //
		  remove_edge(e, this->_g);
		  this->add_connection(src, n, w);// todo : find a kind of 1 ??
		  this->add_connection(n, tgt, w);
	  }

	  void _del_neuron(vertex_desc_t v){
		  if (this->get_nb_neurons() <= Params::dnn::min_nb_neurons) return;
		  clear_vertex(v, this->_g);
		  remove_vertex(v, this->_g);
	  }


	  void _del_conns()
	  {
	      if (!this->get_nb_connections()) return;
	      std::vector<edge_desc_t> to_remove;

		  BGL_FORALL_EDGES_T(e, this->_g, graph_t){
			  if (misc::rand<float>() < Params::dnn::m_rate_del_conn){
				  to_remove.push_back(e);
			  }
		  }

		  BOOST_FOREACH(edge_desc_t e, to_remove){
			  remove_edge(e, this->_g);
		  }
	  }

	  void _add_neuron_on_conn()
	  {
		  if (!num_edges(this->_g)) return;

		  std::vector<edge_desc_t> to_add;

		  BGL_FORALL_EDGES_T(e, this->_g, graph_t){
			  if (misc::rand<float>() < Params::dnn::m_rate_add_neuron){
				  to_add.push_back(e);
			  }
		  }

		  BOOST_FOREACH(edge_desc_t e, to_add){
	    	vertex_desc_t src = source(e, this->_g);
	    	vertex_desc_t tgt = target(e, this->_g);
	    	typename nn_t::weight_t w = this->_g[e].get_weight();
	    	vertex_desc_t n = this->add_neuron("n");
	    	this->_g[n].get_pfparams().random();
	    	this->_g[n].get_afparams().random();
	    	//
	    	remove_edge(e, this->_g);
	    	this->add_connection(src, n, w);// todo : find a kind of 1 ??
	    	this->add_connection(n, tgt, w);
		  }
	  }


	  // add only feed-forward connections
	  void _add_conn()
	  {
		  using namespace boost;
		  BGL_FORALL_VERTICES_T(v, this->_g, graph_t){
//			  if (misc::rand<float>() < Params::dnn::m_rate_add_conn){
				  std::set<vertex_desc_t> preds;
				  nn::bfs_pred_visitor<vertex_desc_t> vis(preds);
				  breadth_first_search(make_reverse_graph(this->_g), v, color_map(get(&N::_color, this->_g)).visitor(vis));
				  this->_make_all_vertices();
				  std::set<vertex_desc_t> tmp, avail, in;
				  // avoid to connect to predecessors
				  std::set_difference(this->_all_vertices.begin(), this->_all_vertices.end(),
						  preds.begin(), preds.end(),
						  std::insert_iterator<std::set<vertex_desc_t> >(tmp, tmp.begin()));
				  // avoid to connect to inputs
				  BOOST_FOREACH(vertex_desc_t v, this->_inputs){ // inputs need
					  // to be sorted
					  in.insert(v);
				  }
				  std::set_difference(tmp.begin(), tmp.end(),
						  in.begin(), in.end(),
						  std::insert_iterator<std::set<vertex_desc_t> >(avail, avail.begin()));

				  if (avail.empty()) return;

				  BOOST_FOREACH(vertex_desc_t tgt, avail){
					  if (misc::rand<float>() < Params::dnn::m_rate_add_conn){
						  _add_conn(v, tgt);
					  }
				  }
			  //}
		  }

	  }

	  void _del_neuron()
	  {
		  assert(num_vertices(this->_g));

		  if (this->get_nb_neurons() <= this->get_nb_inputs() + this->get_nb_outputs())
			  return;
		  std::vector<vertex_desc_t> to_remove;

		  BGL_FORALL_VERTICES_T(v, this->_g, graph_t){
			  if(!is_output(v) && !is_input(v)){
				  if (misc::rand<float>() < Params::dnn::m_rate_del_neuron){
					  to_remove.push_back(v);
				  }
			  }
		  }

		  BOOST_FOREACH(vertex_desc_t v, to_remove){
			  _del_neuron(v);
		  }
	  }

  };

  }
}


#endif /* DNN_FF_ALT_MUT_HPP_ */
