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


#ifndef DNN_SPATIAL_HPP_
#define DNN_SPATIAL_HPP_

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>

#include <boost/property_map/vector_property_map.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/datatools/position3.hpp>

#include "opt_placement.hpp"
#include "svg_writer.hpp"


#define DNN_SPATIAL_BACKGROUND_COLOR SvgColor(\
        Params::visualisation::background(0),\
        Params::visualisation::background(1),\
        Params::visualisation::background(2)\
)


namespace sferes
{
namespace gen
{


template<typename N, typename C, typename Params>
class DnnSpatial : public Dnn<N, C, Params>
{
public:
    typedef N neuron_t;
    typedef C conn_t;
    typedef typename gen::DnnSpatial<neuron_t, conn_t, Params> nn_t;
    //typedef nn::NN<N, C> nn_t;

    typedef typename conn_t::weight_t con_weight_t;
    typedef typename neuron_t::af_t::params_t neu_bais_t;
    typedef typename nn_t::io_t io_t;
    typedef typename nn_t::vertex_desc_t vertex_desc_t;
    typedef typename nn_t::edge_desc_t edge_desc_t;
    typedef typename nn_t::graph_t graph_t;
    typedef std::vector<spatial::Pos> coords_t;
    typedef std::vector<vertex_desc_t> layer_t;
    typedef std::map<vertex_desc_t, std::pair<int, int> > layer_map_t;

    void init()
    {
        dbg::trace trace("develop", DBG_HERE);
        Dnn<N, C, Params>::init();
        //      _reinit_layers();
    }
    size_t nb_layers() const { return Params::dnn::spatial::layers_size(); }
    size_t layer_size(size_t n) const { return Params::dnn::spatial::layers(n); }
    size_t get_depth() const { return nb_layers() + 1; }

    const nn_t get_nn() const{return *this;}
    nn_t get_nn(){return *this;}


    void random(int nb_conns)
    {
        _reset();
        _layers.resize(nb_layers());
        this->set_nb_inputs(Params::dnn::spatial::layers(0));
        this->set_nb_outputs(Params::dnn::spatial::layers(nb_layers() - 1));
        _layers[0] = this->get_inputs();
        _layers[nb_layers() - 1] = this->get_outputs();

        assert(_layers[0].size() == Params::dnn::nb_inputs);
        for (size_t i = 1; i < nb_layers() - 1; ++i)
        {
            std::vector<vertex_desc_t> layer;
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j)
            {
                vertex_desc_t v =
                        this->add_neuron("n" + boost::lexical_cast<std::string>(i));
                layer.push_back(v);
                _layer_map[v] = std::make_pair(i, j);
            }
            _layers[i] = layer;
        }

        assert(_layers[nb_layers() - 1].size() == Params::dnn::nb_outputs);

        this->_random_neuron_params();
        _copy_coords();

        for (size_t i = 0; i < nb_conns; ++i)
            this->_add_conn();
        _rand_obj = misc::rand<float>();
    }



    void random()
    {
        _reset();
        _layers.resize(nb_layers());
        this->set_nb_inputs(Params::dnn::spatial::layers(0));
        this->set_nb_outputs(Params::dnn::spatial::layers(nb_layers() - 1));
        _layers[0] = this->get_inputs();
        _layers[nb_layers() - 1] = this->get_outputs();

        assert(_layers[0].size() == Params::dnn::nb_inputs);
        for (size_t i = 1; i < nb_layers() - 1; ++i)
        {
            std::vector<vertex_desc_t> layer;
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j)
            {
                vertex_desc_t v =
                        this->add_neuron("n" + boost::lexical_cast<std::string>(i));
                layer.push_back(v);
                _layer_map[v] = std::make_pair(i, j);
            }
            _layers[i] = layer;
        }

        assert(_layers[nb_layers() - 1].size() == Params::dnn::nb_outputs);

        this->_random_neuron_params();
        _copy_coords();
        if (Params::dnn::spatial::rand_coords)
        {
            typedef boost::mt19937 base_generator_t;
            typedef boost::variate_generator<base_generator_t, boost::uniform_real<> > rand_t;
            assert(Params::seed >= 0);
            rand_t rand_gen(base_generator_t(Params::seed),
                    boost::uniform_real<>(Params::dnn::spatial::min_coord,
                            Params::dnn::spatial::max_coord));
            for (size_t i = 0; i < _layers[0].size(); ++i)
            {
                _xpos[i] = rand_gen();
                _coords[i] = spatial::Pos(_xpos[i], _ypos[i], _zpos[i]);
            }
        }

