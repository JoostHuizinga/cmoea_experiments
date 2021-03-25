/*
 * mod_robot.cpp
 *
 *  Created on: Sep 26, 2013
 *      Author: Joost Huizinga
 */

/*************************************************************
 ********************* DERIVED DEFINES ***********************
 *************************************************************/

//Since I expect to constantly keep adding information to the serialize function
//I think it useful to track versions, rather than adding a new pre-processor macro
//every time I add something. This will be version 1, the previous version will be 0,
//and the next version will be 2, and so on.
#if defined(MRV1)
#define HIDDEN_LAYER_MODULARITY
#define CPPN_MODULARITY
#endif

//In contrast to the versions above I will also keep track of setups,
//setups track cases in which I keep a large number of parameters constant,
//changing only a small sub-set.
//To prevent filenames from becoming longer and longer and longer over time,
//I will consolidate everything that I do not plan to change right now to a
//setup.
//Setup 1.
#if defined(MRS1)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define SIXTASKS    //We are running the six-task experiment
#define MRMULT      //We are multiplying, rather than taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define CIE         //We are using the combinatorial innovation engine as our EA
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#endif

//Setup 2, specifically designed for MAP-Elites
#if defined(MRS2)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define SIXTASKS    //We are running the six-task experiment
#define MRMULT      //We are multiplying, rather than taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#endif

//Shortcut for MAP-Elites with Hidden Modularity on one axis and CPPN Modularity of the other axis
#if defined(ME)
#define MAPELITE
#define DESC_HMOD
#define DESC_CPPNMOD
#endif

#if defined(RANDOMFOOD) || defined(PREDATOR)
#define EA_EVAL_ALL
#endif

#if defined(VISUALIZE)
#define STEP_FUNCTION(steps) Params::simulator->stepsVisual(steps);
#else
#define STEP_FUNCTION(steps) Params::simulator->stepsNoVisual(steps);
#endif

#if not defined(PREDOBJ)
#define PREDDIST
#endif

#if defined(REGACT)
#define REGULATOR
#endif

#if defined(SIXTASKS)
#define FORWARD
#define BACKWARD
#define TURNLEFT
#define TURNRIGHT
#define JUMP
#define GETDOWN
#endif

#if defined(FORWARD) || defined(BACKWARD) || defined(TURNLEFT) || defined(TURNRIGHT) || defined(JUMP) || defined(GETDOWN)
#define TRACKER
#endif

#if defined(OACT)
#define ONEMODACTIVE
#endif

#if defined(CIE)
#define MAPELITEINOVPNSGA
#endif

#if defined(GRID)
#define X_LINE_INPUTS
#define Z_LINE_OUTPUTS
#define GRID_HIDDEN
#define NX6
#define NY5
#endif

#if defined(CIRCLE)
#define CIRCLE_HIDDEN
#endif

//Easy Inhibition
#if defined(EI)
#define USE_OUTPUT_POTENTIAL
#define SIGMOID_CTRNN_ACTIVATION
#define NEGATIVE_INACTIVE_VALUE
#endif

//Use a Sigmoid activation function in the hidden layer of the CTRNN only
#if defined(SIGH)
#define SIGMOID_CTRNN_ACTIVATION
#define USE_OUTPUT_TANH
#endif

//Enable forced modularity, either for inputs and hidden (FMOD), or just for hidden (FMODH)

//Force modules in the hidden layer only, and allow the inputs to be connected to every module
#if defined(FMOD)
#define FORCEDMODULES
#endif

#if defined(FMODH)
#define SEPARATE_SENSOR_GROUPS
#define INPUT_HIDDEN_ONE_TO_ALL
#define HIDDEN_TO_HIDDEN_WITHIN
#define ADD_ONE_HIDDEN_PLANE_PER_MODULE
#endif

#if defined(FORCEDMODULES)
#define SEPARATE_SENSOR_GROUPS
#define INPUT_HIDDEN_ONE_TO_ONE
#define HIDDEN_TO_HIDDEN_WITHIN
#define ADD_ONE_HIDDEN_PLANE_PER_MODULE
#endif

//Remap CPPN outputs, either One Output Per Module (OMOD), Separate Outputs (SO), or standard
#if defined(OMOD)
#define ONE_OUTPUT_SET_PER_MODULE
#elif not defined(SO) and defined(ADD_ONE_HIDDEN_PLANE_PER_MODULE)
#define SHAREDOUTPUTS
#endif

#if defined(CMOWTA)
#define CTRNN_WINNER_TAKES_ALL
#define CTRNN_MULTIPLE_OUTPUTSETS
#endif

#if defined(CMOSUM)
#define CTRNN_MULTIPLE_OUTPUTSETS_SUM
#define CTRNN_MULTIPLE_OUTPUTSETS
#endif

#if defined(CMORT)
#define CMO_RELATIVE_THRESHOLD
#endif

#if defined(REGCON)
#define REGULATORY_CONNECTION
#endif

#if defined(NOMSS)
#define NO_MULTI_SPATIAL_SUBSTRATE
#endif

#if not defined(ONE_SENSOR_GROUP)
#define SEPARATE_SENSOR_GROUPS
#endif

#if not defined(ONE_HIDDEN_GROUP)
#define ADD_ONE_HIDDEN_PLANE_PER_MODULE
#endif

#if not defined(HIDDEN_TO_HIDDEN_WITHIN)
#define HIDDEN_TO_HIDDEN_FULL
#endif

#if not defined(INPUT_HIDDEN_ONE_TO_ONE)
#define INPUT_HIDDEN_ONE_TO_ALL
#endif

#if not defined(SEPARATE_OUTPUTS)
#define SHAREDOUTPUTS
#endif



/*************************************************************
 ******************* NON-MANUAL DEFINES **********************
 *************************************************************/
//These defines should only be derived, and never set manually
#if defined(ADD_ONE_HIDDEN_PLANE_PER_BATCH)
#error "ADD_ONE_HIDDEN_PLANE_PER_BATCH should not be set manually"
#endif

#if defined(ADD_ONE_HIDDEN_PLANE_PER_NEURON_IN_BATCH)
#error "ADD_ONE_HIDDEN_PLANE_PER_NEURON_IN_BATCH should not be set manually"
#endif

#if defined(DIVHAMMING)
#error "DIVHAMMING should not be set manually"
#endif

#if defined(EUCLIDIAN)
#error "EUCLIDIAN should not be set manually"
#endif

#if defined(INNOV_CATEGORIES)
#error "INNOV_CATEGORIES should not be set manually"
#endif

#if defined(INNOV_DIVERSITY)
#error "INNOV_DIVERSITY should not be set manually"
#endif

#if defined(INNOV_CLOSEST_DIST)
#error "INNOV_CLOSEST_DIST should not be set manually"
#endif

#if defined(INPUT_VISUALIZE_GROUP_OFFSETS)
#error "INPUT_VISUALIZE_GROUP_OFFSETS should not be set manually"
#endif

#if defined(GRID) and defined(ADD_ONE_HIDDEN_PLANE_PER_MODULE)
#define ADD_ONE_HIDDEN_PLANE_PER_BATCH
#elif defined(ADD_ONE_HIDDEN_PLANE_PER_MODULE)
#define ADD_ONE_HIDDEN_PLANE_PER_NEURON_IN_BATCH
#endif

//Set the correct distance measure
#if defined(DIVINOUT) || defined(DIVREACT) || defined(DIVMINAVG)
#define DIVHAMMING
#endif

#if defined(DIVENDPOS)
#define EUCLIDIAN
#endif

//Combinatorial innovation engine defines
#if defined(MAPELITEINOVPNSGA)
#define INNOV_CATEGORIES
#define INNOV_DIVERSITY
#endif

#if defined(MAPELITEINOVDIV)
#define INNOV_CATEGORIES
#define INNOV_DIVERSITY
#define INNOV_CLOSEST_DIST
#endif

#if defined(SIXTASKS) || defined(TESTFIT)
#define INNOV_CATEGORIES
#endif

//Make sure the inputs are not overlapping when predator is defined
#if defined(PREDATOR)
#define INPUT_VISUALIZE_GROUP_OFFSETS
#endif


/*************************************************************
 ************************** GUARDS ***************************
 *************************************************************/

//Objective guards
#if defined(RANDOMFOOD)
#define OBJECTIVE_DEFINED
#endif

#if defined(PREDATOR)
#if defined(OBJECTIVE_DEFINED)
#define TWO_OR_MORE_OBJECTIVES_DEFINED
#else
#define OBJECTIVE_DEFINED
#endif
#endif

#if defined(TWO_OR_MORE_OBJECTIVES_DEFINED)
#if defined(RANDOMFOOD)
#warning "Objective: random food"
#endif
#if defined(PREDATOR)
#warning "Objective: predator"
#endif
#error "Two or more conflicting objectives defined, please remove one."
#endif

//Diversity guards
#if defined(DIV)
#error "DIV defined externally, which is not allowed."
#endif

#if defined(DIVENDPOS)
#if defined(DIV)
#define TWO_OR_MORE_DIVERSITIES_DEFINED
#else
#define DIV
#endif
#endif

#if defined(DIVINOUT)
#if defined(DIV)
#define TWO_OR_MORE_DIVERSITIES_DEFINED
#else
#define DIV
#endif
#endif

#if defined(DIVREACT)
#if defined(DIV)
#define TWO_OR_MORE_DIVERSITIES_DEFINED
#else
#define DIV
#endif
#endif

#if defined(DIVMINAVG)
#if defined(DIV)
#define TWO_OR_MORE_DIVERSITIES_DEFINED
#else
#define DIV
#endif
#endif

#if defined(TWO_OR_MORE_DIVERSITIES_DEFINED)
#if defined(DIVREACT)
#warning "Objective: divreact"
#endif
#if defined(DIVMINAVG)
#warning "Objective: divminavg"
#endif
#if defined(DIVENDPOS)
#warning "Objective: divendpos"
#endif
#if defined(DIVINOUT)
#warning "Objective: divinout"
#endif
#error "Two or more conflicting diversities defined, please remove one."
#endif

//Input configuration guards
#if defined(X_LINE_INPUTS)
#if defined(INPUT_CONFIGURATION_DEFINED)
#define TWO_INPUT_CONFIGURATIONS_DEFINED
#else
#define INPUT_CONFIGURATION_DEFINED
#endif
#endif

#if defined(CIRCULAR_INPUTS)
#if defined(INPUT_CONFIGURATION_DEFINED)
#define TWO_INPUT_CONFIGURATIONS_DEFINED
#else
#define INPUT_CONFIGURATION_DEFINED
#endif
#endif

#if not defined(INPUT_CONFIGURATION_DEFINED)
#define CIRCULAR_INPUTS
#endif

#if defined(TWO_INPUT_CONFIGURATIONS_DEFINED)
#error "Two input configurations defined"
#endif

//Output configuration guards
#if defined(Z_LINE_OUTPUTS)
#if defined(OUTPUT_CONFIGURATION_DEFINED)
#define TWO_OUTPUT_CONFIGURATIONS_DEFINED
#else
#define OUTPUT_CONFIGURATION_DEFINED
#endif
#endif

#if defined(CIRCULAR_OUTPUTS)
#if defined(OUTPUT_CONFIGURATION_DEFINED)
#define TWO_OUTPUT_CONFIGURATIONS_DEFINED
#else
#define OUTPUT_CONFIGURATION_DEFINED
#endif
#endif

#if not defined(OUTPUT_CONFIGURATION_DEFINED)
#define CIRCULAR_OUTPUTS
#endif

#if defined(TWO_OUTPUT_CONFIGURATIONS_DEFINED)
#error "Two output configurations defined"
#endif


//Set hidden configuration guards
#if defined(CIRCLE_HIDDEN)
#if defined(HIDDEN_CONFIGURATION_DEFINED)
#define TWO_HIDDEN_CONFIGURATIONS_DEFINED
#else
#define HIDDEN_CONFIGURATION_DEFINED
#endif
#endif

#if defined(GRID_HIDDEN)
#if defined(HIDDEN_CONFIGURATION_DEFINED)
#define TWO_HIDDEN_CONFIGURATIONS_DEFINED
#else
#define HIDDEN_CONFIGURATION_DEFINED
#endif
#endif

#if not defined(HIDDEN_CONFIGURATION_DEFINED)
#define GRID_HIDDEN
#endif

#if defined(TWO_HIDDEN_CONFIGURATIONS_DEFINED)
#error "Two hidden configurations defined"
#endif


//Output-set guards
#if defined(OS1)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS2)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS3)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS4)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS5)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS6)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if defined(OS7)
#if defined(OS_CONFIGURATION_DEFINED)
#define TWO_OS_CONFIGURATIONS_DEFINED
#else
#define OS_CONFIGURATION_DEFINED
#endif
#endif

#if not defined(OS_CONFIGURATION_DEFINED)
#define OS7
#endif

#if defined(TWO_OS_CONFIGURATIONS_DEFINED)
#error "Two output-set configurations defined"
#endif

// Diversity guards
#if defined(DIV) && !defined(EUCLIDIAN) && !defined(DIVHAMMING)
#error "Diversity defined, but no valid distance metric defined."
#endif


//Input to hidden guards
#if defined(INPUT_HIDDEN_ONE_TO_ALL) and defined(INPUT_HIDDEN_ONE_TO_ONE)
#error "INPUT_HIDDEN_ONE_TO_ALL and INPUT_HIDDEN_ONE_TO_ONE defined. Chose one."
#endif

//Use output guards
#if defined(USE_OUTPUT_POTENTIAL) and defined(USE_OUTPUT_TANH)
#error "USE_OUTPUT_POTENTIAL and USE_OUTPUT_TANH defined. Chose one."
#endif

//Bin size guards
#if defined(BIN1000)
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN100)
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN10)
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BINPOPSIZE)
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(TWO_BIN_SIZES_DEFINED)
#error "Two different bin sizes defined. Please define only one."
#endif

#if not defined(BIN_SIZE_DEFINED)
#define BINPOPSIZE
#endif

//Gen guards
#if defined(GEN5000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN3000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN1500)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN100)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN10)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN1)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(TWO_GENS_DEFINED)
#error "Two different number of generations defined. Please define only one."
#endif

#if defined(JHDEBUG) and not defined(GEN_DEFINED)
#define GEN100
#define GEN_DEFINED
#define NO_PARALLEL
#endif

#if not defined(GEN_DEFINED)
#error "Number of generations is not defined!"
#endif

//Pop Guards
//IMPORTANT: Population size has to be divisible by 4
#if defined(POP1000)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP100)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP12)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(TWO_POPS_DEFINED)
#error "Two different population sizes defined. Please define only one."
#endif

#if defined(JHDEBUG) and not defined(POP_DEFINED)
#define POP12
#define POP_DEFINED
#endif

#if not defined(POP_DEFINED)
#error "Population size is not defined!"
#endif

/*************************************************************
 ************************ INCLUDES ***************************
 *************************************************************/

//Standard header files
#include <typeinfo>
#include <iostream>
#include <bitset>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//Boost header files
#include <boost/fusion/container.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/spirit/include/karma.hpp>

//Sferes header files
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>
#include <sferes/dbg/dbg.hpp>


#if defined(REGULATOR)
#include <modules/hnn/af_dummy.hpp>
#include <modules/hnn/pf_regulator.hpp>
#endif

#if defined(REGULATORY_CONNECTION)
#include <modules/hnn/af_dummy.hpp>
#include <modules/hnn/pf_regulatory_conn.hpp>
#endif

//Sferes modules
#include <modules/nn2/nn.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/phen_dnn.hpp>
#include <modules/hnn/af_cppn.hpp>
#include <modules/modularity/modularity.hpp>
#include <modules/continue/continue_run.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>
#include <modules/continue/io_convenience.hpp>
#include <modules/misc/position3.hpp>

#if defined(MAPELITEINOVDIV)
#include <modules/mapelite/map_elite_innovation_diversity.hpp>
#include <modules/mapelite/continue_mapelite_innov_div.hpp>
#include "stat_mod_robot_map_elite_innov_div.hpp"
#endif

#if defined(MAPELITEINOVPNSGA)
#include <modules/mapelite/map_elite_innovation_nsga2.hpp>
#include <modules/mapelite/continue_mapelite_innov_nsga2.hpp>
#include "stat_mod_robot_map_elite_innov_div.hpp"
#endif

#if defined(MAPELITE)
#include <modules/mapelite/map_elite.hpp>
#include <modules/mapelite/continue_mapelite.hpp>
#include <modules/mapelite/stat_map.hpp>
#endif

#include "stat_mod_robot_innov_first_bin.hpp"

//Local header files
#include "behavior_div.hpp"
#include "gen_spatial.hpp"
#include "stat_mod_robot.hpp"
#include "mvg.hpp"
#include "modularity.hpp"
#include "mod2.hpp"
#include "stat_logall.hpp"
#include "mut_sweep_macros.hpp"
//#include "gen_hnn.hpp"

#if defined(CTRNN_WINNER_TAKES_ALL)
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/phen_hnn_ctrnn_winner_takes_all.hpp>
#elif defined(CTRNN_MULTIPLE_OUTPUTSETS_SUM)
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/phen_hnn_ctrnn_sum.hpp>
#elif defined(NOLEO)
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/phen_hnn_ctrnn.hpp>
#else
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/phen_hnn_ctrnn_leo.hpp>
#endif



#if defined(MPI_ENABLED) && !defined(NOMPI)
#include "eval_mpi_world_tracking.hpp"
#else
#include "eval_world_tracking.hpp"
#endif

#include "lzw.hpp"
#include "mod_robot_sim.hpp"
#include "mod_robot_pie_sensor.hpp"
#include "mod_robot_cone_sensor.hpp"
#include "mod_robot_control_sensor.hpp"
#include "clock.hpp"
#include "pnsga.hpp"
#include "nsga2_custom.hpp"
#include "dom_sort_no_duplicates.hpp"
#include "dom_sort_fast.hpp"
#include "common_compare.hpp"


/*************************************************************
 ************************ DEFINES ****************************
 *************************************************************/
#define NUMBER_OF_PERMUTATIONS 500
#define SQUARE_ROOT_3 1.7320508
#define SQ3 SQUARE_ROOT_3
#define D2 1.25
#define D3 1.5


/*************************************************************
 ************************* MACROS ****************************
 *************************************************************/
//This goes right before main
//Struct should point to where the array is defined (e.g Params::dnn::spatial)
//Name should be the name of the array
//Doesn't actually do anything, but it is required for compilation (otherwise you get linking errors)
#define SFERES_DECLARE_NON_CONST_ARRAY(STRUCT, NAME) \
        STRUCT::NAME##_t* STRUCT::_##NAME = 0; \
        size_t STRUCT::_##NAME##_size = 0;

//This goes where you actually build the array
//Struct should point to where the array is defined (e.g Params::dnn::spatial)
//Name should be the name of the array
//Size should be the size of the array
//
//Note: because I did not implement a set function yet,
//you'll have to initialize the values of the array through the _NAME array pointer
//E.g. Params::dnn::spatial::_x[0] = 1
//Or an other example
//E.g. (size_t i=0; i<Params::dnn::spatial::x_size();++i) Params::dnn::spatial::_x[i] = i
#define SFERES_INIT_NON_CONST_ARRAY(STRUCT, NAME, SIZE)    \
        STRUCT::_##NAME = new STRUCT::NAME##_t[SIZE]; \
        STRUCT::_##NAME##_size = SIZE;

