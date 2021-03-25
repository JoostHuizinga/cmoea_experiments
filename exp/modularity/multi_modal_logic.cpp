/*
 * multi_modal_logic.cpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 *
 * The 'multi-modal' logic problem.
 *
 * The idea is that this network has to provide 'multi-modal' behavior.
 * In practice this comes down to a network that has to solve multiple
 * completely unrelated problems, even though the entire network is encoded
 * by only a single CPPN.
 *
 * The file is based on a number of problems, and a set of patterns for each
 * problem.
 */
//This cpp file relies on TBB
#define TBB

#if defined(LAPTOP)
#define JHDEBUG
#define NO_PARALLEL
#endif

#if defined(SETUP1) or defined(SET1)
#define HNN
#define GEN 25000
#define POP 1000
#endif

#if defined(SET2)
#define HNN
#define GEN 25000
#define POP 1000
#define DIV
#endif

#if defined(VERSION1)
#elif defined(VER2)
#define VERSION2
#endif

#if defined(PRO1)
#define PROBLEM_1
#elif defined(PRO2)
#define PROBLEM_2
#else
#define PROBLEM_1
#endif

#if defined(NODUP)
#define NO_DUPLICATES_SORT
#endif

#if defined(NODUP2)
#define NO_DUPLICATES_SORT_2
#endif

#if not defined(ME)
#define PARETO
#define MIXED_POP
#endif

//Shortcut for MAP-Elites with Hidden Modularity on one axis and CPPN Modularity of the other axis
#if defined(ME)
#define MAPELITE
#define DESC_CPPNCON
#define DESC_CPPNMOD
#endif

#if defined(US)
#define UNSIGNED_CPPN_ACTIVATION_FUNCTIONS
#endif

#if defined(RP)
#define RANDOM_PERFORMANCE
#endif

#if defined(TBS)
#define TOP_BOTTOM_SPLIT
#endif

//FMG stands for: Forced Modularity Genotype
#if defined(COOPP) || defined(FMG) || defined(ASG)
#define CPPN_OUTPUTGROUP_ONE_PER_PROBLEM
#else
#define CPPN_OUTPUTGROUP_ONE_FOR_ALL
#endif

//FMP stands for: Forced Modularity Phenotype
#if defined(COTO) || defined(FMP) || defined(ASP)
#define CONNECTED_ONE_TO_ONE
#else
#define CONNECTED_ALL_TO_ALL
#endif

//OSG wants us to limit the number of threads the the maximum requested, which is 8 in this case
#if defined(OSG) and not defined(STHREAD)
#define NB_THREADS 8
#endif

#if defined(STHREAD)
#define NB_THREADS 1
#endif

#if defined(RANDOM_PERFORMANCE)
#define EA_EVAL_ALL
#endif

//Boost includes
#include <boost/serialization/bitset.hpp>

//Sferes includes
#include <sferes/eval/eval.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>

//Module includes
#include <modules/misc/params.hpp>
#include <modules/misc/pnsga_params.hpp>
#include <modules/misc/af_extended.hpp>
#include <modules/misc/rand.hpp>
#include <modules/misc/neuron_groups.hpp>
#include <modules/misc/range.hpp>
#include <modules/misc/patterns.hpp>
#include <modules/misc/positions.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/phen_dnn.hpp>
#include <modules/continue/continue_run.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>
#include <modules/continue/stat_store_pop.hpp>
#include <modules/hnn/phen_hnn.hpp>
#include <modules/hnn/phen_hnn_leo.hpp>
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/af_cppn.hpp>
#include <modules/hnn/af_cppn_unsigned.hpp>
#include <modules/nsgaext/nsga2_custom.hpp>

#if defined(MAPELITE)
#include <modules/mapelite/map_elite.hpp>
#include <modules/mapelite/continue_mapelite.hpp>
#include <modules/mapelite/stat_map.hpp>
#endif

#if defined(MPI_ENABLED) && !defined(NOMPI)
#include <modules/mpiworldtrack/eval_mpi_world_tracking.hpp>
#else
#include <modules/mpiworldtrack/eval_world_tracking.hpp>
#include <modules/mpiworldtrack/eval_parallel_world_tracking.hpp>
#endif

//Local includes
#include "pnsga.hpp"
#include "gen_spatial_ws.hpp"
#include "mut_sweep_macros.hpp"
#include "dom_sort_no_duplicates.hpp"
#include "stat_multi_modal_logic.hpp"
#include "regularity.hpp"
//#include "modularity.hpp"
#include "behavior_div.hpp"
#include "modif_mvg.hpp"



//Include namespaces
using namespace sferes;
using namespace nn;
using namespace sferes::gen::evo_float;
using namespace sferes::gen::dnn;


/********************************
 *         PARAMETERS           *
 ********************************/

class UnsignedSeed{
public:
    template<typename CppnType>
    static void set_seed(CppnType& cppn){
        dbg::trace trace("seed", DBG_HERE);
        cppn.random();
        for(unsigned i=0; i<cppn.get_nb_outputs(); ++i){
            cppn.get_graph()[cppn.get_output(i)].get_afparams().set(nn::cppn::linear);
            cppn.get_graph()[cppn.get_output(i)].get_afparams().freeze();
        }
    }
};

// Define based parameters
SFERES_TYPE_VALUE_PARAM(NbGenerations, static const size_t v, GEN)
SFERES_TYPE_VALUE_PARAM(PopSize, static const size_t v, POP)
//SFERES_TYPE_VALUE_PARAM(NbSteps, static const size_t v, STEPS)