        size_t nb_conns = misc::rand(Params::dnn::min_nb_conns,
                Params::dnn::max_nb_conns);
        for (size_t i = 0; i < nb_conns; ++i)
            this->_add_conn();
        _rand_obj = misc::rand<float>();
    }


    template<typename Archive>
    void save(Archive& a, const unsigned v) const
    {
        Dnn<N, C, Params>::save(a, v);
        a& BOOST_SERIALIZATION_NVP(_coords);
        a& BOOST_SERIALIZATION_NVP(_xpos);
        a& BOOST_SERIALIZATION_NVP(_ypos);
        a& BOOST_SERIALIZATION_NVP(_zpos);
        //          a& BOOST_SERIALIZATION_NVP(_world_seeds);
    }
    template<typename Archive>
    void load(Archive& a, const unsigned v)
    {
        Dnn<N, C, Params>::load(a, v);
        a& BOOST_SERIALIZATION_NVP(_coords);
        a& BOOST_SERIALIZATION_NVP(_xpos);
        a& BOOST_SERIALIZATION_NVP(_ypos);
        a& BOOST_SERIALIZATION_NVP(_zpos);
        //          a& BOOST_SERIALIZATION_NVP(_world_seeds);

        _reinit_layers();
        if (!Params::dnn::spatial::rand_coords)
            _copy_coords();
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();

    void mutate()
    {
        _reinit_layers();

        this->_change_conns();
        this->_change_neurons();

        if (misc::rand<float>() < Params::dnn::m_rate_add_conn)
            this->_add_conn();

        if (misc::rand<float>() < Params::dnn::m_rate_del_conn
                && this->get_nb_connections() > Params::dnn::min_nb_conns)
            this->_del_conn();

        _rand_obj += misc::gaussian_rand<float>(0.0f, Params::rand_obj::std_dev);
#ifdef BOUNDR
        // add some bounds
        if (_rand_obj > 1) _rand_obj = 1;
        if (_rand_obj < 0) _rand_obj = 0;
#endif
    }

    float rand_obj() const { return _rand_obj; }

    void cross(const DnnSpatial& o, DnnSpatial& c1, DnnSpatial& c2)
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

    void simplifyInlcudingIO(){
        this->simplify();

        std::vector<vertex_desc_t> to_remove;
        BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t){
            if (boost::in_degree(v, this->get_graph()) + boost::out_degree(v, this->get_graph()) == 0){
                to_remove.push_back(v);
            }
        }

        for (size_t i = 0; i < to_remove.size(); ++i){
            remove_vertex(to_remove[i], this->get_graph());
        }
    }

    void simplify(){
        //It is important to build the coordinates map before removing nodes otherwise the map will be invalid.
        this->copy_coords();
        this->reinit_layers();
        Dnn<N, C, Params>::simplify();
    }

    nn_t simplified_nn()
    {
        nn_t nn = *this;

        // remove not-connected vertices
        std::vector<vertex_desc_t> to_remove;

        //          nn.copy_coords();
        //          nn.reinit_layers();
        nn.simplify(); //WARNING: suppress some subgraphs !

        BGL_FORALL_VERTICES_T(v, nn.get_graph(), graph_t){
            if (boost::in_degree(v, nn.get_graph()) + boost::out_degree(v, nn.get_graph()) == 0){
                to_remove.push_back(v);
            }
        }

        for (size_t i = 0; i < to_remove.size(); ++i){
            remove_vertex(to_remove[i], nn.get_graph());
        }
        return nn;
    }