//This goes where you would define your SFERES_ARRAY
//It only requires name and type
#define SFERES_NON_CONST_ARRAY(T, A)             \
        static T* _##A;                                \
        static size_t _##A##_size;                     \
        \
        static const T A(size_t i)			         \
        { assert(i < A##_size()); return _##A[i]; }    \
        \
        static const size_t A##_size()                 \
        { return _##A##_size; }                        \
        \
        static const size_t A##_set(size_t i, T value) \
        { return _##A[i] = value; }                    \
        \
        typedef T A##_t;


#define SFERES_DECLARE_NON_CONST_MATRIX(STRUCT, NAME) \
        STRUCT::NAME##_t* STRUCT::_##NAME = 0;            \
        size_t STRUCT::_##NAME##_rows = 0;                \
        size_t STRUCT::_##NAME##_columns = 0;

#define SFERES_INIT_NON_CONST_MATRIX(STRUCT, NAME, COLUMNS, ROWS)  \
        STRUCT::_##NAME = new STRUCT::NAME##_t[COLUMNS*ROWS];          \
        STRUCT::_##NAME##_rows = ROWS;                                 \
        STRUCT::_##NAME##_columns = COLUMNS;

#define SFERES_NON_CONST_MATRIX(T, A)                                 \
        static T* _##A;                                                     \
        static size_t _##A##_rows;                                          \
        static size_t _##A##_columns;                                       \
        \
        static const T A(size_t i)			                              \
        { assert(i < A##_size()); return _##A[i]; }                         \
        \
        static const T A(size_t column_index, size_t row_index)             \
        { assert(column_index < A##_columns());                             \
        assert(row_index < A##_rows());                                   \
        return A(column_index + row_index*A##_columns()); }               \
        \
        static void A##_set(size_t column_index, size_t row_index, T value) \
        { assert(column_index < A##_columns());                             \
        assert(row_index < A##_rows());                                   \
        _##A[column_index + row_index*A##_columns()] = value; }           \
        \
        static const size_t A##_size()                                      \
        { return _##A##_rows*_##A##_columns; }           			          \
        \
        static const size_t A##_rows()                                      \
        { return _##A##_rows; }           			                      \
        \
        static const size_t A##_columns()                                   \
        { return _##A##_columns; }           			                      \
        \
        typedef T A##_t;


#define SFERES_PARAMS(P)                                            \
        struct Ps__ {                                                     \
    Ps__() {                                                      \
            static std::string __params = # P;                        \
            boost::replace_all(__params, ";", ";\n");                 \
            boost::replace_all(__params, "{", "{\n");                 \
            boost::replace_all(__params, "static const", "");         \
            std::cout << "Parameters:" << __params << std::endl;      \
            }                                                             \
            };                                                                \
            P;                                                                \

#define PRINT_PARAMS Ps__ ____p


/*************************************************************
 *********************** NAMESPACES **************************
 *************************************************************/
using namespace sferes;
using namespace nn;
using namespace sferes::gen::evo_float;
using namespace sferes::gen::dnn;


/*************************************************************
 ******************** TYPE DEFINITIONS ***********************
 *************************************************************/
#if defined(SIGMOID_CTRNN_ACTIVATION)
typedef mod_robot::SigmoidNeuron ctrnn_neuron_t;
#else
typedef mod_robot::TanhNeuron ctrnn_neuron_t;
#endif
typedef mod_robot::CTRNN<ctrnn_neuron_t> ctrnn_nn_t;
typedef mod_robot::Spider<ctrnn_nn_t> robot_t;
typedef mod_robot::ModRobotSimulator<robot_t> sim_t;
typedef std::map<std::string, unsigned int> map_t;


/*************************************************************
 ************************ PARAMETERS *************************
 *************************************************************/
SFERES_PARAMS(
        struct Params {
    static long seed;
    static sim_t* simulator;

    static map_t* nodeConnectionMap;
    static map_t* neuronMap;

    static bool init;

    struct mpi
    {
        static mpi_init_ptr_t masterInit;
        static mpi_init_ptr_t slaveInit;
    };

    struct pop
    {
//#if defined(MAPELITEINOVDIV) || defined(MAPELITEINOVPNSGA)
//#if defined(JHDEBUG)
//        static const size_t init_size = 12;
//#else
//        static const size_t init_size = 1000;
//#endif
//#endif


//        static const unsigned size = 12;
//        static const unsigned nb_gen = 100;
//        static const int initial_aleat = 1;
//        static const unsigned size = 1000;
        //IMPORTANT: Population size has to be divisible by 4
#if defined(POP1000)
        static const unsigned size = 1000;
#elif defined(POP100)
        static const unsigned size = 100;
#elif defined(POP12)
        static const unsigned size = 12;
#else  //defined(POP1000)
        static const unsigned size = 1000;
#endif //defined(POP1000)

#if defined(GEN5000)
        static const unsigned nb_gen = 5001;
        static const int nr_of_dumps = 8;
#elif defined(GEN3000)
        static const unsigned nb_gen = 3001;
        static const int nr_of_dumps = 24;
#elif defined(GEN1500)
        static const unsigned nb_gen = 1501;
        static const int nr_of_dumps = 30;
#elif defined(GEN100)
        static const unsigned nb_gen = 100;
        static const int nr_of_dumps = 10;
#elif defined(GEN10)
        static const unsigned nb_gen = 10;
        static const int nr_of_dumps = 10;
#elif defined(GEN1)
        static const unsigned nb_gen = 1;
        static const int nr_of_dumps = 1;
#else  //defined(GEN5000)
        static const unsigned nb_gen = 1001;
        static const int nr_of_dumps = 8;
#endif //defined(GEN5000)
        static const int initial_aleat = 1;
        static int dump_period;
        static int checkpoint_period;

#if defined(MAPELITE)
        static const int initial_mutations = 10;
#endif

#if defined(MAPELITEINOVDIV) || defined(MAPELITEINOVPNSGA) || defined(MAPELITE)
#if defined(INITSIZE10000)
        static const size_t init_size = 10000;
#else
        static const size_t init_size = size;
#endif
#endif
    };

    struct ea
    {
        PARAM_DEPENDENT_PROBABILITY(SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0));
#if defined(NODUPSORT)
        typedef sferes::ea::dom_sort_no_duplicates_f dom_sort_f;
#elif defined(FASTSORT)
        typedef sferes::ea::dom_sort_fast_f dom_sort_f;
#else
        typedef sferes::ea::dom_sort_basic_f dom_sort_f;
#endif
        static const size_t clusters = 10;

#if defined(MAPELITE)
        static const size_t res_x = 100;
        static const size_t res_y = 100;
#endif
    };

#if defined(MAPELITEINOVPNSGA)
    struct innov_pnsga
    {
#if defined(NOPNSGA)
        typedef sferes::ea::_dom_sort_basic::non_dominated_f non_dom_f;
#else
        typedef sferes::ea::innov_pnsga::prob_dom_f<ea> non_dom_f;
#endif
    };
#endif

    struct innov
    {
        static const size_t categories = 63;
        static const size_t obj_index = 0;
#if defined(BIN1000)
        static const size_t bin_size = 1000;
#elif defined(BIN100)
        static const size_t bin_size = 100;
#elif defined(BIN10)
        static const size_t bin_size = 10;
#elif defined(BINPOPSIZE)
        static const size_t bin_size = Params::pop::size;
#else
#error "No bin size defined"
#endif
    };

    struct rand_obj
    {
        static const float std_dev = 0.1;
    };

    //Visualization parameters
    struct visualisation{
#if defined(LENGTH) && defined(HNN)
        SFERES_ARRAY(int, background, 135, 197, 107);
#elif defined(HNN)
        SFERES_ARRAY(int, background, 170, 197, 225);
#elif defined(LENGTH)
        SFERES_ARRAY(int, background, 255, 255, 180);
#else
        SFERES_ARRAY(int, background, 255, 200, 200);
#endif

        //		static const SvgColor color = SvgColor(1,2,3);
        static const size_t max_modularity_split = 3;
        static const float scale = 100;

        SFERES_NON_CONST_ARRAY(float, x);
        SFERES_NON_CONST_ARRAY(float, y);
        SFERES_NON_CONST_ARRAY(float, z);
    };

    struct sim
    {
#ifdef STEP5
        static const size_t steps = 5;
#elif defined(STEP200)
        static const size_t steps = 200;
#else
        static const size_t steps = 1000;
#endif
        static const size_t move_steps = 400;
        static const size_t turn_steps = 200;
        static const size_t jump_steps = 100;
        static const size_t get_down_steps = 200;
    };

    struct stats
    {
        static const size_t period = 1; //print stats every N generations
        static const size_t logall_period = 100;
    };

    //These values are for evolving the substrate directly,
    //They are not used when HyperNEAT is active

    // for weights
    struct weights
    {
        struct evo_float
        {
            static const float cross_rate = 0.5f;
            static const float mutation_rate = 1.0f; //1.0 because dnn handle this globally
            static const float eta_m = 10.0f;
            static const float eta_c = 10.0f;
            static const mutation_t mutation_type = polynomial;
            static const cross_over_t cross_over_type = sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, -2, -1, 1, 2); // no 0 !
            static const float mutation_rate = 1.0f;
            static const float cross_rate = 0.5f;
            static const bool ordered = true;
        };
        struct parameters
        {
            static const float min = -2;
            static const float max = 2;
        };
    };

    //For the bias
    struct bias
    {
        struct evo_float
        {
            static const float cross_rate = 0.5f;
            static const float mutation_rate = 1.0f / 24.0f;
            static const float eta_m = 10.0f;
            static const float eta_c = 10.0f;
            static const mutation_t mutation_type = polynomial;
            static const cross_over_t cross_over_type = sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, -2, -1, 0, 1, 2);
            static const float mutation_rate = 1.0 / 24.0f;
            static const float cross_rate = 0.0f;
            static const bool ordered = false;
        };
        struct parameters
        {
            static const float min = -2;
            static const float max = 2;
        };
    };

    //For the time constant
    struct time_constant
    {
        struct evo_float
        {
            static const float cross_rate = 0.5f;
            static const float mutation_rate = 1.0f / 24.0f;
            static const float eta_m = 10.0f;
            static const float eta_c = 10.0f;
            static const mutation_t mutation_type = polynomial;
            static const cross_over_t cross_over_type = sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, 1, 2, 3, 4, 5);
            static const float mutation_rate = 1.0 / 24.0f;
            static const float cross_rate = 0.0f;
            static const bool ordered = false;
        };
        struct parameters
        {
            static const float min = 1;
            static const float max = 6;
        };
    };

    struct dnn
    {
        // slope of the sigmoid
        static const size_t lambda = 20;

        static const float m_rate_add_conn = PARAM_DEPENDENT_CONADDMUTRATE(0.2f);		//Add Connection rate for the cppn
        static const float m_rate_del_conn = PARAM_DEPENDENT_CONDELMUTRATE(0.2f);		//Delete connection rate for the cppn

        //static const float m_rate_change_conn = 0.15; //old default
        static const float m_rate_change_conn = 0.0; //new default?
        static const float m_rate_add_neuron = 0.0f;
        static const float m_rate_del_neuron = 0.0f;
        // average number of weight changes
        static const float m_avg_weight = 2.0f;

        static const int io_param_evolving = true;
        static const init_t init = ff;

        // Number of neurons per batch in the hidden layer
        // Terminology is vague on purpose
        // When the neurons are organized as a grid the `batches'
        // are columns and the `neurons per batch' are rows.
        // When the neurons are organized as a circle the `batches'
        // are rings and the `neurons per batch' are the number of neurons per ring
#if defined(NX4)
        static const size_t nb_neuron_batches = 4;
#elif defined(NX5)
        static const size_t nb_neuron_batches = 5;
#elif defined(NX6)
        static const size_t nb_neuron_batches = 6;
#else
        static const size_t nb_neuron_batches = 5;
#endif

#if defined(NY1)
        static const size_t nb_neurons_per_batch = 1;
#elif defined(NY4)
        static const size_t nb_neurons_per_batch = 4;
#elif defined(NY5)
        static const size_t nb_neurons_per_batch = 5;
#elif defined(NY6)
        static const size_t nb_neurons_per_batch = 6;
#else
        static const size_t nb_neurons_per_batch = 5;
#endif

        static size_t nb_inputs;     //6
        static const size_t nb_outputs = robot_t::total_actuator_count; //18
        static const size_t nb_of_hidden = nb_neuron_batches*nb_neurons_per_batch;

        static const size_t min_nb_neurons = 10; //useless
        static const size_t max_nb_neurons = 30; //useless
        static const size_t min_nb_conns = 0; // ?? TODO
        static const size_t max_nb_conns = 120; // ?? TODO

        struct spatial{
            //RAND COORDS START
            //These are settings for random coordinates. They are not actually used in this experiment, but they are necessary to compile.
            static const float min_coord = -3.0f;
            static const float max_coord = 3.0f;
            static const bool rand_coords = false;
            //RAND COORDS END

            SFERES_NON_CONST_ARRAY(int, layers);
            SFERES_NON_CONST_ARRAY(float, x);
            SFERES_NON_CONST_ARRAY(float, y);
            SFERES_NON_CONST_ARRAY(float, z);
        };

    };

    struct fitness
    {
        //#ifdef DIVREACT
        static const unsigned int nb_bits  = Params::dnn::nb_outputs;
        //#else
        //		static const unsigned int nb_bits  = Params::dnn::nb_inputs + Params::dnn::nb_outputs;
        //#endif
    };

    /***********************/
    /*** CPPN PARAMETERS ***/
    /***********************/
    struct ParamsHnn {
        //This struct defines the planes of the multi-spatial HyperNEAT model.
        struct multi_spatial{
            SFERES_NON_CONST_ARRAY(int, planes);
            SFERES_NON_CONST_MATRIX(bool, connected);
            SFERES_NON_CONST_ARRAY(size_t, neuron_offsets);
            SFERES_NON_CONST_ARRAY(size_t, connection_offsets);
        };

        struct cppn {
            // params of the CPPN
            // These values define the mutations on the activation function of the CPPN nodes
            // As such, they define which activation function can be chosen and the mutation rate for changing the activation function
            struct sampled {
                SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid, nn::cppn::gaussian, nn::cppn::linear); //Possible activation functions
                static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);			//Mutation rate for the cppn nodes
                static const float cross_rate = 0.0f;				//Not actually used, even if non-zero
                static const bool ordered = false;					//Mutation will randomly pick an other activation function from the list
                //The alternative has bias towards adjacent values
            };

            // These value determine the mutation rate for the weights and biases of the cppn
            // Note that, in practice, the bias is determined by the weight of the connection between
            // the bias input node and the target node, so biases are not mutated 'directly'.
            struct evo_float {
                static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);			//Mutation rate for the cppn nodes
                static const float cross_rate = 0.0f; 				//Not actually used, even if non-zero
                static const mutation_t mutation_type = polynomial;	//Mutation type
                static const cross_over_t cross_over_type = sbx;	//Not used
                static const float eta_m = 10.0f;					//Mutation eta (parameter for polynomial mutation.
                static const float eta_c = 10.0f;					//?
            };

            struct parameters{
                static const float min = -3; // maximum value of parameters (weights & bias)
                static const float max = 3;  // minimum value
            };

#if defined(CTRNN_MULTIPLE_OUTPUTSETS)
#if defined(OS1)
            static const size_t nb_output_sets = 1;
#elif defined(OS2)
            static const size_t nb_output_sets = 2;
#elif defined(OS3)
            static const size_t nb_output_sets = 3;
#elif defined(OS4)
            static const size_t nb_output_sets = 4;
#elif defined(OS5)
            static const size_t nb_output_sets = 5;
#elif defined(OS6)
            static const size_t nb_output_sets = 6;
#elif defined(OS7)
            static const size_t nb_output_sets = 7;
#else
#error "Multiple output-sets defined, but the number of output-sets is not defined."
#endif
#endif

            //This threshold determines at what value of the Link-Expression Output (LEO) a link will be expressed or not.
            //Before LEO values are currently between -1.0 and 1.0 and only values equal to or above the threshold are expressed.
            //As a result a value of -1.0 means all links are expressed (effectively disabling the LEO node)
            //while a value greater than 1.0 means no values will ever be expressed (probably a bad idea).
            //A value of 0.0 is default
#if defined(CMO_RELATIVE_THRESHOLD)
            //These values are set such that the initial probability of any one connection existing is 0.5,
            //regardless of the number of output sets.
#if defined(OS1)
            static const float leo_threshold = 1.0f - (0.5f * 2);
#elif defined(OS2)
            static const float leo_threshold = 1.0f - (0.292893f * 2);
#elif defined(OS3)
            static const float leo_threshold = 1.0f - (0.206299f * 2);
#elif defined(OS4)
            static const float leo_threshold = 1.0f - (0.159104f * 2);
#elif defined(OS5)
            static const float leo_threshold = 1.0f - (0.129449f * 2);
#elif defined(OS6)
            static const float leo_threshold = 1.0f - (0.109101f * 2);
#elif defined(OS7)
            static const float leo_threshold = 1.0f - (0.0942763f * 2);
#else
#error "Multiple output-sets defined, but the number of output-sets is not defined."
#endif
#else
            static const float leo_threshold = 0.0f;
#endif
            typedef gen::RandomSeed seed_t;
        };

        //Set the pheontype
#if defined(CTRNN_WINNER_TAKES_ALL)
        typedef phen::hnn::HnnCtrnnWinnerTakesAllConstants hnn_const_t;
#elif defined(CTRNN_MULTIPLE_OUTPUTSETS_SUM)
        typedef phen::hnn::HnnCtrnnWinnerTakesAllConstants hnn_const_t;
#elif defined(NOLEO)
        typedef phen::hnn::HnnCtrnnConstants hnn_const_t;
#else
        typedef phen::hnn::HnnCtrnnLeoConstants hnn_const_t;
#endif

#if defined(REGULATOR)
        struct regulator {
            struct sampled {
                SFERES_ARRAY(float, values, nn::sine, nn::sigmoid, nn::gaussian, nn::linear, nn::regulator); //Possible activation functions
                static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);			//Mutation rate for the cppn nodes
                static const float cross_rate = 0.0f;				//Not actually used, even if non-zero
                static const bool ordered = false;					//Mutation will randomly pick an other activation function from the list
            };
        };
#endif

#if defined(REGULATORY_CONNECTION)
        struct regulatory_conn{
            struct pf_param{
                struct sampled {
                    SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid, nn::cppn::gaussian, nn::cppn::linear); //Possible activation functions
                    static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);          //Mutation rate for the cppn nodes
                    static const float cross_rate = 0.0f;               //Not actually used, even if non-zero
                    static const bool ordered = false;                  //Mutation will randomly pick an other activation function from the list
                };
            };
            struct weight_param{
                struct sampled {
                    SFERES_ARRAY(float, values, nn::normal, nn::regulatory); //Indicating that a weight can be either normal or regulatory
                    static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);          //Mutation rate for the cppn nodes
                    static const float cross_rate = 0.0f;               //Not actually used, even if non-zero
                    static const bool ordered = false;                  //Mutation will randomly pick an other activation function from the list
                };
                struct evo_float {
                    static const float mutation_rate = PARAM_DEPENDENT_CONCHANGEMUTRATE(0.1f);          //Mutation rate for the cppn nodes
                    static const float cross_rate = 0.0f;               //Not actually used, even if non-zero
                    static const mutation_t mutation_type = polynomial; //Mutation type
                    static const cross_over_t cross_over_type = sbx;    //Not used
                    static const float eta_m = 10.0f;                   //Mutation eta (parameter for polynomial mutation.
                    static const float eta_c = 10.0f;                   //?
                };

                struct parameters{
                    static const float min = -3; // maximum value of parameters (weights & bias)
                    static const float max = 3;  // minimum value
                };
            };
        };

#endif

        //These are the parameters used for setting the CPPN
        //The 7 inputs are the x, y, z coordinates for neuron 1
        //The x, y, z coordinates for neuron 2
        //And a bias.
        //The outputs are the weights and the link expression, and the bias.
        struct dnn
        {

#ifdef DISTI
            static const size_t nb_inputs = 8;
            typedef DistanceInput<Params::dnn> input_t;
#else
            static const size_t nb_inputs = 7;
            typedef phen::hnn::ThreeDInput<Params::dnn> input_t;
#endif
            static size_t nb_outputs;
            static const init_t init = ff;

            static const float m_rate_add_conn = PARAM_DEPENDENT_CONADDMUTRATE(0.09f);		//Add Connection rate for the cppn
            static const float m_rate_del_conn = PARAM_DEPENDENT_CONDELMUTRATE(0.08f);		//Delete connection rate for the cppn
            static const float m_rate_add_neuron = PARAM_DEPENDENT_NEUADDMUTRATE(0.05f);		//Add neuron rate for the cppn
            static const float m_rate_del_neuron = PARAM_DEPENDENT_NEUDELMUTRATE(0.04f);		//Delete neuron rate for the cppn

            static const float m_rate_change_conn = 0.0f;		//Not used for the cppn, even if value is non zero.

            static const size_t min_nb_neurons = 7;
            static const size_t max_nb_neurons = 200;
            static const size_t min_nb_conns = 0;
            static const size_t max_nb_conns = 1500;
        };
    };
};
)




//template <typename Type>
//inline std::string toString(Type value){
//	std::ostringstream stream;
//	stream << value;
//	return stream.str();
//}

void set_seed(){
    dbg::trace trace("init", DBG_HERE);
    if(options::map.count("seed")){
        int seed = options::map["seed"].as<int>();
        std::cout<<"seed: " << seed << std::endl;
        srand(seed);
        misc::seed(seed);
    } else {
        time_t t = time(0) + ::getpid();
        std::cout<<"seed: " << t << std::endl;
        srand(t);
        misc::seed(t);
    }
}


inline std::string inputToHiddenKey(const uint& inputIndex, const uint& hiddenIndex){
    std::ostringstream stream;
    stream << "i" << inputIndex << '_' << hiddenIndex;
    return stream.str();
}


inline std::string hiddenToHiddenKey(const uint& hiddenIndex1, const uint& hiddenIndex2){
    std::ostringstream stream;
    if(hiddenIndex1 >= Params::dnn::nb_of_hidden){
        stream << 'o' << (hiddenIndex1 - Params::dnn::nb_of_hidden);
    } else {
        stream << hiddenIndex1;
    }
    stream << '_';

    if(hiddenIndex2 >= Params::dnn::nb_of_hidden){
        stream << 'o' << (hiddenIndex2 - Params::dnn::nb_of_hidden);
    } else {
        stream << hiddenIndex2;
    }
    return stream.str();
}


robot_t* init_spider(){
    dbg::trace trace("init", DBG_HERE);
    btDynamicsWorld* world = Params::simulator->getWorld();
    robot_t* spider = new robot_t(world);
    dbg::out(dbg::info, "init") << "Building spider success" << std::endl;
    btScalar angle;
    std::vector<btVector3> positions;
    const btVector3 up(0,1,0);
    const btVector3 xVector(1,0,0);

    //For now we'll assume that there are always 6 positions
    static const unsigned int nb_of_positions = 6;

#if defined(CIRCULAR_INPUTS)
    btVector3 orientation (1,0,0);
    angle = M_PI/nb_of_positions;
    for(size_t i=0; i<nb_of_positions; i++){
        positions.push_back(orientation + up);
        orientation = orientation.rotate(up, angle*2);
    }
#elif defined(X_LINE_INPUTS)
    static const btScalar x_min = -1.0;
    static const btScalar x_max = 1.0;
    const btScalar increment = (x_max - x_min)/btScalar(nb_of_positions-1);

    for(size_t i=0; i<nb_of_positions; i++){
        positions.push_back(xVector*(x_min + i*increment) + up);
    }
#else
#error "No setup for the inputs specified (what happened to the default?)."
#endif

    //Setup food sensors in six directions
#if defined(PREDATOR) || defined(RANDOMFOOD)
    dbg::assertion(DBG_ASSERTION(positions.size() == 6));
    spider->addSensorGroup();
    spider->getLastSensorGroup().addDetects(sim_t::food_type);

    for(size_t i=0; i<6; i++){
        mod_robot::PieSensor* food_sensor = new mod_robot::PieSensor(spider->getMainBody(), food_orientation, angle);
        food_sensor->setModifier(btVector3(1, 0, 1));
        food_sensor->setNeuronPosition(positions[i]);
        food_sensor->setActiveColor(Color(0.5, 1.0, 0.5, 0.5));
        spider->getLastSensorGroup().add(food_sensor);
    }
#endif

    //Setup control sensors (or dummy sensors if not controlled)
    std::vector<mod_robot::ControlSensor*> controlSensors;
#if defined(NEGATIVE_INACTIVE_VALUE)
    btScalar inactiveValue = -1.0f;
#else
    btScalar inactiveValue = 0.0f;
#endif

#if defined(FORWARD)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(BACKWARD)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(TURNLEFT)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(TURNRIGHT)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(JUMP)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(GETDOWN)
    controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
#endif

#if defined(DUMFIT) || defined(TESTFIT)
    for(int i=0; i<6; i++){
        controlSensors.push_back(new mod_robot::ControlSensor(controlSensors.size(), inactiveValue));
    }
#endif

    //Position the control sensors
    if(controlSensors.size() > 0){
        btVector3 control_orientation (1,0,0);
        angle = (2*M_PI)/controlSensors.size();
        for(int i=0; i<controlSensors.size(); i++){
            controlSensors[i]->setNeuronPosition(positions[i]);
            control_orientation = control_orientation.rotate(btVector3(0,1,0), angle);
        }

        //Add sensors to sensor groups
#if defined(SEPARATE_SENSOR_GROUPS)
        for(int i=0; i<controlSensors.size(); i++){
            spider->addSensorGroup();
            spider->getLastSensorGroup().addDetects(sim_t::control_type);
            spider->getLastSensorGroup().add(controlSensors[i]);
        }
#else
        spider->addSensorGroup();
        spider->getLastSensorGroup().addDetects(sim_t::control_type);
        for(int i=0; i<controlSensors.size(); i++){
            spider->getLastSensorGroup().add(controlSensors[i]);
        }
#endif
    }

#if defined(REACHING)
    btScalar top_angle = M_PI/4;
    btMatrix3x3 rotation;
    rotation.setEulerZYX(0, 0, M_PI_2);
    mod_robot::ConeSensor* food_sensor = new mod_robot::ConeSensor(spider->getMainBody(), rotation, top_angle);
    food_sensor->setNeuronPosition(btVector3(0,1,0));
    food_sensor->setActiveColor(Color(0.5, 1.0, 0.5, 0.5));
    spider->getLastSensorGroup().add(food_sensor);
#endif


#if defined(PREDATOR)
    spider->addSensorGroup();
    spider->getLastSensorGroup().addDetects(sim_t::predator_type);
//    btVector3 predator_orientation (1,0,0);
//    angle = M_PI/6;

    for(size_t i=0; i<6; i++){
        mod_robot::PieSensor* predator_sensor = new mod_robot::PieSensor(spider->getMainBody(), predator_orientation, angle);
        predator_sensor->setModifier(btVector3(1, 0, 1));
        predator_sensor->setNeuronPosition(positions[i]);
        predator_sensor->setActiveColor(Color(1.0, 0.5, 0.5, 0.5));
        spider->getLastSensorGroup().add(predator_sensor);
//        predator_orientation = predator_orientation.rotate(btVector3(0,1,0), angle*2);
    }
#endif

    return spider;
    //	spider->addSensor();
}

// Sets
inline void setPos(const btScalar& x, const btScalar& y, const btScalar& z,
        const btScalar& x_vis, const btScalar& y_vis, const btScalar& z_vis, size_t& neuron_index){
    Params::dnn::spatial::_x[neuron_index] = x;
    Params::dnn::spatial::_y[neuron_index] = y;
    Params::dnn::spatial::_z[neuron_index] = z;
    Params::visualisation::_x[neuron_index] = x_vis;
    Params::visualisation::_y[neuron_index] = y_vis;
    Params::visualisation::_z[neuron_index] = z_vis;
    dbg::out(dbg::info, "neuronpos") << "index:" << neuron_index <<
            " x: " << Params::dnn::spatial::_x[neuron_index] <<
            " y: " << Params::dnn::spatial::_y[neuron_index] <<
            " z: " << Params::dnn::spatial::_z[neuron_index] << std::endl;
    ++neuron_index;
}

inline void setPos(const btScalar& x, const btScalar& y, const btScalar& z, size_t& neuron_index){
    setPos(x, y, z, x, y, z, neuron_index);
}

inline void setPos(const btScalar& x, const btScalar& z, size_t& neuron_index){
    setPos(x, 0, z, x, 0, z, neuron_index);
}

inline void setPos(const btScalar& x, const btScalar& z, const btScalar& x_vis, const btScalar& z_vis, size_t& neuron_index){
    setPos(x, 0, z, x_vis, 0, z_vis, neuron_index);
}

class NeuronGroup{
public:
    NeuronGroup(size_t size, bool is_input = false, size_t offset = 0, std::string prefix = "" , size_t local_index = 0):
        _size(size),
        _offset(offset),
        _is_input(is_input)
    {
        _name = prefix + boost::lexical_cast<std::string>(local_index);
    }
    size_t size(){return _size;}
    size_t offset(){return _offset;}
    bool is_input(){return _is_input;}
    std::string name(){return _name;}
private:
    size_t _size;
    size_t _offset;
    bool _is_input;
    std::string _name;
};

class ConnectionGroup{
public:
    ConnectionGroup(size_t source, size_t target, size_t offset = 0):
        _source(source),
        _target(target),
        _offset(offset)
    {}
    size_t source(){return _source;}
    size_t target(){return _target;}
    size_t offset(){return _offset;}
private:
    size_t _source;
    size_t _target;
    size_t _offset;
};

class NeuronGroups{
public:
    enum CppnIndex{
        staticIndex,
        sourceIndex,
        targetIndex,
        combinatorialIndex
    };

    enum Layers{
        inputLayer = 0,
        hiddenLayer,
        outputLayer
    };


    NeuronGroups():
        _separateOutputsPerLayer(true),
        _separateOutputsPerLayerNeurons(true),
        _hiddenCppnIndex(sourceIndex),
        _outputCppnIndex(sourceIndex),
        _inputToHiddenCppnIndex(sourceIndex),
        _hiddenToHiddenCppnIndex(staticIndex),
        _hiddenToOutputCppnIndex(targetIndex),
        _nb_cppn_outputs(0)
    {}

    /**
     * Adds an input group of size size_of_neuron_group.
     *
     * Substrate input groups are not associated with a CPPN neuron output group,
     * because substrate inputs do not require bias or time-constant information.
     */
    void addInput(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Input neuron group added of size: " << size_of_neuron_group << std::endl;
        _inputLayer.push_back(NeuronGroup(size_of_neuron_group, true, 0, "in", _inputLayer.size()));
    }

    /**
     * Adds a hidden group of size size_of_neuron_group.
     *
     * If the _hiddenCppnIndex is 'static', all substrate hidden groups will be assigned
     * to the same CPPN neuron output group. Otherwise every substrate hidden group
     * will be assigned to a new CPPN neuron output group.
     *
     * After adding all hidden groups, _nb_cppn_neuron_groups_assigned_to_hidden_layer will reflect
     * the number of CPPN neuron groups currently assigned to neuron groups in the hidden layer.
     */
    void addHidden(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Hidden neuron group added of size: " << size_of_neuron_group << std::endl;
        std::string key = getCppnOutputGroupKey(_hiddenCppnIndex, hiddenLayer, _hiddenLayer.size());
        _hiddenLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "hid", _hiddenLayer.size()));
    }

    /**
     * Adds an output group of size size_of_neuron_group.
     *
     * If the _outputCppnIndex is 'static', all substrate output groups will be assigned
     * to the same CPPN neuron output group. If _separateOutputsPerLayerNeurons is false,
     * the CPPN neuron output group will be 0. If _separateOutputsPerLayerNeurons is true,
     * the CPPN neuron output group will be _nb_cppn_neuron_groups_assigned_to_hidden_layer.
     *
     */
    void addOutput(size_t size_of_neuron_group){
        dbg::out(dbg::info, "neuron_groups") << "Output neuron group added of size: " << size_of_neuron_group << std::endl;
        std::string key = getCppnOutputGroupKey(_outputCppnIndex, outputLayer, _outputLayer.size());
        _outputLayer.push_back(NeuronGroup(size_of_neuron_group, false, assign(key), "out", _outputLayer.size()));
    }

    size_t getNbInputGroups() const{ return _inputLayer.size();}
    size_t getNbHiddenGroups() const{ return _hiddenLayer.size();}
    size_t getNbOutputGroups() const{ return _outputLayer.size();}

//    size_t getNbCppnNeuronOutputGroupsHidden() const{
//        if(getNbHiddenGroups() == 0) return 0;
//        return _hiddenCppnIndex == staticIndex ? 1 : getNbHiddenGroups();
//    }
//
//    size_t getNbCppnNeuronOutputGroupsOutput() const{
//        if(getNbOutputGroups() == 0) return 0;
//        return _outputCppnIndex == staticIndex ? 1 : getNbOutputGroups();
//    }
//
//    size_t getNbCppnNeuronOutputGroups() const {
//        size_t result;
//        if(_separateOutputsPerLayerNeurons){
//            result = getNbCppnNeuronOutputGroupsHidden() + getNbCppnNeuronOutputGroupsOutput();
//        } else {
//            result = std::max(getNbCppnNeuronOutputGroupsHidden(), getNbCppnNeuronOutputGroupsOutput());
//        }
//        return result;
//    }

    void connectInputToHidden(size_t input_index, size_t hidden_index){
        std::string key = getCppnOutputGroupKey(_inputToHiddenCppnIndex, inputLayer, input_index, hidden_index);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(input_index, real_hidden_index, assign(key)));
//        if(_max_cppn_index_input_to_hidden < cppnIndex) _max_cppn_index_input_to_hidden = cppnIndex;
    }

    void connectHiddenToHidden(size_t hidden_index_1, size_t hidden_index_2){
        std::string key = getCppnOutputGroupKey(_hiddenToHiddenCppnIndex, hiddenLayer, hidden_index_1, hidden_index_2);
        size_t real_hidden_index_1 = hidden_index_1 + getNbInputGroups();
        size_t real_hidden_index_2 = hidden_index_2 + getNbInputGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index_1, real_hidden_index_2, assign(key)));
//        if(_max_cppn_index_hidden_to_hidden < cppnIndex) _max_cppn_index_hidden_to_hidden = cppnIndex;
    }

    void connectHiddenToOutput(size_t hidden_index, size_t output_index){
        std::string key = getCppnOutputGroupKey(_hiddenToOutputCppnIndex, outputLayer, hidden_index, output_index);
        size_t real_hidden_index = hidden_index + getNbInputGroups();
        size_t real_output_index = output_index + getNbInputGroups() + getNbHiddenGroups();
        _connection_groups.push_back(ConnectionGroup(real_hidden_index, real_output_index, assign(key)));
//        if(_max_cppn_index_hidden_to_output < cppnIndex) _max_cppn_index_hidden_to_output = cppnIndex;
    }

//    size_t getNbCppnOutputsForNeurons(){
//        return std::max(_max_cppn_index_output, _max_cppn_index_hidden) + 1;
//    }

//    size_t getMaxCppnIndexConnections(){
//        return std::max(std::max(_max_cppn_index_input_to_hidden, _max_cppn_index_hidden_to_hidden), _max_cppn_index_hidden_to_output);
//    }


//    size_t getCppnIndexHidden(size_t hidden_index){
//        size_t cppn_output_index = 0;
//
//        switch(_hiddenCppnIndex){
//        case staticIndex:
//            //Do nothing
//            break;
//        case sourceIndex:
//        case targetIndex:
//        case combinatorialIndex:
//            cppn_output_index += hidden_index;
//            break;
//        default:
//            dbg::sentinel(DBG_HERE);
//        }
//        return cppn_output_index;
//    }
//
//    size_t getCppnIndexOutput(size_t output_index){
//        size_t cppn_output_index = 0;
//
//        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
//       //greater than highest number used in the previous layer
//        if(_separateOutputsPerLayerNeurons){
//            cppn_output_index = getNbCppnNeuronOutputGroupsHidden();
//        }
//
//        switch(_outputCppnIndex){
//        case staticIndex:
//            //Do nothing
//            break;
//        case sourceIndex:
//        case targetIndex:
//        case combinatorialIndex:
//            cppn_output_index += output_index;
//            break;
//        default:
//            dbg::sentinel(DBG_HERE);
//        }
//        return cppn_output_index;
//    }
//
//    size_t getCppnIndexInputToHidden(size_t input_index, size_t hidden_index){
//        size_t cppn_output_index = getNbCppnOutputsForNeurons();
//
//        switch(_inputToHiddenCppnIndex){
//        case staticIndex:
//            //Do nothing
//            break;
//        case sourceIndex:
//            cppn_output_index += input_index;
//            break;
//        case targetIndex:
//            cppn_output_index += hidden_index;
//            break;
//        case combinatorialIndex:
//            cppn_output_index += input_index * getNbInputGroups() + hidden_index;
//            break;
//        default:
//            dbg::sentinel(DBG_HERE);
//        }
//        return cppn_output_index;
//    }
//
//
//
//    size_t getCppnIndexHiddenToHidden(size_t hidden_index_1, size_t hidden_index_2){
//        size_t cppn_output_index = getNbCppnOutputsForNeurons();
//
//        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
//       //greater than highest number used in the previous layer
//        if(_separateOutputsPerLayer){
//            cppn_output_index = _max_cppn_index_input_to_hidden + 1;
//        }
//
//        switch(_hiddenToHiddenCppnIndex){
//        case staticIndex:
//            //Do nothing
//            break;
//        case sourceIndex:
//            cppn_output_index += hidden_index_1;
//            break;
//        case targetIndex:
//            cppn_output_index += hidden_index_2;
//            break;
//        case combinatorialIndex:
//            cppn_output_index += hidden_index_1 * getNbHiddenGroups() + hidden_index_2;
//            break;
//        default:
//            dbg::sentinel(DBG_HERE);
//        }
//
//        return cppn_output_index;
//    }
//
//    size_t getCppnIndexHiddenToOutput(size_t hidden_index, size_t output_index){
//        size_t cppn_output_index = getNbCppnOutputsForNeurons();
//
//        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
//       //greater than highest number used in the previous layer
//        if(_separateOutputsPerLayer){
//            cppn_output_index = _max_cppn_index_hidden_to_hidden + 1;
//        }
//
//        switch(_hiddenToOutputCppnIndex){
//        case staticIndex:
//            //Do nothing
//            break;
//        case sourceIndex:
//            cppn_output_index += hidden_index;
//            break;
//        case targetIndex:
//            cppn_output_index += output_index;
//            break;
//        case combinatorialIndex:
//            cppn_output_index += hidden_index * getNbHiddenGroups() + output_index;
//            break;
//        default:
//            dbg::sentinel(DBG_HERE);
//        }
//
//        return cppn_output_index;
//    }



    void connectInputToHiddenFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: fully connected." << std::endl;
        for(size_t i=0; i<getNbInputGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                connectInputToHidden(i,j);
            }
        }
    }

    void connectInputToHiddenOneToOne(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: one-to-one connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_inputToHiddenCppnIndex != combinatorialIndex));
        for(size_t i=0; i<getNbInputGroups(); ++i){
            connectInputToHidden(i,i);
        }
    }

    void connectHiddenToHiddenFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: fully connected." << std::endl;
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbHiddenGroups(); ++j){
                connectHiddenToHidden(i,j);
            }
        }
    }

    void connectHiddenToHiddenWithin(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: within groups connected." << std::endl;
        dbg::assertion(DBG_ASSERTION(_hiddenToHiddenCppnIndex != combinatorialIndex));
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            connectHiddenToHidden(i,i);
        }
    }

    void connectHiddenToOutputFull(){
        dbg::trace trace("neuron_groups", DBG_HERE);
        dbg::out(dbg::info, "neuron_groups") << "Hidden to output: fully connected." << std::endl;
        for(size_t i=0; i<getNbHiddenGroups(); ++i){
            for(size_t j=0; j<getNbOutputGroups(); ++j){
                connectHiddenToOutput(i,j);
            }
        }
    }



    void setSeparateIndexPerLayer(bool flag){
        _separateOutputsPerLayer = flag;
    }

    void setSeparateIndexPerLayerNeurons(bool flag){
        _separateOutputsPerLayerNeurons = flag;
    }

    CppnIndex getHiddenCppnIndex(){
        return _hiddenCppnIndex;
    }

    void setHiddenCppnIndex(CppnIndex cppnIndex){
        _hiddenCppnIndex = cppnIndex;
    }

    CppnIndex getOutputCppnIndex(){
        return _outputCppnIndex;
    }

    void setOutputCppnIndex(CppnIndex cppnIndex){
        _outputCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the input layer to the hidden layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getInputToHiddenCppnIndex(){
        return _inputToHiddenCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the input layer to the hidden layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setInputToHiddenCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Input to hidden: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _inputToHiddenCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the hidden layer to the hidden layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getHiddenToHiddenCppnIndex(){
        return _hiddenToHiddenCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the hidden layer to the hidden layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setHiddenToHiddenCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Hidden to hidden: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _hiddenToHiddenCppnIndex = cppnIndex;
    }

    /**
     * Returns the general pattern for which CPPN output is responsible for which connection,
     * from the hidden layer to the output layer.
     *
     * @return An enumerator indicating the CPPN output pattern.
     */
    CppnIndex getHiddenToOutputCppnIndex(){
        return _hiddenToOutputCppnIndex;
    }

    /**
     * Sets the general pattern dictating which CPPN output is responsible for which connection,
     * from the hidden layer to the output layer.
     *
     * @param cppnIndex An enumerator indicating the CPPN output pattern.
     */
    void setHiddenToOutputCppnIndex(CppnIndex cppnIndex){
        dbg::out(dbg::info, "neuron_groups") << "Hidden to output: " << getCppnOutputGroupPattern(cppnIndex) << std::endl;
        _hiddenToOutputCppnIndex = cppnIndex;
    }

    static std::string getCppnOutputGroupPattern(CppnIndex cppnIndex){
        switch(cppnIndex){
        case staticIndex:
            return "static";
            break;
        case sourceIndex:
            return "source";
            break;
        case targetIndex:
            return "target";
            break;
        case combinatorialIndex:
            return "combinatorial";
            break;
        default:
            dbg::sentinel(DBG_HERE);
            return "";
        }
    }

    void build(){
        std::vector<NeuronGroup> allLayers;
        allLayers.insert(allLayers.end(), _inputLayer.begin(), _inputLayer.end());
        allLayers.insert(allLayers.end(), _hiddenLayer.begin(), _hiddenLayer.end());
        allLayers.insert(allLayers.end(), _outputLayer.begin(), _outputLayer.end());

        SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, planes, allLayers.size());
        SFERES_INIT_NON_CONST_MATRIX(Params::ParamsHnn::multi_spatial, connected, allLayers.size(), allLayers.size());
        SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, neuron_offsets, allLayers.size());
        SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, connection_offsets, _connection_groups.size());

        //Initialize connected matrix with zeros
        for(size_t i=0; i<allLayers.size(); ++i){
            for(size_t j=0; j<allLayers.size(); ++j){
                Params::ParamsHnn::multi_spatial::connected_set(i, j, false);
            }
        }

        //Set planes
        for(size_t i=0; i<allLayers.size(); ++i){
//            size_t offset = allLayers[i].getCppnNeuronOutputGroup() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
//            size_t offset = _cppn_output_groups[allLayers[i].getGroupKey()].offset();
            Params::ParamsHnn::multi_spatial::planes_set(i, allLayers[i].size());
            Params::ParamsHnn::multi_spatial::neuron_offsets_set(i, allLayers[i].offset());
        }

        for(size_t i=0; i<_connection_groups.size(); ++i){
//            size_t offset = base_offset + _connection_groups[i].getCppnConnectionOutputGroup() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
            Params::ParamsHnn::multi_spatial::connected_set(_connection_groups[i].source(), _connection_groups[i].target(), true);
            Params::ParamsHnn::multi_spatial::connection_offsets_set(i, _connection_groups[i].offset());
        }
