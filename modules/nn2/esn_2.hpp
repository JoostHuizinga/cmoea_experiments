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




#ifndef _NN_ESN_HPP_
#define _NN_ESN_HPP_

#include <Eigen/Core>
#include <Eigen/Array>
#include <Eigen/QR> 

#include <sferes/misc/rand.hpp>
#include <sferes/misc.hpp>

#include "trait.hpp"
#include "nn.hpp"
#include "connection.hpp"
#include "neuron.hpp"

namespace nn
{
  // a basic echo state network
  template<typename N, typename C>
  class Esn_2 : public NN<N, C>
  {
  public:
    typedef nn::NN<N, C> nn_t;
    typedef typename nn_t::io_t io_t;
    typedef typename nn_t::vertex_desc_t vertex_desc_t;
    typedef typename nn_t::edge_desc_t edge_desc_t;
    typedef typename nn_t::adj_it_t adj_it_t;    
    typedef typename nn_t::graph_t graph_t;
    typedef N neuron_t;
    typedef C conn_t;
    typedef typename N::weight_t weight_t;

    /// @see http://aureservoir.sourceforge.net/init_8hpp-source.html
    //Constructor
    Esn_2(size_t nb_inputs, 
	  size_t nb_hidden,
	  size_t nb_outputs,
	  float rho, float alpha,
	  float input_connexity)
    {
      // neurons
      this->set_nb_inputs(nb_inputs + 1);
      this->set_nb_outputs(nb_outputs);
      for (size_t i = 0; i < nb_hidden; ++i)
	_hidden_neurons.
	  push_back(this->add_neuron(std::string("h") 
				     + boost::lexical_cast<std::string>(i)));
      
      //generate graph    
      this->full_connect(this->_inputs, this->_hidden_neurons, trait<weight_t>::zero());
      this->full_connect(this->_hidden_neurons, this->_outputs, trait<weight_t>::zero());

      _random_neuron_params();
      
    }
    void set_input_weights(const std::vector<weight_t>& ws)
    {
      assert(this->_inputs.size()*_hidden_neurons.size() == ws.size());
      size_t k = 0;
      BOOST_FOREACH(vertex_desc_t& u, this->_inputs)
	BGL_FORALL_OUTEDGES_T(u, e, this->_g, graph_t)
	this->_g[e].set_weight(ws[++k]);
    }
    void set_hidden_weights(Eigen::MatrixXf& conn)
    {
      int i_mat=0;
      int j_mat=0;
      BOOST_FOREACH(vertex_desc_t& i, _hidden_neurons) {
	BOOST_FOREACH(vertex_desc_t& j, _hidden_neurons) {	  
	  if(conn(i_mat, j_mat)!=0.0)
	    this->add_connection(i, j, conn(i_mat,j_mat));
	  j_mat++;
	}
	j_mat=0;
	i_mat++;
      }
    }
    void set_output_weights(const std::vector<weight_t>& ws)
    {
      assert(this->_outputs.size()*_hidden_neurons.size() == ws.size());
      size_t k = 0;
      BOOST_FOREACH(vertex_desc_t& u, this->_outputs)
	BGL_FORALL_INEDGES_T(u, e, this->_g, graph_t)
	this->_g[e].set_weight(ws[++k]);
    }

    unsigned get_nb_inputs() const { return this->_inputs.size() - 1; }
    unsigned get_nb_hidden() const { return _hidden_neurons.size(); }
    void step(const std::vector<io_t>& in)
    {
      assert(in.size() == this->get_nb_inputs());
      std::vector<io_t> inf = in;
      inf.push_back(1.0f);
      nn_t::_step(inf);
    }
  protected:
    std::vector<vertex_desc_t> _hidden_neurons;

    void _random_neuron_params()
    {
      BGL_FORALL_VERTICES_T(v, this->_g, graph_t)
	{
	  this->_g[v].get_pfparams().random();
	  this->_g[v].get_afparams().random();
	}
    }
    /*typename nn_t::weight_t _random_weight()
    {
      typename nn_t::weight_t w;
      w.random();
      return w;
      }*/
  };

  Eigen::MatrixXf esn_conn_i;
  std::vector<float> esn_input_weights_i;
  void esn_generate(size_t nb_inputs, 
				 size_t nb_hidden,
				 size_t nb_outputs,
				 float rho, float alpha,
				 float input_connexity)
  {
    Eigen::MatrixXf conn = Eigen::MatrixXf::Zero(nb_hidden,
						 nb_hidden);
    for (size_t j = 0; j < conn.cols(); ++j)
      for (size_t i = 0; i < conn.rows(); ++i)
	if (sferes::misc::rand<float>() < alpha)
	  conn(i, j) = sferes::misc::rand(-1.0f, 1.0f);
    // spectral radius
    float max = (conn.eigenvalues().real().cwise().square()
		 + conn.eigenvalues().imag().cwise().square()
		 ).maxCoeff();
    assert(max > 0);
    conn *= alpha / sqrtf(max);
    esn_conn_i = conn;
    for (size_t i = 0; i < nb_inputs*nb_hidden; i++)
      esn_input_weights_i.push_back(sferes::misc::rand(-1.0f, 1.0f));
  }
  
  // a basic ESN with float weights
  typedef Esn_2<Neuron<PfWSum<>, AfSigmoidNoBias<> >, Connection<> > esn_2_t;

}

#endif