    /**
     * Calculate and return the a coordinate map with the optimal placement for each vertex,
     * where the optimal placement is the placement where each connection has a minimal length.
     */
    std::map<vertex_desc_t, spatial::Pos> get_optimal_coordsmap(){
        dbg::trace trace("optlength", DBG_HERE);
        assert(_layers.size());

        if (this->get_nb_neurons() == 0 || this->get_nb_connections() == 0)
            return std::map<vertex_desc_t, spatial::Pos>();

        // rebuild i/o and copy coordinates
        dbg::out(dbg::info, "optlength") << "Rebuilding i/o and copy coordinates..." << std::endl;
        std::vector<vertex_desc_t> inputs, outputs;
        std::vector<float> x_in, y_in, x_out, y_out;
        assert(_xpos.size() >= this->get_nb_neurons());
        assert(_ypos.size() >= this->get_nb_neurons());
        BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t){
            if (this->get_graph()[v].get_in() != -1)
            {
                inputs.push_back(v);
                x_in.push_back(_xpos[this->get_graph()[v].get_in()]);
                y_in.push_back(_ypos[this->get_graph()[v].get_in()]);
            }
            else if (this->get_graph()[v].get_out() != -1)
            {
                outputs.push_back(v);
                int o = this->get_graph()[v].get_out();
                int n = Params::dnn::spatial::x_size() - _layers[_layers.size() - 1].size();
                assert(n + o < _xpos.size());
                assert(n + o < _ypos.size());
                x_out.push_back(_xpos[n + o]);
                y_out.push_back(_ypos[n + o]);
            }
        }

        // compute the optimal placement
        dbg::out(dbg::info, "optlength") << "Compute optimal placement x..." << std::endl;
        Eigen::VectorXf x_pos = opt_placement::compute(this->get_graph(), inputs, x_in, outputs, x_out);

        dbg::out(dbg::info, "optlength") << "Compute optimal placement y..." << std::endl;
        Eigen::VectorXf y_pos = opt_placement::compute(this->get_graph(), inputs, y_in, outputs, y_out);

        // compute length
        dbg::out(dbg::info, "optlength") << "Build coords map..." << std::endl;
        std::map<vertex_desc_t, spatial::Pos> coords;