#if defined(CTRNN_MULTIPLE_OUTPUTSETS)
        Params::ParamsHnn::dnn::nb_outputs = Params::ParamsHnn::cppn::nb_output_sets * phen::hnn::HnnCtrnnWinnerTakesAllConstants::nb_of_cppn_outputs;
#else
//        size_t nb_of_cppn_outputs_for_neurons = getNbCppnNeuronOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
//        size_t nb_of_cppn_outputs_for_connections = getNbCppnConnectionOutputGroups() * Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
        Params::ParamsHnn::dnn::nb_outputs = _nb_cppn_outputs;
#endif
    }

//    size_t getNbCppnConnectionOutputGroups(){
//        return _assigned_cppn_connection_output_groups.size();
//    }
//
//    void assignConnectionOutputGroupInputToHidden(size_t id){
//        size_t group_index = _cppn_connection_output_groups_input_to_hidden.size();
//        _cppn_connection_output_groups_input_to_hidden[id] = group_index;
//    }

    void print(){
        using namespace boost::spirit::karma;
        size_t offset;
        std::vector<NeuronGroup> allLayers;
        allLayers.insert(allLayers.end(), _inputLayer.begin(), _inputLayer.end());
        allLayers.insert(allLayers.end(), _hiddenLayer.begin(), _hiddenLayer.end());
        allLayers.insert(allLayers.end(), _outputLayer.begin(), _outputLayer.end());

        std::map<size_t, std::string> offset_neuron_map;
        std::map<size_t, std::set<std::pair<size_t, size_t> > > offset_con_map;
        std::cout << "Neuron groups total: " << allLayers.size() << std::endl;

        std::cout << std::endl;
        std::cout << "Input groups: " << _inputLayer.size() << std::endl;
        for(size_t i=0; i<_inputLayer.size(); ++i){
            offset = _inputLayer[i].offset();
            std::cout << " - input group: " << i << " size: " << _inputLayer[i].size() << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Hidden groups: " << _hiddenLayer.size() << std::endl;
        for(size_t i=0; i<_hiddenLayer.size(); ++i){
            offset = _hiddenLayer[i].offset();
            std::cout << " - hidden group: " << i << " size: " << _hiddenLayer[i].size() << " offset: " << _hiddenLayer[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " " + _hiddenLayer[i].name();
        }

        std::cout << std::endl;
        std::cout << "Output groups: " << _outputLayer.size() << std::endl;
        for(size_t i=0; i<_outputLayer.size(); ++i){
            offset = _outputLayer[i].offset();
            std::cout << " - output group: " << i << " size: " << _outputLayer[i].size() << " offset: " << _outputLayer[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " " + _outputLayer[i].name();
        }

        dbg::out(dbg::info, "neuron_groups") << "Connection groups: " << _connection_groups.size() << std::endl;
        for(size_t i=0; i<_connection_groups.size(); ++i){
            offset = _connection_groups[i].offset();
            std::string source_str = allLayers[_connection_groups[i].source()].name();
            std::string target_str = allLayers[_connection_groups[i].target()].name();
            dbg::out(dbg::info, "neuron_groups") << " - connection group: " << i
                    << " source: " << source_str << "(" << _connection_groups[i].source() << ")"
                    << " target: " << target_str << "(" << _connection_groups[i].target() << ")"
                    << " offset: " << _connection_groups[i].offset() << std::endl;
            offset_neuron_map[offset] = offset_neuron_map[offset] + " con_" + boost::lexical_cast<std::string>(i) + "_s_" + source_str + "_t_" + target_str;
            offset_con_map[offset];
            offset_con_map[offset].insert(std::pair<size_t, size_t>(_connection_groups[i].source(), _connection_groups[i].target()));
        }

        size_t cppn_outputs = _nb_cppn_outputs;
        std::cout << std::endl;
        std::cout << "CPPN outputs: " << cppn_outputs << std::endl;

        std::cout << std::endl;
        std::cout << "Hidden layer:" << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], _hiddenLayer[j].offset()) << " ";
        }
        std::cout << std::endl;

        std::cout << std::endl;
        std::cout << "Output layer:" << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << _outputLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], _outputLayer[j].offset()) << " ";
        }
        std::cout << std::endl;

        //Print Input to Hidden
        std::cout << std::endl;
        std::cout << "Input to hidden:" << std::endl;
        std::cout << "      ";
        for(size_t j=0; j<_inputLayer.size(); ++j){
            std::cout << _inputLayer[j].name() << "  ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
            for(size_t k=0; k<_inputLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k, j + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << " .  ";
                }
            }
            std::cout << std::endl;
        }

        //Print Hidden to Hidden
        std::cout << std::endl;
        std::cout << "Hidden to hidden:" << std::endl;
        std::cout << "     ";
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
            for(size_t k=0; k<_hiddenLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k + _inputLayer.size(), j + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << "  .  ";
                }
            }
            std::cout << std::endl;
        }

        //Print Hidden to Output
        std::cout << std::endl;
        std::cout << "Hidden to output:" << std::endl;
        std::cout << "     ";
        for(size_t j=0; j<_hiddenLayer.size(); ++j){
            std::cout << _hiddenLayer[j].name() << " ";
        }
        std::cout << std::endl;
        for(size_t j=0; j<_outputLayer.size(); ++j){
            std::cout << _outputLayer[j].name() << " ";
            for(size_t k=0; k<_hiddenLayer.size(); ++k){
                bool flag = true;
                for(size_t i = 0; i<cppn_outputs; ++i){
                    if(offset_con_map[i].count(std::pair<size_t, size_t>(k + _inputLayer.size(), j + _hiddenLayer.size() + _inputLayer.size()))){
                        std::cout << format(right_align(4, ' ')[maxwidth(4)[int_]], i) << " ";
                        flag = false;
                    }
                }
                if(flag) {
                    std::cout << "  .  ";
                }
            }
            std::cout << std::endl;
        }
    }