SFERES_PARAMS(
struct Params{
    static long seed;
    static bool init;

    struct mvg
    {
        SFERES_CONST size_t period = MvgPeriod::v;
        static size_t current_obj;
    };

    struct mpi
    {
        static mpi_init_ptr_t masterInit;
        static mpi_init_ptr_t slaveInit;
    };

    //Population and generation parameters
    struct pop
    {
        SFERES_CONST unsigned size = PopSize::v;
        SFERES_CONST unsigned select_size = PopSize::v;
        SFERES_CONST unsigned nb_gen = NbGenerations::v;
        SFERES_CONST int initial_aleat = 1;
//        SFERES_CONST int nr_of_dumps = NbGenerations::v/500;
        static int dump_period;
        static int checkpoint_period;
#if defined(MAPELITE)
        SFERES_CONST int initial_mutations = 10;
#if defined(INITSIZE10000)
        SFERES_CONST size_t init_size = 10000;
#else
        SFERES_CONST size_t init_size = size;
#endif
#endif
    };

    //Parameters related to the EA (PNSGA in this case)
    struct ea
    {
        SFERES_ARRAY(float, obj_pressure, 1.0, PnsgaPressure::v,  1.0, 1.0);
#if defined(NO_DUPLICATES_SORT)
        typedef sferes::ea::dom_sort_no_duplicates_f dom_sort_f;
#elif defined(NO_DUPLICATES_SORT_2)
        typedef sferes::ea::dom_sort_no_duplicates_fast_f dom_sort_f;
#else
        typedef sferes::ea::dom_sort_basic_f dom_sort_f;
#endif

#if defined(MAPELITE)
        SFERES_CONST size_t res_x = 100;
        SFERES_CONST size_t res_y = 100;
#endif
    };

    //Visualization parameters
    struct visualisation{
        SFERES_CONST size_t max_modularity_split = 3;
        SFERES_CONST float scale = 100.0f;

        SFERES_ARRAY(int, background, 255, 200, 200);

        SFERES_NON_CONST_ARRAY(float, x);
        SFERES_NON_CONST_ARRAY(float, y);
        SFERES_NON_CONST_ARRAY(float, z);
    };

    //Parameters for the directly encoded neural network
    struct dnn
    {
        // for weights
        struct weights
        {
            struct evo_float
            {
                SFERES_CONST float cross_rate = 0.0f;
                //1.0 because gen_spatial handles this according to the
                //m_avg_weight parameter defined below
                SFERES_CONST float mutation_rate = 1.0f;
                SFERES_CONST float eta_m = 10.0f;
                SFERES_CONST float eta_c = 10.0f;
                SFERES_CONST mutation_t mutation_type = polynomial;
                SFERES_CONST cross_over_t cross_over_type = sbx;
            };
            struct parameters
            {
                SFERES_CONST float min = -2;
                SFERES_CONST float max = 2;
            };
        };
        struct bias
        {
            struct evo_float
            {
                SFERES_CONST float cross_rate = 0.0f;
                SFERES_CONST float mutation_rate = 1.0f / 24.0f;
                SFERES_CONST float eta_m = 10.0f;
                SFERES_CONST float eta_c = 10.0f;
                SFERES_CONST mutation_t mutation_type = polynomial;
                SFERES_CONST cross_over_t cross_over_type = sbx;
            };
            struct parameters
            {
                SFERES_CONST float min = -2;
                SFERES_CONST float max = 2;
            };
        };

        SFERES_CONST size_t nb_inputs  = 8;
        SFERES_CONST size_t nb_outputs = 2;

        // Slope of the sigmoid
        SFERES_CONST float lambda = 20;

        //** NOT USED - START**//
        SFERES_CONST size_t min_nb_neurons = 0;
        SFERES_CONST size_t max_nb_neurons = 0;
        //** NOT USED - END**//

        //The range of connections that the direct encoding is initialized with
        SFERES_CONST size_t min_nb_conns = 0;
        SFERES_CONST size_t max_nb_conns = 104;  //(8*8+8*4+4*2)

        // Mutation rates
        SFERES_CONST float m_rate_add_conn = PARAM_DEPENDENT_CONADDMUTRATE(0.2f);       //Add Connection rate for the cppn
        SFERES_CONST float m_rate_del_conn = PARAM_DEPENDENT_CONDELMUTRATE(0.2f);       //Delete connection rate for the cppn
        SFERES_CONST float m_rate_change_conn = 0.0f;
        SFERES_CONST float m_rate_add_neuron  = 0.0f;
        SFERES_CONST float m_rate_del_neuron  = 0.0f;

        // Average number of weight changes
        SFERES_CONST float m_avg_weight = 2.0f;
        SFERES_CONST int io_param_evolving = true;
        SFERES_CONST init_t init = ff;

        struct spatial
        {
            //** NOT USED - START**//
            SFERES_CONST float min_coord = -4.0f;
            SFERES_CONST float max_coord = 4.0f;
            SFERES_CONST bool rand_coords = false;
            //** NOT USED - END**//

            // The layers for gen_spatial
            SFERES_NON_CONST_ARRAY(int, layers);
            SFERES_NON_CONST_ARRAY(float, x);
            SFERES_NON_CONST_ARRAY(float, y);
            SFERES_NON_CONST_ARRAY(float, z);
        };
    };

    struct stats
    {
        SFERES_CONST size_t period = 1; //print stats every N generations
        SFERES_CONST size_t logall_period = 100;
    };

    //** NOT USED - START**//
    struct rand_obj
    {
        SFERES_CONST float std_dev = 0.0;
    };
    //** NOT USED - END**//

    struct cppn_params{
        //This struct defines the planes of the multi-spatial HyperNEAT model.
        struct multi_spatial{
            static NeuronGroups* substrate_structure;

//            SFERES_NON_CONST_ARRAY(int, planes);
//            SFERES_NON_CONST_MATRIX(bool, connected);
//            SFERES_NON_CONST_ARRAY(size_t, neuron_offsets);
//            SFERES_NON_CONST_ARRAY(size_t, connection_offsets);
        };

        //Set the constants
#ifdef LEO
        typedef phen::hnn::HnnLeoConstants hnn_const_t;
#else
        typedef phen::hnn::HnnConstants hnn_const_t;
#endif

        struct cppn {
            // params of the CPPN
            // These values define the mutations on the activation function of the CPPN nodes
            // As such, they define which activation function can be chosen and the mutation rate for changing the activation function
            struct sampled {
#ifdef UNSIGNED_CPPN_ACTIVATION_FUNCTIONS
                SFERES_ARRAY(float, values, nn::cppn::usine, nn::cppn::usigmoid, nn::cppn::ugaussian, nn::cppn::ulinear, nn::cppn::linear); //Possible activation functions
#else
                SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid, nn::cppn::gaussian, nn::cppn::linear); //Possible activation functions
#endif
                SFERES_ARRAY(float, allowed_values, 0, 1, 2, 3)
                SFERES_CONST float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);          //Mutation rate for the cppn nodes
                SFERES_CONST float cross_rate = 0.0f;               //Not actually used, even if non-zero
                SFERES_CONST bool ordered = false;                  //Mutation will randomly pick an other activation function from the list
                //The alternative has bias towards adjacent values
            };

            // These value determine the mutation rate for the weights and biases of the cppn
            // Note that, in practice, the bias is determined by the weight of the connection between
            // the bias input node and the target node, so biases are not mutated 'directly'.
            struct evo_float {
                SFERES_CONST float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);          //Mutation rate for the cppn nodes
                SFERES_CONST float cross_rate = 0.0f;               //Not actually used, even if non-zero
                SFERES_CONST mutation_t mutation_type = polynomial; //Mutation type
                SFERES_CONST cross_over_t cross_over_type = sbx;    //Not used
                SFERES_CONST float eta_m = 10.0f;                   //Mutation eta (parameter for polynomial mutation.
                SFERES_CONST float eta_c = 10.0f;                   //?
            };

            struct parameters{
                SFERES_CONST float min = -3; // maximum value of parameters (weights & bias)
                SFERES_CONST float max = 3;  // minimum value
            };

            //This threshold determines at what value of the Link-Expression Output (LEO) a link will be expressed or not.
            //Before LEO values are currently between -1.0 and 1.0 and only values equal to or above the threshold are expressed.
            //As a result a value of -1.0 means all links are expressed (effectively disabling the LEO node)
            //while a value greater than 1.0 means no values will ever be expressed (probably a bad idea).
            //A value of 0.0 is default
            SFERES_CONST float leo_threshold = 0.0f;

            //In the absence of LEO, or to prevent extremely weak connections, this thresholds determines above
            //what strength a connection is added. Note: the connections below this value become impossible.
            SFERES_CONST float con_threshold = 0.01f;

#ifdef UNSIGNED_CPPN_ACTIVATION_FUNCTIONS
            typedef UnsignedSeed seed_t;
#else
            typedef gen::RandomSeed seed_t;
#endif
        };

        //These are the parameters used for setting the CPPN
        //The 7 inputs are the x, y, z coordinates for neuron 1
        //The x, y, z coordinates for neuron 2
        //And a bias.
        //The outputs are the weights and the link expression, and the bias.
        struct dnn
        {
            typedef phen::hnn::TwoDInput<Params::dnn> input_t;
            SFERES_CONST size_t nb_inputs = input_t::inputSize;
            static size_t nb_outputs;
            SFERES_CONST init_t init = ff;

            SFERES_CONST float m_rate_add_conn = PARAM_DEPENDENT_CONADDMUTRATE(0.09f);      //Add Connection rate for the cppn
            SFERES_CONST float m_rate_del_conn = PARAM_DEPENDENT_CONDELMUTRATE(0.08f);      //Delete connection rate for the cppn
            SFERES_CONST float m_rate_add_neuron = PARAM_DEPENDENT_NEUADDMUTRATE(0.05f);        //Add neuron rate for the cppn
            SFERES_CONST float m_rate_del_neuron = PARAM_DEPENDENT_NEUDELMUTRATE(0.04f);        //Delete neuron rate for the cppn

            SFERES_CONST float m_rate_change_conn = 0.0f;       //Not used for the cppn, even if value is non zero.

            //** NOT USED - START**//
            SFERES_CONST size_t min_nb_neurons = 0;
            SFERES_CONST size_t max_nb_neurons = 0;
            SFERES_CONST size_t min_nb_conns = 0;
            SFERES_CONST size_t max_nb_conns = 0;
            //** NOT USED - END**//
        };
    };

    struct problems{
        SFERES_CONST unsigned nb_of_problems = 2;
        SFERES_CONST unsigned outputs_per_problem = 1;
        SFERES_CONST unsigned inputs_per_problem = 4;
        SFERES_ARRAY(int, layers, inputs_per_problem, 4, 2, outputs_per_problem);
        SFERES_CONST unsigned nb_of_bits = nb_of_problems * inputs_per_problem;
        SFERES_CONST unsigned nb_of_patterns = 1 << nb_of_bits;
        SFERES_NON_CONST_MATRIX(bool, answers);
    };
};
)


