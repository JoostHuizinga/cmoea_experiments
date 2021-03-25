#ifndef C_MODULARITY_HPP_
#define C_MODULARITY_HPP_

template<typename NN>
float compute_mod_simplified(const NN& nn_)
{
  // copy
  NN nn = nn_;
  // remove useless subgraphs
  //nn.simplify();
  // remove low weights
  std::list<typename NN::edge_desc_t> to_remove;
  BGL_FORALL_EDGES_T(e, nn.get_graph(), typename NN::graph_t)
    {
      typename NN::weight_t weight = nn.get_graph()[e].get_weight();
      float w = nn::trait<typename NN::weight_t>::single_value(weight);
      if (fabs(w) < 0.05)// here is the threshold
	to_remove.push_back(e);
    }
  // we need to do this to avoid any edge invalidation
  while (!to_remove.empty())
    {
      BGL_FORALL_EDGES_T(e, nn.get_graph(), typename NN::graph_t)
	if (to_remove.front() == e)
	  {
	    remove_edge(e, nn.get_graph());
	    to_remove.pop_front();
	    break;
	  }
    }
  // compute the Q score
  float q = mod::modularity(nn.get_graph());
  return q;  
}



/**
 * Analysis of the left-right modularity. If the network receives an s score of 8 the network is perfectly left-right modular.
 */
template<typename I>
int analysis_io_mod(I& indiv, const std::set<int>& left, const std::set<int>& right) {
//	using namespace boost::assign;
	typename I::nn_t nn = indiv.nn().simplified_nn();
	std::vector<std::string>  mods;

	if (boost::num_edges(nn.get_graph()) == 0 || boost::num_vertices(nn.get_graph()) == 0){
//			left_right_mod = false;
		return 0;
	}
	mod::split(nn.get_graph(), mods);

	int m_0_left = 0, m_1_left = 0, m_0_right = 0, m_1_right = 0;;
	int i=0;
	BGL_FORALL_VERTICES_T(v, nn.get_graph(), typename I::nn_t::graph_t){
		if (left.find(nn.get_graph()[v].get_in()) != left.end()){
			if(mods[i].substr(0, 1) != "0"){
				m_1_left++;
			} else {
				m_0_left++;
			}
		}
		if (right.find(nn.get_graph()[v].get_in()) != right.end()){
			if(mods[i].substr(0, 1) != "0"){
				m_1_right++;
			} else {
				m_0_right++;
			}
		}
		++i;
	}
	int s = abs(m_1_left - m_1_right) + abs(m_0_left - m_0_right);

//		float qs = compute_mod_simplified(indiv.nn());

//	std::cout << "- IOM: " << s << std::endl;
//		std::cout << "- QS: " << qs << std::endl;

//		if(s==8){
//			left_right_mod = true;
//		} else {
//			left_right_mod = false;
//		}

	return s;
}

#endif