private:
    std::vector<NeuronGroup> _inputLayer;
    std::vector<NeuronGroup> _hiddenLayer;
    std::vector<NeuronGroup> _outputLayer;
    std::vector<ConnectionGroup> _connection_groups;

//    size_t _max_cppn_index_hidden;
//    size_t _max_cppn_index_output;
//    size_t _max_cppn_index_input_to_hidden;
//    size_t _max_cppn_index_hidden_to_hidden;
//    size_t _max_cppn_index_hidden_to_output;
//
//    size_t _nb_cppn_neuron_groups_assigned_to_hidden_layer;
//    size_t _nb_cppn_neuron_groups_assigned_to_output_layer;


    bool _separateOutputsPerLayer;
    bool _separateOutputsPerLayerNeurons;

    CppnIndex _hiddenCppnIndex;
    CppnIndex _outputCppnIndex;
    CppnIndex _inputToHiddenCppnIndex;
    CppnIndex _hiddenToHiddenCppnIndex;
    CppnIndex _hiddenToOutputCppnIndex;

    std::map<std::string, size_t> _cppn_output_groups;
    size_t _nb_cppn_outputs;

    size_t assign(std::string key){
        if(_cppn_output_groups.count(key) == 0){
            _cppn_output_groups[key] = _nb_cppn_outputs;
            if(key.substr(0,3) == "con"){
                _nb_cppn_outputs += Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection;
            } else {
                _nb_cppn_outputs += Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron;
            }
        }
        return _cppn_output_groups[key];
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, size_t source_index, size_t target_index){
        std::string key = "con_";

        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
       //greater than highest number used in the previous layer
        if(_separateOutputsPerLayer){
            key += boost::lexical_cast<std::string>(layer) + "_";
        }

        switch(cppnIndex){
        case staticIndex:
            key += "static";
            break;
        case sourceIndex:
            key += boost::lexical_cast<std::string>(source_index);
            break;
        case targetIndex:
            key += boost::lexical_cast<std::string>(target_index);
            break;
        case combinatorialIndex:
            key += boost::lexical_cast<std::string>(source_index) + "_" + boost::lexical_cast<std::string>(target_index);
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        return key;
    }

    std::string getCppnOutputGroupKey(CppnIndex cppnIndex, size_t layer, size_t index){
        std::string key = "neu_";

        //If we use separate cppn outputs for each layer, make sure the cppn_output_index is one
        //greater than highest number used in the previous layer
        if(_separateOutputsPerLayerNeurons){
            key += boost::lexical_cast<std::string>(layer) + "_";
        }

        switch(cppnIndex){
        case staticIndex:
            key += "static";
            break;
        case sourceIndex:
        case targetIndex:
        case combinatorialIndex:
            key += boost::lexical_cast<std::string>(index);
            break;
        default:
            dbg::sentinel(DBG_HERE);
        }

        return key;
    }
};

void joint_init(){
    dbg::trace trace("init", DBG_HERE);

    if(Params::init) return;

    //Create neuron object and set its properties
    NeuronGroups neuronGroups;
#if defined(ONE_OUTPUT_SET_PER_MODULE)
    neuronGroups.setSeparateIndexPerLayer(false);
    neuronGroups.setHiddenCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setOutputCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setInputToHiddenCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setHiddenToOutputCppnIndex(NeuronGroups::sourceIndex);
#elif defined(NO_MULTI_SPATIAL_SUBSTRATE)
    neuronGroups.setSeparateIndexPerLayer(false);
    neuronGroups.setSeparateIndexPerLayerNeurons(false);
    neuronGroups.setHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setOutputCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setInputToHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setHiddenToOutputCppnIndex(NeuronGroups::staticIndex);
#elif defined(SHAREDOUTPUTS)
    neuronGroups.setSeparateIndexPerLayer(true);
    neuronGroups.setHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setOutputCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setInputToHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups::staticIndex);
    neuronGroups.setHiddenToOutputCppnIndex(NeuronGroups::targetIndex);
#else
    neuronGroups.setSeparateIndexPerLayer(true);
    neuronGroups.setHiddenCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setOutputCppnIndex(NeuronGroups::sourceIndex);
    neuronGroups.setInputToHiddenCppnIndex(NeuronGroups::combinatorialIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups::combinatorialIndex);
    neuronGroups.setHiddenToOutputCppnIndex(NeuronGroups::combinatorialIndex);
#endif


    //Create simulator
    Params::simulator = new sim_t();
    Params::simulator->initPhysics();


    //Create spider
    robot_t* spider = init_spider();

    //Set planes and connect them
//    std::vector<size_t> hidden_layer_indices;
    size_t nb_sensor_groups = spider->getNumberOfSensorGroups();
//    size_t nb_planes = nb_sensor_groups;

    //////////////////////////////////////////////////////////////
    for(size_t input_index=0; input_index<nb_sensor_groups; ++input_index){
        neuronGroups.addInput(spider->getNumberOfSensorsInGroup(input_index));
    }
    //////////////////////////////////////////////////////////////

#if defined(ADD_ONE_HIDDEN_PLANE_PER_NEURON_IN_BATCH)
//    for(size_t i=0; i<Params::dnn::nb_neurons_per_batch; ++i){
//        hidden_layer_indices.push_back(nb_planes++);
//    }
//    size_t hidden_neurons_per_plane = Params::dnn::nb_neuron_batches;

    //////////////////////////////////////////////////////////////
    for(size_t i=0; i<Params::dnn::nb_neurons_per_batch; ++i){
        neuronGroups.addHidden(Params::dnn::nb_neuron_batches);
    }
    //////////////////////////////////////////////////////////////

#elif defined(ADD_ONE_HIDDEN_PLANE_PER_BATCH)
//    for(size_t i=0; i<Params::dnn::nb_neuron_batches; ++i){
//        hidden_layer_indices.push_back(nb_planes++);
//    }
//    size_t hidden_neurons_per_plane = Params::dnn::nb_neurons_per_batch;

    //////////////////////////////////////////////////////////////
    for(size_t i=0; i<Params::dnn::nb_neuron_batches; ++i){
        neuronGroups.addHidden(Params::dnn::nb_neurons_per_batch);
    }
    //////////////////////////////////////////////////////////////

#else
//    hidden_layer_indices.push_back(nb_planes++);
//    size_t hidden_neurons_per_plane = Params::dnn::nb_of_hidden;

    //////////////////////////////////////////////////////////////
    neuronGroups.addHidden(Params::dnn::nb_of_hidden);
    //////////////////////////////////////////////////////////////
#endif
//    dbg::out(dbg::info, "init") <<
//            "hidden_layer_indices front: " << hidden_layer_indices.front() <<
//            " hidden_layer_indices back: " << hidden_layer_indices.back() <<std::endl;
//
//    size_t knee_layer_index = nb_planes++;
//    size_t hip_pitch_layer_index = nb_planes++;
//    size_t hip_yaw_layer_index = nb_planes++;

    //////////////////////////////////////////////////////////////
    neuronGroups.addOutput(robot_t::knee_joint_count);
    neuronGroups.addOutput(robot_t::hip_pitch_joint_count);
    neuronGroups.addOutput(robot_t::hip_yaw_joint_count);



    //Connect input to hidden
#if defined(INPUT_HIDDEN_ONE_TO_ONE)
    if(neuronGroups.getInputToHiddenCppnIndex() == NeuronGroups::combinatorialIndex){
        neuronGroups.setInputToHiddenCppnIndex(NeuronGroups::sourceIndex);
    }
    neuronGroups.connectInputToHiddenOneToOne();
#elif defined(INPUT_HIDDEN_ONE_TO_ALL)
    neuronGroups.connectInputToHiddenFull();
#else
#error "No connectivity pattern defined from input to hidden"
#endif

    //Connect hidden to hidden
#if defined(HIDDEN_TO_HIDDEN_WITHIN)
    if(neuronGroups.getHiddenToHiddenCppnIndex() == NeuronGroups::combinatorialIndex){
        neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups::sourceIndex);
    }
    neuronGroups.connectHiddenToHiddenWithin();
#elif defined(HIDDEN_TO_HIDDEN_FULL)
    neuronGroups.connectHiddenToHiddenFull();
#else
#error "No connectivity pattern defined from hidden to hidden"
#endif

    neuronGroups.connectHiddenToOutputFull();

    neuronGroups.build();
    if(options::map["print-neuron-groups"].as<bool>()){
        neuronGroups.print();
    }
    //////////////////////////////////////////////////////////////