//Declare all non-constant parameters
long Params::seed = -1;
size_t Params::mvg::current_obj = 0;
int Params::pop::dump_period = -1;
int Params::pop::checkpoint_period = -1;
bool Params::init = false;
mpi_init_ptr_t Params::mpi::masterInit = 0;
mpi_init_ptr_t Params::mpi::slaveInit = 0;
NeuronGroups* Params::cppn_params::multi_spatial::substrate_structure = 0;
size_t Params::cppn_params::dnn::nb_outputs = 0;

SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, layers);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, x);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, y);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, z);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, x);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, y);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, z);
//SFERES_DECLARE_NON_CONST_ARRAY(Params::cppn_params::multi_spatial, planes);
//SFERES_DECLARE_NON_CONST_ARRAY(Params::cppn_params::multi_spatial, neuron_offsets);
//SFERES_DECLARE_NON_CONST_ARRAY(Params::cppn_params::multi_spatial, connection_offsets);
//SFERES_DECLARE_NON_CONST_MATRIX(Params::cppn_params::multi_spatial, connected);
SFERES_DECLARE_NON_CONST_MATRIX(Params::problems, answers);


void set_obj(size_t n){
    for(unsigned i=0; i<Params::problems::nb_of_problems; ++i){
        for(unsigned j=0; j<Params::problems::nb_of_patterns; ++j){
            Params::problems::answers_set(i, j, false);
        }
    }

    //Problem 1: Patterns with 2 trues and 2 false
    addPattern<Params>(0, true,  true,  false, false);
    addPattern<Params>(0, false, false, true,  true);
    addPattern<Params>(0, true,  false, true,  false);
    addPattern<Params>(0, false, true,  false, true);

    //Problem 1: Patterns with 3 trues and 1 false
    addPattern<Params>(0, true,  true,  false, true);
    addPattern<Params>(0, true,  true,  true,  false);
    addPattern<Params>(0, true,  false, true,  true);
    addPattern<Params>(0, false, true,  true,  true);

#if defined(PROBLEM_1)
    //Problem 2: Patterns with 3 trues and 1 false
    addPattern<Params>(1, true,  true,  false, true);
    addPattern<Params>(1, true,  true,  true,  false);
    addPattern<Params>(1, true,  false, true,  true);
    addPattern<Params>(1, false, true,  true,  true);

    switch(n){
    case 0:
        addPattern<Params>(1, true,  false, false, false);
        addPattern<Params>(1, false, true,  false, false);
        addPattern<Params>(1, false, false, true,  false);
        addPattern<Params>(1, false, false, false, true);
        break;
    case 1:
        addPattern<Params>(1, true,  true, true, true);
        addPattern<Params>(1, false, false, false, false);
        addPattern<Params>(1, true, false, false, true);
        addPattern<Params>(1, false, true, true, false);
        break;
    default:
        dbg::sentinel(DBG_HERE);
    }
#elif defined(PROBLEM_2)
    //Problem 2: Patterns with 3 trues and 1 false
    addPattern<Params>(1, true,  true,  false, false);
    addPattern<Params>(1, false,  false,  true,  true);
    addPattern<Params>(1, true,  false, false,  true);
    addPattern<Params>(1, false, true,  true,  false);

    switch(n){
    case 0:
        addPattern<Params>(1, true,  true,  true,  false);
        addPattern<Params>(1, true,  true,  false, true);
        addPattern<Params>(1, true,  false, false, false);
        addPattern<Params>(1, false, true,  false, false);
        break;
    case 1:
        addPattern<Params>(1, true,  false, true,  true);
        addPattern<Params>(1, false, true,  true,  true);
        addPattern<Params>(1, false, false, true,  false);
        addPattern<Params>(1, false, false, false, true);
        break;
    default:
        dbg::sentinel(DBG_HERE);
    }

#else
#error "No problem set defined"
#endif

    if(options::map["print-patterns"].as<bool>()){
        printPatterns<Params>();
    }
}

/********************************
 *       INITIALIZATION         *
 ********************************/