        size_t k = 0;
        BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t)
        {
            dbg::out(dbg::info, "optlength") << "Optimal pos: " << x_pos[k] << " " << y_pos[k] << std::endl;
            coords[v] = spatial::Pos(x_pos[k], y_pos[k], 0);
            ++k;
        }

        return coords;
    }

    /**
     * Compute the length of this graph given a coordinate map for its members.
     */
    float compute_length_with_coordsmap(std::map<vertex_desc_t, spatial::Pos> coords, float threshold = 0){
        dbg::trace trace("optlength", DBG_HERE);
        if (this->get_nb_neurons() == 0 || this->get_nb_connections() == 0)
            return 0;

        float length = 0;
        BGL_FORALL_EDGES_T(e, this->get_graph(), graph_t)
        {
            vertex_desc_t src = boost::source(e, this->get_graph());
            vertex_desc_t tgt = boost::target(e, this->get_graph());
            double weight = this->_g[e].get_weight().data(0);
            float d = coords[src].dist(coords[tgt]);
            if (d > threshold) length += d * d;
        }
        return length;
    }

    /**
     * Legacy function to not break older experiments.
     */
    float compute_optimal_length(float threshold = 0,
            bool write_graph = false,
            std::string svg_name = "nn_opt.svg",
            std::map<std::string, size_t> shape_map = std::map<std::string, size_t>())
    {
        dbg::trace trace("optlength", DBG_HERE);
        dbg::out(dbg::info, "optlength") << "Simplifying nn..." << std::endl;
        nn_t nn = simplified_nn();

        dbg::out(dbg::info, "optlength") << "Calculating optimal positions..." << std::endl;
        std::map<vertex_desc_t, spatial::Pos> optimal_coordinates_map = nn.get_optimal_coordsmap();

        dbg::out(dbg::info, "optlength") << "Computing length..." << std::endl;
        float length = nn.compute_length_with_coordsmap(optimal_coordinates_map, threshold);

        if (write_graph)
        {
            dbg::out(dbg::info, "optlength") << "Creating fixed coords map." << std::endl;
            std::map<vertex_desc_t, spatial::Pos> coords_fixed_y;
            std::map<vertex_desc_t, spatial::Pos> original_coordinates_map = nn.getCoordsMap();
            BGL_FORALL_VERTICES_T(v, nn.get_graph(), graph_t){
                spatial::Pos original_coordinate = original_coordinates_map[v];
                spatial::Pos optimal_coordinate = optimal_coordinates_map[v];
                dbg::out(dbg::info, "optlength") << "Original coordinate: " << original_coordinate.x() << " " << original_coordinate.y() << " " << original_coordinate.z() << std::endl;
                dbg::out(dbg::info, "optlength") << "Optimal  coordinate: " << optimal_coordinate.x() << " " << optimal_coordinate.y() << " " << optimal_coordinate.z() << std::endl;
                coords_fixed_y[v] = spatial::Pos(optimal_coordinate.x(), original_coordinate.y(), optimal_coordinate.z());
            }

            dbg::out(dbg::info, "optlength") << "Calculating optimal split." << std::endl;
            std::vector<std::string> mods;
            mod::split(nn.get_graph(), mods);
            std::vector<SvgColor> colors = getColors(mods);

            dbg::out(dbg::info, "optlength") << "Writing graph..." << std::endl;
            std::ofstream ofs(svg_name.c_str());
            SvgWriter writer;
            writer.setColors(colors);
            writer.setDefaultShape(4);
            writer.setShapes(nn.get_graph(), shape_map);
            nn.write_svg(ofs, writer, coords_fixed_y);
        }

        return length;
    }



    std::vector<SvgColor> getColors(const std::vector<std::string>& mods){
        //Assign color to modular split
        //We probably want to move these elsewhere
        std::vector<SvgColor> colors;
        std::vector<SvgColor> color_presets(8);
        color_presets[0] = (SvgColor(0, 0, 133));
        color_presets[1] = (SvgColor(255, 48, 0));
        color_presets[2] = (SvgColor(0, 133, 133));
        color_presets[3] = (SvgColor(255, 200, 0));
        color_presets[4] = (SvgColor(0, 133, 255));
        color_presets[5] = (SvgColor(255, 255, 0));
        color_presets[6] = (SvgColor(0, 133, 0));
        color_presets[7] = (SvgColor(255, 200, 160));

        int color_nr = 0;
        size_t i = 0;
        BGL_FORALL_VERTICES_T(v, this->get_graph(), graph_t){
            dbg::assertion(DBG_ASSERTION(i<mods.size()));
            if(boost::in_degree(v, this->get_graph()) + boost::out_degree(v, this->get_graph()) == 0){
                colors.push_back(SvgColor(125, 125, 125));
            } else{
                size_t param_max_split = Params::visualisation::max_modularity_split;
                //    				size_t max_split = std::min(Params::visualisation::max_modularity_split, mods[i].size());
                size_t max_split = std::min(param_max_split, mods[i].size());
                color_nr=0;
                for(size_t j=0; j<max_split; j++){
                    if(mods[i][j] == '0') color_nr+=pow(2, j);
                }
                colors.push_back(color_presets[color_nr]);
            }
            i++;
        }
        return colors;
    }


    /**
     * Get a coordinate map based on the coordinates supplied by the Params::visualization parameters.
     */
    std::map<vertex_desc_t, spatial::Pos> getVisualizationCoordsmap(){
        std::map<vertex_desc_t, spatial::Pos> result;
        for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i){
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
            {
                result[_layers[i][j]] = spatial::Pos(Params::visualisation::x(k), Params::visualisation::y(k), Params::visualisation::z(k));
                dbg::out(dbg::info, "coordsmap") <<
                        "Layer: " << i <<
                        " neuron: " << j <<
                        " index: " << k <<
                        " x: " << Params::visualisation::x(k) <<
                        " y: " << Params::visualisation::y(k) <<
                        " z: " << Params::visualisation::z(k) <<
                        std::endl;
            }
        }

        return result;
    }


    /**
     * Writes an svg using an svg writer. The svg writer can be configured extensively before it is being passed to this function.
     */
    void write_svg(std::ofstream& ofs, SvgWriter& writer, std::map<vertex_desc_t, spatial::Pos>& coords_map)
    {
        //        	for(size_t i = 0; i<Params::visualisation::axis_order_size(); i++){
        //        		writer.setAxis(i, Params::visualisation::axis_order(i));
        //        	}
        //        	writer.setScale(Params::visualisation::scale);
        writer.setScaledBoundaries(_min_xpos, _max_xpos, _min_ypos, _max_ypos);
        writer.setBackground(DNN_SPATIAL_BACKGROUND_COLOR);
        writer.to_svg(this->get_graph(), coords_map, ofs);
    }


    /**
     * Legacy function as to not break old code.
     */
    void write_svg(std::ofstream& ofs)
    {
        _reinit_layers();

        nn_t nn = *this;
        nn.init_coords_and_layers();

        std::map<vertex_desc_t, spatial::Pos> coords_map;
        for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i){
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
            {
                coords_map[nn._layers[i][j]] = spatial::Pos(Params::visualisation::x(k), Params::visualisation::y(k), Params::visualisation::z(k));
            }
        }
        nn.simplify();

        SvgWriter writer;
        //        	for(size_t i = 0; i<Params::visualisation::axis_order_size(); i++){
        //        		writer.setAxis(i, Params::visualisation::axis_order(i));
        //        	}
        //        	writer.setScale(Params::visualisation::scale);

        writer.setScaledBoundaries(_min_xpos, _max_xpos, _min_ypos, _max_ypos);
        writer.setBackground(DNN_SPATIAL_BACKGROUND_COLOR);
        writer.setPlotBias();
        writer.to_svg(nn.get_graph(), coords_map, ofs);
    }

    void write_csv(std::ofstream& ofs){
        _reinit_layers();
        nn_t nn = *this;
        nn.init_coords_and_layers();
        graph_t& g = nn.get_graph();

        std::map<vertex_desc_t, spatial::Pos> coords_map;
        for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i){
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
            {
                coords_map[nn._layers[i][j]] = spatial::Pos(Params::dnn::spatial::x(k), Params::dnn::spatial::y(k), Params::dnn::spatial::z(k));
            }
        }

        BGL_FORALL_VERTICES_T(v, g, graph_t){
            bool first = true;
            BGL_FORALL_VERTICES_T(u, g, graph_t){
                std::pair<edge_desc_t, bool> edge_p = boost::edge(v,u,g);
                if(!first){
                    ofs << ", ";
                }
                first = false;
                if(edge_p.second){
                    ofs << g[edge_p.first].get_weight().data(0);
                } else {
                    ofs << "0";
                }
            }
            ofs << "\n";
        }
        ofs << "\n";

        BGL_FORALL_VERTICES_T(v, g, graph_t){
            ofs << g[v].get_id() << ", ";
            //ofs << g[v].get_label() << ", ";
            ofs << g[v].get_afparams().data(0) << ", ";
            ofs << g[v].get_afparams().data(1) << ", ";
            ofs << coords_map[v].x() << ", ";
            ofs << coords_map[v].y() << ", ";
            ofs << coords_map[v].z() << "\n";
        }
    }


    float compute_length()
    {
        _reinit_layers(); //to remove ?
        float length = 0;
        BGL_FORALL_EDGES_T(e, this->_g, graph_t)
        {
            vertex_desc_t src = boost::source(e, this->_g);
            vertex_desc_t tgt = boost::target(e, this->_g);
            double weight = this->_g[e].get_weight().data(0);
            float d = _coords_map[src].dist(_coords_map[tgt]);
            length += d * d;
        }
        return length;
    }

    float compute_length(float min_length)
    {
        _reinit_layers(); //to remove ?
        float length = 0;
        BGL_FORALL_EDGES_T(e, this->_g, graph_t)
        {
            vertex_desc_t src = boost::source(e, this->_g);
            vertex_desc_t tgt = boost::target(e, this->_g);
            double weight = this->_g[e].get_weight().data(0);
            float l = _coords_map[src].dist(_coords_map[tgt]);
            length += l > min_length ? l : 0;
        }
        return length;
    }

    float max_length()
    {
        _reinit_layers(); //to remove ?
        float length = 0;
        BGL_FORALL_EDGES_T(e, this->_g, graph_t)
        {
            vertex_desc_t src = boost::source(e, this->_g);
            vertex_desc_t tgt = boost::target(e, this->_g);
            double weight = this->_g[e].get_weight().data(0);
            length = std::max(_coords_map[src].dist(_coords_map[tgt]), length);
            //std::cout<<_coords_map[src].dist(_coords_map[tgt])<<std::endl;
        }
        return length;
    }

    void write_dot(std::ostream& ofs)
    {
        graph_t network = this->_g;
        ofs << "digraph G {" << std::endl;
        BGL_FORALL_VERTICES_T(v, network, graph_t)
        {
            ofs << network[v].get_id();
            ofs << " [label=\""<< network[v].get_id();
            ofs << " af"<< nn::trait<neu_bais_t>::single_value(network[v].get_afparams());
            // ofs << "| pf"<< this->network.get_pfparams() <<"\"";
            ofs <<"\"";
            if (network[v].is_input() || network[v].is_output())
                ofs<<" shape=doublecircle";

            ofs <<"]"<< std::endl;
        }
        BGL_FORALL_EDGES_T(e, network, graph_t)
        {
            ofs << network[source(e, network)].get_id()
            		         << " -> " << network[target(e, network)].get_id()
            		         << "[label=\"" << nn::trait<con_weight_t>::single_value(network[e].get_weight()) << "\"]" << std::endl;
        }
        ofs << "}" << std::endl;
    }


    const layer_t& layer(size_t n) { assert(n < _layers.size()); return _layers[n]; }

    void init_coords_and_layers(){
        copy_coords();
        reinit_layers();
    }

    void copy_coords(){
        _copy_coords();
    }

    void reinit_layers(){
        _reinit_layers();
    }

    std::map<vertex_desc_t, spatial::Pos>& getCoordsMap(){
        return _coords_map;
    }

    void stepAll(const std::vector<io_t>& inf)
    {
        assert(this->_init_done);
        // in
        this->_set_in(inf);

        // activate
        for(size_t i=0; i<_layers.size(); i++){
            for(size_t j=0; j<_layers[i].size(); j++){
                this->_activate(_layers[i][j]);
                this->_g[_layers[i][j]].step();
            }
        }

        // out
        this->_set_out();
    }

    //        void add_world_seed(size_t world_seed){
    //        	_world_seeds.push_back(world_seed);
    //        }
    //
    //        size_t get_world_seed(){
    //        	return _world_seeds.back();
    //        }