//
//
//    //Initialize the connectivity matrix such that no planes are connected
//    SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, planes, nb_planes);
//    SFERES_INIT_NON_CONST_MATRIX(Params::ParamsHnn::multi_spatial, connected, nb_planes, nb_planes);
//    for(size_t i=0; i<Params::ParamsHnn::multi_spatial::connected_size(); ++i){
//        Params::ParamsHnn::multi_spatial::_connected[i] = false;
//    }
//
//    //Set input planes
//    for(size_t input_index=0; input_index<nb_sensor_groups; ++input_index){
//        Params::ParamsHnn::multi_spatial::_planes[input_index] = spider->getNumberOfSensorsInGroup(input_index);
//    }
//
//#if defined(INPUT_HIDDEN_ONE_TO_ONE)
//    //Connect each input to a single neuron group in the hidden layer
//    dbg::out(dbg::info, "init") << "Hidden layer indices: " << hidden_layer_indices.size() << " sensor groups: " <<  nb_sensor_groups << std::endl;
//    dbg::assertion(DBG_ASSERTION(hidden_layer_indices.size() == nb_sensor_groups));
//    for(size_t input_index=0; input_index<nb_sensor_groups; ++input_index){
//        size_t hidden_layer_index = hidden_layer_indices[input_index];
//        Params::ParamsHnn::multi_spatial::connected_set(input_index, hidden_layer_index, true);
//    }
//#else
//    //Connected each input to every neuron group in the hidden layer
//    for(size_t input_index=0; input_index<nb_sensor_groups; ++input_index){
//        for(size_t i=0; i<hidden_layer_indices.size(); ++i){
//            size_t hidden_layer_index = hidden_layer_indices[i];
//            Params::ParamsHnn::multi_spatial::connected_set(input_index, hidden_layer_index, true);
//        }
//    }
//#endif
//
//    //Set hidden plane
//    for(size_t i=0; i<hidden_layer_indices.size(); ++i){
//        size_t hidden_layer_index = hidden_layer_indices[i];
//        Params::ParamsHnn::multi_spatial::connected_set(hidden_layer_index, hidden_layer_index, true);
//        Params::ParamsHnn::multi_spatial::_planes[hidden_layer_index] = hidden_neurons_per_plane;
//    }
//
//    //Set knee joints plane
//    for(size_t i=0; i<hidden_layer_indices.size(); ++i){
//        size_t hidden_layer_index = hidden_layer_indices[i];
//        Params::ParamsHnn::multi_spatial::connected_set(hidden_layer_index, knee_layer_index, true);
//    }
//    Params::ParamsHnn::multi_spatial::_planes[knee_layer_index] = robot_t::knee_joint_count;
//
//    //Set hip joints plane
//    for(size_t i=0; i<hidden_layer_indices.size(); ++i){
//        size_t hidden_layer_index = hidden_layer_indices[i];
//        Params::ParamsHnn::multi_spatial::connected_set(hidden_layer_index, hip_pitch_layer_index, true);
//    }
//    Params::ParamsHnn::multi_spatial::_planes[hip_pitch_layer_index] = robot_t::hip_pitch_joint_count;
//
//    //Set yaw joints plane
//    for(size_t i=0; i<hidden_layer_indices.size(); ++i){
//        size_t hidden_layer_index = hidden_layer_indices[i];
//        Params::ParamsHnn::multi_spatial::connected_set(hidden_layer_index, hip_yaw_layer_index, true);
//    }
//    Params::ParamsHnn::multi_spatial::_planes[hip_yaw_layer_index] = robot_t::hip_yaw_joint_count;
//
//    //Count the number of connected planes
//    size_t connected = 0;
//    for(size_t i=0; i<Params::ParamsHnn::multi_spatial::connected_size(); ++i){
//        if(Params::ParamsHnn::multi_spatial::connected(i)){
//            ++connected;
//        }
//    }
//
//
//    SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, neuron_offsets, connected);
//    SFERES_INIT_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, connection_offsets, connected);
//
//    static const size_t nb_of_outputs_neuron = phen::hnn::ctrnn::nb_of_outputs_neuron;
//    static const size_t nb_of_outputs_connection = phen::hnn::ctrnn::nb_of_outputs_connection;
//    size_t offset = 0;
//    size_t conn_index = 0;
//
//#if defined(ONE_OUTPUT_SET_PER_MODULE)
//    std::vector<std::vector<size_t> > offset_matrix(nb_planes, std::vector<size_t>(nb_planes, 0));
//
//    //Account for all neuron outputs
//    for(size_t i=0; i<(hidden_layer_indices.back()+1); ++i){
//        Params::ParamsHnn::multi_spatial::_neuron_offsets[i] = (i%6)*nb_of_outputs_neuron;
//        dbg::out(dbg::info, "init") << "Neuron offset " << i << ":" << Params::ParamsHnn::multi_spatial::_neuron_offsets[i] << std::endl;
//    }
//
//    for(size_t i=(hidden_layer_indices.back()+1); i<nb_planes; ++i){
//        Params::ParamsHnn::multi_spatial::_neuron_offsets[i] = 6*nb_of_outputs_neuron;
//        dbg::out(dbg::info, "init") << "Neuron offset " << i << ":" << Params::ParamsHnn::multi_spatial::_neuron_offsets[i] << std::endl;
//    }
//
//    offset = 7*nb_of_outputs_neuron;
//
//    //From input to hidden
//    for(size_t i=0; i<hidden_layer_indices.front(); ++i){
//        for(size_t j=hidden_layer_indices.front(); j<(hidden_layer_indices.back()+1); ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset + (i%6)*nb_of_outputs_connection;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset_matrix[i][j] << std::endl;
//            }
//        }
//    }
//
//    //From hidden to hidden
//    for(size_t i=hidden_layer_indices.front(); i<(hidden_layer_indices.back()+1); ++i){
//        for(size_t j=hidden_layer_indices.front(); j<(hidden_layer_indices.back()+1); ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset + (i%6)*nb_of_outputs_connection;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset_matrix[i][j] << std::endl;
//            }
//        }
//    }
//
//    //From hidden to output
//    for(size_t j=(hidden_layer_indices.back()+1); j<nb_planes; ++j){
//        for(size_t i=hidden_layer_indices.front(); i<(hidden_layer_indices.back()+1); ++i){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset + (i%6)*nb_of_outputs_connection;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset_matrix[i][j] << std::endl;
//            }
//        }
//    }
//
//    //Set offsets
//    for(size_t i=0; i<nb_planes; ++i){
//        for(size_t j=0; j<nb_planes; ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                Params::ParamsHnn::multi_spatial::_connection_offsets[conn_index] = offset_matrix[i][j];
//                dbg::out(dbg::info, "init") <<
//                        "Connection offset " << conn_index << ":" << Params::ParamsHnn::multi_spatial::_connection_offsets[conn_index] << std::endl;
//                ++conn_index;
//            }
//        }
//    }
//
//    offset += 6*nb_of_outputs_connection;
//
//#elif defined(SHAREDOUTPUTS)
//    std::vector<std::vector<size_t> > offset_matrix(nb_planes, std::vector<size_t>(nb_planes, 0));
//
//    //Account for all neuron outputs
//    for(size_t i=0; i<nb_planes; ++i){
//        //Add an output when we switch from input to hidden
//        //And add an output for every output plane
//        if(i==(hidden_layer_indices.front()) || i>hidden_layer_indices.back()){
//            offset+=nb_of_outputs_neuron;
//        }
//        Params::ParamsHnn::multi_spatial::_neuron_offsets[i] = offset;
//        dbg::out(dbg::info, "init") << "Neuron offset " << i << ":" << offset << std::endl;
//    }
//
//
//    offset+=nb_of_outputs_connection;
//
//    //From input to hidden
//    for(size_t i=0; i<hidden_layer_indices.front(); ++i){
//        for(size_t j=hidden_layer_indices.front(); j<(hidden_layer_indices.back()+1); ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset << std::endl;
//            }
//        }
//    }
//    offset+=nb_of_outputs_connection;
//
//    //From hidden to hidden
//    for(size_t i=hidden_layer_indices.front(); i<(hidden_layer_indices.back()+1); ++i){
//        for(size_t j=hidden_layer_indices.front(); j<(hidden_layer_indices.back()+1); ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset << std::endl;
//            }
//        }
//    }
//    offset+=nb_of_outputs_connection;
//
//    //From hidden to output
//    for(size_t j=(hidden_layer_indices.back()+1); j<nb_planes; ++j){
//        for(size_t i=hidden_layer_indices.front(); i<(hidden_layer_indices.back()+1); ++i){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                offset_matrix[i][j] = offset;
//                dbg::out(dbg::info, "init") << "Connection offset " << i << "," << j << ":" << offset << std::endl;
//            }
//        }
//        offset+=nb_of_outputs_connection;
//    }
//
//    //Set offsets
//    for(size_t i=0; i<nb_planes; ++i){
//        for(size_t j=0; j<nb_planes; ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                Params::ParamsHnn::multi_spatial::_connection_offsets[conn_index] = offset_matrix[i][j];
//                dbg::out(dbg::info, "init") <<
//                        "Connection offset " << conn_index << ":" << Params::ParamsHnn::multi_spatial::_connection_offsets[conn_index] << std::endl;
//                ++conn_index;
//            }
//        }
//    }
//
//#else
//    //Account for all neuron outputs
//    for(size_t i=0; i<nb_planes; ++i){
//        Params::ParamsHnn::multi_spatial::_neuron_offsets[i] = offset;
//        dbg::out(dbg::info, "init") << "Neuron offset " << i << ":" << offset << std::endl;
//        offset+=nb_of_outputs_connection;
//    }
//
//    //Account for all connection outputs
//    for(size_t i=0; i<nb_planes; ++i){
//        for(size_t j=0; j<nb_planes; ++j){
//            if(Params::ParamsHnn::multi_spatial::connected(i, j)){
//                Params::ParamsHnn::multi_spatial::_connection_offsets[conn_index] = offset;
//                dbg::out(dbg::info, "init") << "Connection offset " << conn_index << ":" << offset << std::endl;
//                ++conn_index;
//                offset+=nb_of_outputs_connection;
//            }
//        }
//    }
//#endif


    //Set the number of outputs for the cppn
//    Params::ParamsHnn::dnn::nb_outputs = Params::ParamsHnn::multi_spatial::planes_size()*nb_of_outputs_neuron + connected*nb_of_outputs_connection;


    //Set the number of inputs for the dnn
    Params::dnn::nb_inputs = spider->getNumberOfSensors();
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, layers, 3);

    Params::dnn::spatial::_layers[0] = Params::dnn::nb_inputs;
    Params::dnn::spatial::_layers[1] = Params::dnn::nb_of_hidden;
    Params::dnn::spatial::_layers[2] = Params::dnn::nb_outputs;
    size_t nb_of_neurons = Params::dnn::nb_inputs + Params::dnn::nb_of_hidden + Params::dnn::nb_outputs;

    dbg::out(dbg::info, "init") << "layers : number: "<< Params::dnn::spatial::layers_size()
        << " layer 0: " << Params::dnn::spatial::layers(0)
        << " layer 1: " << Params::dnn::spatial::layers(1)
        << " layer 2: " << Params::dnn::spatial::layers(2) << std::endl;

    //Set the neuron positions
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, z, nb_of_neurons);

    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, z, nb_of_neurons);

    //Set the positions for the input layer
    float vis_dist_mod = 0.35;
    size_t neuron_index = 0;
    for(size_t input_group_index=0; input_group_index<nb_sensor_groups; ++input_group_index){
        mod_robot::SensorGroup sensor_group = spider->getSensors(input_group_index);
        for(size_t input_index=0; input_index<sensor_group.size(); ++input_index){
            btVector3 neuron_position = sensor_group[input_index]->getNeuronPosition();
            Params::dnn::spatial::_x[neuron_index] = neuron_position.x();
            Params::dnn::spatial::_y[neuron_index] = neuron_position.y();
            Params::dnn::spatial::_z[neuron_index] = neuron_position.z();
#if defined(INPUT_VISUALIZE_GROUP_OFFSETS)
            Params::visualisation::_x[neuron_index] = neuron_position.x() + neuron_position.x()*vis_dist_mod*input_group_index;
            Params::visualisation::_y[neuron_index] = neuron_position.y();
            Params::visualisation::_z[neuron_index] = neuron_position.z() + neuron_position.z()*vis_dist_mod*input_group_index;
#else
            Params::visualisation::_x[neuron_index] = neuron_position.x();
            Params::visualisation::_y[neuron_index] = neuron_position.y();
            Params::visualisation::_z[neuron_index] = neuron_position.z();
#endif
            ++neuron_index;
        }
    }


#if defined(MINRAD05)
    btScalar minimumRadius = 0.5;
#else
    btScalar minimumRadius = 0.5;
#endif

#if defined(MAXRAD10)
    btScalar maximumRadius = 1.0;
#else
    btScalar maximumRadius = 1.0;
#endif


#if defined(CIRCLE_HIDDEN)
    {
        size_t nrOfRings = Params::dnn::nb_neuron_batches;
        size_t neuronsPerRing = Params::dnn::nb_neurons_per_batch;
        btVector3 orientation (1,0,0);
        btScalar angle = (2*M_PI)/neuronsPerRing;
        btScalar radiusIncrement = (maximumRadius - minimumRadius) / btScalar(nrOfRings - 1);

        dbg::out(dbg::info, "neuronpos") << "Rings: " << nrOfRings <<
                " neurons per ring: " << neuronsPerRing << std::endl;

        for(size_t i=0; i<neuronsPerRing; i++){
            btScalar currentRadius = minimumRadius;
            for(size_t j=0; j < nrOfRings; ++j){
                setPos(orientation.getX()*currentRadius,
                        orientation.getZ()*currentRadius,
                        orientation.getX()*currentRadius*1.5,
                        orientation.getZ()*currentRadius*1.5,
                        neuron_index);
                currentRadius += radiusIncrement;
            }
            orientation = orientation.rotate(btVector3(0,1,0), angle);
        }
    }
#elif defined(GRID_HIDDEN)
    {
        size_t nb_of_columns = Params::dnn::nb_neuron_batches;
        size_t nb_of_rows = Params::dnn::nb_neurons_per_batch;
        dbg::out(dbg::info, "neuronpos") << "Rows: " << nb_of_rows << " columns: " << nb_of_columns << std::endl;

        float x_pos_current = 0.0f;
        float x_pos_min = -1.0f;
        float x_pos_max = 1.0f;
        float x_pos_increment = 0.0f;
        if(nb_of_columns > 1){
            x_pos_increment = (x_pos_max - x_pos_min) / float(nb_of_columns - 1);
        } else if(nb_of_columns == 1){
            x_pos_min = 0.0f;
            x_pos_max = 0.0f;
        } else {
            dbg::sentinel(DBG_HERE);
        }
        dbg::out(dbg::info, "neuronpos") << "X min: " << x_pos_min << " max: " << x_pos_max << " incr: " << x_pos_increment << std::endl;

        float z_pos_current = 0.0f;
        float z_pos_min = -1.0f;
        float z_pos_max = 1.0f;
        float z_pos_increment = 0.0f;
        if(nb_of_rows > 1){
            z_pos_increment = (z_pos_max - z_pos_min) / float(nb_of_rows - 1);
        } else if(nb_of_rows == 1){
            z_pos_min = 0.0f;
            z_pos_max = 0.0f;
        } else {
            dbg::sentinel(DBG_HERE);
        }
        dbg::out(dbg::info, "neuronpos") << "Z min: " << z_pos_min << " max: " << z_pos_max << " incr: " << z_pos_increment << std::endl;

        x_pos_current = x_pos_min;
        for(size_t column =0; column < nb_of_columns; ++column){
            z_pos_current = z_pos_min;
            for(size_t row =0; row < nb_of_rows; ++row){
                setPos(x_pos_current, z_pos_current, neuron_index);
                z_pos_current += z_pos_increment;
            }
            x_pos_current += x_pos_increment;
        }
    }
#else
#error "No hidden configuration defined!"
#endif


    //Set the output layer
    static const unsigned int nb_of_output_positions = 6;
    static const unsigned int nb_of_output_groups = 3;

#if defined(CIRCULAR_OUTPUTS)
    btScalar angle = (M_PI*2)/nb_of_output_positions;
    for(size_t output_group = 0; output_group<nb_of_output_groups; ++output_group){
        btVector3 actuator_orientation (1,0,0);
        for(size_t i=0; i<6; i++){
            btVector3 actuator_position = actuator_orientation + btVector3(0,-1,0);
            Params::dnn::spatial::_x[neuron_index] = actuator_position.x();
            Params::dnn::spatial::_y[neuron_index] = actuator_position.y();
            Params::dnn::spatial::_z[neuron_index] = actuator_position.z();
            Params::visualisation::_x[neuron_index] = actuator_position.x()+ actuator_position.x()*vis_dist_mod*output_group;
            Params::visualisation::_y[neuron_index] = actuator_position.y();
            Params::visualisation::_z[neuron_index] = actuator_position.z()+ actuator_position.z()*vis_dist_mod*output_group;
            actuator_orientation = actuator_orientation.rotate(btVector3(0,1,0), angle);
            ++neuron_index;
        }
    }
#elif defined(Z_LINE_OUTPUTS)
    static const btScalar z_output_min = -1.0;
    static const btScalar z_output_max = 1.0;
    const btScalar z_output_increment = (z_output_max - z_output_min)/btScalar(nb_of_output_positions-1);
    static const btScalar x_output_min = -1.0;
    static const btScalar x_output_max = 1.0;
    const btScalar x_output_increment = (x_output_max - x_output_min)/btScalar(nb_of_output_groups-1);

    for(size_t output_group = 0; output_group<nb_of_output_groups; ++output_group){
        for(size_t i=0; i<nb_of_output_positions; i++){
            Params::dnn::spatial::_x[neuron_index] = x_output_min + (x_output_increment*output_group);
            Params::dnn::spatial::_y[neuron_index] = -1;
            Params::dnn::spatial::_z[neuron_index] = z_output_min + (z_output_increment*i);
            Params::visualisation::_x[neuron_index] = x_output_min + (x_output_increment*output_group);
            Params::visualisation::_y[neuron_index] = -1;
            Params::visualisation::_z[neuron_index] = z_output_min + (z_output_increment*i);
            ++neuron_index;
        }
    }
#else
#error "No output configuration defined!"
#endif

    delete spider;

    Params::init = true;
}

void exit(){
    Params::simulator->exitPhysics();
    delete Params::simulator;
    delete[] Params::dnn::spatial::_x;
    delete[] Params::dnn::spatial::_y;
    delete[] Params::dnn::spatial::_z;
    delete[] Params::visualisation::_x;
    delete[] Params::visualisation::_y;
    delete[] Params::visualisation::_z;
    delete[] Params::dnn::spatial::_layers;
    delete[] Params::ParamsHnn::multi_spatial::_connected;
    delete[] Params::ParamsHnn::multi_spatial::_planes;
    if(Params::nodeConnectionMap) delete Params::nodeConnectionMap;
    if(Params::neuronMap) delete Params::neuronMap;
    Params::nodeConnectionMap = 0;
    Params::neuronMap = 0;
}


void master_init(){
    dbg::trace trace("init", DBG_HERE);
    PRINT_PARAMS;

    //Only the master will dump
    joint_init();
#ifdef JHDEBUG
    Params::pop::checkpoint_period = 1;
#else
    Params::pop::checkpoint_period = (Params::pop::nb_gen - 1)/Params::pop::nr_of_dumps;
#endif
    if(options::map.count("load")){
        Params::pop::dump_period = -1; //Prevents the creation of a useless result directory
    } else {
        Params::pop::dump_period = Params::pop::nb_gen + 1; //Don't dump, but do write the modularity file
    }
}