void joint_init(){
    dbg::trace trace("init", DBG_HERE);
    if(Params::init) return;

    //Bring some static variables into scope
    SFERES_CONST unsigned nb_problems = Params::problems::nb_of_problems;
    static const unsigned nb_layers = Params::problems::layers_size();

    //Set layers
    size_t nb_of_neurons = 0;
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, layers, Params::problems::layers_size());
    for(size_t i=0; i<Params::problems::layers_size(); ++i){
        Params::dnn::spatial::layers_set(i, Params::problems::layers(i) * nb_problems);
        nb_of_neurons += Params::problems::layers(i) * nb_problems;
    }

    //Create neuron groups object and set its properties
    NeuronGroups* neuronGroups = new NeuronGroups();
    neuronGroups->setSeparateIndexPerLayer(false);
    neuronGroups->setSeparateIndexPerLayerNeurons(false);
    neuronGroups->setPostfixFlag(NeuronGroupsEnum::target);
#if defined(CPPN_OUTPUTGROUP_ONE_FOR_ALL)
    neuronGroups->setHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setOutputCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setInputToHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setHiddenToHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setHiddenToOutputCppnIndex(NeuronGroupsEnum::staticIndex);
#elif defined(CPPN_OUTPUTGROUP_ONE_PER_PROBLEM)
    neuronGroups->setHiddenCppnIndex(NeuronGroupsEnum::layeredSourceIndex);
    neuronGroups->setOutputCppnIndex(NeuronGroupsEnum::layeredSourceIndex);
    neuronGroups->setInputToHiddenCppnIndex(NeuronGroupsEnum::layeredSourceIndex);
    neuronGroups->setHiddenToHiddenCppnIndex(NeuronGroupsEnum::layeredSourceIndex);
    neuronGroups->setHiddenToOutputCppnIndex(NeuronGroupsEnum::layeredSourceIndex);
#else
#error "CPPN output group configuration not defined"
#endif
    neuronGroups->setNbOfCppnOutputsNeuron(Params::cppn_params::hnn_const_t::nb_of_outputs_neuron);
    neuronGroups->setNbOfCppnOutputsCon(Params::cppn_params::hnn_const_t::nb_of_outputs_connection);

    //Set groups and connect them
    for(size_t input_index=0; input_index<nb_problems; ++input_index){
#if defined(TOP_BOTTOM_SPLIT)
        neuronGroups->addInput(Params::problems::layers(0), 0, "first");
#else
        neuronGroups->addInput(Params::problems::layers(0), 0);
#endif
    }

    for(size_t layer_index=1; layer_index<(nb_layers-1); ++layer_index){
        for(size_t i=0; i<nb_problems; ++i){
#if defined(TOP_BOTTOM_SPLIT)
            if(layer_index < 2){
                neuronGroups->addHidden(Params::problems::layers(layer_index), layer_index, "first");
            } else {
                neuronGroups->addHidden(Params::problems::layers(layer_index), layer_index, "second");
            }
#else
            neuronGroups->addHidden(Params::problems::layers(layer_index), layer_index);
#endif
        }
    }

    for(size_t i=0; i<nb_problems; ++i){
#if defined(TOP_BOTTOM_SPLIT)
        neuronGroups->addOutput(Params::problems::layers(nb_layers-1), nb_layers-1, "second");
#else
        neuronGroups->addOutput(Params::problems::layers(nb_layers-1), nb_layers-1);
#endif
    }

#if defined(CONNECTED_ALL_TO_ALL)
    neuronGroups->connectLayeredFeedForwardFull();
#elif defined(CONNECTED_ONE_TO_ONE)
    neuronGroups->connectLayeredFeedForwardOneToOne();
#else
#error "No connectivity defined"
#endif
    neuronGroups->build();
    Params::cppn_params::dnn::nb_outputs = neuronGroups->getNbCppnOutputs();

    if(options::map["print-neuron-groups"].as<bool>()){
        neuronGroups->print();
    }

    Params::cppn_params::multi_spatial::substrate_structure = neuronGroups;


    //Set the neuron positions
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, z, nb_of_neurons);

    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, z, nb_of_neurons);

    //Set and calculate the bounds
    float min_x_pos = -1.0f;
    float max_x_pos = 1.0f;
    float min_y_pos = -1.0f;
    float max_y_pos = 1.0f;
    float width = (max_y_pos - min_y_pos);
    float y_step = width / float(Params::dnn::spatial::layers_size()-1);
    float x_step = (max_x_pos - min_x_pos) / float(Params::dnn::spatial::layers(0)-1);
    float problem_center_offset = min_x_pos + x_step * (Params::problems::layers(0)-1) * 0.5;

    size_t neuron_index = 0;
    for(size_t layer=0; layer<Params::dnn::spatial::layers_size(); ++layer){
        size_t layer_size = Params::dnn::spatial::layers(layer);
        size_t neurons_per_problem = layer_size / Params::problems::nb_of_problems;
        //float x_offset = -x_step * ((float(layer_size/Params::problems::nb_of_problems)-1.0f)/2.0f);
        for(size_t neuron=0; neuron<layer_size; ++neuron){
            unsigned problem = neuron/neurons_per_problem;
            unsigned neuron_of_problem = neuron%neurons_per_problem;
            float offset_from_center = -x_step * (neurons_per_problem-1) * 0.5;
            float problem_center = problem_center_offset + x_step * Params::problems::layers(0) * problem;
            setPos<Params>(problem_center + offset_from_center + x_step*neuron_of_problem, max_y_pos - y_step*layer, 0, neuron_index);
        }
    }
    if(options::map["print-nn"].as<bool>()){
        print_positions<Params>();
    }



    //Create the answers
    SFERES_INIT_NON_CONST_MATRIX(Params::problems, answers, Params::problems::nb_of_problems, Params::problems::nb_of_patterns);
    set_obj(0);
    Params::init = true;
}


void master_init(){
    dbg::trace trace("init", DBG_HERE);
    joint_init();
#ifdef JHDEBUG
    Params::pop::checkpoint_period = 1;
#else
    Params::pop::checkpoint_period = 500;
#endif
    if(options::map.count("load")){
        //Prevents the creation of a useless result directory
        Params::pop::dump_period = -1;
    } else {
        //Don't dump, but do write the modularity file
        Params::pop::dump_period = Params::pop::nb_gen + 1;
    }
}


void slave_init(){
    dbg::trace trace("init", DBG_HERE);
    joint_init();
}

