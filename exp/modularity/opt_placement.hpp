#ifndef _OPT_PLACEMENT_HPP_
#define _OPT_PLACEMENT_HPP_


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/foreach.hpp>
#include <Eigen/Core>
#include <Eigen/LU>
#include "mod2.hpp"
#include <modules/modularity/modularity.hpp>
//#include "svg_writer.hpp"
#include <math.h>

namespace opt_placement
{
  template<typename G>
  inline bool _linked(const G& g,
                      const typename G::vertex_descriptor& v1,
                      const typename G::vertex_descriptor& v2)
  {
    BGL_FORALL_OUTEDGES_T(v1, e, g, G)
    if (target(e, g) == v2)
      return true;
    return false;
  }

   // consider _only_ verts in the verts set
  template<typename G>
  inline Eigen::MatrixXf _laplacian(const G& g,
                                    const std::set<typename G::vertex_descriptor>& verts)
  {
    using namespace Eigen;
    using namespace boost;

    MatrixXf l = MatrixXf::Zero(num_vertices(g), num_vertices(g));
    size_t i = 0;
    BGL_FORALL_VERTICES_T(v1, g, G)
    {
      size_t j = 0;
      BGL_FORALL_VERTICES_T(v2, g, G)
      {
        if (verts.find(v1) == verts.end()
            || verts.find(v2) == verts.end())
          l(i, j) = 0;
        else
        if (i == j)
          l(i, j) = in_degree(v1, g) + out_degree(v1, g);
        else
        if (_linked(g, v1, v2) || _linked(g, v2, v1))
          l(i, j) = -1;
        else
          l(i, j) = 0;
        ++j;
      }
      ++i;
    }
    return l;
  }
   // include in the adjacency iff source _or_ target is in verts
  template<typename G>
  inline Eigen::MatrixXf _adjacency(const G& g,
                                    const std::set<typename G::vertex_descriptor>& verts)
  {
    using namespace Eigen;
    using namespace boost;

    MatrixXf a = MatrixXf::Zero(num_vertices(g), num_vertices(g));
    size_t i = 0;
    BGL_FORALL_VERTICES_T(v1, g, G)
    {
      size_t j = 0;
      BGL_FORALL_VERTICES_T(v2, g, G)
      {
        if ((verts.find(v1) != verts.end()
             || verts.find(v2) != verts.end())
            && (_linked(g, v1, v2) || _linked(g, v2, v1)))
          a(i, j) = 1;
        else
          a(i, j) = 0;
        ++j;
      }
      ++i;
    }
    return a;
  }