void slave_init(){
    dbg::trace trace("init", DBG_HERE);
    //Initialize the simulator
    joint_init();

    for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i){
        for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
        {
            Params::simulator->addNeuronPos(Params::visualisation::x(k), Params::visualisation::y(k), Params::visualisation::z(k));
        }
    }


    //Initialize the maps
    Params::nodeConnectionMap = new map_t();
    Params::neuronMap = new map_t();

    //Create a temporary dnn to get connection indices.
    ctrnn_nn_t tempDnn(Params::dnn::nb_inputs, Params::dnn::nb_of_hidden, Params::dnn::nb_outputs);

    //Set map for the inputs
    for(uint firstLayerNeuron=0; firstLayerNeuron<Params::dnn::nb_inputs; firstLayerNeuron++){
        for(uint secondLayerNeuron=0; secondLayerNeuron<Params::dnn::nb_of_hidden; secondLayerNeuron++){
            std::string key = inputToHiddenKey(firstLayerNeuron, secondLayerNeuron);
//            std::ostringstream stream;
//            stream << "i" << firstLayerNeuron << '_' << secondLayerNeuron;
            dbg::out(dbg::info, "con_map") << key << " " << tempDnn.getConnectionIndexInput(firstLayerNeuron, secondLayerNeuron) << std::endl;
            (*Params::nodeConnectionMap)[key] = tempDnn.getConnectionIndexInput(firstLayerNeuron, secondLayerNeuron);
        }
    }

    //Set map for hidden and outputs nodes (there is no difference here between hidden and outputs nodes)
    for(uint firstLayerNeuron=0; firstLayerNeuron < Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; firstLayerNeuron++){
        for(uint secondLayerNeuron=0; secondLayerNeuron < Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; secondLayerNeuron++){
            std::string key = hiddenToHiddenKey(firstLayerNeuron, secondLayerNeuron);

//            std::ostringstream stream;
//            if(firstLayerNeuron >= Params::dnn::nb_of_hidden){
//                stream << 'o' << (firstLayerNeuron - Params::dnn::nb_of_hidden);
//            } else {
//                stream << firstLayerNeuron;
//            }
//            stream << '_';
//
//            if(secondLayerNeuron >= Params::dnn::nb_of_hidden){
//                stream << 'o' << (secondLayerNeuron - Params::dnn::nb_of_hidden);
//            } else {
//                stream << secondLayerNeuron;
//            }

            dbg::out(dbg::info, "con_map") << key << " " << tempDnn.getConnectionIndexHidden(firstLayerNeuron, secondLayerNeuron) << std::endl;
            (*Params::nodeConnectionMap)[key] = tempDnn.getConnectionIndexHidden(firstLayerNeuron, secondLayerNeuron);
        }
    }

    //Set map from neuron id to neuron index for inputs
    for(uint firstLayerNeuron=0; firstLayerNeuron < Params::dnn::nb_inputs; firstLayerNeuron++){
        std::ostringstream stream;
        //This code WILL fail if the network has only a single layer (E.G. inputs are also outputs)
        //Should not happen in any real experiment though
        stream << 'i' << firstLayerNeuron;
        dbg::out(dbg::info, "neur_map") << "Map input neuron: "<< stream.str() << " to: " << tempDnn.getNeuronIndex(0, firstLayerNeuron) << std::endl;
        (*Params::neuronMap)[stream.str()] = tempDnn.getNeuronIndex(0, firstLayerNeuron);
    }

    //Set hidden neurons
    for(uint firstLayerNeuron=0; firstLayerNeuron < Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; firstLayerNeuron++){
        std::ostringstream stream;
        //This code WILL fail if the network has only a single layer (E.G. inputs are also outputs)
        //Should not happen in any real experiment though
        if(firstLayerNeuron >= Params::dnn::nb_of_hidden){
            stream << 'o' << (firstLayerNeuron - Params::dnn::nb_of_hidden);
        } else {
            stream << firstLayerNeuron;
        }
        dbg::out(dbg::info, "neur_map") << "Map hidden neuron: "<< stream.str() << " to: " << tempDnn.getNeuronIndex(1, firstLayerNeuron) << std::endl;
        (*Params::neuronMap)[stream.str()] = tempDnn.getNeuronIndex(1, firstLayerNeuron);
    }
}


float inverseDistance(float distance, float max_distance){
    if(max_distance == 0){
        return 0.0f;
    }
    float scaled_distance = distance/max_distance;
    return std::max(1.0f - scaled_distance, 0.0f);
}


float getFoodDistFit(sim_t* simulator){
    btVector3 previous_food_pos = simulator->getPreviousFoodPos();
    btVector3 next_food_pos = simulator->getFoodPos();
    btVector3 robot_pos = simulator->getIndivPos();

    float inverse_food_dist = inverseDistance(robot_pos.distance(next_food_pos), previous_food_pos.distance(next_food_pos));

    return inverse_food_dist;
}

float calculateFoodFitness(sim_t* simulator){
    float inverse_food_dist = getFoodDistFit(simulator);
    float fitness = simulator->getFoodCollected() + inverse_food_dist;

    //debug output
    dbg::out(dbg::info, "fitness")
    << " food collected: " << Params::simulator->getFoodCollected()
    << " distance to closest food: " << inverse_food_dist
    << " total: " << fitness << std::endl;

    return fitness;
}

float calculatePredatorFitness(sim_t* simulator){
    btVector3 current_predator_pos = simulator->getPredatorPos();
    btVector3 robot_pos = simulator->getIndivPos();
    float inverse_predator_dist = inverseDistance(robot_pos.distance(current_predator_pos), simulator->getPredatorDist());
    float fitness = -(simulator->getCapturedByPredator() + inverse_predator_dist);

    //debug output
    dbg::out(dbg::info, "fitness")
    << " caught: " << Params::simulator->getCapturedByPredator()
    << " closeness to predator: " << inverse_predator_dist
    << " total: " << fitness << std::endl;

    return fitness;
}

/**
 * The fitness for the robot moving towards a particular target from the initial position (0, 1, 0).
 *
 * The fitness will be the 1 - (dist_robot_to_target / dist_initial_to_target).
 * The fitness can be negative.
 *
 * When the target is equal to the initial position (or closer than 0.001 to the initial position)
 * the fitness will instead be 1 - dist_robot_to_target.
 */
float calculateTargetFitness(sim_t* simulator, btVector3& target){
    btVector3 indiv_position = simulator->getIndivPos();
    btScalar totalDistance = btVector3(0.0, 1.0, 0.0).distance(target);
    if(totalDistance < 0.001){
        totalDistance = 1.0;
    }
    btScalar normalizedDistance = indiv_position.distance(target)/totalDistance;
    btScalar fitness = 1-normalizedDistance;

    //debug output
    dbg::out(dbg::info, "tracker") << "Target: " << fitness << std::endl;
    return fitness;
}

float calculateTargetOrientationFitness(sim_t* simulator, btVector3& targetHeading, btScalar targetComponent){
    btVector3 indiv_heading = simulator->getIndivHeading();
    btScalar angle = indiv_heading.angle(targetHeading);
    btScalar normalizedAngle = angle/M_PI;
    btScalar headingFitness = 1-normalizedAngle;

    btScalar fitness = (targetComponent + headingFitness)/2;
    //debug output
    dbg::out(dbg::info, "fitness") << "Heading: " << fitness << std::endl;
    return fitness;
}

bool upright(sim_t* simulator){
    btVector3 up(0, 1, 0);
    btVector3 currentUp = Params::simulator->getIndividual()->getUp();
    btScalar angleToUp = currentUp.angle(up);
    return angleToUp < (M_PI/3);
}


SFERES_FITNESS(FitRobot, sferes::fit::Fitness) {
public:
    FitRobot() : _postFix("") {

#if defined(MAPELITEINOVDIV)
        _closest_dist.resize(Params::innov::categories);
#endif
        //nix
    }
    ~FitRobot(){
        //nix
    }

    template<typename Indiv>
    float dist(const Indiv& o) const {
        dbg::trace trace("fit", DBG_HERE);
#if defined(EUCLIDIAN)
        return (o.fit()._behavior.dist(_behavior));
#elif defined(DIVHAMMING)
        return hamming_dist<Params::fitness::nb_bits>(_behavior,o.fit()._behavior);
#else
        return 0;
#endif
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


    template<typename Indiv>
    void eval(Indiv& ind) {
        dbg::trace trace("fit", DBG_HERE);

        using namespace io;


        _food_fitness = 0.0f;
        _predator_fitness = 0.0f;
        this->_value = 0;

        _eval_clock.resetAndStart();

#if !defined(TESTFIT)
        // Test fit simply give a random fitness to each individual
        // The world seed would cause every individual to have the same fitness
        // Note: this also kills determinism in parallel scenarios
        misc::pushRand();
        misc::seed(ind.gen().get_world_seed());
        dbg::out(dbg::info, "seed") << "World seed received: " << ind.gen().get_world_seed() << std::endl;
#endif

        btScalar test;
        //		std::cout <<"Type: " << typeid(test).name() << " size of: " << sizeof(test) << std::endl;

        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::graph_t graph_t;
        typedef typename Indiv::cppn_graph_t cppn_graph_t;
        typedef typename Indiv::nn_t::neuron_t neuron_t;
        typedef typename Indiv::nn_t::neuron_t::af_t::params_t neu_bais_t;
        typedef typename Indiv::nn_t::weight_t weight_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;


        //Clear data
        this->_objs.clear();

#if defined(DIVHAMMING)
        _behavior.clear();
#endif
        dbg::out(dbg::info, "fit") << "Creating neural network" << std::endl;
        //Create the neural network
        ind.nn().init();

        //Get graphs and networks
        dbg::out(dbg::info, "fit") << "Simplifying graph" << std::endl;
        nn_t nnsimp = ind.nn().simplified_nn();
        graph_t gsimp = nnsimp.get_graph();
        ctrnn_nn_t* dnn = buildCtrnn(nnsimp);

        //Calculate metrics
        //Calculate within hidden layer modularity
        _hidden_mod = calculateHiddenModularity(gsimp);
//        std::cout << "Hidden layer modularity: " << calculateHiddenModularity(gsimp) << std::endl;



#if defined(VISUALIZE)
        setNeuronVisData(ind);
#endif


        _sim_clock.resetAndStart();



#if defined(REACHING)
        Params::simulator->setFoodHeight(2.0);
#endif

#if defined(DISTANCE)
        Params::simulator->reset();
        mod_robot::Spider* spider = init_spider();
        dnn->reset();
        spider->setNN(dnn);
        Params::simulator->setIndividual(spider);

        //Run simulation
        STEP_FUNCTION(1000);

        this->_value = Params::simulator->getIndivPos().distance(btVector3(0,0,0));
#endif

#if defined(RANDOMFOOD)
        std::vector<double> fitness;
        Params::simulator->setReplaceFood();

        for(int i = 0; i < 5; i++){
            //Setup the run for this trial
            Params::simulator->reset();
            mod_robot::Spider* spider = init_spider();
            dnn->reset();
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addFood();


            //Run simulation
            STEP_FUNCTION(Params::sim::steps);

            //Determine fitness for this trial
            fitness.push_back(calculateFoodFitness(Params::simulator));
        }
        _food_fitness = compare::average(fitness);
        this->_value = _food_fitness;
        dbg::out(dbg::info, "fitness") << "Final fitness: " << this->_value << std::endl;
#endif

#if defined(PREDATOR)
        //IN_FUNCTION_DOUBLE_DEFINE_GUARD(eval_define, EVAL_DOUBLE_DEFINE_MESSAGE)

#if defined(FOODDISTFIT)
        class FitTracker: public mod_robot::Tracker {
        public:
            void track()
            {
                sum_food_dist+=getFoodDistFit(_simulator);
                ++count;
            }

            void reset()
            {
                sum_food_dist = 0.0;
                count = 0;
            }

            float average(){
                return sum_food_dist/(float)count;
            }

            float sum_food_dist;
            int count;
        };

        FitTracker* tracker = new FitTracker();
        Params::simulator->setTracker(tracker);
#endif //FOODDISTFIT

        std::vector<double> food_fitness;
        std::vector<double> predator_fitness;

        Params::simulator->setReplaceFood();
        Params::simulator->setReplacePredator();

#if defined(SPAWNFRONT)
        Params::simulator->setSpawnPredatorFront();
#endif

        for(int i = 0; i < 5; i++){
            //Setup trial
#if defined(FOODDISTFIT)
            tracker->reset();
#endif //FOODDISTFIT
            Params::simulator->reset();
            dnn->reset();
            mod_robot::Spider* spider = init_spider();
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addFood();


            //Run trial part 1
            STEP_FUNCTION(Params::sim::steps);

            //Add a predator
            Params::simulator->addPredator();

            //Run trial part 2
            STEP_FUNCTION(Params::sim::steps);

            //Determine fitness
#if defined(FOODDISTFIT)
            food_fitness.push_back(Params::simulator->getFoodCollected() + tracker->average());
            dbg::out(dbg::info, "fitness")
            << " food collected: " << Params::simulator->getFoodCollected()
            << " average food closeness: " << tracker->average()
            << " total: " << Params::simulator->getFoodCollected() + tracker->average() << std::endl;
#else //FOODDISTFIT
            food_fitness.push_back(calculateFoodFitness(Params::simulator));
#endif //FOODDISTFIT

#if defined(PREDDIST)
            predator_fitness.push_back(calculatePredatorFitness(Params::simulator));
#else //PREDDIST
            predator_fitness.push_back(-Params::simulator->getCapturedByPredator());
#endif //PREDDIST
        }
        _food_fitness = compare::average(food_fitness);
        _predator_fitness = compare::average(predator_fitness);
        this->_value = _food_fitness + _predator_fitness*4;

        dbg::out(dbg::info, "fitness") << "Final fitness: " << this->_value << std::endl;

#if defined(FOODDISTFIT)
        delete tracker;
#endif //FOODDISTFIT
#endif //PREDATOR


#if defined(TRACKER)
        //Define tracker
        class TargetTracker: public mod_robot::Tracker<sim_t> {
        public:
            enum direction{
                LEFT = 0,
                RIGHT = 1
            };

            TargetTracker(btVector3 target):target(target){
                reset();
                targetDirection = LEFT;
            }

            TargetTracker(btVector3 target, direction targetDirection):target(target), targetDirection(targetDirection){
                reset();
            }

            void track()
            {
                float fitness = calculateTargetFitness(Params::simulator, target);
                if(fitness > maxFitness){
                    maxFitness = fitness;
                }
                sum_food_dist+=fitness;
                btVector3 indiv_heading = Params::simulator->getIndivHeading();
                indiv_heading.normalize();
                indiv_heading.setY(0);
                btScalar originalLength = indiv_heading.length();
                if(originalLength > 0.0){

                    //Check that the robot is still upright
                    btVector3 up(0, 1, 0);
                    btVector3 currentUp = Params::simulator->getIndividual()->getUp();
                    btScalar angleToUp = currentUp.angle(up);

                    indiv_heading.normalize();

                    btScalar angle = indiv_heading.angle(prev_heading);
                    //                    angle = angle*originalLength; //Prevent exploits by tipping over
                    direction currentDirection;
                    if(indiv_heading.cross(prev_heading).y() > 0.0){
                        currentDirection = LEFT;
                    } else {
                        currentDirection = RIGHT;
                    }

                    if(currentDirection == targetDirection){
                        //Check that the robot is still upright
                        if(upright(Params::simulator)){
                            dbg::out(dbg::info, "tracker") << "Angle added: " << angle << std::endl;
                            totalAngle+=angle;
                        } else {
                            dbg::out(dbg::info, "tracker") << "Robot is not upright, angle to up: " << angleToUp << std::endl;
                        }
                    } else {
                        dbg::out(dbg::info, "tracker") << "Angle subtracted: " << angle << std::endl;
                        totalAngle-=angle;
                    }
                    prev_heading = indiv_heading;
                }

                ++count;
            }

            void reset()
            {
                sum_food_dist = 0.0;
                count = 0;
                prev_heading.setValue(1.0, 0.0, 0.0);
                totalAngle = 0.0;
                maxFitness = 0.0;
            }

            float average(){
                return sum_food_dist/(float)count;
            }

            float getMaxFitness(){
                return maxFitness;
            }

            btScalar averageAngle(){
                return totalAngle/(btScalar)count;
            }

            float sum_food_dist;
            int count;
            btVector3 target;
            btVector3 prev_heading;
            btScalar totalAngle;
            direction targetDirection;
            float maxFitness;
        };

        std::vector<double> fitness;
        robot_t* spider;
        float taskFitness;
        size_t neuron_index = 0;
#endif

#if defined(FORWARD)
        {
            //Move forwards
            dbg::out(dbg::info, "fitness") << "Move forwards..." << std::endl;
            Params::simulator->reset();
            spider = init_spider();
            dnn->reset();

#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i < 5);
            }
#endif
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            STEP_FUNCTION(Params::sim::move_steps);
            taskFitness = Params::simulator->getIndivPos().x()/12.5;
            dbg::out(dbg::info, "fitness") << "Move forwards fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
        }
#endif

#if defined(BACKWARD)
        {
            //Move backwards
            dbg::out(dbg::info, "fitness") << "Move backwards..." << std::endl;
            Params::simulator->reset();
            spider = init_spider();
            dnn->reset();

#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i>=5 && i <10);
            }
#endif

            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            STEP_FUNCTION(Params::sim::move_steps);
            taskFitness = -Params::simulator->getIndivPos().x()/12.5;
            dbg::out(dbg::info, "fitness") << "Move backwards fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
        }
#endif

#if defined(TURNLEFT)
        {
            //Turn left on the spot
            dbg::out(dbg::info, "fitness") << "Turn left..." << std::endl;
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,1,0), TargetTracker::LEFT);
            Params::simulator->setTracker(tracker);
            spider = init_spider();
            dnn->reset();
#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i>=10 && i <15);
            }
#endif
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            STEP_FUNCTION(Params::sim::turn_steps);
            float positionPenalty = 1 - std::min(tracker->average()+1.0, 1.0);
            taskFitness = (tracker->averageAngle()*25 - positionPenalty);
            dbg::out(dbg::info, "fitness") << "Turn fitness: " << tracker->averageAngle()*10  << std::endl;
            dbg::out(dbg::info, "fitness") << "Position penalty: " << positionPenalty  << std::endl;
            dbg::out(dbg::info, "fitness") << "Turn left fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            Params::simulator->deleteTracker();
            ++neuron_index;
        }
#endif

#if defined(TURNRIGHT)
        {
            //Turn right on the spot
            dbg::out(dbg::info, "fitness") << "Turn right..." << std::endl;
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,1,0), TargetTracker::RIGHT);
            Params::simulator->setTracker(tracker);
            spider = init_spider();
            dnn->reset();
#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i>=15 && i <20);
            }
#endif
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            STEP_FUNCTION(Params::sim::turn_steps);
            float positionPenalty = 1 - std::min(tracker->average()+1.0, 1.0);
            taskFitness = (tracker->averageAngle()*25 - positionPenalty);
            dbg::out(dbg::info, "fitness") << "Turn right fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            Params::simulator->deleteTracker();
            ++neuron_index;
        }
#endif

#if defined(JUMP)
        {
            //Get high
            dbg::out(dbg::info, "fitness") << "Get high..." << std::endl;
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,2,0));
            Params::simulator->setTracker(tracker);
            spider = init_spider();
            dnn->reset();
#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i>=20 && i <25);
            }
#endif
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));

            // Jump phase
            STEP_FUNCTION(Params::sim::jump_steps);
            float jumpFitness = tracker->getMaxFitness();

            // Landing phase
            float landFitness = 0;
            STEP_FUNCTION(Params::sim::jump_steps);
            if(jumpFitness > 0.5 && upright(Params::simulator)){
                btVector3 target(0,1,0);
                landFitness = calculateTargetFitness(Params::simulator, target);
            }

            float totalFitness = (jumpFitness + landFitness)/2;

            dbg::out(dbg::info, "fitness") << "Jump fitness: " << jumpFitness <<
                    " land fitness: " << landFitness <<
                    " total fitness: " << totalFitness << std::endl;
            fitness.push_back(totalFitness);
            Params::simulator->deleteTracker();
            ++neuron_index;
        }
#endif

#if defined(GETDOWN)
        {
            //Get low
            dbg::out(dbg::info, "fitness") << "Get low..." << std::endl;
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,0.3f,0));
            Params::simulator->setTracker(tracker);
            spider = init_spider();
            dnn->reset();
