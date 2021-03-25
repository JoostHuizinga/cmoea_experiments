/*
 * test_hierarchy.cpp
 *
 *  Created on: Apr 7, 2017
 *      Author: Joost Huizinga
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE hierarchy

// Boost includes
#include <boost/test/unit_test.hpp>

// Sferes includes
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/fit/fitness.hpp>

// Module includes
#include <modules/nn2/nn.hpp>
#include <modules/nn2/pf.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>

// Local includes
#include "hierarchy.hpp"

using namespace sferes;
using namespace boost;
using namespace std;

struct Params {
    static long seed;
    static bool init;

    //These values are for evolving the substrate directly,
    //They are not used when HyperNEAT is active

    // for weights
    struct weights
    {
        struct evo_float
        {
            SFERES_CONST float cross_rate = 0.5f;
            SFERES_CONST float mutation_rate = 1.0f;
            SFERES_CONST float eta_m = 10.0f;
            SFERES_CONST float eta_c = 10.0f;
            SFERES_CONST gen::evo_float::mutation_t mutation_type = gen::evo_float::polynomial;
            SFERES_CONST gen::evo_float::cross_over_t cross_over_type = gen::evo_float::sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, -2, -1, 1, 2); // no 0 !
            SFERES_CONST float mutation_rate = 1.0f;
            SFERES_CONST float cross_rate = 0.5f;
            SFERES_CONST bool ordered = true;
        };
        struct parameters
        {
            SFERES_CONST float min = 0;
            SFERES_CONST float max = 1;
        };
    };

    //For the bias
    struct bias
    {
        struct evo_float
        {
            SFERES_CONST float cross_rate = 0.5f;
            SFERES_CONST float mutation_rate = 1.0f / 24.0f;
            SFERES_CONST float eta_m = 10.0f;
            SFERES_CONST float eta_c = 10.0f;
            SFERES_CONST gen::evo_float::mutation_t mutation_type = gen::evo_float::polynomial;
            SFERES_CONST gen::evo_float::cross_over_t cross_over_type = gen::evo_float::sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, -2, -1, 0, 1, 2);
            SFERES_CONST float mutation_rate = 1.0 / 24.0f;
            SFERES_CONST float cross_rate = 0.0f;
            SFERES_CONST bool ordered = false;
        };
        struct parameters
        {
            SFERES_CONST float min = 0;
            SFERES_CONST float max = 1;
        };
    };

    struct dnn
    {
        // slope of the sigmoid
        SFERES_CONST size_t lambda = 20;

        //Add Connection rate for the cppn
        SFERES_CONST float m_rate_add_conn = 0.2f;
        //Delete connection rate for the cppn
        SFERES_CONST float m_rate_del_conn = 0.2f;

        //SFERES_CONST float m_rate_change_conn = 0.15; //old default
        SFERES_CONST float m_rate_change_conn = 0.0; //new default?
        SFERES_CONST float m_rate_add_neuron = 0.0f;
        SFERES_CONST float m_rate_del_neuron = 0.0f;
        // average number of weight changes
        SFERES_CONST float m_avg_weight = 2.0f;

        SFERES_CONST int io_param_evolving = true;
        SFERES_CONST gen::dnn::init_t init = gen::dnn::ff;

        SFERES_CONST size_t nb_inputs = 5;
        SFERES_CONST size_t nb_outputs = 1; //18
        SFERES_CONST size_t nb_of_hidden = 10;

        SFERES_CONST size_t min_nb_neurons = 10; //useless
        SFERES_CONST size_t max_nb_neurons = 30; //useless
        SFERES_CONST size_t min_nb_conns = 0; // ?? TODO
        SFERES_CONST size_t max_nb_conns = 120; // ?? TODO

    };

};
typedef fit::FitDummy<> dum_fit_t;
typedef gen::EvoFloat<1, Params::weights> weight_gen_t;
typedef gen::EvoFloat<1, Params::bias> bias_gen_t;
typedef phen::Parameters<weight_gen_t, dum_fit_t, Params::weights> weight_t;
typedef phen::Parameters<bias_gen_t, dum_fit_t, Params::bias> bias_t;
typedef nn::PfWSum<weight_t> pf_t;
typedef nn::Af<bias_t> af_t;
typedef nn::Neuron<pf_t, af_t> neu_t;
typedef nn::Connection<weight_t> con_t;


typedef adjacency_list<listS, listS, bidirectionalS, neu_t, con_t> graph_t;
typedef typename graph_traits<graph_t>::vertex_iterator vertex_it_t;
typedef typename graph_traits<graph_t>::edge_iterator edge_it_t;
typedef typename graph_traits<graph_t>::out_edge_iterator out_edge_it_t;
typedef typename graph_traits<graph_t>::in_edge_iterator in_edge_it_t;
typedef typename graph_traits<graph_t>::edge_descriptor edge_desc_t;
typedef typename graph_traits<graph_t>::vertex_descriptor vert_t;
typedef typename graph_traits<graph_t>::adjacency_iterator adj_it_t;

int id = 0;

template<typename graph_t>
vert_t addNeuron(graph_t& graph){
	vert_t v0 = add_vertex(graph);
	graph[v0]._id = lexical_cast<std::string>(id);
	id++;
	return v0;
}

template<typename graph_t>
void addCon(graph_t& graph, vert_t v1, vert_t v2, float weight = 1.f){
	std::pair<edge_desc_t, bool> e = add_edge(v1, v2, graph);
	weight_t w;
	w.gen().data(0, weight);
	w.develop();
	BOOST_CHECK(e.second);
	if (e.second){
		graph[e.first].set_weight(w);
		graph[e.first].get_weight().develop();
//		float real_weight = graph[e.first].get_weight().data(0);
//		std::cout << "Con weight: "<< real_weight << std::endl;
	}
}

BOOST_AUTO_TEST_CASE(hierarchy)
{
	// Test graph with perfect hierarchy (star graph)
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v2, v0, 1);
		addCon(graph, v2, v1, 1);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 1:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 1, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 1, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 0.25, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 0.25, 0.00001);
	}

	// Test graph with 0 hierarchy (no connections in graph)
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 2:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 0, 0.00001);
	}

	// Test graph with low hierarchy (two nodes have the same LRC)
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v0, v2, 1);
		addCon(graph, v1, v2, 1);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 3:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 0.25, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 0.25, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 1, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 1, 0.00001);
	}

	// Test graph with cycle (0 hierarchy, because every node can reach every
	// other node).
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 1);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 4:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 0, 0.00001);
	}

	// Test graph with cycle (medium hierarchy, because there is a single output
	// node that can reach all others).
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 1);
		addCon(graph, v3, v0, 1);
		addCon(graph, v0, v4, 1);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 5:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 0.4375, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 0.4375, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 0.4375, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 0.4375, 0.00001);
	}

	// Test graph with cycle, but with different connections values.
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 0.1);
		float hier_normal = hierar::Get_GRC(graph);
		float hier_unweighted = hierar::Get_GRC_Unweighted(graph);
		float hier_rev = hierar::Get_GRC_Rev(graph);
		float hier_unw_rev = hierar::Get_GRC_Unweighted_Rev(graph);
		cout << "Network 6:" << endl;
		cout << "  Normal hierarchy: "<< hier_normal << endl;
		cout << "  Unweighted hierarchy: "<< hier_unweighted << endl;
		cout << "  Reversed hierarchy: "<< hier_rev << endl;
		cout << "  Unweighted reversed hierarchy: "<< hier_unw_rev << endl;
		BOOST_CHECK_CLOSE(hier_normal, 0.45, 0.00001);
		BOOST_CHECK_CLOSE(hier_unweighted, 0, 0.00001);
		BOOST_CHECK_CLOSE(hier_rev, 0.45, 0.00001);
		BOOST_CHECK_CLOSE(hier_unw_rev, 0, 0.00001);
	}

//    typedef sferes::gen::DnnFF<neu_t, con_t>, Params> substrate_t;
//    substrate_t nn;
//    nn.random();
//    nn.develop();
//    nn.init();
//    float value = hierar::Get_GRC(nn.get_graph());
//    std::cout << value << std::endl;
}