protected:
    std::vector<layer_t> _layers;
    std::vector<float> _xpos, _ypos, _zpos;
    float _min_xpos, _max_xpos, _min_ypos, _max_ypos, _min_zpos, _max_zpos;
    std::map<vertex_desc_t, spatial::Pos> _coords_map;
    coords_t _coords;
    layer_map_t _layer_map;
    float _rand_obj;
    //        std::vector<size_t> _world_seeds;

    void _reset()
    {
        _coords_map.clear();
        //      _coords.clear();
        _layers.clear();
        _layer_map.clear();
        _layers.resize(nb_layers());
    }

    void _reinit_layers()
    {
        _reset();

        //Make the input and output layers
        _layers[0] = this->get_inputs();
        _layers[nb_layers() - 1] = this->get_outputs();
        for (size_t k = 0; k < this->_inputs.size(); ++k)
            _layer_map[this->_inputs[k]] = std::make_pair(0, k);
        for (size_t k = 0; k < this->_outputs.size(); ++k)
            _layer_map[this->_outputs[k]] = std::make_pair(nb_layers() - 1, k);


        int l = 1, k = 0;
        BGL_FORALL_VERTICES_T(v, this->_g, graph_t){
            if (!this->is_input(v) && !this->is_output(v))
            {
                _layers[l].push_back(v);
                _layer_map[v] = std::make_pair(l, k);
                ++k;
                if (k == Params::dnn::spatial::layers(l))
                {
                    k = 0;
                    ++l;
                }
            }
        }

        // build coordinates map
        for (size_t i = 0, k = 0; i < _layers.size(); ++i)
            for (size_t j = 0; j < _layers[i].size(); ++j, ++k)
            {
                assert(_coords.size());
                assert(k < _coords.size());
                _coords_map[_layers[i][j]] = _coords[k];
            }
    }

    void _change_conns()
    {
        float p = Params::dnn::m_avg_weight / (float) this->get_nb_connections();

        BGL_FORALL_EDGES_T(e, this->_g, graph_t)
        if (misc::rand<float>() < p)
            this->_g[e].get_weight().mutate();
        BGL_FORALL_EDGES_T(e, this->_g, graph_t)
        if (misc::rand<float>() < Params::dnn::m_rate_change_conn)
        {
            vertex_desc_t src = source(e, this->_g);
            vertex_desc_t tgt = target(e, this->_g);
            size_t src_layer = _layer_map[src].first;
            size_t tgt_layer = _layer_map[tgt].first;

            int max_tries = num_vertices(this->_g) * num_vertices(this->_g),
                    nb_tries = 0;
            if (misc::flip_coin())
                do
                    src =
                            _layers[src_layer][misc::rand < int > (0, _layers[src_layer].size())];
                while (++nb_tries < max_tries && is_adjacent(this->_g, src, tgt));
            else
                do
                    tgt =
                            _layers[tgt_layer][misc::rand < int > (0, _layers[tgt_layer].size())];
                while (++nb_tries < max_tries && is_adjacent(this->_g, src, tgt));
            if (nb_tries < max_tries)
            {
                typename nn_t::weight_t w = this->_g[e].get_weight();
                remove_edge(e, this->_g);
                this->add_connection(src, tgt, w);
            }
            return;
        }
    }

    void _copy_coords()
    {
        _xpos.clear();
        _ypos.clear();
        _zpos.clear();
        _coords.clear();
        _min_xpos = Params::dnn::spatial::x(0);
        _max_xpos = Params::dnn::spatial::x(0);
        _min_ypos = Params::dnn::spatial::y(0);
        _max_ypos = Params::dnn::spatial::y(0);
        _min_zpos = Params::dnn::spatial::z(0);
        _max_zpos = Params::dnn::spatial::z(0);
        for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i)
            for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
            {
                _xpos.push_back(Params::dnn::spatial::x(k));
                _ypos.push_back(Params::dnn::spatial::y(k));
                _zpos.push_back(Params::dnn::spatial::z(k));
                _coords.push_back(spatial::Pos(_xpos[k], _ypos[k], _zpos[k]));
                if(_xpos[k] < _min_xpos) _min_xpos = _xpos[k];
                if(_xpos[k] > _max_xpos) _max_xpos = _xpos[k];
                if(_ypos[k] < _min_ypos) _min_ypos = _ypos[k];
                if(_ypos[k] > _max_ypos) _max_ypos = _ypos[k];
                if(_zpos[k] < _min_zpos) _min_zpos = _zpos[k];
                if(_zpos[k] > _max_zpos) _max_zpos = _zpos[k];
            }
    }

    void _add_conn()
    {
        vertex_desc_t src, tgt;
        size_t max_tries = num_vertices(this->_g) * num_vertices(this->_g),
                nb_tries = 0;
        do
        {
            size_t l = misc::rand<int>(0, nb_layers() - 1);
            src = _layers[l][misc::rand < int > (0, _layers[l].size())];
            tgt = _layers[l + 1][misc::rand < int > (0, _layers[l + 1].size())];
        }
        while (is_adjacent(this->_g, src, tgt) && ++nb_tries < max_tries);
        if (nb_tries < max_tries)
        {
            typename nn_t::weight_t w; // = nn::trait<typename
            // nn_t::weight_t>::zero();
            //      w.gen().data(0, 0);
            w.random();
            this->add_connection(src, tgt, w);
        }
    }

    //Shortest path algorithm that only works on simplified feed-forward networks, created to account for a bug in the boost library.
    size_t custom_shortest_path(vertex_desc_t v, graph_t g){
        vertex_desc_t current;
        typename boost::graph_traits<graph_t>::out_edge_iterator e, e_end;
        boost::tie(e, e_end) = boost::out_edges(v, g);

        size_t result = 0;
        while(e != e_end){
            edge_desc_t vertexID = *e;
            current = boost::target(vertexID, g);
            boost::tie(e, e_end) = boost::out_edges(current, g);
            result++;
        }

        return result;
    }

    template<typename G>
    void _compute_all_depths(const G& g,
            const std::vector<vertex_desc_t>& outputs,
            std::map<vertex_desc_t, size_t>& final_dmap)
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
        BGL_FORALL_EDGES_T(e, g, graph_t)
        em[e] = 1;
        e_map_t wmap(em);

        BGL_FORALL_VERTICES_T(v, g, graph_t)
        final_dmap[v] = 1e10;

        //  // we compute the shortest path between outputs and v


        //        	BOOST_FOREACH(vertex_desc_t s, outputs)
        //        	{
        //        		int_map_t im; a_map_t dmap(im);
        //        		           dag_shortest_paths(make_reverse_graph(g), s, dmap, wmap, cmap, pmap,
        //        		 dijkstra_visitor<null_visitor>(),
        //        		 std::greater<int>(),
        //        		 closed_plus<int>(),
        //        		 (std::numeric_limits<int>::min)(), 0);
        //
        //        		BGL_FORALL_VERTICES_T(v, g, graph_t)
        //        		{
        //        			size_t d = get(dmap, v);
        //        			if (d <= num_vertices(g))
        //        				final_dmap[v] = std::min(final_dmap[v], d);
        //        		}
        //        	}

        size_t d = 0;
        //size_t max = 0;
        BGL_FORALL_VERTICES_T(v, g, graph_t){
            d = custom_shortest_path(v, g);
            //if(d>max) max = d;
            final_dmap[v] = d;
        }

    }
};

}
}

#endif