#if defined(ONEMODACTIVE)
            for(size_t i=0; i<30; ++i){
                dnn->getHiddenNeuron(i)->setActive(i>=25 && i <30);
            }
#endif
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            STEP_FUNCTION(Params::sim::get_down_steps);
            fitness.push_back(tracker->average());
            Params::simulator->deleteTracker();
            ++neuron_index;
        }
#endif

#if defined(TRACKER)
        this->_value = compare::average(fitness);
        dbg::out(dbg::info, "fitness") << "Final fitness: " << this->_value << std::endl;
#endif

#if defined(SIXTASKS)
        _innov_categories.clear();
        std::vector<float> fitness_combination;
        for(size_t i=0; i<Params::innov::categories; ++i){
            fitness_combination.clear();
            for(size_t j=0; j<fitness.size(); ++j){
                if(int(i/(1<<j))%2 == 0){
                    fitness_combination.push_back(fitness[j]);
                }
            }

#if defined(MRMULT)
            //Do not allow negative values, they have an odd effect when multiplied.
            for(size_t j=0; j<fitness_combination.size(); ++j){
                fitness_combination[j] = fmax(fitness_combination[j], 0.0);
            }
            float local_fitness = compare::mult(fitness_combination);
#else
            float local_fitness = compare::average(fitness_combination);
#endif
            dbg::out(dbg::info, "fitness") << "obj: " << i << " fitness " << local_fitness << std::endl;
            _innov_categories.push_back(local_fitness);
        }
        dbg::assertion(DBG_ASSERTION(_innov_categories.size() == Params::innov::categories));
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::innov::obj_index));
        this->_value = _innov_categories.front();
#endif


#ifdef DUMFIT
        this->_value = 0;
        dbg::out(dbg::info, "fitness") << "Dummy fitness: 0" << std::endl;
#endif

#if defined(TESTFIT)
        dbg::out(dbg::info, "fitness") << "Fitness: " << std::endl;
        for(size_t i=0; i<Params::innov::categories; ++i){
            this->_innov_categories.push_back(misc::rand<float>());
            dbg::out(dbg::info, "fitness") <<  this->_innov_categories.back() << std::endl;
        }
        dbg::out(dbg::info, "fitness") << std::endl;

        dbg::assertion(DBG_ASSERTION(_innov_categories.size() == Params::innov::categories));
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::innov::obj_index));
        this->_value = _innov_categories.front();
#endif






#ifdef DIVREACT
        dbg::out(dbg::info, "eval_behavior") << "##### STARTING BEHAVIOR #####" << _behavior << std::endl;
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            dnn->clear();
            Params::simulator->reaction(dnn, _behavior, i, 5);
        }
        dbg::out(dbg::info, "eval_behavior") << "behavior "<<":\n" << _behavior << std::endl;
#endif

#ifdef DIVMINAVG
        dbg::out(dbg::info, "eval_behavior") << "##### STARTING BEHAVIOR #####" << _behavior << std::endl;

        std::vector<double> count;
        count.assign(Params::fitness::nb_bits, 0);
        static const size_t time_steps = 5;

        dbg::out(dbg::info, "eval_behavior") << "Bits: " << Params::fitness::nb_bits
                << " inputs: " << Params::dnn::nb_inputs
                << " time-steps: " << time_steps << std::endl;
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            dbg::out(dbg::info, "eval_behavior") << "Input " << i << std::endl;
            dnn->clear();
            Params::simulator->reaction(dnn, _behavior, i, time_steps);
//            dbg::out(dbg::info, "eval_behavior") << "Reaction ";
            for(size_t k=1; k <= time_steps; ++k){
                for(size_t j=0; j < Params::fitness::nb_bits; ++j){
//                    dbg::out(dbg::info, "eval_behavior") << _behavior[_behavior.size() - k][j];
                    if(_behavior[_behavior.size() - k][j]){
                        count[j] += 1.0;
                    }
                }
            }
        }

        dbg::out(dbg::info, "eval_behavior") << "count "<<":\n" << count << std::endl;
        dbg::out(dbg::info, "eval_behavior") << "original behavior "<<":\n" << _behavior << std::endl;

        std::bitset<Params::fitness::nb_bits> median_response;
        for(size_t j=0; j<Params::fitness::nb_bits; ++j){
            median_response[j] = (count[j]/_behavior.size() > 0.5);
        }

        for(size_t i=0; i<_behavior.size(); ++i){
            _behavior[i] ^= median_response;
        }
        dbg::out(dbg::info, "eval_behavior") << "median behavior "<<":\n" << median_response << std::endl;
        dbg::out(dbg::info, "eval_behavior") << "adjusted behavior "<<":\n" << _behavior << std::endl;
#endif

        _sim_clock.stop();

        delete dnn;

        //Calculate modularity
        _mod = calculateModularity(gsimp);

        cppn_graph_t cppn_graph = ind.cppn().get_graph();
        _cppn_mod = calculateModularity(cppn_graph);


//        if (boost::num_edges(gsimp) == 0 || boost::num_vertices(gsimp) == 0)
//            _mod = 0;
//        else
//            _mod = mod::modularity(gsimp);



        //Set number of connections, neurons and length
        this->_nb_conns = boost::num_edges(gsimp);
        this->_nb_nodes = boost::num_vertices(gsimp);
        _length = ind.nn().compute_length();

        //Set time
        _nn_time = Params::simulator->getNNTime();
        _physics_time = Params::simulator->getPhysicsTime();
        _reset_time = Params::simulator->getResetTime();


        /**********************
         ***** DESCRIPTORS ****
         **********************/
#if defined(MAPELITE)
      size_t desc_index = 0;

#if defined(DESC_HMOD)
      dbg::out(dbg::info, "desc") << "Behavioral descriptor " << desc_index << " is _hidden_mod: " << _hidden_mod << std::endl;
      _behavioral_descriptors[desc_index++] = _hidden_mod;
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

        //Push back the primary objective objective
#if defined(PREDOBJ)
        dbg::out(dbg::info, "obj") << "Objective pushed: _food_fitness" << std::endl;
        this->_objs.push_back(_food_fitness);
#else
        dbg::out(dbg::info, "obj") << "Objective pushed: this->_value" << std::endl;
        this->_objs.push_back(this->_value);
#endif

        /***************************************************
         * CONNECTION COST OBJECTIVES                      *
         * To make sure that pnsga works correctly,        *
         * the CCT objective should always be the second   *
         * objective in the list, and there should only be *
         * at most one connection cost objective.          *
         ***************************************************/

#ifdef NCONNS
        dbg::out(dbg::info, "obj") << "Objective pushed: num_edges" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_edges(gsimp));
#endif

#ifdef NNODES
        dbg::out(dbg::info, "obj") << "Objective pushed: num_vertices" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_vertices(gsimp));
#endif

#ifdef NCNODES
        dbg::out(dbg::info, "obj") << "Objective pushed: num_edges and num_vertices" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_edges(gsimp) - (int)boost::num_vertices(gsimp));
#endif

#ifdef MOD
        dbg::out(dbg::info, "obj") << "Objective pushed: _mod" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_mod);
#endif

#ifdef HMOD
        dbg::out(dbg::info, "obj") << "Objective pushed: _hidden_mod" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_hidden_mod);
#endif

#ifdef CPPNMOD
        dbg::out(dbg::info, "obj") << "Objective pushed: _cppn_mod" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_cppn_mod);
#endif

#ifdef MAX_LENGTH
        dbg::out(dbg::info, "obj") << "Objective pushed: max_length" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-ind.gen().max_length());
#endif

#ifdef LENGTH
        dbg::out(dbg::info, "obj") << "Objective pushed: LENGTH" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-_length);
#endif

#ifdef CPPNLENGTH
        dbg::out(dbg::info, "obj") << "Objective pushed: cppn length" << std::endl;
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-ind.cppn().get_nb_connections());
#endif

        /*************************************
         * OTHER OBJECTIVES                  *
         * These are other objectives        *
         * that are not:                     *
         * - the first performance objective *
         * - the connection cost objective   *
         * - the diversity objective         *
         *************************************/
#if defined(PREDOBJ)
        dbg::out(dbg::info, "obj") << "Objective pushed: _predator_fitness" << std::endl;
        this->_objs.push_back(_predator_fitness);
#endif

        /**************************************
         * DIVERSITY OBJECTIVES               *
         * These are the diversity objectives *
         * There can only be one diversity    *
         * objective active at a time, and    *
         * this objective always has to be    *
         * the last element in the list.      *
         **************************************/
#if defined(DIV) || defined(AGE) || defined(NOVMAP)
        dbg::out(dbg::info, "obj") << "Objective pushed: diversity dummy" << std::endl;
        this->_objs.push_back(0);
#endif



        /*************************************
         ********** VISUALIZATIONS ***********
         *************************************/

        if (this->mode() == sferes::fit::mode::view || this->mode() == sferes::fit::mode::usr1) {
            //Do any calculation here

            //regularity_score = analysis_regularity(ind);
            //			std::cout << "- Skipping regularity analysis: " << std::endl;
            regularity_score = 0;

            if (this->mode() == sferes::fit::mode::view) {
                visualizeNetwork(ind);
            }
        }

#if !defined(TESTFIT)
        misc::popRand();
#endif
        _eval_clock.stop();
    }

    template<typename Indiv>
    void visualizeNetwork(Indiv& ind){
        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;
        using namespace io;

//        static const size_t no_objs = 63;
        static const size_t obj_forward = 62;
        static const size_t obj_backward = 61;
        static const size_t obj_turnleft = 59;
        static const size_t obj_turnright = 55;
        static const size_t obj_jump = 47;
        static const size_t obj_down = 31;

        _setPostfix();
        nn_t nnsimp = ind.nn();
        nnsimp.simplify();

        std::cout << "behavior:\n" << _behavior << std::endl;
        std::cout << "- Performance: " << this->_value << std::endl;

        std::cout << "- Forward   : " << this->cat(obj_forward) << std::endl; // best fitnesss (value)
        std::cout << "- Backward  : " << this->cat(obj_backward) << std::endl;  // best fitnesss (value)
        std::cout << "- Turn left : " << this->cat(obj_turnleft) << std::endl;  // best fitnesss (value)
        std::cout << "- Turn right: " << this->cat(obj_turnright) << std::endl;  // best fitnesss (value)
        std::cout << "- Jump      : " << this->cat(obj_jump) << std::endl;  // best fitnesss (value)
        std::cout << "- Get down  : " << this->cat(obj_down) << std::endl;  // best fitnesss (value)


        std::cout << "- Modularity: " << _mod << std::endl;
        std::cout << "- length: " << _length << std::endl;
        std::cout << "- nb neurons: " << ind.nn().get_nb_neurons() << std::endl;
        std::cout << "- nb conns: " << ind.nn().get_nb_connections() << std::endl;
        std::cout << "- reg: " << regularity_score << std::endl;

        std::ofstream ofs2(("nn"+ _postFix +".dot").c_str());
        ind.nn().write_dot(ofs2);

        std::ofstream ofs3(("nn"+ _postFix +".svg").c_str());
        SvgWriter normal_writer;
        normal_writer.switchAxis(1,2);
        normal_writer.setScale(100);
        normal_writer.setPlotBias();
        normal_writer.setRectangles();
        std::map<vertex_desc_t, gen::spatial::Pos> coords_map = nnsimp.getVisualizationCoordsmap();
        nnsimp.write_svg(ofs3, normal_writer, coords_map);


        //		std::cout << "Calculating optimal neuron placement..." <<std::endl;

        //		ind.nn().compute_optimal_length(0, true, "onp" + _postFix + ".svg");


        std::vector<std::string> mods;
        mod::split(nnsimp.get_graph(), mods);
        std::vector<SvgColor> colors = nnsimp.getColors(mods);
        SvgWriter mod_writer;
        mod_writer.setScale(100);
        mod_writer.switchAxis(1,2);
        mod_writer.setColors(colors);
        mod_writer.setDefaultShape(4);
        mod_writer.setRectangles();
        std::ofstream ofs4(("onp"+ _postFix +".svg").c_str());
        nnsimp.write_svg(ofs4, mod_writer, coords_map);

        mod_writer.addSourceDepth(1);
        mod_writer.addTargetDepth(1);
        std::ofstream ofs5(("hidden"+ _postFix +".svg").c_str());
        nnsimp.write_svg(ofs5, mod_writer, coords_map);

        mod_writer.clearSourceDepth();
        mod_writer.clearTargetDepth();
        mod_writer.addSourceDepth(0);
        mod_writer.addTargetDepth(1);
        std::ofstream ofs6(("input"+ _postFix +".svg").c_str());
        nnsimp.write_svg(ofs6, mod_writer, coords_map);

        mod_writer.clearSourceDepth();
        mod_writer.clearTargetDepth();
        mod_writer.addSourceDepth(1);
        mod_writer.addTargetDepth(2);
        std::ofstream ofs7(("output"+ _postFix +".svg").c_str());
        nnsimp.write_svg(ofs7, mod_writer, coords_map);


#ifdef HNN
        std::ofstream ofs(("cppn" + _postFix + ".dot").c_str());
        ind.write_dot_cppn(ofs);
#endif
        std::cout << "Visualize done" << std::endl;
    }

    template<typename Indiv>
    void runSimulation(Indiv& ind){
        dbg::trace trace("visualize", DBG_HERE);
        srand(ind.gen().get_world_seed());
        typedef typename Indiv::nn_t nn_t;

        //Initialize the neural network
        ind.nn().init();

        //Simplify the network
        nn_t nn = ind.nn().simplified_nn();

        ctrnn_nn_t* dnn = buildCtrnn(nn);
        //Setup trial
        Params::simulator->reset();
        Params::simulator->addItemRandomPosition(new mod_robot::Food());
        dnn->reset();
        robot_t* spider = init_spider();
        spider->setNN(dnn);
        Params::simulator->setIndividual(spider);
        Params::simulator->stepsVisual(-1);
        return;
    }

    template<typename NN>
    ctrnn_nn_t* buildCtrnn(NN& nn){
        dbg::trace trace("visualize", DBG_HERE);

        typedef typename NN::graph_t graph_t;
        typedef typename NN::neuron_t neuron_t;
        typedef typename NN::neuron_t::af_t::params_t neu_bais_t;
        typedef typename NN::weight_t weight_t;
        typedef typename NN::vertex_desc_t vertex_desc_t;

        graph_t gsimp = nn.get_graph();

        ctrnn_nn_t* dnn = new ctrnn_nn_t(Params::dnn::nb_inputs, Params::dnn::nb_of_hidden, Params::dnn::nb_outputs);

#if defined(TESTCTRNN)
        for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
            dnn->getNeuron(i)->setBias(0);
            dnn->getNeuron(i)->setTimeConstant(2);
        }

        for(size_t i=1; i<Params::dnn::nb_inputs; ++i){
            uint start = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            uint end = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*(i+1);
            for(size_t j=start; j<end; ++j){
                std::string key = inputToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
            }
        }

        for(size_t i=1; i<Params::dnn::nb_inputs; ++i){
            uint node = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            for(size_t k=Params::dnn::nb_of_hidden; k<Params::dnn::nb_outputs + Params::dnn::nb_of_hidden; ++k){
                std::string key = hiddenToHiddenKey(node, k);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
            }
        }

        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            std::string key = hiddenToHiddenKey(i, i);
            dnn->setConnection((*Params::nodeConnectionMap)[key], -1.0);
        }
#else



        BGL_FORALL_VERTICES_T(v, gsimp, graph_t){
            neuron_t node = gsimp[v];
            ctrnn_neuron_t* neuron = dnn->getNeuron((*Params::neuronMap)[node.get_id()]);
            neu_bais_t bias = node.get_afparams();

            dbg::out(dbg::info, "map_use") <<
                    "Neuron key: "<< node.get_id() <<
                    " maps to: " << (*Params::neuronMap)[node.get_id()] <<
                    " bias: " << bias.data(0) <<
                    " time-constant: " << bias.data(1) << std::endl;
            neuron->setBias(bias.data(0));
            neuron->setTimeConstant(bias.data(1));
        }

        BGL_FORALL_EDGES_T(e, gsimp, graph_t){
            std::string key = gsimp[source(e, gsimp)].get_id() + "_" + gsimp[target(e, gsimp)].get_id();
            dbg::out(dbg::info, "map_use") <<
                    "Connection key: " << key <<
                    " maps to: " << (*Params::nodeConnectionMap)[key] <<
                    " from " << dnn->getSourceIndex((*Params::nodeConnectionMap)[key]) <<
                    " to " << dnn->getTargetIndex((*Params::nodeConnectionMap)[key]) <<
                    " weight: " << nn::trait<weight_t>::single_value(gsimp[e].get_weight()) << std::endl;
            dbg::assertion(DBG_ASSERTION(nn::trait<weight_t>::single_value(gsimp[e].get_weight()) != 0));
            dnn->setConnection((*Params::nodeConnectionMap)[key], nn::trait<weight_t>::single_value(gsimp[e].get_weight()));
        }
