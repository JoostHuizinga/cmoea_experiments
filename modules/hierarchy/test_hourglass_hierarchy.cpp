/*
 * test_hourglass_hierarchy.cpp
 *
 *  Created on: Apr 10, 2017
 *      Author: Joost Huizinga
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE hourglass_hierarchy

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
#include "hourglass_hierarchy.hpp"

using namespace sferes;
using namespace boost;
using namespace std;
using namespace hierar;

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
typedef std::vector<std::vector<vert_t> > comp_t;


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
	}
}

template<typename graph_t>
void printComponents(graph_t& graph, comp_t c){
	std::cout << "Number of components: " << c.size() << std::endl;
	for(size_t i=0; i<c.size(); ++i){
		std::cout << "  Component " << i << ": ";
		for(size_t j=0; j<c[i].size(); ++j){
			std::cout << graph[c[i][j]]._id << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

template<typename graph_t>
void printGraph(graph_t& graph){
	std::cout << "Printing new graph:" << std::endl;
	std::cout << "  Number of nodes: " << num_vertices(graph) << std::endl;
	BGL_FORALL_VERTICES_T(v, graph, graph_t){
		std::cout << "    " << graph[v]._id << std::endl;
	}

	std::cout << "  Number of edges: " << num_edges(graph) << std::endl;
	BGL_FORALL_EDGES_T(e, graph, graph_t){
		vert_t edge_source = source(e, graph);
		vert_t edge_target = target(e, graph);
		std::string s = graph[edge_source]._id;
		std::string t = graph[edge_target]._id;
		std::cout << "   (" << s << ") -> (" << t << ")" << std::endl;
	}
	std::cout << std::endl;
}


BOOST_AUTO_TEST_CASE(hourglass_hierarchy)
{
	// Test graph with perfect hierarchy (star graph)
	{
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v2, v0, 1);
		addCon(graph, v2, v1, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 1: Feed-forward graph with 3 nodes" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Test graph with a single cycle
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 2: One single component"  << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Test graph with two completely separate cycles
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 1);
		addCon(graph, v3, v4, 1);
		addCon(graph, v4, v5, 1);
		addCon(graph, v5, v3, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 3: Two separate components"  << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Test graph with two connected cycles
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v0, 1);
		addCon(graph, v3, v4, 1);
		addCon(graph, v4, v5, 1);
		addCon(graph, v5, v3, 1);
		addCon(graph, v2, v3, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 4: Two connected components"  << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Graph that should require joining
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v3, 1);
		addCon(graph, v3, v0, 1);
		addCon(graph, v1, v4, 1);
		addCon(graph, v4, v5, 1);
		addCon(graph, v5, v2, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 5: Component that requires joining" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Complex graph with core component
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		vert_t v6 = addNeuron(graph);
		vert_t v7 = addNeuron(graph);
		vert_t v8 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v3, 1);
		addCon(graph, v3, v4, 1);
		addCon(graph, v4, v1, 1);
		addCon(graph, v2, v5, 1);
		addCon(graph, v5, v6, 1);
		addCon(graph, v6, v3, 1);
		addCon(graph, v4, v7, 1);
		addCon(graph, v6, v8, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 6: Graph with core component" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Empty graph
	{
		id = 0;
		graph_t graph;
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 7: Empty graph" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Layered graph
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		vert_t v6 = addNeuron(graph);
		vert_t v7 = addNeuron(graph);
		vert_t v8 = addNeuron(graph);
		addCon(graph, v0, v3, 1);
		addCon(graph, v0, v4, 1);
		addCon(graph, v0, v5, 1);
		addCon(graph, v1, v3, 1);
		addCon(graph, v1, v4, 1);
		addCon(graph, v1, v5, 1);
		addCon(graph, v2, v3, 1);
		addCon(graph, v2, v4, 1);
		addCon(graph, v2, v5, 1);
		addCon(graph, v3, v6, 1);
		addCon(graph, v3, v7, 1);
		addCon(graph, v3, v8, 1);
		addCon(graph, v4, v6, 1);
		addCon(graph, v4, v7, 1);
		addCon(graph, v4, v8, 1);
		addCon(graph, v5, v6, 1);
		addCon(graph, v5, v7, 1);
		addCon(graph, v5, v8, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 8: Layered graph" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}


	// Four inputs, one output (no hidden)
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		addCon(graph, v0, v4, 1);
		addCon(graph, v1, v4, 1);
		addCon(graph, v2, v4, 1);
		addCon(graph, v3, v4, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 9: Four inputs, one output" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Four outputs, one input (no hidden)
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		addCon(graph, v0, v1, 1);
		addCon(graph, v0, v2, 1);
		addCon(graph, v0, v3, 1);
		addCon(graph, v0, v4, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 10: Four outputs, one input" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Two inputs, two outputs (one hidden)
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		addCon(graph, v0, v2, 1);
		addCon(graph, v1, v2, 1);
		addCon(graph, v2, v3, 1);
		addCon(graph, v2, v4, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 11: Two inputs, two outputs" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

	// Four inputs, four outputs (one hidden)
	{
		id = 0;
		graph_t graph;
		vert_t v0 = addNeuron(graph);
		vert_t v1 = addNeuron(graph);
		vert_t v2 = addNeuron(graph);
		vert_t v3 = addNeuron(graph);
		vert_t v4 = addNeuron(graph);
		vert_t v5 = addNeuron(graph);
		vert_t v6 = addNeuron(graph);
		vert_t v7 = addNeuron(graph);
		vert_t v8 = addNeuron(graph);
		addCon(graph, v0, v4, 1);
		addCon(graph, v1, v4, 1);
		addCon(graph, v2, v4, 1);
		addCon(graph, v3, v4, 1);
		addCon(graph, v4, v5, 1);
		addCon(graph, v4, v6, 1);
		addCon(graph, v4, v7, 1);
		addCon(graph, v4, v8, 1);
		HourglassHierarchyCalculator<graph_t> hierarchy_calculator(graph);
		std::cout << "*******************************************" << std::endl;
		std::cout << "Network 12: Four inputs, four outputs" << std::endl;
		printGraph(hierarchy_calculator.getDag());
		double hierarchy = hierarchy_calculator.getHierarchy();
		std::cout << "Hourglass hierarchy: " << hierarchy << std::endl;
		std::cout << std::endl;
	}

}