   // include in the degree matrix iff vertex is in verts
  template<typename G>
  inline Eigen::MatrixXf _degree(const G& g,
                                 const std::set<typename G::vertex_descriptor>& verts)
  {
    using namespace Eigen;
    using namespace boost;

    MatrixXf d = MatrixXf::Zero(num_vertices(g), num_vertices(g));
    size_t i = 0;
    BGL_FORALL_VERTICES_T(v, g, G)
    {
      if (verts.find(v) != verts.end())
        d(i, i) = in_degree(v, g) + out_degree(v, g);
      ++i;
    }
    return d;
  }
  template<typename V, typename T>
  inline int _find_index(const V& vect, const T& e)
  {
    for (size_t i = 0; i < vect.size(); ++i)
      if (e == vect[i])
        return i;
    return -1;
  }
  template<typename G>
  inline Eigen::VectorXf _f(const G& g,
                            const std::vector<typename G::vertex_descriptor>& inputs,
                            const std::vector<float>& coords_inputs,
                            const std::vector<typename G::vertex_descriptor>& outputs,
                            const std::vector<float>& coords_outputs)
  {
    using namespace boost;
    Eigen::VectorXf f = Eigen::VectorXf::Zero(num_vertices(g));
    size_t i = 0, i_in = 0, i_out = 0;
    assert(inputs.size() == coords_inputs.size());
    assert(outputs.size() == coords_outputs.size());

    BGL_FORALL_VERTICES_T(v, g, G)
    {
      int in = _find_index(inputs, v);
      assert(in < (int)coords_inputs.size());
      if (in != -1)
        f(i) = coords_inputs[in];
      int out = _find_index(outputs, v);
      assert(out < (int)coords_outputs.size());
      if (out != -1)
        f(i) = coords_outputs[out];
      ++i;
    }
    return f;
  }

//  template<typename G>
//  inline void to_svg(const G& g,
//		  const Eigen::VectorXf& x_coords,
//		  const Eigen::VectorXf& y_coords,
//		  std::ofstream& ofs)
//  {
//	  std::vector<std::string> mods;
//	  mod::split(g, mods);
//
//	  ofs << "<?xml version=\"1.0\" standalone=\"no\"?>" << std::endl;
//	  ofs << "<svg width=\"400\" height=\"250\" version=\"1.1\""
//			  << " xmlns=\"http://www.w3.org/2000/svg\">"
//			  << "<rect style=\"fill:#aabbcc;fill-opacity:1;stroke:none\" "
//			  << " width=\"400\" height=\"250\" x=\"0\" y=\"1.059322\"/>"
//			  << "<g transform=\"translate(-300,-80)\">" // WARNING translation
//			  << std::endl;
//	  typedef typename G::vertex_descriptor v_t;
//
//	  std::map<v_t, float> x_map, y_map;
//
//	  Eigen::VectorXf x_c = x_coords;
//	  for (size_t i = 0; i < x_c.size(); ++i)
//		  for (size_t j = 0; j < x_c.size(); ++j)
//			  if (i != j && fabs(x_c(i) - x_c(j)) < 0.1)
//			  {
//				  x_c(i) += 0.1;
//				  x_c(j) -= 0.1;
//			  }
//
//	  size_t i = 0;
//	  BGL_FORALL_VERTICES_T(v, g, G)
//	  {
//		  x_map[v] = x_c(i);
//		  y_map[v] = y_coords(i);
//		  ++i;
//	  }
//
//	  BGL_FORALL_EDGES_T(e, g, G)
//	  {
//		  v_t src = boost::source(e, g);
//		  v_t tgt = boost::target(e, g);
//		  float x1 = x_map[src] * 50 + 500;
//		  float y1 = y_map[src] * 50 + 100;
//		  float x2 = x_map[tgt] * 50 + 500;
//		  float y2 = y_map[tgt] * 50 + 100;
//		  double weight = 1.5; //g[e].get_weight().data(0);
//		  ofs << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" "
//				  << "x2=\"" << x2 << "\" y2=\"" << y2 << "\""
//				  << " style=\"stroke:rgb("
//				  << (weight > 0 ? "0,0,0" : "0,0,0")
//				  << ");stroke-width:" << fabs(weight)
//				  << "\"/>"
//				  << std::endl;
//	  }
//
//	  i=0;
//	  BGL_FORALL_VERTICES_T(v, g, G)
//	  {
//		  float x = x_map[v] * 50 + 500;
//		  float y = y_map[v] * 50 + 100;
//		  ofs << "<circle cx=\"" << x << "\" cy=\"" << y
//				  << "\" r=\"10\" stroke=\"black\" stroke-width=\"1\" fill=\""
//				  << (mods[i].substr(0, 1) != "0" ? "#00008F" : "#FF3000")
//				  << "\"/>" << std::endl;
//		  //if(mods[i] != "0" && mods[i] !="1")
//		//	  std::cout << mods[i] << std::endl;
//		//  std::cout << mods[i];
//		  //      ofs << "<text id=\"TextElement\" x=\"" << x - 5 << "\" y=\"" << y + 5 << "\""
//		  //  << " style=\"font-family:Verdana;font-size:10\">"
//		  // << g[v]._id
//		  // << "</text>" << std::endl;
//		  ++i;
//	  }
//	  //std::cout << std::endl;
//	  ofs << "</g>" << std::endl;
//	  ofs << "</svg>" << std::endl;
//  }

  template<typename G>
  inline Eigen::VectorXf compute(const G& g,
                                 const std::vector<typename G::vertex_descriptor>&
                                 inputs,
                                 const std::vector<float>& coords_inputs,
                                 const std::vector<typename G::vertex_descriptor>&
                                 outputs,
                                 const std::vector<float>& coords_outputs)
  {
    using namespace Eigen;
    typedef typename G::vertex_descriptor v_d_t;
    std::set<v_d_t> all_set, io_set, no_io_set;
    BGL_FORALL_VERTICES_T(v, g, G) all_set.insert(v);
    BOOST_FOREACH(v_d_t v, inputs) io_set.insert(v);
    BOOST_FOREACH(v_d_t v, outputs) io_set.insert(v);
    std::set_difference(all_set.begin(), all_set.end(),
                        io_set.begin(), io_set.end(),
                        std::insert_iterator<std::set<v_d_t> >(no_io_set,
                                                               no_io_set.begin()));
    MatrixXf l = _laplacian(g, no_io_set);
    MatrixXf b = _adjacency(g, io_set);
    MatrixXf d = _degree(g, io_set);
    VectorXf f = _f(g, inputs, coords_inputs, outputs, coords_outputs);
    VectorXf r = (l + d).inverse() * b * f + f;
     //    std::cout << "l\n" << l << std::endl
     //              << "b\n" << b << std::endl
     //              << "d\n" << d << std::endl
     //              << "f\n" << f.transpose() << std::endl
     //              << "r\n" << r.transpose() << std::endl
     //              << "num edge" << num_edges(g) << std::endl;
    return r;
  }
}
#endif