#endif

        return dnn;
    }

    template<typename Indiv>
    void setNeuronVisData(Indiv& ind){
        dbg::trace trace("visualize", DBG_HERE);

        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::graph_t graph_t;
        typedef typename Indiv::nn_t::neuron_t neuron_t;
        typedef typename Indiv::nn_t::neuron_t::af_t::params_t neu_bais_t;
        typedef typename Indiv::nn_t::weight_t weight_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;


        nn_t nnsimp = ind.nn();
        nnsimp.simplify();


        std::vector<std::string> mods;

        //Option 1
        std::vector<bool> removed;
        mod::split(extractHiddenLayer(nnsimp.get_graph(), removed), mods);
        std::vector<SvgColor> temp_colors = getColors(mods);
        std::vector<SvgColor> colors;
        size_t j=0;
        for(size_t i=0; i<removed.size(); ++i){
            if(removed[i]){
                colors.push_back(SvgColor(125, 125, 125));
            } else {
                colors.push_back(temp_colors[j]);
                ++j;
            }
        }
        //End option 1

        //Option 2
//        mod::split(nnsimp.get_graph(), mods);
//        std::vector<SvgColor> colors = nnsimp.getColors(mods);
        //End option 2


        std::map<vertex_desc_t, gen::spatial::Pos> coords_map = nnsimp.getVisualizationCoordsmap();

        size_t i = 0;
        BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t){
            dbg::out(dbg::info, "visualize") << coords_map[v].x() << " "<< coords_map[v].y() << " " << coords_map[v].z() << std::endl;

            neuron_t node = nnsimp.get_graph()[v];
            double r = ((double)colors[i].r)/255.0;
            double g = ((double)colors[i].g)/255.0;
            double b = ((double)colors[i].b)/255.0;

            Params::simulator->addNeuronVisData(coords_map[v].x(), coords_map[v].y(), coords_map[v].z(), opengl_draw::Color(r,g,b), (*Params::neuronMap)[node.get_id()]);
            ++i;
        }



        //        BGL_FORALL_EDGES_T(e, gsimp, graph_t){
        //        	std::string key = gsimp[source(e, gsimp)].get_id() + "_" + gsimp[target(e, gsimp)].get_id();
        //        	dbg::out(dbg::info, "map_use") << "Connection key: " << key
        //        			<< " maps to: " << (*Params::nodeConnectionMap)[key]
        //        			<< " weight: " << nn::trait<weight_t>::single_value(gsimp[e].get_weight()) << std::endl;
        //        	dbg::assertion(DBG_ASSERTION(nn::trait<weight_t>::single_value(gsimp[e].get_weight()) != 0));
        //        	dnn->setConnection((*Params::nodeConnectionMap)[key], nn::trait<weight_t>::single_value(gsimp[e].get_weight()));
        //        }
    }

    template<typename G>
    inline double calculateModularity(const G& g){
        if (boost::num_edges(g) == 0 || boost::num_vertices(g) == 0)
            return 0;
        else
            return mod::modularity(g);
    }

    template<typename G>
    inline double calculateHiddenModularity(const G& g){
        std::vector<bool> __i_dont_care__;
        return calculateModularity(extractHiddenLayer(g, __i_dont_care__));
    }

    template<typename G>
    inline G extractHiddenLayer(const G& g, std::vector<bool>& removed){
        G hiddenLayers = g;
        typedef typename boost::graph_traits<G>::vertex_descriptor vertex_desc_t;
        typedef typename G::vertex_property_type neuron_t;
//        typedef typename
        std::vector<vertex_desc_t> to_remove;

//        std::cout << "Iterating over all neurons" << std::endl;
        BGL_FORALL_VERTICES_T(v, hiddenLayers, G){
//            std::cout << "- dereferencing vertex descriptor " << v << std::endl;
            neuron_t neuron = hiddenLayers[v];
            if(neuron.is_input() || neuron.is_output()){
                to_remove.push_back(v);
                removed.push_back(true);
            } else {
                removed.push_back(false);
            }
        }
//        std::cout << "Removing neurons" << std::endl;
        for (size_t i = 0; i < to_remove.size(); ++i){
//            std::cout << "- removing neuron " << i << std::endl;
            clear_vertex(to_remove[i], hiddenLayers);
            remove_vertex(to_remove[i], hiddenLayers);
        }

        to_remove.clear();
//        std::cout << "Iterating over all neurons, again" << std::endl;
        size_t i = 0;
        BGL_FORALL_VERTICES_T(v, hiddenLayers, G){
            //Skip neurons that are removed
            while(i < removed.size() && removed[i]){
                ++i;
            }
//            std::cout << "- processing vertex descriptor " << v << std::endl;
            if (boost::in_degree(v, hiddenLayers) + boost::out_degree(v, hiddenLayers) == 0){
                to_remove.push_back(v);
                removed[i] = true;
            }
            ++i;
        }
//        std::cout << "Removing neurons" << std::endl;
        for (size_t i = 0; i < to_remove.size(); ++i){
//            std::cout << "- removing neuron " << i << std::endl;
            remove_vertex(to_remove[i], hiddenLayers);
        }

        return hiddenLayers;

//        std::cout << "Calculating modularity" << std::endl;
//        std::cout << "Hidden layer modularity: " << calculate_modularity(hiddenLayers) << std::endl;
    }


    std::vector<SvgColor> getColors(const std::vector<std::string>& mods){
        //Assign color to modular split
        //We probably want to move these elsewhere
        std::vector<SvgColor> colors;
        std::vector<SvgColor> color_presets(8);
        color_presets[0] = (SvgColor(255, 0, 0));
        color_presets[1] = (SvgColor(0, 255, 0));
        color_presets[2] = (SvgColor(0, 0, 255));
        color_presets[3] = (SvgColor(0, 255, 255));
        color_presets[4] = (SvgColor(255, 0, 255));
        color_presets[5] = (SvgColor(255, 255, 0));
        color_presets[6] = (SvgColor(133, 0, 0));
        color_presets[7] = (SvgColor(0, 133, 0));

        std::map<std::string, size_t> moduleMap;

        size_t color_nr = 0;
        for(size_t i=0; i<mods.size(); ++i){
            size_t param_max_split = Params::visualisation::max_modularity_split;
            size_t max_split = std::min(param_max_split, mods[i].size());
            std::string modKey = mods[i];
            if(!moduleMap.count(modKey)){
                moduleMap[modKey] = color_nr;
                ++color_nr;
            }

            colors.push_back(color_presets[moduleMap[modKey]]);
        }

        return colors;
    }





    //Dummy values

    float values(size_t k) const { return this->_value; }

    size_t nb_values() const { return 1; }

    void set_value(float v) { this->_value = v; }

    float mod() const { return _mod; }

    float length() const { return _length; }

    float hidden_mod() const { return _hidden_mod; }

    float cppn_mod() const { return _cppn_mod; }

    int nb_nodes() const { return _nb_nodes; }

    int nb_conns() const { return _nb_conns; }

    void setPostFix(std::string postFix){_postFix = postFix;}

    float get_regularity_score(){ return regularity_score; }

#if defined(MAPELITE)
    const boost::array<float, 2>& desc() const{ return _behavioral_descriptors; }
#endif

    template<typename I>
    float get_original_length(I& indiv){
        std::vector<std::string> weightsVector = get_weights_vector(indiv);
        std::string weightsString = "";
        BOOST_FOREACH(std::string weight, weightsVector){
            weightsString.append(weight);
        }
        return (weightsString.size()+1)*8;
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        dbg::trace trace("fit", DBG_HERE);
        sferes::fit::Fitness<Params,  typename stc::FindExact<FitRobot<Params, Exact>, Exact>::ret>::serialize(ar, version);
        ar & BOOST_SERIALIZATION_NVP(_id);
        ar & BOOST_SERIALIZATION_NVP(_mod);
        ar & BOOST_SERIALIZATION_NVP(_length);
        ar & BOOST_SERIALIZATION_NVP(_nb_nodes);
        ar & BOOST_SERIALIZATION_NVP(_nb_conns);
        ar & BOOST_SERIALIZATION_NVP(_eval_clock);
        ar & BOOST_SERIALIZATION_NVP(_sim_clock);
        ar & BOOST_SERIALIZATION_NVP(_nn_time);
        ar & BOOST_SERIALIZATION_NVP(_physics_time);
        ar & BOOST_SERIALIZATION_NVP(_reset_time);
        ar & BOOST_SERIALIZATION_NVP(_develop_time);
        ar & BOOST_SERIALIZATION_NVP(_behavior);
        ar & BOOST_SERIALIZATION_NVP(_food_fitness);
        ar & BOOST_SERIALIZATION_NVP(_predator_fitness);
#if defined(INNOV_CATEGORIES)
        ar & BOOST_SERIALIZATION_NVP(_innov_categories);
#endif

#if defined(INNOV_DIVERSITY)
        ar & BOOST_SERIALIZATION_NVP(_innov_diversity);
#endif

#if defined(INNOV_CLOSEST_DIST)
        ar & BOOST_SERIALIZATION_NVP(_closest_dist);
#endif

#if defined(HIDDEN_LAYER_MODULARITY)
        ar & BOOST_SERIALIZATION_NVP(_hidden_mod);
#endif

#if defined(CPPN_MODULARITY)
        ar & BOOST_SERIALIZATION_NVP(_cppn_mod);
#endif

#if defined(MAPELITE)
        ar & BOOST_SERIALIZATION_NVP(_behavioral_descriptors);
#endif
    }

    double getEvalTime(){
        return _eval_clock.time();
    }

    double getSimTime(){
        return _sim_clock.time();
    }

    double getNNTime(){
        return _nn_time;
    }

    double getPhysicsTime(){
        return _physics_time;
    }

    double getResetTime(){
        return _reset_time;
    }

    double getDevelopTime(){
        return _develop_time;
    }

    float getFoodFitness(){
        return _food_fitness;
    }

    float getPredatorFitness(){
        return _predator_fitness;
    }

    void setDevelopTime(double time){
        _develop_time = time;
    }

    void setId(size_t id){
        _id = id;

    }

    size_t getId(){
        return _id;
    }

#if defined(INNOV_CATEGORIES)
public:
    std::vector<float> &getInnovCategories(){
        return _innov_categories;
    }

    float getInnovCategory(size_t index){
        dbg::check_bounds(dbg::error, 0, index, _innov_categories.size(), DBG_HERE);
        return _innov_categories[index];
    }

    float cat(size_t index){
        return getInnovCategory(index);
    }
protected:
    std::vector<float> _innov_categories;
#endif

#if defined(INNOV_DIVERSITY)
public:
    void initDiv(){
        if(_innov_diversity.size() != Params::innov::categories){
            _innov_diversity.resize(Params::innov::categories);
        }
    }

    std::vector<float>& getDiv(){
        return _innov_diversity;
    }

    void setDiv(size_t index, float div){
        dbg::check_bounds(dbg::error, 0, index, _innov_diversity.size(), DBG_HERE);
        _innov_diversity[index] = div;
    }

    float div(size_t index){
        dbg::check_bounds(dbg::error, 0, index, _innov_diversity.size(), DBG_HERE);
        return _innov_diversity[index];
    }
protected:
    std::vector<float> _innov_diversity;
#endif

#if defined(INNOV_CLOSEST_DIST)
public:
    void setClosestDist(size_t obj_index, float closest_dist){
        dbg::check_bounds(dbg::error, 0, obj_index, _closest_dist.size(), DBG_HERE);
        _closest_dist[obj_index] = closest_dist;
    }

    float getClosestDist(size_t obj_index){
        dbg::check_bounds(dbg::error, 0, obj_index, _closest_dist.size(), DBG_HERE);
        return _closest_dist[obj_index];
    }
protected:
    std::vector<float> _closest_dist;
#endif

protected:
    float regularity_score;

    std::string _postFix;
    size_t _id;

#if defined(EUCLIDIAN)
    sferes::gen::spatial::Pos _behavior;
#elif defined(DIVHAMMING)
    std::vector< ::std::bitset<Params::fitness::nb_bits> > _behavior;
#else
    std::vector<int> _behavior; //DUMMY
#endif

    float _mod, _length;
    float _hidden_mod;
    float _cppn_mod;
    int _nb_nodes, _nb_conns;

    float _food_fitness;
    float _predator_fitness;

#if defined(MAPELITE)
    boost::array<float, 2> _behavioral_descriptors;
#endif

    //Timing variables
    Clock _eval_clock;
    Clock _sim_clock;
    double _nn_time;
    double _physics_time;
    double _reset_time;
    double _develop_time;

    void _setPostfix(){
        std::ostringstream ss;
        ss << std::setw( 4 ) << std::setfill( '0' ) << _id;
        setPostFix(ss.str());
    }

    template<typename I>
    std::vector<std::string> get_weights_vector(I& indiv){
        std::vector<std::string> result;
        typename I::nn_t::graph_t network = indiv.nn().get_graph();

        BGL_FORALL_VERTICES_T(v, network, typename I::nn_t::graph_t)
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(5) << std::showpos;

            ss << nn::trait<typename I::nn_t::neuron_t::af_t::params_t>::single_value(network[v].get_afparams());
            std::string s = ss.str();
            result.push_back(s);
        }
        BGL_FORALL_EDGES_T(e, network, typename I::nn_t::graph_t)
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(5) << std::showpos;
            ss << nn::trait<typename I::nn_t::conn_t::weight_t>::single_value(network[e].get_weight());
            std::string s = ss.str();
            result.push_back(s);
        }
        return result;
    }

    long _compressed_difference(std::string weights){
        LZWEncoding codec;
        long originalLength = weights.size()+1;
        uint length = weights.size()+1;

        uchar* data = new uchar[length];
        uchar* encodedResult;
        //uchar* decodedResult;

        weights.copy((char*)data, length);
        data[length-1] = 0;

        encodedResult = codec.Encode(data, length);

        delete[] data;
        delete[] encodedResult;

        return (originalLength*8 - codec.getTotalBits());
    }

    template<typename I>
    float analysis_regularity(I& indiv){
        long totalCompression = 0;
        for(int i=0; i < NUMBER_OF_PERMUTATIONS; i++){
            std::vector<std::string> weightsVector = get_weights_vector(indiv);
            //std::cout << weightsVector.size() <<std::endl;
            std::random_shuffle ( weightsVector.begin(), weightsVector.end() );
            std::string weightsString = "";

            BOOST_FOREACH(std::string weight, weightsVector){

                weightsString.append(weight);
            }

            totalCompression += _compressed_difference(weightsString);
        }

        return float(totalCompression)/NUMBER_OF_PERMUTATIONS;
    }
};

long Params::seed = -1;

sim_t* Params::simulator = 0;
bool Params::init = false;
map_t* Params::nodeConnectionMap = 0;
map_t* Params::neuronMap = 0;


mpi_init_ptr_t Params::mpi::masterInit = 0;
mpi_init_ptr_t Params::mpi::slaveInit = 0;

int Params::pop::checkpoint_period = -1;
int Params::pop::dump_period = -1;

size_t Params::ParamsHnn::dnn::nb_outputs = 0;
size_t Params::dnn::nb_inputs = 0;


SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, layers);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, x);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, y);
SFERES_DECLARE_NON_CONST_ARRAY(Params::dnn::spatial, z);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, x);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, y);
SFERES_DECLARE_NON_CONST_ARRAY(Params::visualisation, z);
SFERES_DECLARE_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, planes);
SFERES_DECLARE_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, neuron_offsets);
SFERES_DECLARE_NON_CONST_ARRAY(Params::ParamsHnn::multi_spatial, connection_offsets);
SFERES_DECLARE_NON_CONST_MATRIX(Params::ParamsHnn::multi_spatial, connected);



int main(int argc, char **argv) {
    std::cout << "Entering main" <<std::endl;
    //Add options
    options::add()("seed", boost::program_options::value<int>(), "program seed");
    options::positionalAdd("seed", 1);
    options::add()("plot-best", boost::program_options::value<bool>()->default_value(true)->implicit_value(true), "Plot the best individual when loading");
    options::add()("recalc-best", boost::program_options::value<bool>()->default_value(false)->implicit_value(true),
            "Always recalculate the best individual upon loading");
    options::add()("print-neuron-groups", boost::program_options::value<bool>()->default_value(false)->implicit_value(true), "Print neuron groups.");

    //Add the option to continue
    cont::init();
    options::parse_and_init(argc, argv, false);
    set_seed();

    std::cout << "Master init" << std::endl;
    master_init();

    std::cout << "Slave init" << std::endl;
    slave_init();

    std::cout << "Retrieving world" << std::endl;
    btDynamicsWorld* world = Params::simulator->getWorld();

    std::cout << "Creating spider" << std::endl;
    robot_t* spider = new robot_t(world);

    std::cout << "Resetting simulator" << std::endl;
    Params::simulator->reset();

    std::cout << "Initializing spider" << std::endl;
    spider = init_spider();

    std::cout << "Creating neural network" << std::endl;
    //Create the neural network
    ctrnn_nn_t* dnn = new ctrnn_nn_t(Params::dnn::nb_inputs, Params::dnn::nb_of_hidden, Params::dnn::nb_outputs);
    for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
        dnn->getNeuron(i)->setBias(0);
        dnn->getNeuron(i)->setTimeConstant(2);
    }
    float transparancy = 0.2;
    bool asp = false;
    bool asg = true;
    if(asp){
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            //uint start = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            //uint end = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*(i+1);
            for(size_t j=0; j<Params::dnn::nb_of_hidden; ++j){
                int imod = i;
                int kmod = j / 5;
                std::string key = inputToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                }
                //            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, 0.5), i, j);
            }
        }
        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            //uint node = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            for(size_t j=0; j<Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; ++j){
                int imod = i / 5;
                int kmod = j / 5;
                if( imod == kmod || j >= Params::dnn::nb_of_hidden){
                    std::string key = hiddenToHiddenKey(i, j);
                    dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                    if(asg){
                        switch(imod){
                        case 0:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                            break;
                        case 1:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                            break;
                        case 2:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                            break;
                        case 3:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                            break;
                        case 4:
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                            break;
                        case 5:
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                            break;
                        }
                    } else {
                        if(j < Params::dnn::nb_of_hidden ){
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        } else if(j >= Params::dnn::nb_of_hidden && j < Params::dnn::nb_of_hidden + 6){
                            Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        } else if(j >= Params::dnn::nb_of_hidden+6 && j < Params::dnn::nb_of_hidden + 12){
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        } else {
                            Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        }
                    }

                    //                if(k >= Params::dnn::nb_of_hidden){
                    //                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, 0.5), i, k);
                    //                } else {
                    //                    Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, 0.5), i, k);
                    //                }
                }
            }
        }
    } else {
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            for(size_t j=0; j<Params::dnn::nb_of_hidden; ++j){
                int imod = i;
                int kmod = j / 5;
                std::string key = inputToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                }
            }
        }
        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            //uint node = (Params::dnn::nb_of_hidden / Params::dnn::nb_inputs)*i;
            for(size_t j=0; j<Params::dnn::nb_of_hidden + Params::dnn::nb_outputs; ++j){
                int imod = i / 5;
                int kmod = j / 5;

                std::string key = hiddenToHiddenKey(i, j);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
                if(asg){
                    switch(imod){
                    case 0:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, transparancy), i, j);
                        break;
                    case 1:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                        break;
                    case 2:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                        break;
                    case 3:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                        break;
                    case 4:
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                        break;
                    case 5:
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .8, transparancy), i, j);
                        break;
                    }
                } else {
                    if(j < Params::dnn::nb_of_hidden ){
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, transparancy), i, j);
                    } else if(j >= Params::dnn::nb_of_hidden && j < Params::dnn::nb_of_hidden + 6){
                        Params::simulator->addConVisData(opengl_draw::Color(.0, .0, .8, transparancy), i, j);
                    } else if(j >= Params::dnn::nb_of_hidden+6 && j < Params::dnn::nb_of_hidden + 12){
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .8, .0, transparancy), i, j);
                    } else {
                        Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .8, transparancy), i, j);
                    }
                }
            }
        }

    }
    dnn->reset();

    std::cout << "Setting visualization colors" << std::endl;
    //Set the visualization of the network
    std::vector<SvgColor> colors;
    size_t j=0;
     //for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
     //   colors.push_back(SvgColor(125, 0, 0));
     //}
    for(size_t i=0; i<dnn->getNumberOfNeurons(); ++i){
        int imod;
        if(asg){
            imod = i % 6;
            if(i > 5 && i <= (5 + 6*5)){
                imod = (i-6) / 5;
            } else if (i > (5 + 6*5)){
                imod = 6;
            }
        } else {
            if(i < 6){
                imod = 0;
            } else if(i < (6*6)){
                imod = 1;
            } else if(i < (6*7)){
                imod = 2;
            } else if(i < (6*8)){
                imod = 3;
            } else {
                imod = 4;
            }
        }
        //double r = ((double)colors[i].r)/255.0;
        //  double g = ((double)colors[i].g)/255.0;
        //  double b = ((double)colors[i].b)/255.0;
        switch(imod){
        case 0:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.8,.0,.0),
                    i);
            break;
        case 1:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.0,.8,.0),
                    i);
            break;
        case 2:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.0,.0,.8),
                    i);
            break;
        case 3:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.8,.8,.0),
                    i);
            break;
        case 4:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.8,.0,.8),
                    i);
            break;
        case 5:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.0,.8,.8),
                    i);
            break;
        case 6:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.4,.4,1.0),
                    i);
            break;
        default:
            Params::simulator->addNeuronVisData(
                    Params::visualisation::x(i),
                    Params::visualisation::y(i),
                    Params::visualisation::z(i),
                    opengl_draw::Color(.8,.8,.8),
                    i);
            break;
        }

        //                if(k >= Params::dnn::nb_of_hidden){
        //                    Params::simulator->addConVisData(opengl_draw::Color(.8, .0, .0, 0.5), i, k);
        //                } else {
        //                    Params::simulator->addConVisData(opengl_draw::Color(.0, .8, .0, 0.5), i, k);
        //                }
    }
     Params::simulator->setOverlayOffset(btVector3(0,0,-5));


     std::cout << "Setting dnn" << std::endl;
     spider->setNN(dnn);

     std::cout << "Setting individual" << std::endl;
     Params::simulator->setIndividual(spider);
     //Params::simulator->addItem(new mod_robot::Control(neuron_index));
     std::cout << "Stepping the simulator" << std::endl;
     Params::simulator->stepsVisual(10000);

     return 0;
}