template<typename Graph>
std::vector<SvgColor> getColors(Graph& graph, const std::vector<std::string>& mods){
    dbg::trace trace("gen_spatial", DBG_HERE);
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
    BGL_FORALL_VERTICES_T(v, graph, Graph){
        dbg::out(dbg::info, "visualize") << "vertex: " << i << " mod size: " << mods.size() << std::endl;
        dbg::assertion(DBG_ASSERTION(i<mods.size()));
        if(boost::in_degree(v, graph) + boost::out_degree(v, graph) == 0){
            colors.push_back(SvgColor(125, 125, 125));
        } else{
            size_t param_max_split = Params::visualisation::max_modularity_split;
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




/********************************
 *           FITNESS            *
 ********************************/

SFERES_FITNESS(FitMultiModalLogic, sferes::fit::Fitness) {
public:
    template<typename Indiv>
    float dist(const Indiv& o) const {
        return (o.fit()._behavior ^ _behavior).count() / (float) _behavior.size();
    }

    template<typename Indiv>
    bool compare(const Indiv& o) const {
        dbg::trace trace("fit", DBG_HERE);
#if defined(MAPELITE)
        return (this->_value > o->fit().value());
#else
        return false;
#endif
    }

    // size of the behavior vector
    SFERES_CONST size_t b_size = Params::problems::nb_of_problems * Params::problems::nb_of_patterns;

    template<typename NN_T>
    inline float get_performance(NN_T& nn){
        float err = 0;
        std::vector<float> inputs(8);
        _values.clear();
        _values.resize(Params::problems::nb_of_problems, 0);

        nn.init();
        for (unsigned input_pattern = 0; input_pattern < Params::problems::nb_of_patterns; ++input_pattern) {
            //Create the input pattern
            std::bitset<8> in(input_pattern);
            for (size_t i = 0; i < in.size(); ++i){
                inputs[i] = in[i] ? 1 : -1;
            }
            dbg::out(dbg::info, "patterns") << "Testing pattern: " << inputs << std::endl;

            //Step the network
//            nn.stepAll(inputs);
            for (size_t i = 0; i < nn.get_depth(); ++i){
                nn.step(inputs);
            }

            //Check the answer
            for(size_t i=0; i<Params::problems::nb_of_problems; ++i){
                bool answer = nn.get_outf(i) < 0;
                bool correct_answer = checkAnswer<Params>(input_pattern, i);
                dbg::out(dbg::info, "patterns") << "  - Answer: " << answer << " correct answer: " << correct_answer << std::endl;
                float score = (answer==correct_answer ? 0.0f : 1.0f);
                err += (answer==correct_answer ? 0.0f : 1.0f);
                _values[i] += score;
                _behavior[input_pattern*Params::problems::nb_of_problems + i] = answer;
            }
        }

        //Normalize the score per problem
        for(size_t i=0; i<Params::problems::nb_of_problems; ++i){
            _values[i] /= Params::problems::nb_of_patterns;
        }

        return 1 - (err / (Params::problems::nb_of_patterns*Params::problems::nb_of_problems));
    }

    template<typename Indiv>
    void eval(Indiv& ind) {
        dbg::out(dbg::info, "fitness") << "Evaluating individual" << std::endl;

        //Typedefs
        typedef typename Indiv::nn_t nn_t;
        typedef typename nn_t::graph_t graph_t;
        typedef typename nn_t::layer_map_t layer_map_t;
        typedef typename nn_t::vertex_desc_t vertex_desc_t;
        typedef typename Indiv::cppn_nn_t cppn_nn_t;

        //Clear objectives
        this->_objs.clear();

        //Create the neural network
        ind.nn().init();

#if defined(RANDOM_PERFORMANCE)
        this->_value = misc::randReal<float>();
#else
        float err = 0;
        std::vector<float> inputs(8);

        for (unsigned input_pattern = 0; input_pattern < Params::problems::nb_of_patterns; ++input_pattern) {
            //Create the input pattern
            std::bitset<8> in(input_pattern);
            for (size_t i = 0; i < in.size(); ++i){
                inputs[i] = in[i] ? 1 : -1;
            }
            dbg::out(dbg::info, "patterns") << "Testing pattern: " << inputs << std::endl;

            //Step the network
            for (size_t i = 0; i < ind.nn().get_depth(); ++i){
                ind.nn().step(inputs);
            }

            //Check the answer
            for(size_t i=0; i<Params::problems::nb_of_problems; ++i){
                bool answer = ind.nn().get_outf(i) < 0;
                bool correct_answer = checkAnswer<Params>(input_pattern, i);
                dbg::out(dbg::info, "patterns") << "  - Answer: " << answer << " correct answer: " << correct_answer << std::endl;
                err += (answer==correct_answer ? 0.0f : 1.0f);
                _behavior[input_pattern*Params::problems::nb_of_problems + i] = answer;
            }
        }

        //Calculate performance
        this->_value = 1 - (err / (Params::problems::nb_of_patterns*Params::problems::nb_of_problems));
#endif
        //Calculate modularity
        typename Indiv::nn_t nn_simp = ind.nn().simplified_nn();

        //DON'T COPY!!!!!!!!
        typename Indiv::nn_t::graph_t& gsimp = nn_simp.get_graph();

        _mod = mod::modularity(gsimp, mod::null_model::Directed());
        dbg::out(dbg::info, "fitness") << " modularity: " << _mod << std::endl;

        _lmod = mod::modularity(gsimp, nn_simp.getLayeredNullModel());
        dbg::out(dbg::info, "fitness") << " layered modularity: " << _lmod << std::endl;

        //Calculate length
        _length = ind.nn().compute_length();
        dbg::out(dbg::info, "fitness") << " length: " << _length << std::endl;
        _optlength = ind.nn().compute_optimal_length();
        dbg::out(dbg::info, "fitness") << " optimal length: " << _optlength << std::endl;

        //Calculate number of connections
        _nb_conns = boost::num_edges(gsimp);
        dbg::out(dbg::info, "fitness") << " number of connections: " << _nb_conns << std::endl;

        //Calculate number of nodes
        _nb_nodes = boost::num_vertices(gsimp);
        dbg::out(dbg::info, "fitness") << " number of nodes: " << _nb_nodes << std::endl;

        //Always push back performance
        this->_objs.push_back(this->_value);
        dbg::out(dbg::info, "fitness") << " Pushed back performance: " <<  this->_objs.back() << std::endl;

#ifdef HNN
        cppn_nn_t cppn_simp = ind.cppn();
        cppn_simp.simplify();
        _cppn_mod = mod::modularity(cppn_simp.get_graph(), mod::null_model::Directed());
#else
        //An impossible value for modularity, but still set, meaning it is easier to detect
        //when values in memory are being misplaced.
        _cppn_mod = -2;
#endif


        /**********************
         ***** DESCRIPTORS ****
         **********************/
#if defined(MAPELITE)
      size_t desc_index = 0;

#if defined(DESC_CPPNCON)
      dbg::out(dbg::info, "desc") << "Behavioral descriptor " << desc_index << " is _cppn_con: " << float(boost::num_edges(cppn_simp.get_graph())) / 100.0f << std::endl;
      _behavioral_descriptors[desc_index++] = float(boost::num_edges(cppn_simp.get_graph())) / 100.0f;
#endif

#if defined(DESC_CPPNMOD)
      dbg::out(dbg::info, "desc") << "Behavioral descriptor " << desc_index << " is _cppn_mod: " << _cppn_mod << std::endl;
      _behavioral_descriptors[desc_index++] = _cppn_mod;
#endif

      dbg::assertion(DBG_ASSERTION(desc_index == 2));
#endif


      /**********************
       ***** OBJECTIVES *****
       **********************/
#ifdef NCPPNCONNS
        int cppn_nb_conns =  boost::num_edges(ind.cppn().get_graph());
        this->_objs.push_back(-cppn_nb_conns);
        dbg::out(dbg::info, "fitness") << " Pushed back number of cppn connections: " <<  this->_objs.back()  << std::endl;
#endif

#ifdef NCONNS
        this->_objs.push_back(-(int)_nb_conns);
        dbg::out(dbg::info, "fitness") << " Pushed back number of connections: " <<  this->_objs.back()  << std::endl;
#endif

#ifdef NNODES
        this->_objs.push_back(-(int)_nb_nodes);
        dbg::out(dbg::info, "fitness") << " Pushed back number of nodes: " <<  this->_objs.back()  << std::endl;
#endif

#ifdef NCNODES
        this->_objs.push_back(-(int)_nb_conns -(int)_nb_nodes);
        dbg::out(dbg::info, "fitness") << " Pushed back number of nodes and connections: " <<  this->_objs.back()  << std::endl;
#endif

#ifdef MOD
        this->_objs.push_back(_mod);
        dbg::out(dbg::info, "fitness") << " Pushed back modularity: " <<  this->_objs.back()  << std::endl;
#endif

#ifdef CPPNMOD
        this->_objs.push_back(_cppn_mod);
        dbg::out(dbg::info, "fitness") << " Pushed back CPPN modularity: " <<  this->_objs.back()  << std::endl;
#endif

#if defined(LENGTH) && !defined(ONP)
        this->_objs.push_back(-_length);
        dbg::out(dbg::info, "fitness") << " Pushed back connection cost: " <<  this->_objs.back()  << std::endl;
#endif

#if defined(LENGTH) && defined(ONP)
        this->_objs.push_back(-_optlength);
        dbg::out(dbg::info, "fitness") << " Pushed back ONP connection cost: " <<  this->_objs.back()  << std::endl;
#endif

#if defined(DIV)
        this->_objs.push_back(0);
        dbg::out(dbg::info, "fitness") << " Pushed back dummy diversity: " <<  this->_objs.back()  << std::endl;
#endif
    }

    /**
     * Performs additional analysis of this individual, calculating metrics that are
     * too computational expensive to do every generation. Should guarantee that it,
     * does not write any additional files.
     */
    template<typename Indiv>
    void analyze(Indiv& ind){
        //Left-right analysis
//        std::set<int> left = boost::assign::list_of(0)(1)(2)(3);
//        std::set<int> right = boost::assign::list_of(4)(5)(6)(7);
//        left_right_mod = (analysis_io_mod(ind, left, right)==8);
        left_right_mod = 0;

        //Regularity analysis
        regularity_score = regularity::analysis_regularity(ind);

#ifdef HNN
        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;
        typedef typename Indiv::cppn_nn_t cppn_t;
        typedef typename Indiv::af_t af_t;
        typedef typename Indiv::cppn_graph_t cppn_graph_t;
        typedef typename Indiv::cppn_nn_t::weight_t cppn_weight_t;
        typedef typename Indiv::cppn_nn_t::edge_desc_t cppn_edge_desc_t;
        //WARNING: The network of this individual is no longer its actual genome!
        Indiv ind_copy = ind;
        ind_copy.cppn().simplify();
        ind_copy.develop();
        get_performance(ind_copy.nn());
        std::vector<float> original_performance = _values;
        std::vector<int> results;
        std::vector<cppn_edge_desc_t> all_edges;
        size_t redundant = 0;
        size_t critical = 0;
        size_t left_only = 0;
        size_t right_only = 0;


        BGL_FORALL_EDGES_T(v, ind_copy.cppn().get_graph(), cppn_graph_t){
            all_edges.push_back(v);
        }

//        std::cout << "Real original performance: " << std::endl;
//        for(size_t i=0; i<original_performance.size(); ++i){
//            std::cout << " - "<< original_performance[i] << std::endl;
//        }

        cppn_weight_t null_weight;
        null_weight.gen().data(0, (cppn_weight_t::max_p + cppn_weight_t::min_p)/2);
        null_weight.develop();

        for(size_t i=0; i<all_edges.size(); ++i){
            cppn_weight_t original_weight = ind_copy.cppn().get_graph()[all_edges[i]].get_weight();
            ind_copy.cppn().get_graph()[all_edges[i]].set_weight(null_weight);
            ind_copy.develop();

            get_performance(ind_copy.nn());
//            std::cout << "New performance: " << std::endl;
//            for(size_t i=0; i<original_performance.size(); ++i){
//                std::cout << " - "<< original_performance[i] << std::endl;
//            }
//
//            std::cout << "Original performance: " << std::endl;
//            for(size_t i=0; i<_values.size(); ++i){
//                std::cout << " - "<< _values[i] << std::endl;
//            }
            if(_values[0] == original_performance[0] && _values[1] == original_performance[1]){
                results.push_back(0);
                ++redundant;
            } else if (_values[0] == original_performance[0] && _values[1] != original_performance[1]){
                results.push_back(1);
                ++right_only;
            } else if (_values[0] != original_performance[0] && _values[1] == original_performance[1]){
                results.push_back(2);
                ++left_only;
            }else if (_values[0] != original_performance[0] && _values[1] != original_performance[1]){
                results.push_back(2);
                ++critical;
            }
            ind_copy.cppn().get_graph()[all_edges[i]].set_weight(original_weight);
        }

        _left_right_cppn_score = float(left_only + right_only)/float(results.size());

        std::cout << "Redundant: " << redundant << std::endl;
        std::cout << "Critical: " << critical << std::endl;
        std::cout << "Left_only: " << left_only << std::endl;
        std::cout << "Right_only: " << right_only << std::endl;
        std::cout << "Left-right factor: " << _left_right_cppn_score << std::endl;
#endif
    }

    /**
     * Dumps additional data about this individual.
     *
     * In general, calling this function should write various additional files to the
     * disk. Should be called after analyze, since some functions may require data
     * calculated previously.
     */
    template<typename Indiv>
    void dump(Indiv& ind){
        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;
        typedef typename Indiv::nn_t::graph_t graph_t;


        nn_t nnsimp = ind.nn();
        nnsimp.simplify();

        std::ofstream ofs3(("nn"+ _postFix +".svg").c_str());
        ind.nn().write_svg(ofs3);

        std::map<vertex_desc_t, gen::spatial::Pos> original_coords_map = nnsimp.getVisualizationCoordsmap();
        std::map<vertex_desc_t, gen::spatial::Pos> coords_map = nnsimp.get_optimal_coordsmap();

        BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t){
            coords_map[v] =  gen::spatial::Pos(coords_map[v].x(), original_coords_map[v].y(), 0);
        }
        {
            std::vector<std::string> mods;
            dbg::out(dbg::info, "modularity") << "=== CALCULATING NN DIRECTED MODULARITY ===" << std::endl;
            mod::split(nnsimp.get_graph(), mods, nnsimp.getLayeredNullModel());
            //        mod::split(nnsimp.get_graph(), mods, mod::null_model::Directed());
            dbg::out(dbg::info, "modularity") << "=== NN DIRECTED MODULARITY CALCULATED  ===" << std::endl;

            for(size_t i=0; i<mods.size(); ++i){
                dbg::out(dbg::info, "modularity") << "Visualized modules: " << mods[i] << std::endl;
            }
            std::vector<SvgColor> colors = nnsimp.getColors(mods);
            SvgWriter mod_writer;
            mod_writer.setScale(100);
            //        mod_writer.switchAxis(1,2);
            mod_writer.setColors(colors);
            mod_writer.setDefaultShape(4);
            mod_writer.setRectangles();
            std::ofstream ofs4(("onp"+ _postFix +".svg").c_str());
            nnsimp.write_svg(ofs4, mod_writer, coords_map);
        }

        {
            std::vector<std::string> mods;
            dbg::out(dbg::info, "modularity") << "=== CALCULATING NN DIRECTED MODULARITY ===" << std::endl;
            mod::split_fine(nnsimp.get_graph(), mods, nnsimp.getLayeredNullModel());
    //        mod::split(nnsimp.get_graph(), mods, mod::null_model::Directed());
            dbg::out(dbg::info, "modularity") << "=== NN DIRECTED MODULARITY CALCULATED  ===" << std::endl;

            for(size_t i=0; i<mods.size(); ++i){
                dbg::out(dbg::info, "modularity") << "Visualized modules: " << mods[i] << std::endl;
            }
            std::vector<SvgColor> colors = nnsimp.getColors(mods);
            SvgWriter mod_writer;
            mod_writer.setScale(100);
    //        mod_writer.switchAxis(1,2);
            mod_writer.setColors(colors);
            mod_writer.setDefaultShape(4);
            mod_writer.setRectangles();
            std::ofstream ofs4(("onp_fine"+ _postFix +".svg").c_str());
            nnsimp.write_svg(ofs4, mod_writer, coords_map);
        }


//        ind.nn().compute_optimal_length(0, true, "onp" + _postFix + ".svg");
#ifdef HNN
        typedef typename Indiv::cppn_nn_t cppn_t;
        typedef typename Indiv::af_t af_t;
        typedef typename Indiv::cppn_graph_t cppn_graph_t;
        typedef typename Indiv::cppn_nn_t::weight_t cppn_weight_t;
        typedef typename Indiv::cppn_nn_t::edge_desc_t cppn_edge_desc_t;


        std::ofstream ofs(("cppn" + _postFix + ".dot").c_str());
        ind.write_dot_cppn(ofs);

       //Write cppn svg
        cppn_t cppn_simp = ind.cppn();
        cppn_simp.simplify();
        std::map<vertex_desc_t, gen::spatial::Pos> cppn_coords_map = cppn_simp.get_coordsmap();

        std::vector<std::string> cppn_mods;
        dbg::out(dbg::info, "modularity") << std::endl;
        dbg::out(dbg::info, "modularity") << "=== CALCULATING CPPN MODULARITY ===" << std::endl;
        mod::split(cppn_simp.get_graph(), cppn_mods, mod::null_model::Directed());
        dbg::out(dbg::info, "modularity") << "=== CPPN MODULARITY CALCULATED  ===" << std::endl;
        std::vector<SvgColor> cppn_colors = getColors(cppn_simp.get_graph(), cppn_mods);

        std::vector<std::string> labels;
        BGL_FORALL_VERTICES_T(v, cppn_simp.get_graph(), cppn_graph_t){
            std::string label;
            label = boost::lexical_cast<std::string>(cppn_simp.get_graph()[v].get_id()) + " ";
            label += af_t::actToString(cppn_simp.get_graph()[v].get_afparams().type());
            labels.push_back(label);
        }

        SvgWriter cppn_writer;
        cppn_writer.setScale(800);
        cppn_writer.setColors(cppn_colors);
        cppn_writer.setDefaultShape(4);
        cppn_writer.setDefaultShapeSize(25);
        cppn_writer.setSideMargin(40);
        cppn_writer.setTopMargin(40);
        cppn_writer.setRectangles();
        cppn_writer.setScaledBoundaries(0, 1, 0, 1);
        cppn_writer.setBackground(DNN_SPATIAL_BACKGROUND_COLOR);
        cppn_writer.setLabels(labels);

        std::ofstream ofs_cppn_svg(("cppn" + _postFix + ".svg").c_str());
        cppn_writer.to_svg(cppn_simp.get_graph(), cppn_coords_map, ofs_cppn_svg);



#endif
    }

    /**
     * Synonym of dump.
     */
    template<typename Indiv>
    void visualizeNetwork(Indiv& ind){dump(ind);}


    static void next_obj(){
        static size_t nb_of_objectives = 2;
        Params::mvg::current_obj = (Params::mvg::current_obj+1) % nb_of_objectives;
        dbg::out(dbg::info, "mvg") << "Switching to objective: " << Params::mvg::current_obj << std::endl;
        set_obj(Params::mvg::current_obj);
    }

    //Performance values for various different objectives (in this case problems)
    //that are not evaluated by NSGA
    float values(size_t k) const { return _values[k]; }
    size_t nb_values() const { return _values.size(); }
    void set_value(float v) { this->_value = v; }

    //The behavior vector
    const std::bitset<b_size>& behavior() const { return _behavior; }

    //Metrics of interest for this experiment.
    //All information here should be serialized.
    float mod() const { return _mod; }
    float lmod() const { return _lmod; }
    float cppn_mod() const { return _cppn_mod; }
    float length() const { return _length; }
    float optlength() const { return _optlength; }
    int nb_nodes() const { return _nb_nodes; }
    int nb_conns() const { return _nb_conns; }

    //Metrics filled by analyze.
    //There is no point is serializing this data.
    bool get_left_right_mod() const{ return left_right_mod;}

    float get_regularity_score() const{ return regularity_score; }
    float get_left_right_cppn_score() const{ return _left_right_cppn_score; }
    template<typename I> float get_original_length(I& indiv){return regularity::get_original_length(indiv);}

    //ID related functions, such that we know which individual of the population we are talking about
    void setId(size_t id){_id = id;}
    size_t getId(){ return _id;}
    void setPostFix(std::string postFix){_postFix = postFix;}

#if defined(MAPELITE)
    const boost::array<float, 2>& desc() const{ return _behavioral_descriptors; }
#endif

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        dbg::trace trace("fit", DBG_HERE);
        sferes::fit::Fitness<Params,  typename stc::FindExact<FitMultiModalLogic<Params, Exact>, Exact>::ret>::serialize(ar, version);
        ar & BOOST_SERIALIZATION_NVP(_values);
        ar & BOOST_SERIALIZATION_NVP(_behavior);
        ar & BOOST_SERIALIZATION_NVP(_mod);
        ar & BOOST_SERIALIZATION_NVP(_length);
        ar & BOOST_SERIALIZATION_NVP(_optlength);
        ar & BOOST_SERIALIZATION_NVP(_nb_nodes);
        ar & BOOST_SERIALIZATION_NVP(_nb_conns);
#ifdef VERSION2
        ar & BOOST_SERIALIZATION_NVP(_lmod);
        ar & BOOST_SERIALIZATION_NVP(_cppn_mod);
#endif
#if defined(MAPELITE)
        ar & BOOST_SERIALIZATION_NVP(_behavioral_descriptors);
#endif
    }

protected:
    //Set by stat (not serialized)
    size_t _id;
    std::string _postFix;

    //Calculated on analysis (not serialized)
    bool left_right_mod;
    float regularity_score;
    float _left_right_cppn_score;

    //Calculated each generation (serialized)
    std::vector<float> _values;
    std::bitset<b_size> _behavior;
    float _mod;
    float _lmod;
    float _cppn_mod;
    float _length, _optlength;
    int _nb_nodes, _nb_conns;
#if defined(MAPELITE)
    boost::array<float, 2> _behavioral_descriptors;
#endif
};


/********************************
 *             MAIN             *
 ********************************/

int main(int argc, char **argv) {
    std::cout << "Entering main" << std::endl;
    Params::mpi::slaveInit = &slave_init;
    Params::mpi::masterInit = &master_init;

    //Substrate
    typedef FitMultiModalLogic<Params> fit_t;
    typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::weights>, fit::FitDummy<>, Params::dnn::weights> weight_t;
    typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::bias>, fit::FitDummy<>, Params::dnn::bias> bias_t;
    typedef PfWSum<weight_t> pf_t;
    typedef AfTanhLambda<bias_t, Params::dnn> af_t;
    typedef sferes::gen::DnnSpatialWS<Neuron<pf_t, af_t>, Connection<weight_t>, Params> substrate_t;

    //HyperNEAT (or not, if HNN is not defined)
#ifdef HNN
    typedef phen::Parameters<gen::EvoFloat<1, Params::cppn_params::cppn>,fit::FitDummy<>,Params::cppn_params::cppn> cppn_weight_t;
    typedef nn::PfWSum<cppn_weight_t> cppn_pf_t;
    typedef nn::AfCppn<nn::cppn::AfParamsFreezable<Params::cppn_params::cppn> > cppn_af_t;
    typedef gen::DnnFF<nn::Neuron<cppn_pf_t, cppn_af_t>, nn::Connection<cppn_weight_t>, Params::cppn_params> cppn_t;
    typedef gen::HnnWS<cppn_t, Params::cppn_params, Params::cppn_params::cppn> gen_t;

#ifdef LEO
    typedef phen::hnn::HnnLeo<substrate_t, gen_t, fit_t, Params::cppn_params> phen_t;
#else
    typedef phen::hnn::Hnn<substrate_t, gen_t, fit_t, Params::cppn_params> phen_t;
#endif
#else
    //The phenotype is the direct Neural network, using the genotype, fitness function and parameters defined above.
    typedef phen::Dnn<substrate_t, fit_t, Params> phen_t;
#endif

    typedef eval::ParallelWorldTracking<Params> eval_t;
    typedef boost::fusion::vector<
        stat::MultiModalLogic<phen_t, Params>, stat::SerializeRand<phen_t, Params>
#if defined(MAPELITE)
        ,stat::Map<phen_t, Params>
#endif
    >  stat_t;

    typedef boost::fusion::vector<
#if defined(DIV)        //Behavioral distance based only on current population.
    modif::BehaviorDiv<>
#endif
#if defined(MVG)
    ,modif::ModifMvg<Params>
#endif
    > modifier_t;

    //Set the ea
#if defined(PNSGA)
    typedef ea::Pnsga<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(MAPELITE)
    typedef ea::MapElite<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueMapElite<ea_t, Params> continue_ea_t;
#else
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#endif


    //Add options
    using namespace options;
    using namespace boost::program_options;
    add()("seed", value<int>(), "program seed");
    positionalAdd("seed", 1);
//    add()("plot-best", value<bool>()->default_value(true)->implicit_value(true), "Plot the best individual when loading");
//    add()("plot-all", value<bool>()->default_value(false)->implicit_value(true), "Plot all individuals when loading");
//    add()("recalc-best", value<bool>()->default_value(false)->implicit_value(true), "Always recalculate the best individual upon loading");
    add()("print-neuron-groups", value<bool>()->default_value(false)->implicit_value(true), "Print neuron groups.");
    add()("print-nn", value<bool>()->default_value(false)->implicit_value(true), "Print network layout.");
    add()("print-patterns", value<bool>()->default_value(false)->implicit_value(true), "Print network layout.");
    add()("test-dom-sort", value<bool>()->default_value(false)->implicit_value(true), "The the some-sort no duplicates.");
//    add()("visualize", value<bool>()->default_value(false)->implicit_value(true), "Write a network for all individuals.");
//    add()("analyze", value<bool>()->default_value(false)->implicit_value(true), "Perform additional analysis.");

    stat::MultiModalLogic<phen_t, Params>::initOptions();

    //Add the option to continue
    cont::init();
    options::parse_and_init(argc, argv, false);
    modules::rand::set_seed();
    cont::Continuator<continue_ea_t, Params> continuator;

#ifdef OSG
    continuator.setTransferCommand("transfer.sh");
#endif

    //Create the evolutionary algorithm
    continue_ea_t ea;

    //This will cause all slaves to enter the slave loop if MPI is defined
    //Prevents a lot of useless work, and makes debugging easier.
    std::vector<boost::shared_ptr<phen_t> > empty;
    fit_t _proto_fit;
    ea.eval().eval(empty, 0, 0, _proto_fit);

    //Start the run
    continuator.run(ea);

#if defined(MAPELITE)
    std::string fname = ea.res_dir() + "/archive.dat";
    if(fname == "/archive.dat") fname = "archive.dat";
    boost::fusion::at_c<2>(ea.stat()).write_archive(fname, ea);
#endif

    //Do some cleanup after the run if necessary
    if(continuator.performCleanup()){
        stat::mergeOneLineGenFiles(ea.res_dir(), "modularity.dat");
        continuator.deleteCheckpoints();
        continuator.moveGenFiles(ea.res_dir());
        io::deleteEmptyDirectories();
    }

    return 0;
}
