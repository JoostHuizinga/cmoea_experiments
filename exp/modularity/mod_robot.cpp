/*
 * mod_robot.cpp
 *
 *  Created on: Sep 26, 2013
 *      Author: Joost Huizinga
 */

// Currently, stat_cmoea tracks the population, so sferes doesn't have to.
#define SFERES_NO_STATE
#define BT_USE_FREEGLUT 0
//#define MRV2
//#define MRS7
//#define BIN100
//#define POP2
//#define GEN10

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

#if defined(MRV2)
#define HIDDEN_LAYER_MODULARITY
#define CPPN_MODULARITY
#define HIERARCHY
#define SERIALIZE_FOOD_PREDATOR
#define SERIALIZE_FIGHT_FLIGHT
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
#define MRMULT      //We are multiplying, rather then taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define CIE         //We are using CMOEA as our EA
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

//Setup 3, do not define the EA for use with regular NSGA-2
#if defined(MRS3)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define SIXTASKS    //We are running the six-task experiment
#define MRMULT      //We are multiplying, rather than taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#endif

//Dummy version
#if defined(MRS4)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define DUMFIT      //We are running the six-task experiment
#define MRMULT      //We are multiplying, rather than taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define CIE         //We are using the combinatorial innovation engine as our EA
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#endif

//Setup 5, use Massively parallel C-MOEA as our EA
#if defined(MRS5)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define SIXTASKS    //We are running the six-task experiment
#define MRMULT      //We are multiplying, rather then taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define MPCMOEA     //We are using massively parallel CMOEA as our EA
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#endif

//Setup 6, use Massively parallel C-MOEA as our EA on a small problem
#if defined(MRS6)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define MRMULT      //We are multiplying, rather then taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#define XTASKS
#endif

//Setup 7, use Massively parallel C-MOEA as our EA on a small problem
#if defined(MRS7)
#define HNN         //We are using HyperNEAT
#define GRID        //We are using a grid setup for the input, output and hidden layers
#define MRMULT      //We are multiplying, rather then taking the average, of the performance in the different objectives
#define SIGH        //We are using [0, 1] Sigmoids in the hidden layer.
#define DIVMINAVG   //We are using average episodic diversity, encouraging individuals to behave differently in different episodes
#define XTASKS
#define FORWARD
#define TURNLEFT
#define TURNRIGHT
#define TURNTO
#define MOVETO
#define STRIKE
#define ATTACK
#define TURNFROM
#define RETREAT
#define MPCMOEA
#define STAT_FIGHT_FLIGHT
#endif

//Setup 5, controls for C-MOEA
#if defined(NPAT)
#define NSGA_PUSH_ALL_TASKS
#endif

#if defined(NPCO)
#define NSGA_PUSH_COMBINED_OBJECTIVE
#endif

// Shorthand for release version, which disables all asserts
#if defined(RLS)
#define NDEBUG
#undef DBG_ENABLED
#endif

// Shorthand for debug version, which enables all asserts
#if defined(DBG)
#define DBG_ENABLED
#undef NDEBUG
#endif

//Shortcut for MAP-Elites with Hidden Modularity on one axis and CPPN Modularity
//of the other axis
#if defined(ME)
#define MAPELITE
#define DESC_HMOD
#define DESC_CPPNMOD
#endif

#if defined(RENDERNETWORK)
#define VISUALIZE
#endif

#if defined(VISUALIZE)
#define STEP_FUNCTION(steps) Params::simulator->stepsVisual(steps);
#else
#define STEP_FUNCTION(steps) Params::simulator->stepsNoVisual(steps);
#endif

#ifdef NOGL
#define NO_OPEN_GL
#endif

#if not defined(PREDOBJ)
#define PREDDIST
#endif

#if defined(PMOD)
#define MOD
#endif

#if defined(GMOD)
#define CPPNMOD
#endif

#if defined(REGACT)
#define REGULATOR
#endif

#if defined(SEVENTASKS)
#define FORWARD
#define BACKWARD
#define TURNLEFT
#define TURNRIGHT
#define JUMP
#define GETDOWN
#define RETINA
#define XTASKS
#endif

#if defined(SIXTASKS)
#define FORWARD
#define BACKWARD
#define TURNLEFT
#define TURNRIGHT
#define JUMP
#define GETDOWN
#define XTASKS
#endif

#if defined(TWOTASKS)
#define FORWARD
#define BACKWARD
#define XTASKS
#endif

#if defined(FORWARD) || defined(BACKWARD) || defined(TURNLEFT) || \
    defined(TURNRIGHT) || defined(JUMP) || defined(GETDOWN)
#define TRACKER
#endif

#if defined(OACT)
#define ONEMODACTIVE
#endif

#if defined(CMOEA)
#define CIE
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

//Enable forced modularity, either for inputs and hidden (FMOD), or just for
//hidden (FMODH)

//Force modules in the hidden layer only, and allow the inputs to be connected
//to every module
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

//Remap CPPN outputs, either One Output Per Module (OMOD),
//Separate Outputs (SO), or standard
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

#if defined(ROBINTER)
#define ROBUST_INTERCEPTS
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

#if (defined(MAPELITEINOVPNSGA) || defined(MAPELITEINOVDIV)) && not defined(CMOEA)
#define CMOEA
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

#if defined(TURNTO)
#define PIE_FOOD_SENSORS
#endif

#if defined(MOVETO)
#define PIE_FOOD_SENSORS
#endif

#if defined(ATTACK)
#define PIE_FOOD_SENSORS
#endif

#if defined(TURNFROM)
#define PIE_FOOD_SENSORS
#endif

#if defined(RETREAT)
#define PIE_FOOD_SENSORS
#endif

#if defined(FIGHTFLIGHT)
#define PIE_FOOD_SENSORS
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

#if defined(XTASKS) || defined(TESTFIT) || defined(DUMFIT)
#define INNOV_CATEGORIES
#endif

#if defined(REACHING)
#define PIE_FOOD_SENSORS
#define TRACKER
#define EA_EVAL_ALL
#endif

#if defined(RETINA)
#define FOOD_SENSORS
#define TRACKER
#define EA_EVAL_ALL
#endif

#if defined(RANDOMFOOD)
#define PIE_FOOD_SENSORS
#define TRACKER
#define EA_EVAL_ALL
#endif

//Make sure the inputs are not overlapping when predator is defined
#if defined(PREDATOR)
#define INPUT_VISUALIZE_GROUP_OFFSETS
#define PIE_FOOD_SENSORS
#define TRACKER
#define EA_EVAL_ALL
#endif

#if defined(PIE_FOOD_SENSORS)
#define FOOD_SENSORS
#endif

#if defined(MPI_ENABLED) and defined(CMOEA) and !defined(NOMPI)
#define NR_OF_MPI_TASKS 2
#elif defined(MPI_ENABLED) and !defined(NOMPI)
#define NR_OF_MPI_TASKS 1
#endif

// Thus far, the robotics experiment has performed poorly with TBB paralization
#if not defined(NO_PARALLEL)
#define NO_PARALLEL
#endif

#if defined(MPCMOEA)
#define BARRIER_BEFORE_DELETE
#define INIT_SIZE_IS_BIN_SIZE
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

#if defined(REFPOINT)
#define ADD_EQUAL_WEIGHT_REFERENCE_POINT
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
#define BIN_SIZE 1000
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN100)
#define BIN_SIZE 100
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN10)
#define BIN_SIZE 10
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN4)
#define BIN_SIZE 4
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN2)
#define BIN_SIZE 2
#if defined(BIN_SIZE_DEFINED)
#define TWO_BIN_SIZES_DEFINED
#else
#define BIN_SIZE_DEFINED
#endif
#endif

#if defined(BIN1)
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
#if defined(GEN75000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN50000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN250)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN500)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN1000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

#if defined(GEN15000)
#if defined(GEN_DEFINED)
#define TWO_GENS_DEFINED
#else
#define GEN_DEFINED
#endif
#endif

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

#if defined(GEN25)
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
#if defined(POP10000)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP1000)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP6300)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP630)
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

#if defined(POP16)
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

#if defined(POP4)
#if defined(POP_DEFINED)
#define TWO_POPS_DEFINED
#else
#define POP_DEFINED
#endif
#endif

#if defined(POP2)
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

//#define VISUALIZE_FONT "/Library/Fonts/HelveticaCY.dfont"
#if defined(MORAN)
#define VISUALIZE_FONT "/home/jhuizing/sferes/sferes2/modules/oglft/Arial.ttf"
#endif
#if not defined(VISUALIZE_FONT)
#define VISUALIZE_FONT "/Library/Fonts/Arial.ttf"
#endif

#if defined(E20)
#define ELEX_USE_OFFSET
#define ELEX_OFFSET_0_20
#elif defined(E15)
#define ELEX_USE_OFFSET
#define ELEX_OFFSET_0_15
#elif defined(E10)
#define ELEX_USE_OFFSET
#define ELEX_OFFSET_0_10
#elif defined(E05)
#define ELEX_USE_OFFSET
#define ELEX_OFFSET_0_05
#elif defined(E01)
#define ELEX_USE_OFFSET
#define ELEX_OFFSET_0_01
#endif

#if defined(PMAX)
#define POOL_MAX
#endif

#if defined(PMAD)
#define POOL_MAD
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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

//Sferes header files
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/ea/crowd.hpp>
#include <sferes/fit/fitness.hpp>


#if defined(REGULATOR)
#include <modules/hnn/af_dummy.hpp>
#include <modules/hnn/pf_regulator.hpp>
#endif

#if defined(REGULATORY_CONNECTION)
#include <modules/hnn/af_dummy.hpp>
#include <modules/hnn/pf_regulatory_conn.hpp>
#endif

//Sferes modules
//
#include <modules/nn2/nn.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/phen_dnn.hpp>
#include <modules/hnn/af_cppn.hpp>
#include <modules/modularity/modularity.hpp>
#include <modules/continue/continue_run.hpp>
#include <modules/continue/continue_ea.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>
//#include <modules/continue/io_convenience.hpp>
#include <modules/datatools/position3.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/params.hpp>
#include <modules/datatools/io_convenience.hpp>
#include <modules/nsgaext/nsga2_custom.hpp>
#include <modules/cmoea/nsga_util.hpp>
#include <modules/diversity/behavior_div.hpp>
#include <modules/hierarchy/hierarchy.hpp>
#include <modules/hierarchy/hourglass_hierarchy.hpp>
#include <modules/opengldraw/opengl_draw_painter.hpp>
#include <modules/lexicase/lexicase.hpp>
#include <modules/lexicase/e_lexicase.hpp>
#include <modules/continue/stat_store_pop.hpp>
#include <modules/nsga3/nsga3.hpp>
#include <modules/nsga3/continue_ea_nsga3.hpp>
#include <modules/nsga3/stat_nsga3.hpp>

#if defined(MAPELITEINOVDIV)
#include <modules/mapelite/map_elite_innovation_diversity.hpp>
#include <modules/mapelite/continue_mapelite_innov_div.hpp>
#include "stat_mod_robot_map_elite_innov_div.hpp"
#endif

#if defined(MAPELITEINOVPNSGA)
#if defined(MPI_ENABLED) and !defined(NOMPI)
#include <modules/cmoea/cmoea_nsga2_mpi.hpp>
#else
#include <modules/cmoea/cmoea_nsga2.hpp>
#endif
#include <modules/cmoea/continue_cmoea_nsga2.hpp>
#include "stat_mod_robot_map_elite_innov_div.hpp"
#endif

#if defined(MPCMOEA)
#include <modules/cmoea/mp_cmoea_nsga2_mpi.hpp>
#include <modules/cmoea/continue_cmoea_nsga2.hpp>
#endif

#if defined(MAPELITE)
#include <modules/mapelite/map_elite.hpp>
#include <modules/mapelite/continue_mapelite.hpp>
#include <modules/mapelite/stat_map.hpp>
#endif

#if defined(VISUALIZE)
#include <modules/oglft/OGLFT.h>
#include <modules/opengldraw/opengl_draw_text.hpp>
#endif

#include "stat_mod_robot_innov_first_bin.hpp"
#include "stat_mod_robot_fight_flight.hpp"

//Local header files
#include "gen_spatial.hpp"
#include "stat_mod_robot.hpp"
#include "stat_mod_robot_food.hpp"
#include "mvg.hpp"
#include "modularity.hpp"
#include "mod2.hpp"
#include "stat_logall.hpp"
#include "stat_render_video_pop_only.hpp"
#include "mut_sweep_macros.hpp"
#include "neuron_group.hpp"
#include "graph_util.hpp"
#include "mod_robot_visualize_network.hpp"
#include "nn_conn_to_key.hpp"
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

#include <modules/mpiworldtrack/core.hpp>

#if defined(MPI_ENABLED) && !defined(NOMPI) && !defined(MPCMOEA)
#include <modules/mpiworldtrack/eval_mpi_world_tracking.hpp>
#else
#include <modules/mpiworldtrack/eval_world_tracking.hpp>
#endif

#include "lzw.hpp"

// Simulator includes (should become a module)
#include "mod_robot_sim.hpp"
#include "mod_robot_spider.hpp"
#include "mod_robot_pie_sensor.hpp"
#include "mod_robot_cone_sensor.hpp"
#include "mod_robot_control_sensor.hpp"
#include "mod_robot_raycast_sensor.hpp"
#include "clock.hpp"
#include "pnsga.hpp"

#include "dom_sort_no_duplicates.hpp"
#include "dom_sort_fast.hpp"


/*************************************************************
 ************************ DEFINES ****************************
 *************************************************************/
#define NUMBER_OF_PERMUTATIONS 500
#define SQUARE_ROOT_3 1.7320508
#define SQ3 SQUARE_ROOT_3
#define D2 1.25
#define D3 1.5
#define DBOI dbg::out(dbg::info, "init")
#define DBOO dbg::out(dbg::info, "obj")
#define DBOF dbg::out(dbg::info, "fitness")
#define DBOT dbg::out(dbg::info, "tracker")

#ifndef FRICTION
#define FRICTION 0.5
#endif

#ifndef NNPERIOD
#define NNPERIOD 1
#endif

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
typedef std::map<std::string, unsigned int> map_t;


SFERES_TYPE_VALUE_PARAM(Friction, SFERES_CONST float v, FRICTION)
SFERES_TYPE_VALUE_PARAM(NnDelay, SFERES_CONST float v, NNPERIOD)

struct ParamsRobot {
    struct robot
    {
    	SFERES_CONST float friction = Friction::v;
    	SFERES_CONST int nn_delay = NnDelay::v;
    };
};
typedef mod_robot::Spider<ctrnn_nn_t, ParamsRobot> robot_t;
typedef mod_robot::ModRobotSimulator<robot_t> sim_t;


//int indivs_serialized = 0;

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
        //IMPORTANT: Population size has to be divisible by 4
      //SFERES_CONST size_t init_batch = 1;
    	// The maximum number of individuals to try an create at the same time.
    	SFERES_CONST unsigned max_batch_size = 1024;
#if defined(POP1000)
        SFERES_CONST unsigned size = 1000;
        SFERES_CONST unsigned select_size = 1000;
#elif defined(POP10000)
        SFERES_CONST unsigned size = 10000;
        SFERES_CONST unsigned select_size = 10000;
#elif defined(POP6300)
        SFERES_CONST unsigned size = 6300;
        SFERES_CONST unsigned select_size = 1000;
#elif defined(POP630)
        SFERES_CONST unsigned size = 630;
        SFERES_CONST unsigned select_size = 100;
#elif defined(POP100)
        SFERES_CONST unsigned size = 100;
        SFERES_CONST unsigned select_size = 100;
#elif defined(POP16)
        SFERES_CONST unsigned size = 16;
        SFERES_CONST unsigned select_size = 16;
#elif defined(POP12)
        SFERES_CONST unsigned size = 12;
        SFERES_CONST unsigned select_size = 12;
#elif defined(POP4)
        SFERES_CONST unsigned size = 4;
        SFERES_CONST unsigned select_size = 4;
#elif defined(POP2)
        SFERES_CONST unsigned size = 2;
        SFERES_CONST unsigned select_size = 2;
#else  //defined(POP1000)
        SFERES_CONST unsigned size = 1000;
        SFERES_CONST unsigned select_size = 1000;
#endif //defined(POP1000)

#if defined(GEN5000)
        SFERES_CONST unsigned nb_gen = 5001;
        SFERES_CONST int nr_of_dumps = 8;
#elif defined(GEN75000)
        SFERES_CONST unsigned nb_gen = 75001;
        SFERES_CONST int nr_of_dumps = 316;
#elif defined(GEN50000)
        SFERES_CONST unsigned nb_gen = 50001;
        SFERES_CONST int nr_of_dumps = 216;
#elif defined(GEN25000)
        SFERES_CONST unsigned nb_gen = 25001;
        SFERES_CONST int nr_of_dumps = 216;
#elif defined(GEN15000)
        SFERES_CONST unsigned nb_gen = 15001;
        SFERES_CONST int nr_of_dumps = 216;
#elif defined(GEN3000)
        SFERES_CONST unsigned nb_gen = 3001;
        SFERES_CONST int nr_of_dumps = 24;
#elif defined(GEN1500)
        SFERES_CONST unsigned nb_gen = 1501;
        SFERES_CONST int nr_of_dumps = 30;
#elif defined(GEN1000)
        SFERES_CONST unsigned nb_gen = 1001;
        SFERES_CONST int nr_of_dumps = 5;
#elif defined(GEN500)
        SFERES_CONST unsigned nb_gen = 501;
        SFERES_CONST int nr_of_dumps = 5;
#elif defined(GEN250)
        SFERES_CONST unsigned nb_gen = 251;
        SFERES_CONST int nr_of_dumps = 5;
#elif defined(GEN100)
        SFERES_CONST unsigned nb_gen = 101;
        SFERES_CONST int nr_of_dumps = 10;
#elif defined(GEN25)
        SFERES_CONST unsigned nb_gen = 26;
        SFERES_CONST int nr_of_dumps = 1;
#elif defined(GEN10)
        SFERES_CONST unsigned nb_gen = 11;
        SFERES_CONST int nr_of_dumps = 10;
#elif defined(GEN1)
        SFERES_CONST unsigned nb_gen = 1;
        SFERES_CONST int nr_of_dumps = 1;
#else  //defined(GEN5000)
        SFERES_CONST unsigned nb_gen = 1001;
        SFERES_CONST int nr_of_dumps = 8;
#endif //defined(GEN5000)
        SFERES_CONST int initial_aleat = 1;
        static int dump_period;
        static int checkpoint_period;

#if defined(MAPELITE)
        SFERES_CONST int initial_mutations = 10;
#endif

#if defined(MAPELITEINOVDIV) || defined(MAPELITEINOVPNSGA) || defined(MAPELITE) || defined(MPCMOEA)
#if defined(INITSIZE10000)
        SFERES_CONST size_t init_size = 10000;
#elif defined(INITSIZE10)
        SFERES_CONST size_t init_size = 10;
#elif defined(INIT_SIZE_IS_BIN_SIZE)
        SFERES_CONST size_t init_size = BIN_SIZE;
#else
        SFERES_CONST size_t init_size = size;
#endif
#endif
    };

    struct ea
    {
    	PNSGA_PROB(SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0, 1.0));
#if defined(NODUPSORT)
        typedef sferes::ea::dom_sort_no_duplicates_f dom_sort_f;
#elif defined(FASTSORT)
        typedef sferes::ea::dom_sort_fast_f dom_sort_f;
#else
        typedef sferes::ea::dom_sort_basic_f dom_sort_f;
#endif
        SFERES_CONST size_t clusters = 10;

#if defined(MAPELITE)
        SFERES_CONST size_t res_x = 100;
        SFERES_CONST size_t res_y = 100;
#endif
    };

    struct e_lexicase
    {
#if defined(POOL_MAD)
    	SFERES_CONST bool pool_mad = true;
#else
    	SFERES_CONST bool pool_mad = false;
#endif
#if defined(POOL_MAX)
    	SFERES_CONST bool pool_max = true;
#else
    	SFERES_CONST bool pool_max = false;
#endif
#if defined(ELEX_USE_OFFSET)
    	SFERES_CONST bool fixed_offset = true;
#if defined(ELEX_OFFSET_0_20)
    	SFERES_ARRAY(float, offset, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 6.4e-5)
#elif defined(ELEX_OFFSET_0_15)
    	SFERES_ARRAY(float, offset, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 1.139062e-5)
#elif defined(ELEX_OFFSET_0_10)
    	SFERES_ARRAY(float, offset, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 1.0e-6)
#elif defined(ELEX_OFFSET_0_05)
		SFERES_ARRAY(float, offset, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 1.5625e-8)
#elif defined(ELEX_OFFSET_0_01)
    	SFERES_ARRAY(float, offset, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 1e-12)
#else
#warning "No valid offset defined!"
#endif
#else
    	SFERES_CONST bool fixed_offset = false;
    	SFERES_ARRAY(float, offset, 0, 0, 0, 0, 0, 0, 0)
#endif

    };

#if defined(MAPELITEINOVPNSGA)
    struct cmoea_nsga
    {
#if defined(NOPNSGA)
        typedef sferes::ea::_dom_sort_basic::non_dominated_f non_dom_f;
#else
        typedef sferes::prob_dom_f<ea> non_dom_f;
#endif
    };
#endif

#if defined(MPCMOEA)
    struct cmoea_nsga{
    	typedef sferes::ea::_dom_sort_basic::non_dominated_f non_dom_f;
    };
#endif

    struct cmoea
    {
        static size_t nb_of_bins;

        SFERES_CONST size_t obj_index = 0;
#if defined(REP2)
        SFERES_CONST int replicates = 2;
#elif defined(REP32)
        SFERES_CONST int replicates = 32;
#else
        SFERES_CONST int replicates = 1;
#endif

        SFERES_CONST size_t max_send_size = 10;

#if defined(BIN1000)
        SFERES_CONST size_t bin_size = 1000;
#elif defined(BIN100)
        SFERES_CONST size_t bin_size = 100;
#elif defined(BIN10)
        SFERES_CONST size_t bin_size = 10;
#elif defined(BIN4)
        SFERES_CONST size_t bin_size = 4;
#elif defined(BIN2)
        SFERES_CONST size_t bin_size = 2;
#elif defined(BIN1)
        SFERES_CONST size_t bin_size = 1;
#elif defined(BINPOPSIZE)
        SFERES_CONST size_t bin_size = Params::pop::size;
#else
#error "No bin size defined"
#endif
    };

    struct nsga3
    {
    	static size_t nb_of_objs;
    	static float ref_points_delta;
    };

    struct rand_obj
    {
        SFERES_CONST float std_dev = 0.1;
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

        //		SFERES_CONST SvgColor color = SvgColor(1,2,3);
        SFERES_CONST size_t max_modularity_split = 3;
        SFERES_CONST float scale = 100;

        SFERES_NON_CONST_ARRAY(float, x);
        SFERES_NON_CONST_ARRAY(float, y);
        SFERES_NON_CONST_ARRAY(float, z);
    };

    struct sim
    {
#ifdef STEP5
        SFERES_CONST size_t steps = 5;
        SFERES_CONST size_t move_steps = 5;
        SFERES_CONST size_t turn_steps = 5;
        SFERES_CONST size_t jump_steps = 5;
        SFERES_CONST size_t get_down_steps = 5;
        SFERES_CONST size_t turn_to_steps = 5;
        SFERES_CONST size_t move_to_steps = 5;
        SFERES_CONST size_t strike_steps = 5;
        SFERES_CONST size_t attack_steps = 5;
        SFERES_CONST size_t turn_from_steps = 5;
        SFERES_CONST size_t retreat_steps = 5;
        SFERES_CONST size_t fight_flight_steps = 5;
#elif STEP1000
        SFERES_CONST size_t move_steps = 1000;
#else
        SFERES_CONST size_t steps = 1000;
        SFERES_CONST size_t move_steps = 400;
        SFERES_CONST size_t turn_steps = 200;
        SFERES_CONST size_t jump_steps = 100;
        SFERES_CONST size_t get_down_steps = 200;
        SFERES_CONST size_t turn_to_steps = 200;
        SFERES_CONST size_t move_to_steps = 400;
        SFERES_CONST size_t strike_steps = 200;
        SFERES_CONST size_t attack_steps = 600;
        SFERES_CONST size_t turn_from_steps = 200;
        SFERES_CONST size_t retreat_steps = 400;
        SFERES_CONST size_t fight_flight_steps = 400;
#endif

#ifdef TRIALS0
        SFERES_CONST size_t turn_to_trails = 0;
        SFERES_CONST size_t move_to_trails = 0;
        SFERES_CONST size_t strike_trails = 0;
        SFERES_CONST size_t attack_trails = 0;
        SFERES_CONST size_t turn_from_trails  = 0;
        SFERES_CONST size_t retreat_trails = 0;
        SFERES_CONST size_t fight_flight_trails = 0;
        SFERES_CONST size_t trials = 0;
#else
        SFERES_CONST size_t turn_to_trails = 4;
        SFERES_CONST size_t move_to_trails = 4;
        SFERES_CONST size_t strike_trails = 1;
        SFERES_CONST size_t attack_trails = 4;
        SFERES_CONST size_t turn_from_trails  = 4;
        SFERES_CONST size_t retreat_trails = 4;
        SFERES_CONST size_t fight_flight_trails = 4;
        SFERES_CONST size_t trials = 5;
#endif

    };

    struct stats
    {
        SFERES_CONST size_t period = 1; //print stats every N generations
        SFERES_CONST size_t logall_period = 100;
    };

    //These values are for evolving the substrate directly,
    //They are not used when HyperNEAT is active

    // for weights
    struct weights
    {
        struct evo_float
        {
            SFERES_CONST float cross_rate = 0.5f;
            //1.0 because dnn handle this globally
            SFERES_CONST float mutation_rate = 1.0f;
            SFERES_CONST float eta_m = 10.0f;
            SFERES_CONST float eta_c = 10.0f;
            SFERES_CONST mutation_t mutation_type = polynomial;
            SFERES_CONST cross_over_t cross_over_type = sbx;
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
            SFERES_CONST float min = -2;
            SFERES_CONST float max = 2;
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
            SFERES_CONST mutation_t mutation_type = polynomial;
            SFERES_CONST cross_over_t cross_over_type = sbx;
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
            SFERES_CONST float min = -2;
            SFERES_CONST float max = 2;
        };
    };

    //For the time constant
    struct time_constant
    {
        struct evo_float
        {
            SFERES_CONST float cross_rate = 0.5f;
            SFERES_CONST float mutation_rate = 1.0f / 24.0f;
            SFERES_CONST float eta_m = 10.0f;
            SFERES_CONST float eta_c = 10.0f;
            SFERES_CONST mutation_t mutation_type = polynomial;
            SFERES_CONST cross_over_t cross_over_type = sbx;
        };
        struct sampled
        {
            SFERES_ARRAY(float, values, 1, 2, 3, 4, 5);
            SFERES_CONST float mutation_rate = 1.0 / 24.0f;
            SFERES_CONST float cross_rate = 0.0f;
            SFERES_CONST bool ordered = false;
        };
        struct parameters
        {
            SFERES_CONST float min = 1;
            SFERES_CONST float max = 6;
        };
    };

    struct dnn
    {
        // slope of the sigmoid
        SFERES_CONST size_t lambda = 20;

        //Add Connection rate for the cppn
        SFERES_CONST float m_rate_add_conn = CONADDMUTRATE(0.2f);
        //Delete connection rate for the cppn
        SFERES_CONST float m_rate_del_conn = CONDELMUTRATE(0.2f);

        //SFERES_CONST float m_rate_change_conn = 0.15; //old default
        SFERES_CONST float m_rate_change_conn = 0.0; //new default?
        SFERES_CONST float m_rate_add_neuron = 0.0f;
        SFERES_CONST float m_rate_del_neuron = 0.0f;
        // average number of weight changes
        SFERES_CONST float m_avg_weight = 2.0f;

        SFERES_CONST int io_param_evolving = true;
        SFERES_CONST init_t init = ff;

        // Number of neurons per batch in the hidden layer
        // Terminology is vague on purpose
        // When the neurons are organized as a grid the `batches'
        // are columns and the `neurons per batch' are rows.
        // When the neurons are organized as a circle the `batches'
        // are rings and the `neurons per batch' are the number of neurons per
        // ring
#if defined(NX4)
        SFERES_CONST size_t nb_neuron_batches = 4;
#elif defined(NX5)
        SFERES_CONST size_t nb_neuron_batches = 5;
#elif defined(NX6)
        SFERES_CONST size_t nb_neuron_batches = 6;
#else
        SFERES_CONST size_t nb_neuron_batches = 5;
#endif

#if defined(LAY2)
        SFERES_CONST size_t nb_layers = 2;
#else
        SFERES_CONST size_t nb_layers = 1;
#endif

#if defined(NY1)
        SFERES_CONST size_t nb_neurons_per_batch = 1;
#elif defined(NY4)
        SFERES_CONST size_t nb_neurons_per_batch = 4;
#elif defined(NY5)
        SFERES_CONST size_t nb_neurons_per_batch = 5;
#elif defined(NY6)
        SFERES_CONST size_t nb_neurons_per_batch = 6;
#else
        SFERES_CONST size_t nb_neurons_per_batch = 5;
#endif

        static size_t nb_inputs;     //6
        SFERES_CONST size_t nb_outputs = robot_t::total_actuator_count; //18
        SFERES_CONST size_t nb_of_hidden = nb_neuron_batches*
        		nb_neurons_per_batch * nb_layers;

        SFERES_CONST size_t min_nb_neurons = 10; //useless
        SFERES_CONST size_t max_nb_neurons = 30; //useless
        SFERES_CONST size_t min_nb_conns = 0; // ?? TODO
        SFERES_CONST size_t max_nb_conns = 120; // ?? TODO

        struct spatial{
            // RAND COORDS START
            // These are settings for random coordinates. They are not actually
        	// used in this experiment, but they are necessary to compile.
            SFERES_CONST float min_coord = -3.0f;
            SFERES_CONST float max_coord = 3.0f;
            SFERES_CONST bool rand_coords = false;
            // RAND COORDS END

            SFERES_NON_CONST_ARRAY(int, layers);
            SFERES_NON_CONST_ARRAY(float, x);
            SFERES_NON_CONST_ARRAY(float, y);
            SFERES_NON_CONST_ARRAY(float, z);
        };
    };

    struct fitness
    {
        SFERES_CONST unsigned int nb_bits  = Params::dnn::nb_outputs;
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
            // These values define the mutations on the activation function of
        	// the CPPN nodes. As such, they define which activation function
        	// can be chosen and the mutation rate for changing the activation
        	// function
            struct sampled {
            	//Possible activation functions
                SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid,
                		nn::cppn::gaussian, nn::cppn::linear);
                //Mutation rate for the cppn nodes
                SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                //Not actually used, even if non-zero
                SFERES_CONST float cross_rate = 0.0f;
                //Mutation will randomly pick activation function from the list
                SFERES_CONST bool ordered = false;
                //The alternative has bias towards adjacent values
            };

            // These value determine the mutation rate for the weights and
            // biases of the cppn. Note that, in practice, the bias is
            // determined by the weight of the connection between the bias
            // input node and the target node, so biases are not mutated
            // 'directly'.
            struct evo_float {
            	//Mutation rate for the cppn nodes
                SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                //Not actually used, even if non-zero
                SFERES_CONST float cross_rate = 0.0f;
                //Mutation type
                SFERES_CONST mutation_t mutation_type = polynomial;
                //Mutation eta (parameter for polynomial mutation).
                SFERES_CONST float eta_m = 10.0f;
                //Not used
                SFERES_CONST cross_over_t cross_over_type = sbx;
                //SBX eta (not used)
                SFERES_CONST float eta_c = 10.0f;
            };

            struct parameters{
            	// maximum value of parameters (weights & bias)
                SFERES_CONST float min = -3;
                // minimum value
                SFERES_CONST float max = 3;
            };

#if defined(CTRNN_MULTIPLE_OUTPUTSETS)
#if defined(OS1)
            SFERES_CONST size_t nb_output_sets = 1;
#elif defined(OS2)
            SFERES_CONST size_t nb_output_sets = 2;
#elif defined(OS3)
            SFERES_CONST size_t nb_output_sets = 3;
#elif defined(OS4)
            SFERES_CONST size_t nb_output_sets = 4;
#elif defined(OS5)
            SFERES_CONST size_t nb_output_sets = 5;
#elif defined(OS6)
            SFERES_CONST size_t nb_output_sets = 6;
#elif defined(OS7)
            SFERES_CONST size_t nb_output_sets = 7;
#else
#error "Multiple output-sets defined, but the number of sets is not defined."
#endif
#endif

            // This threshold determines at what value of the Link-Expression
            // Output (LEO) a link will be expressed or not. Before LEO values
            // are currently between -1.0 and 1.0 and only values equal to or
            // above the threshold are expressed. As a result a value of -1.0
            // means all links are expressed (effectively disabling the LEO
            // node) while a value greater than 1.0 means no values will ever be
            // expressed (probably a bad idea). A value of 0.0 is default.
#if defined(CMO_RELATIVE_THRESHOLD)
            // These values are set such that the initial probability of any one
            // connection existing is 0.5, regardless of the number of output
            // sets.
#if defined(OS1)
            SFERES_CONST float leo_threshold = 1.0f - (0.5f * 2);
#elif defined(OS2)
            SFERES_CONST float leo_threshold = 1.0f - (0.292893f * 2);
#elif defined(OS3)
            SFERES_CONST float leo_threshold = 1.0f - (0.206299f * 2);
#elif defined(OS4)
            SFERES_CONST float leo_threshold = 1.0f - (0.159104f * 2);
#elif defined(OS5)
            SFERES_CONST float leo_threshold = 1.0f - (0.129449f * 2);
#elif defined(OS6)
            SFERES_CONST float leo_threshold = 1.0f - (0.109101f * 2);
#elif defined(OS7)
            SFERES_CONST float leo_threshold = 1.0f - (0.0942763f * 2);
#else
#error "Multiple output-sets defined, but the number of sets is not defined."
#endif
#else
            SFERES_CONST float leo_threshold = 0.0f;
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
            	//Possible activation functions
                SFERES_ARRAY(float, values, nn::sine, nn::sigmoid,
                		nn::gaussian, nn::linear, nn::regulator);
                //Mutation rate for the cppn nodes
                SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                //Not actually used, even if non-zero
                SFERES_CONST float cross_rate = 0.0f;
				//Mutation will randomly pick an other function from the list
                SFERES_CONST bool ordered = false;
            };
        };
#endif

#if defined(REGULATORY_CONNECTION)
        struct regulatory_conn{
            struct pf_param{
                struct sampled {
                	//Possible activation functions
                    SFERES_ARRAY(float, values, nn::cppn::sine,
                    		nn::cppn::sigmoid, nn::cppn::gaussian,
							nn::cppn::linear);
                    //Mutation rate for the cppn nodes
                    SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                    //Not actually used, even if non-zero
                    SFERES_CONST float cross_rate = 0.0f;
                    //Mutation will randomly pick another function from the list
                    SFERES_CONST bool ordered = false;
                };
            };
            struct weight_param{
                struct sampled {
                	//Weight can be either normal or regulatory
                    SFERES_ARRAY(float, values, nn::normal, nn::regulatory);
                    //Mutation rate for the cppn nodes
                    SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                    //Not actually used, even if non-zero
                    SFERES_CONST float cross_rate = 0.0f;
                    //Mutation will randomly pick another function from the list
                    SFERES_CONST bool ordered = false;
                };
                struct evo_float {
                	//Mutation rate for the cppn nodes
                    SFERES_CONST float mutation_rate = CONCHANGEMUTRATE(0.1f);
                    //Not actually used, even if non-zero
                    SFERES_CONST float cross_rate = 0.0f;
                    //Mutation type
                    SFERES_CONST mutation_t mutation_type = polynomial;
                    //Not used
                    SFERES_CONST cross_over_t cross_over_type = sbx;
                    //Mutation eta (parameter for polynomial mutation.
                    SFERES_CONST float eta_m = 10.0f;
                    //?
                    SFERES_CONST float eta_c = 10.0f;
                };

                struct parameters{
                	// maximum value of parameters (weights & bias)
                    SFERES_CONST float min = -3;
                    // minimum value
                    SFERES_CONST float max = 3;
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
            SFERES_CONST size_t nb_inputs = 8;
            typedef DistanceInput<Params::dnn> input_t;
#else
            SFERES_CONST size_t nb_inputs = 7;
            typedef phen::hnn::ThreeDInput<Params::dnn> input_t;
#endif
            static size_t nb_outputs;
            SFERES_CONST init_t init = ff;

            //Add Connection rate for the cppn
            SFERES_CONST float m_rate_add_conn = CONADDMUTRATE(0.09f);
            //Delete connection rate for the cppn
            SFERES_CONST float m_rate_del_conn = CONDELMUTRATE(0.08f);
            //Add neuron rate for the cppn
            SFERES_CONST float m_rate_add_neuron = NEUADDMUTRATE(0.05f);
            //Delete neuron rate for the cppn
            SFERES_CONST float m_rate_del_neuron = NEUDELMUTRATE(0.04f);

            //Not used for the cppn, even if value is non zero.
            SFERES_CONST float m_rate_change_conn = 0.0f;

            SFERES_CONST size_t min_nb_neurons = 7;
            SFERES_CONST size_t max_nb_neurons = 200;
            SFERES_CONST size_t min_nb_conns = 0;
            SFERES_CONST size_t max_nb_conns = 1500;
        };
    };
};
)


/*************************************************************
 ************************ FUNCTIONS **************************
 *************************************************************/

void set_seed(int rank = 0){
    dbg::trace trace("init", DBG_HERE);
    if(options::map.count("seed")){
        int seed = options::map["seed"].as<int>() + rank;
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

size_t getInputLayers(){
    size_t nb_input_layers = 1;
#if defined(FOOD_SENSORS)
    nb_input_layers += 1;
#endif
    return nb_input_layers;
}

robot_t* init_spider(){
    dbg::trace trace("init", DBG_HERE);

    using namespace mod_robot;
    using namespace opengl_draw;

    btDynamicsWorld* world = Params::simulator->getWorld();
    robot_t* spider = new robot_t(world);
    dbg::out(dbg::info, "init") << "Building spider success" << std::endl;
    //btScalar angle;
    std::vector<btVector3> positions;
    const btVector3 up(0,1,0);
    const btVector3 xVector(1,0,0);
    size_t nb_of_hidden_layers = Params::dnn::nb_layers;
    size_t nb_input_layers = getInputLayers();
    size_t total_layers = nb_of_hidden_layers + nb_input_layers + 1;
    btScalar layer_incr = 2.0f/btScalar(total_layers-1);
    btScalar layer_y = 1;

    //For now we'll assume that there are always 6 positions
    SFERES_CONST unsigned int nb_of_positions = 6;

#if defined(CIRCULAR_INPUTS)
    {
    	btVector3 orientation (1,0,0);
    	btScalar angle = M_PI/nb_of_positions;
    	for(size_t i=0; i<nb_of_positions; i++){
    		positions.push_back(orientation + up);
    		orientation = orientation.rotate(up, angle*2);
    	}
    }
#elif defined(X_LINE_INPUTS)
    {
    	SFERES_CONST btScalar x_min = -1.0;
    	SFERES_CONST btScalar x_max = 1.0;
    	const btScalar increment = (x_max - x_min)/btScalar(nb_of_positions-1);

    	for(size_t i=0; i<nb_of_positions; i++){
    		positions.push_back(xVector*(x_min + i*increment) + up*layer_y);
    	}
    }
#else
#error "No setup for the inputs specified (what happened to the default?)."
#endif


    // ** ADD FOOD SENSORS START ** //
#if defined(FOOD_SENSORS)
	spider->addSensorGroup();
	spider->getLastSensorGroup().addDetects(sim_t::food_type);
	spider->getLastSensorGroup().addDetects(sim_t::poison_type);
	spider->getLastSensorGroup().addDetects(sim_t::target_type);
    layer_y -= layer_incr;
#endif

    // Setup food sensors in six directions
#if defined(PIE_FOOD_SENSORS)
    {
    	dbg::assertion(DBG_ASSERTION(positions.size() == nb_of_positions));
    	btVector3 orientation (1,0,0);
    	btScalar angle = M_PI/nb_of_positions;
    	for(size_t i=0; i<nb_of_positions; i++){
    		PieSensor* food_sensor = new PieSensor(spider->getMainBody(),
    				orientation, angle);
    		food_sensor->setModifier(btVector3(1, 0, 1));
    		food_sensor->setNeuronPosition(positions[i]);
    		food_sensor->setActiveColor(opengl_draw::Color(0.5, 1.0, 0.5, 0.5));
    		spider->getLastSensorGroup().add(food_sensor);
    		orientation = orientation.rotate(up, angle*2);
    	}
    }
#endif

    // If Reaching is defined, add a foodsensor on top of the robot
#if defined(REACHING)
    btScalar top_angle = M_PI/4;
    btMatrix3x3 rotation;
    rotation.setEulerZYX(0, 0, M_PI_2);
    ConeSensor* food_sensor = new ConeSensor(spider->getMainBody(),
    		rotation, top_angle);
    food_sensor->setNeuronPosition(btVector3(0,1,0));
    food_sensor->setActiveColor(Color(0.5, 1.0, 0.5, 0.5));
    spider->getLastSensorGroup().add(food_sensor);
#endif

#if defined(RETINA)
    {
    	btMatrix3x3 rotation;
    	//    int retina_width = 1;
    	//    int retina_height = 1;

    	int retina_width = 8;
    	int retina_height = 4;
    	float retina_offset = 0.05;
    	float x_min = -1.0;
    	float x_max = 1.0;
    	float y_min = -1.0;
    	float y_max = 1.0;
    	float xd = (x_max - x_min)/float(retina_width-1);
    	float yd = (y_max - y_min)/float(retina_height-1);
    	float r_x_min = -((retina_width-1)/2) * retina_offset;
    	float r_y_min = -((retina_height-1)/2) * retina_offset;
    	for(int x=0; x<retina_width; ++x){
    		for(int y=0; y<retina_height; ++y){
    			rotation.setEulerZYX(0, r_x_min + 0.05*x, r_y_min + 0.05*y);
    			RaycastSensor* ray = new RaycastSensor(spider->getMainBody(),
    					Params::simulator->getWorld(), rotation);
    			ray->setNeuronPosition(btVector3(x_min+x*xd, 2, y_min+y*yd));
    			ray->setActiveColor(Color(0.5, 1.0, 0.5, 0.5));
    			spider->getLastSensorGroup().add(ray);
    		}
    	}
    }

#endif
    // ** ADD FOOD SENSORS END ** //



    //Setup control sensors (or dummy sensors if not controlled)
    std::vector<ControlSensor*> ctrSens;
#if defined(NEGATIVE_INACTIVE_VALUE)
    btScalar inactiveValue = -1.0f;
#else
    btScalar inactiveValue = 0.0f;
#endif

#if defined(FORWARD)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(BACKWARD)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(TURNLEFT)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(TURNRIGHT)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(JUMP)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(GETDOWN)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(TURNTO)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(MOVETO)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(STRIKE)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(ATTACK)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(TURNFROM)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(RETREAT)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif
#if defined(FIGHTFLIGHT)
    ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
#endif

#if defined(DUMFIT) || defined(TESTFIT)
    for(int i=0; i<6; i++){
    	ctrSens.push_back(new ControlSensor(ctrSens.size(), inactiveValue));
    }
#endif

    //Position the control sensors
    {
    	if(ctrSens.size() > 0){
        	btScalar x_min = -1.0;
        	btScalar x_max = 1.0;
        	btScalar increment = 0;
        	if(ctrSens.size() == 1){
        		x_min = 0;
        	} else {
        		increment = (x_max - x_min)/btScalar(ctrSens.size()-1);
        	}
        	for(size_t i=0; i<ctrSens.size(); i++){
        		ctrSens[i]->setNeuronPosition(xVector*(x_min+i*increment) + up*layer_y);
        	}

////    		btVector3 orientation (1,0,0);
//    		btScalar angle = (2*M_PI)/ctrSens.size();
//    		for(int i=0; i<ctrSens.size(); i++){
//    			ctrSens[i]->setNeuronPosition(positions[i]);
////    			orientation = orientation.rotate(btVector3(0,1,0), angle);
//    		}

    		//Add sensors to sensor groups
#if defined(SEPARATE_SENSOR_GROUPS)
    		for(int i=0; i<ctrSens.size(); i++){
    			spider->addSensorGroup();
    			spider->getLastSensorGroup().addDetects(sim_t::control_type);
    			spider->getLastSensorGroup().add(ctrSens[i]);
    		}
#else
    		spider->addSensorGroup();
    		spider->getLastSensorGroup().addDetects(sim_t::control_type);
    		for(int i=0; i<ctrSens.size(); i++){
    			spider->getLastSensorGroup().add(ctrSens[i]);
    		}
#endif
    	}
    	layer_y -= layer_incr;
    }




#if defined(PREDATOR)
    {
    	spider->addSensorGroup();
    	spider->getLastSensorGroup().addDetects(sim_t::predator_type);
    	btVector3 orientation (1,0,0);
    	btScalar angle = M_PI/nb_of_positions;

    	for(size_t i=0; i<nb_of_positions; i++){
    		PieSensor* predator_sensor = new PieSensor(spider->getMainBody(),
    				orientation, angle);
    		predator_sensor->setModifier(btVector3(1, 0, 1));
    		predator_sensor->setNeuronPosition(positions[i]);
    		predator_sensor->setActiveColor(Color(1.0, 0.5, 0.5, 0.5));
    		spider->getLastSensorGroup().add(predator_sensor);
    		orientation = orientation.rotate(up, angle*2);
    	}
    }
#endif

    return spider;
}


// Sets
inline void setPos(const btScalar& x, const btScalar& y, const btScalar& z,
        const btScalar& x_vis, const btScalar& y_vis, const btScalar& z_vis,
		size_t& neuron_index)
{
	dbg::assertion(DBG_ASSERTION(!std::isnan(x)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(x)));
	dbg::assertion(DBG_ASSERTION(!std::isnan(y)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(y)));
	dbg::assertion(DBG_ASSERTION(!std::isnan(z)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(z)));
	dbg::assertion(DBG_ASSERTION(!std::isnan(x_vis)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(x_vis)));
	dbg::assertion(DBG_ASSERTION(!std::isnan(y_vis)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(y_vis)));
	dbg::assertion(DBG_ASSERTION(!std::isnan(z_vis)));
	dbg::assertion(DBG_ASSERTION(!std::isinf(z_vis)));
    Params::dnn::spatial::_x[neuron_index] = x;
    Params::dnn::spatial::_y[neuron_index] = y;
    Params::dnn::spatial::_z[neuron_index] = z;
    Params::visualisation::_x[neuron_index] = x_vis;
    Params::visualisation::_y[neuron_index] = y_vis;
    Params::visualisation::_z[neuron_index] = z_vis;
    dbg::out(dbg::info, "neuronpos") << "index: " << neuron_index <<
            " x: " << Params::dnn::spatial::_x[neuron_index] <<
            " y: " << Params::dnn::spatial::_y[neuron_index] <<
            " z: " << Params::dnn::spatial::_z[neuron_index] << std::endl;
    ++neuron_index;
}

inline void setPos(const btScalar& x, const btScalar& y, const btScalar& z,
		size_t& neuron_index){
    setPos(x, y, z, x, y, z, neuron_index);
}

inline void setPos(const btScalar& x, const btScalar& z, size_t& neuron_index){
    setPos(x, 0, z, x, 0, z, neuron_index);
}

inline void setPos(const btScalar& x, const btScalar& z, const btScalar& x_vis,
		const btScalar& z_vis, size_t& neuron_index){
    setPos(x, 0, z, x_vis, 0, z_vis, neuron_index);
}



void joint_init(){
    dbg::trace trace("init", DBG_HERE);
    if(Params::init) return;
    typedef NeuronGroups<Params> neu_grp_t;

    // Determine the number of bins
    int nr_of_tasks = 0;
#if defined(FORWARD)
    dbg::out(dbg::info, "obj") << "Forward task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(BACKWARD)
    dbg::out(dbg::info, "obj") << "Backward task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(TURNLEFT)
    dbg::out(dbg::info, "obj") << "Turn left task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(TURNRIGHT)
    dbg::out(dbg::info, "obj") << "Turn right task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(JUMP)
    dbg::out(dbg::info, "obj") << "Jump task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(GETDOWN)
    dbg::out(dbg::info, "obj") << "Crouch task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(RANDOMFOOD)
    dbg::out(dbg::info, "obj") << "Food task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(RETINA)
    dbg::out(dbg::info, "obj") << "Retina task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(PREDATOR)
    dbg::out(dbg::info, "obj") << "Predator task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(TURNTO)
    DBOO << "Turn-to task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(MOVETO)
    DBOO << "Move-to task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(STRIKE)
    DBOO << "Strike task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(ATTACK)
    DBOO << "Attack task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(TURNFROM)
    DBOO << "Turn from task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(RETREAT)
    DBOO << "Retreat task defined" << std::endl;
    nr_of_tasks += 1;
#endif
#if defined(FIGHTFLIGHT)
    DBOO << "Fight-or-flight task defined" << std::endl;
    nr_of_tasks += 1;
#endif
    dbg::out(dbg::info, "obj") << "Total tasks: " << nr_of_tasks << std::endl;
    Params::cmoea::nb_of_bins = pow(2, nr_of_tasks) - 1;

    int nb_of_objs = 0;
#if defined(NSGA_PUSH_ALL_TASKS)
    DBOO << "Push all tasks defined" << std::endl;
    nb_of_objs += nr_of_tasks;
#if defined(NSGA_PUSH_COMBINED_OBJECTIVE)
    nb_of_objs += 1;
#endif
#else
    nb_of_objs += 1;
#endif

#if defined(NCONNS)
    DBOO << "NCONNS defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(NNODES)
    DBOO << "NNODES defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(NCNODES)
    DBOO << "NCNODES defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(MOD)
    DBOO << "MOD defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(HMOD)
    DBOO << "HMOD defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(CPPNMOD)
    DBOO << "CPPNMOD defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(MAX_LENGTH)
    DBOO << "MAX_LENGTH defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(LENGTH)
    DBOO << "LENGTH defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(CPPNLENGTH)
    DBOO << "CPPNLENGTH defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(PREDOBJ)
    DBOO << "PREDOBJ defined" << std::endl;
    nb_of_objs += 1;
#endif
#if defined(DIV) || defined(AGE) || defined(NOVMAP)
    DBOO << "DIV or AGE or NOVMAP defined" << std::endl;
    nb_of_objs += 1;
#endif
    dbg::out(dbg::info, "obj") << "Total objectives: " << nb_of_objs << std::endl;
    Params::nsga3::nb_of_objs = nb_of_objs;

    // Choose a number of reference points closest to the population size
    // Probably not the most efficient way to calculate the reference point delta,
    // but given that it only needs to be calculated once, it doesn't seem necessary to
    // optimize at this point
    size_t partitions = 1;
    size_t nb_of_ref_points = compare::nCr(nb_of_objs + partitions - 1, partitions);
    size_t prev_nb_of_ref_points = 0;
    while(nb_of_ref_points < Params::pop::size){
    	prev_nb_of_ref_points = nb_of_ref_points;
    	partitions += 1;
    	nb_of_ref_points = compare::nCr(nb_of_objs + partitions - 1, partitions);
    }
    size_t abs_diff_curr_ref = nb_of_ref_points - Params::pop::size;
    size_t abs_diff_prev_ref = Params::pop::size - prev_nb_of_ref_points;
    std::cout << "nb_of_ref_points: " << nb_of_ref_points << " prev_nb_of_ref_points: " << prev_nb_of_ref_points << std::endl;
    if((abs_diff_prev_ref <= abs_diff_curr_ref) && prev_nb_of_ref_points > 0){
    	partitions -= 1;
    	nb_of_ref_points = prev_nb_of_ref_points;
    }
    Params::nsga3::ref_points_delta = 1.0/double(partitions);
    std::cout << "NSGA-III Number of partitions: " << partitions
    		<< " Ref. point delta: " << Params::nsga3::ref_points_delta
			<< " Number of ref. points: " << nb_of_ref_points << std::endl;

//    if(nb_of_objs == 6 && Params::pop::size == 6300){
//    	// (6+12-1) choose 12 = 6188
//    	Params::nsga3::ref_points_delta = 1.0/12.0;
//    } else if(nb_of_objs == 6 && Params::pop::size == 4){
//        	// (6+1-1) choose 1 = 6
//        	Params::nsga3::ref_points_delta = 1.0;
//    } else {
//    	dbg::sentinel(DBG_HERE);
//    }

    //Create neuron object and set its properties
    neu_grp_t neuronGroups;
#if defined(ONE_OUTPUT_SET_PER_MODULE)
    neuronGroups.setSeparateIndexPerLayer(false);
    neuronGroups.setHiddenCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setOutputCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setInputToHiddenCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setHiddenToOutputCppnIndex(neu_grp_t::sourceIndex);
#elif defined(NO_MULTI_SPATIAL_SUBSTRATE)
    neuronGroups.setSeparateIndexPerLayer(false);
    neuronGroups.setSeparateIndexPerLayerNeurons(false);
    neuronGroups.setHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setOutputCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setInputToHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setHiddenToOutputCppnIndex(neu_grp_t::staticIndex);
#elif defined(SHAREDOUTPUTS)
    neuronGroups.setSeparateIndexPerLayer(true);
    neuronGroups.setHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setOutputCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setInputToHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(neu_grp_t::staticIndex);
    neuronGroups.setHiddenToOutputCppnIndex(neu_grp_t::targetIndex);
#else
    neuronGroups.setSeparateIndexPerLayer(true);
    neuronGroups.setHiddenCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setOutputCppnIndex(neu_grp_t::sourceIndex);
    neuronGroups.setInputToHiddenCppnIndex(neu_grp_t::combinatorialIndex);
    neuronGroups.setHiddenToHiddenCppnIndex(neu_grp_t::combinatorialIndex);
    neuronGroups.setHiddenToOutputCppnIndex(neu_grp_t::combinatorialIndex);
#endif


    //Create simulator
    Params::simulator = new sim_t();
    Params::simulator->initPhysics();


    //Create spider
    robot_t* spider = init_spider();

    //Set planes and connect them
    size_t nb_sensor_groups = spider->getNumberOfSensorGroups();

    DBOI << "Number of sensor groups: " << nb_sensor_groups << std::endl;
    for(size_t i=0; i<nb_sensor_groups; ++i){
    	size_t nb_sensors = spider->getNumberOfSensorsInGroup(i);
    	DBOI << "Group " << i << ": " << nb_sensors << " sensors." << std::endl;
        neuronGroups.addInput(nb_sensors);
    }

#if defined(ADD_ONE_HIDDEN_PLANE_PER_NEURON_IN_BATCH)
    for(size_t i=0; i<Params::dnn::nb_layers; ++i){
    	for(size_t i=0; i<Params::dnn::nb_neurons_per_batch; ++i){
    		neuronGroups.addHidden(Params::dnn::nb_neuron_batches);
    	}
    }

#elif defined(ADD_ONE_HIDDEN_PLANE_PER_BATCH)
    for(size_t i=0; i<Params::dnn::nb_layers; ++i){
    	for(size_t i=0; i<Params::dnn::nb_neuron_batches; ++i){
    		neuronGroups.addHidden(Params::dnn::nb_neurons_per_batch);
    	}
    }
#else
    neuronGroups.addHidden(Params::dnn::nb_of_hidden);
#endif

    //////////////////////////////////////////////////////////////
    neuronGroups.addOutput(robot_t::knee_joint_count);
    neuronGroups.addOutput(robot_t::hip_pitch_joint_count);
    neuronGroups.addOutput(robot_t::hip_yaw_joint_count);



    // Connect input to hidden
#if defined(INPUT_HIDDEN_ONE_TO_ONE)
    if(neuronGroups.getInputToHiddenCppnIndex() == NeuronGroups<Params>::combinatorialIndex){
        neuronGroups.setInputToHiddenCppnIndex(NeuronGroups<Params>::sourceIndex);
    }
    neuronGroups.connectInputToHiddenOneToOne();
#elif defined(INPUT_HIDDEN_ONE_TO_ALL)
    neuronGroups.connectInputToHiddenFull();
#else
#error "No connectivity pattern defined from input to hidden"
#endif

    // Connect hidden to hidden
#if defined(HIDDEN_TO_HIDDEN_WITHIN)
    if(neuronGroups.getHiddenToHiddenCppnIndex() == NeuronGroups<Params>::combinatorialIndex){
        neuronGroups.setHiddenToHiddenCppnIndex(NeuronGroups<Params>::sourceIndex);
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

    // Set the number of inputs for the dnn
    Params::dnn::nb_inputs = spider->getNumberOfSensors();
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, layers, 3);

    Params::dnn::spatial::_layers[0] = Params::dnn::nb_inputs;
    Params::dnn::spatial::_layers[1] = Params::dnn::nb_of_hidden;
    Params::dnn::spatial::_layers[2] = Params::dnn::nb_outputs;
    size_t nb_of_neurons = Params::dnn::nb_inputs + Params::dnn::nb_of_hidden +
    		Params::dnn::nb_outputs;

    DBOI << "layers: " << Params::dnn::spatial::layers_size() << std::endl;
    DBOI << " layer 0: " << Params::dnn::spatial::layers(0) << std::endl;
    DBOI << " layer 1: " << Params::dnn::spatial::layers(1) << std::endl;
    DBOI << " layer 2: " << Params::dnn::spatial::layers(2) << std::endl;

    //Set the neuron positions
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, z, nb_of_neurons);

    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, x, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, y, nb_of_neurons);
    SFERES_INIT_NON_CONST_ARRAY(Params::visualisation, z, nb_of_neurons);

    //Set the positions for the input layer
    //float vis_dist_mod = 0.35;
    float vis_dist_mod = 0.0;
    size_t neuron_index = 0;
    for(size_t group_i=0; group_i<nb_sensor_groups; ++group_i){
        mod_robot::SensorGroup sensor_group = spider->getSensors(group_i);
        for(size_t input_i=0; input_i<sensor_group.size(); ++input_i){
            btVector3 position = sensor_group[input_i]->getNeuronPosition();
            assert(!std::isinf(position.x()));
            assert(!std::isnan(position.x()));
            assert(!std::isinf(position.y()));
            assert(!std::isnan(position.y()));
            assert(!std::isinf(position.z()));
            assert(!std::isnan(position.z()));
            setPos(position.x(), position.y(), position.z(), neuron_index);
//            Params::dnn::spatial::_x[neuron_index] = position.x();
//            Params::dnn::spatial::_y[neuron_index] = position.y();
//            Params::dnn::spatial::_z[neuron_index] = position.z();
#if defined(INPUT_VISUALIZE_GROUP_OFFSETS)
            setPos(position.x(),
            		position.y(),
					position.z(),
					position.x() + position.x()*vis_dist_mod*group_i,
					position.y(),
					position.z() + position.z()*vis_dist_mod*group_i,
					neuron_index);
//            Params::visualisation::_x[neuron_index] = position.x() +
//            		position.x()*vis_dist_mod*group_i;
//            Params::visualisation::_y[neuron_index] = position.y();
//            Params::visualisation::_z[neuron_index] = position.z() +
//            		position.z()*vis_dist_mod*group_i;
//#else
//            Params::visualisation::_x[neuron_index] = position.x();
//            Params::visualisation::_y[neuron_index] = position.y();
//            Params::visualisation::_z[neuron_index] = position.z();
#endif
//            ++neuron_index;
        }
    }


#if defined(MINRAD05)
    btScalar minRadius = 0.5;
#else
    btScalar minRadius = 0.5;
#endif

#if defined(MAXRAD10)
    btScalar maxRadius = 1.0;
#else
    btScalar maxRadius = 1.0;
#endif


#if defined(CIRCLE_HIDDEN)
    {
        size_t nrOfRings = Params::dnn::nb_neuron_batches;
        size_t neuronsPerRing = Params::dnn::nb_neurons_per_batch;
        btVector3 orientation (1,0,0);
        btScalar angle = (2*M_PI)/neuronsPerRing;
        btScalar radiusIncr = (maxRadius - minRadius) / btScalar(nrOfRings - 1);

        dbg::out(dbg::info, "neuronpos") << "Rings: " << nrOfRings <<
                " neurons per ring: " << neuronsPerRing << std::endl;

        for(size_t i=0; i<neuronsPerRing; i++){
            btScalar currentRadius = minRadius;
            for(size_t j=0; j < nrOfRings; ++j){
                setPos(orientation.getX()*currentRadius,
                        orientation.getZ()*currentRadius,
                        orientation.getX()*currentRadius*(1+vis_dist_mod),
                        orientation.getZ()*currentRadius*(1+vis_dist_mod),
                        neuron_index);
                currentRadius += radiusIncr;
            }
            orientation = orientation.rotate(btVector3(0,1,0), angle);
        }
    }
#elif defined(GRID_HIDDEN)
    {
        size_t nb_of_columns = Params::dnn::nb_neuron_batches;
        size_t nb_of_rows = Params::dnn::nb_neurons_per_batch;
        size_t nb_of_layers = Params::dnn::nb_layers;
        size_t nb_input_layers = getInputLayers();
        size_t total_layers = nb_of_layers + nb_input_layers + 1;
        btScalar layer_incr = 2.0f/btScalar(total_layers-1);
        btScalar layer_y = 1 - nb_input_layers*layer_incr;

        dbg::out(dbg::info, "neuronpos") << "Rows: " << nb_of_rows <<
        		" columns: " << nb_of_columns <<
				" layers: " << nb_of_layers << std::endl;

        float x_pos_current = 0.0f;
        float x_pos_min = -1.0f;
        float x_pos_max = 1.0f;
        float x_pos_increment = 0.0f;
        if(nb_of_columns > 1){
            x_pos_increment = (x_pos_max - x_pos_min) /float(nb_of_columns - 1);
        } else if(nb_of_columns == 1){
            x_pos_min = 0.0f;
            x_pos_max = 0.0f;
        } else {
            dbg::sentinel(DBG_HERE);
        }
        dbg::out(dbg::info, "neuronpos") << "X min: " << x_pos_min <<
        		" max: " << x_pos_max <<
				" incr: " << x_pos_increment << std::endl;

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
        dbg::out(dbg::info, "neuronpos") << "Z min: " << z_pos_min <<
        		" max: " << z_pos_max <<
				" incr: " << z_pos_increment << std::endl;


        float y_pos_current = 0.0f;
        float y_pos_min = -0.5f;
        float y_pos_max = 0.5f;
        float y_pos_increment = 0.0f;
        if(nb_of_layers > 1){
            y_pos_increment = (y_pos_max - y_pos_min) / float(nb_of_layers - 1);
        } else if(nb_of_layers == 1){
            y_pos_min = 0.0f;
            y_pos_max = 0.0f;
        } else {
            dbg::sentinel(DBG_HERE);
        }
        dbg::out(dbg::info, "neuronpos") << "Y min: " << y_pos_min <<
        		" max: " << y_pos_max <<
				" incr: " << y_pos_increment << std::endl;


//        y_pos_current = y_pos_min;
        for(size_t layer=0; layer < nb_of_layers; ++layer){
        	x_pos_current = x_pos_min;
        	for(size_t column =0; column < nb_of_columns; ++column){
        		z_pos_current = z_pos_min;
        		for(size_t row =0; row < nb_of_rows; ++row){
        			setPos(x_pos_current, layer_y, z_pos_current, neuron_index);
        			z_pos_current += z_pos_increment;
        		}
        		x_pos_current += x_pos_increment;
        	}
        	layer_y -= layer_incr;
        }
    }
#else
#error "No hidden configuration defined!"
#endif


    //Set the output layer
    SFERES_CONST unsigned int nb_of_output_positions = 6;
    SFERES_CONST unsigned int nb_of_output_groups = 3;

#if defined(CIRCULAR_OUTPUTS)
    btScalar angle = (M_PI*2)/nb_of_output_positions;
    for(size_t output_group = 0; output_group<nb_of_output_groups; ++output_group){
        btVector3 actuator_orientation (1,0,0);
        for(size_t i=0; i<6; i++){
            btVector3 actuator_position = actuator_orientation + btVector3(0,-1,0);
            Params::dnn::spatial::_x[neuron_index] = actuator_position.x();
            Params::dnn::spatial::_y[neuron_index] = actuator_position.y();
            Params::dnn::spatial::_z[neuron_index] = actuator_position.z();
            Params::visualisation::_x[neuron_index] = actuator_position.x()+
            		actuator_position.x()*vis_dist_mod*output_group;
            Params::visualisation::_y[neuron_index] = actuator_position.y();
            Params::visualisation::_z[neuron_index] = actuator_position.z()+
            		actuator_position.z()*vis_dist_mod*output_group;
            actuator_orientation = actuator_orientation.rotate(btVector3(0,1,0), angle);
            ++neuron_index;
        }
    }
#elif defined(Z_LINE_OUTPUTS)
    SFERES_CONST btScalar z_output_min = -1.0;
    SFERES_CONST btScalar z_output_max = 1.0;
    const btScalar z_output_increment = (z_output_max - z_output_min)/btScalar(nb_of_output_positions-1);
    SFERES_CONST btScalar x_output_min = -1.0;
    SFERES_CONST btScalar x_output_max = 1.0;
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

void print_params(){
    dbg::trace trace("init", DBG_HERE);
    PRINT_PARAMS;
}

void master_init(){
    dbg::trace trace("init", DBG_HERE);

    //Only the master will dump
    print_params();
    joint_init();
#ifdef JHDEBUG
    Params::pop::checkpoint_period = 1;
#else
    Params::pop::checkpoint_period = (Params::pop::nb_gen - 1) /
    		Params::pop::nr_of_dumps;
#endif
    if(options::map.count("load")){
    	//Prevents the creation of a useless result directory
        Params::pop::dump_period = -1;
    } else {
    	//Don't dump, but do write the modularity file
    	//int gen = cont::getNumberOfGenerations<Params>();

//        Params::pop::dump_period = cont::getNumberOfGenerations<Params>() + 1;
        Params::pop::dump_period = 5000;
    }
}

void slave_init(){
    dbg::trace trace("init", DBG_HERE);
    //Initialize the simulator
    joint_init();

    for (size_t i = 0, k = 0; i < Params::dnn::spatial::layers_size(); ++i){
        for (size_t j = 0; j < Params::dnn::spatial::layers(i); ++j, ++k)
        {
            Params::simulator->addNeuronPos(Params::visualisation::x(k),
            		Params::visualisation::y(k),
					Params::visualisation::z(k));
        }
    }


    //Initialize the maps
    Params::nodeConnectionMap = new map_t();
    Params::neuronMap = new map_t();

    //Set local variables
    size_t nb_inputs = Params::dnn::nb_inputs;
    size_t nb_of_hidden = Params::dnn::nb_of_hidden;
    size_t nb_outputs = Params::dnn::nb_outputs;

    //Create a temporary dnn to get connection indices.
    ctrnn_nn_t tempDnn(nb_inputs, nb_of_hidden, nb_outputs);

    //Set map for the inputs
    for(uint first_i=0; first_i<nb_inputs; first_i++){
        for(uint second_i=0; second_i<nb_of_hidden; second_i++){
            std::string key = inputToHiddenKey(first_i, second_i);
            uint con_i = tempDnn.getConnectionIndexInput(first_i, second_i);
            dbg::out(dbg::info, "con_map") << key << " " << con_i << std::endl;
            (*Params::nodeConnectionMap)[key] = con_i;
        }
    }

    //Set map for hidden and outputs nodes (there is no difference here between
    //hidden and outputs nodes)
    for(uint first_i=0; first_i < nb_of_hidden + nb_outputs; first_i++){
        for(uint second_i=0; second_i < nb_of_hidden + nb_outputs; second_i++){
            std::string key = hiddenToHiddenKey(Params::dnn::nb_of_hidden, first_i, second_i);
            uint con_i = tempDnn.getConnectionIndexHidden(first_i, second_i);
            dbg::out(dbg::info, "con_map") << key << " " << con_i << std::endl;
            (*Params::nodeConnectionMap)[key] = con_i;
        }
    }

    //Set map from neuron id to neuron index for inputs
    for(uint first_i=0; first_i < nb_inputs; first_i++){
        std::ostringstream stream;
        //This code WILL fail if the network has only a single layer
        //(E.G. inputs are also outputs)
        //Should not happen in any real experiment though
        stream << 'i' << first_i;
        dbg::out(dbg::info, "neur_map") <<
        		"Map input neuron: "<< stream.str() <<
				" to: " << tempDnn.getNeuronIndex(0, first_i)
				<< std::endl;
        (*Params::neuronMap)[stream.str()] = tempDnn.getNeuronIndex(0, first_i);
    }

    //Set hidden neurons
    for(uint first_i=0; first_i < nb_of_hidden + nb_outputs; first_i++){
        std::ostringstream stream;
        //This code WILL fail if the network has only a single layer
        //(E.G. inputs are also outputs)
        //Should not happen in any real experiment though
        if(first_i >= nb_of_hidden){
            stream << 'o' << (first_i - nb_of_hidden);
        } else {
            stream << first_i;
        }
        dbg::out(dbg::info, "neur_map") <<
        		"Map hidden neuron: "<< stream.str() <<
				" to: " << tempDnn.getNeuronIndex(1, first_i)
				<< std::endl;
        (*Params::neuronMap)[stream.str()] = tempDnn.getNeuronIndex(1, first_i);
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

    float inverse_food_dist = inverseDistance(robot_pos.distance(next_food_pos),
    		previous_food_pos.distance(next_food_pos));

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

float calRetreatFitness(sim_t* simulator, btVector3 target_pos){
    btVector3 robot_pos = simulator->getIndivPos();
    btScalar dist = robot_pos.distance(target_pos);
    float fitness = 1 - inverseDistance(robot_pos.distance(target_pos), 10);
    return fitness;
}

float calculateRetinaFitness(sim_t* simulator){
    float inverse_food_dist = getFoodDistFit(simulator);
    float fitness = simulator->getFoodCollected() + inverse_food_dist;
    fitness -= simulator->getPoisonCollected();
    if(fitness < 0) fitness = 0;

    //debug output
    dbg::out(dbg::info, "fitness")
    << " food collected: " << Params::simulator->getFoodCollected()
	<< " poison collected: " << Params::simulator->getPoisonCollected()
    << " distance to closest food: " << inverse_food_dist
    << " total: " << fitness << std::endl;

    return fitness;
}

/**
 * The fitness for the robot moving towards a particular target from the initial
 * position (0, 1, 0).
 *
 * The fitness will be the 1 - (dist_robot_to_target / dist_initial_to_target).
 * The fitness can be negative.
 *
 * When the target is equal to the initial position (or closer than 0.001 to the
 * initial position) the fitness will instead be 1 - dist_robot_to_target.
 */
float calculateTargetFitness(sim_t* simulator, btVector3& target, btScalar area=0){
    btVector3 indiv_position = simulator->getIndivPos();
    btScalar totalDistance = btVector3(0.0, 1.0, 0.0).distance(target);
    if(totalDistance < 0.001){
        totalDistance = 1.0;
    }
    btScalar normalizedDistance = indiv_position.distance(target)/totalDistance;
    normalizedDistance -= area;
    if(normalizedDistance < 0.0) normalizedDistance = 0;
    btScalar fitness = 1-normalizedDistance;

    //debug output
    dbg::out(dbg::info, "tracker") << "Target: " << fitness << std::endl;
    return fitness;
}

float calcAngleToFit(sim_t* simulator, btVector3& target){
    btVector3 indiv_heading = simulator->getIndivHeading();
    indiv_heading.setY(0);
    if(indiv_heading.length() == 0) return 0;
    indiv_heading.normalize();
    btVector3 indiv_position = simulator->getIndivPos();
    btVector3 vector_to_target = target - indiv_position;
    vector_to_target.setY(0);
    if(vector_to_target.length() == 0) return 0;
    vector_to_target.normalize();
    btScalar angle = indiv_heading.angle(vector_to_target);
    //std::cout << "indiv_heading: " << indiv_heading.x() << ", " << indiv_heading.y() << ", " << indiv_heading.z() << std::endl;
    //std::cout << "vector_to_target: " << vector_to_target.x() << ", " << vector_to_target.y() << ", " << vector_to_target.z() << std::endl;
    //std::cout << "Angle: " << angle << std::endl;
    float angle_fit = 1 - (angle/M_PI);
    dbg::out(dbg::info, "tracker") << "Angle to: " << angle_fit << std::endl;
    return angle_fit;
}

float calcAngleFromFit(sim_t* simulator, btVector3& target){
    btVector3 indiv_heading = simulator->getIndivHeading();
    indiv_heading.setY(0);
    if(indiv_heading.length() == 0) return 0;
    indiv_heading.normalize();
    btVector3 indiv_position = simulator->getIndivPos();
    btVector3 vector_to_target = target - indiv_position;
    vector_to_target.setY(0);
    if(vector_to_target.length() == 0) return 0;
    vector_to_target.normalize();
    btScalar angle = indiv_heading.angle(vector_to_target);
    float angle_fit = angle/M_PI;
    dbg::out(dbg::info, "tracker") << "Angle from: " << angle_fit << std::endl;
    return angle_fit;
}


float calculateTargetOrientationFitness(sim_t* simulator,
		btVector3& targetHeading, btScalar targetComponent){
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

template<typename SimT>
class VisMananger{
public:
	VisMananger(SimT* simulator, std::string custom_text = ""){
#if defined(VISUALIZE)
		_simulator = simulator;
		_custom_text = custom_text;
        if(!boost::filesystem::exists(VISUALIZE_FONT)){
        	std::cerr << "ERROR: Font file: " << VISUALIZE_FONT
        			<< " does not exist!" << std::endl;
        }
        _simulator->setMovieFolder(options::map["movie"].as<std::string>());
        //_simulator->setCameraDistance(5);
        _simulator->setFollowRobot(false);
        _font = new OGLFT::Filled(VISUALIZE_FONT, 36, 200, 2 );
        _font->setHorizontalJustification( OGLFT::Face::CENTER );
        _font->setForegroundColor( 1, 1, 1, 1. );
        _outline = new OGLFT::Outline(VISUALIZE_FONT, 36, 200, 2 );
        _outline->setHorizontalJustification( OGLFT::Face::CENTER );
        _outline->setForegroundColor( 0.2, 0.2, 0.2, 1. );
        _shadow = new OGLFT::TranslucentTexture(VISUALIZE_FONT, 36, 200, 2, 4);
        _shadow->setHorizontalJustification( OGLFT::Face::CENTER );
        _shadow->setForegroundColor( 0, 0, 0, 1);
        //            shadow->setForegroundColor( 0, 0, 0, 1);
#endif
	}

	~VisMananger(){
#if defined(VISUALIZE)
		delete _font;
		delete _outline;
		delete _shadow;
#endif
	}

	void setText(std::string prefix, std::string text){
#if defined(VISUALIZE)
		if(_custom_text != ""){
			text = _custom_text;
		}
		size_t w_width = 1024;
		size_t w_height = 768;
		_simulator->setWindowSize(w_width, w_height);
		_simulator->showText(false);
		_simulator->neurons2d(false);
		_simulator->setFramePrefix(prefix);
        opengl_draw::Text* text_ = new opengl_draw::Text(text, w_width/2, 20);
        text_->setFont(_font);
        text_->setOutline(_outline);
        text_->setShadow(_shadow);
        _simulator->getOverlay()->addDrawableChild(text_);
#endif
	}
private:
#if defined(VISUALIZE)
	SimT* _simulator;
	OGLFT::Filled* _font;
	OGLFT::Outline* _outline;
	OGLFT::TranslucentTexture* _shadow;
	std::string _custom_text;
#endif
};

//SFERES_FITNESS(FitRobot, sferes::fit::Fitness) {
using namespace stc;
//template <typename ParamsT = stc::_Params, typename Exact = stc::Itself>
//class FitRobot : public sferes::fit::Fitness<Params,  FitRobot>{
SFERES_FITNESS(FitRobot, sferes::fit::Fitness) {
public:
    FitRobot() : _postFix("") { // @suppress("Class members should be properly initialized")

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
        dbg::out(dbg::info, "fitness") << "Evaluating individual" << std::endl;

        // Get type definitions
        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::graph_t graph_t;
        typedef typename Indiv::cppn_graph_t cppn_graph_t;
        typedef typename Indiv::nn_t::neuron_t neuron_t;
        typedef typename Indiv::nn_t::neuron_t::af_t::params_t neu_bais_t;
        typedef typename Indiv::nn_t::weight_t weight_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;

        // Start the clock
        _eval_clock.resetAndStart();

        // Reset values to be calculated
        _food_fitness = 0.0f;
        _predator_fitness = 0.0f;
        _fp_fitness = 0.0f;
        _ff_fitness = 0.0f;
        _hidden_mod = 0;
		_mod= 0;
		_lrc_hierarchy= 0;
		_hour_hierarchy= 0;
		_cppn_mod= 0;
		_cppn_lrc_hierarchy= 0;
		_cppn_hour_hierarchy= 0;
		this->_nb_conns= 0;
		this->_nb_nodes= 0;
		_length= 0;
        this->_value = 0;
        this->_objs.clear();
#if defined(DIVHAMMING)
        _behavior.clear();
#endif

        // Define local variables
        //Pointer to the local spider robot
        robot_t* spider;
        //Vector for storing the fitness values of different tasks (for CMOEA)
        std::vector<double> fitness;
        //Temporary variable for storing local fitness
        float taskFitness;
        //Index tracking which input should be used for which task
        size_t neuron_index = 0;
        VisMananger<sim_t> visManager(Params::simulator, _custom_text);
        nn_t nnsimp;
        graph_t gsimp;
        ctrnn_nn_t* dnn = 0;
        cppn_graph_t cppn_graph;

        // Set world seed
#if !defined(TESTFIT)
        // Test fit simply give a random fitness to each individual
        // The world seed would cause every individual to have the same fitness
        // Note: this also kills determinism in parallel scenarios
        misc::pushRand();
        misc::seed(ind.gen().get_world_seed());
        dbg::out(dbg::info, "seed") << "World seed received: " <<
        		ind.gen().get_world_seed() << std::endl;
#endif

        // Get graphs and networks
# if not defined(NONETWORK)
        dbg::out(dbg::info, "fit") << "Creating neural network" << std::endl;
        ind.nn().init();
        dbg::out(dbg::info, "fit") << "Simplifying graph" << std::endl;
        nnsimp = ind.nn().simplified_nn();
        gsimp = nnsimp.get_graph();
        dnn = buildCtrnn(nnsimp);
        cppn_graph = ind.cppn().get_graph();
#endif


# if not defined(NOMETRICS)
        //Calculate metrics
        _hidden_mod = calculateHiddenModularity(gsimp);
        _mod = calculateModularity(gsimp);
        _lrc_hierarchy = calculateHiddenHierarchy(gsimp);
        _hour_hierarchy = calcHiddenHourHierarchy(gsimp);
        _cppn_mod = calculateModularity(cppn_graph);
        _cppn_lrc_hierarchy = hierar::Get_GRC_Unweighted(cppn_graph);
        _cppn_hour_hierarchy = hierar::getHourglassHierarchy(cppn_graph);

        //Set number of connections, neurons and length
        this->_nb_conns = boost::num_edges(gsimp);
        this->_nb_nodes = boost::num_vertices(gsimp);
        _length = ind.nn().compute_length();
#endif // NOMETRICS


#if defined(VISUALIZE)
        setNeuronVisData(ind);
        Params::simulator->pause(options::map["pause"].as<bool>());
#endif

        // Start the simulator clock
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



        // Define target tracker
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

            TargetTracker(btVector3 target, direction targetDirection):
            	target(target), targetDirection(targetDirection)
            {
                reset();
            }

            void track()
            {
                float fit = calculateTargetFitness(Params::simulator, target);
                if(fit > maxFitness){
                    maxFitness = fit;
                }
                sum_food_dist+=fit;
                btVector3 indiv_heading = Params::simulator->getIndivHeading();
                indiv_heading.normalize();
                indiv_heading.setY(0);
                btScalar originalLength = indiv_heading.length();
                if(originalLength > 0.0){

                    //Check that the robot is still upright
                    btVector3 up(0, 1, 0);
                    btVector3 cUp = Params::simulator->getIndividual()->getUp();
                    btScalar angleToUp = cUp.angle(up);

                    indiv_heading.normalize();

                    btScalar angle = indiv_heading.angle(prev_heading);
                    direction currentDirection;
                    if(indiv_heading.cross(prev_heading).y() > 0.0){
                        currentDirection = LEFT;
                    } else {
                        currentDirection = RIGHT;
                    }

                    if(currentDirection == targetDirection){
                        //Check that the robot is still upright
                        if(upright(Params::simulator)){
                        	DBOT << "Angle added: " << angle << std::endl;
                            totalAngle+=angle;
                        } else {
                        	DBOT << "Robot is not upright, angle to up: " <<
                        			angleToUp << std::endl;
                        }
                    } else {
                    	DBOT << "Angle subtracted: " << angle << std::endl;
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


        // Define angle tracker
        class AngleTracker: public mod_robot::Tracker<sim_t> {
        public:
            enum goal_t{
                TO = 0,
                FROM = 1
            };

            AngleTracker(btVector3 target): target(target){
                reset();
            }

            void track(){
            	++count;

            	// Ensure that the robot is upright
            	if(!upright(Params::simulator)) return;

            	// Calculate the fitness for staying at the starting location
            	btVector3 start = btVector3(0, 1, 0);
            	float stay_fit = calculateTargetFitness(Params::simulator, start, 1);

            	// Calculate the angle fitness
            	float angle_fit;
            	if(goal == TO){
            		angle_fit = calcAngleToFit(Params::simulator, target);
            	} else {
            		angle_fit = calcAngleFromFit(Params::simulator, target);
            	}
            	dbg::out(dbg::info, "angle") << angle_fit << std::endl;
                sum_fit += (stay_fit + angle_fit) / 2;
//                sum_fit += angle_fit;
            }

            void reset(){
                sum_fit = 0.0;
                count = 0;
                goal = TO;
            }

            float average(){
                return sum_fit/(float)count;
            }

            void setGoal(goal_t g_){
            	goal = g_;
            }


            float sum_fit;
            int count;
            btVector3 target;
            goal_t goal;
        };

        // Define fitness tracker
        class FitTracker: public mod_robot::Tracker<sim_t> {
        public:
            void track()
            {
                sum_food_dist+=getFoodDistFit(Params::simulator);
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


#if defined(PREDATOR)
        FitTracker* tracker = new FitTracker();
        Params::simulator->setTracker(tracker);
        std::vector<double> food_fitness;
        std::vector<double> predator_fitness;
        Params::simulator->setReplaceFood();
        Params::simulator->setReplacePredator();

#if defined(SPAWNFRONT)
        Params::simulator->setSpawnPredatorFront();
#endif

        for(int i = 0; i < Params::sim::trials; i++){
            //Setup trial
            tracker->reset();
            Params::simulator->reset();
            dnn->reset();
            spider = init_spider();
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
            food_fitness.push_back(Params::simulator->getFoodCollected() +
            		tracker->average());
            dbg::out(dbg::info, "fitness")
            << " food collected: " << Params::simulator->getFoodCollected()
            << " average food closeness: " << tracker->average()
            << " total: " << Params::simulator->getFoodCollected() +
			tracker->average() << std::endl;
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
        _fp_fitness = (_food_fitness + _predator_fitness*4) / 3;
        DBOF << "Food-predator fitness: " << _fp_fitness << std::endl;
        fitness.push_back(_food_fitness);

#if defined(NSGA_PUSH_ALL_TASKS)
        	DBOO << "Obj pushed: collect food " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        delete tracker;
#endif //PREDATOR


#if defined(RANDOMFOOD)
        {
            //Move forwards
        	DBOF << "Collect food..." << std::endl;
        	std::vector<double> trial_fit;
        	for(int i = 0; i < Params::sim::trials; i++){
        		//Setup the run for this trial
        		Params::simulator->reset();
            	Params::simulator->setReplaceFood();
            	spider = init_spider();
        		dnn->reset();
        		spider->setNN(dnn);
        		Params::simulator->setIndividual(spider);
        		Params::simulator->addFood();
        		visManager.setText("food", "Collect food");

        		//Run simulation
        		STEP_FUNCTION(Params::sim::steps);

        		//Determine fitness for this trial
        		trial_fit.push_back(calculateFoodFitness(Params::simulator));
        	}
        	_food_fitness = compare::average(trial_fit) / 3;
        	DBOF << "Collect food fitness: " << _food_fitness << std::endl;
        	fitness.push_back(_food_fitness);

#if defined(NSGA_PUSH_ALL_TASKS)
        	DBOO << "Obj pushed: collect food " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // RANDOMFOOD


#if defined(RETINA)
        {
            //Move forwards
        	DBOF << "Collect food retina..." << std::endl;
        	std::vector<double> trial_fit;
        	for(int i = 0; i < Params::sim::trials; i++){
        		//Setup the run for this trial
        		Params::simulator->reset();
            	Params::simulator->setReplaceFood();
            	Params::simulator->setReplacePoison();
            	spider = init_spider();
        		dnn->reset();
        		spider->setNN(dnn);
        		Params::simulator->setIndividual(spider);
        		Params::simulator->addFood();
        		Params::simulator->addPoison();
        		visManager.setText("food", "Collect food");
        		STEP_FUNCTION(Params::sim::steps);
        		trial_fit.push_back(calculateRetinaFitness(Params::simulator));
        	}
        	_food_fitness = compare::average(trial_fit) / 3;
        	DBOF << "Collect food fitness: " << _food_fitness << std::endl;
        	fitness.push_back(_food_fitness);

#if defined(NSGA_PUSH_ALL_TASKS)
        	DBOO << "Obj pushed: collect food " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // RETINA


#if defined(FORWARD)
        {
            //Move forwards
        	DBOF << "Move forwards..." << std::endl;
        	taskFitness = 0;
#if not defined(TRIALS0)
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
            visManager.setText("forward", "Move forward");
            STEP_FUNCTION(Params::sim::move_steps);
            taskFitness = Params::simulator->getIndivPos().x()/12.5;
#if defined(EFFICIENCY)
            float energy = Params::simulator->getIndividual()->getEnergyUsed();
            float energy_per_step = energy / float(Params::sim::move_steps);
            float efficiency_penalty = (energy_per_step*energy_per_step) / 100000.0f;
            taskFitness -= efficiency_penalty;
            DBOF << "Efficiency penalty: " << efficiency_penalty << std::endl;
#endif
#endif // TRIALS0
            DBOF << "Move forwards fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: forward " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // FORWARD


#if defined(BACKWARD)
        {
            //Move backwards
        	DBOF << "Move backwards..." << std::endl;
        	taskFitness = 0;
#if not defined(TRIALS0)
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
            visManager.setText("backward", "Move backward");

            STEP_FUNCTION(Params::sim::move_steps);
            taskFitness = -Params::simulator->getIndivPos().x()/12.5;
#endif // TRIALS0

            DBOF << "Move backwards fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: backward " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // BACKWARD


#if defined(TURNLEFT)
        {
            //Turn left on the spot
        	DBOF << "Turn left..." << std::endl;
        	taskFitness = 0;
#if not defined(TRIALS0)
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,1,0),
            		TargetTracker::LEFT);
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
            visManager.setText("right", "Turn right");

            STEP_FUNCTION(Params::sim::turn_steps);
            float positionPenalty = 1 - std::min(tracker->average()+1.0, 1.0);
            taskFitness = (tracker->averageAngle()*25 - positionPenalty);
            DBOF << "Turn fitness: "<< tracker->averageAngle()*10  << std::endl;
            DBOF << "Position penalty: " << positionPenalty  << std::endl;
            Params::simulator->deleteTracker();
#endif // TRIALS0
            DBOF << "Turn left fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: turn-left " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // TURNLEFT


#if defined(TURNRIGHT)
        {
            //Turn right on the spot
        	DBOF << "Turn right..." << std::endl;
        	taskFitness = 0;
#if not defined(TRIALS0)
            Params::simulator->reset();
            TargetTracker* tracker = new TargetTracker(btVector3(0,1,0),
            		TargetTracker::RIGHT);
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
            visManager.setText("left", "Turn left");

            STEP_FUNCTION(Params::sim::turn_steps);
            float positionPenalty = 1 - std::min(tracker->average()+1.0, 1.0);
            taskFitness = (tracker->averageAngle()*25 - positionPenalty);
            Params::simulator->deleteTracker();
#endif // TRIALS0
            DBOF << "Turn right fitness: " << taskFitness << std::endl;
            fitness.push_back(taskFitness);
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: turn-right " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // TURNRIGHT


#if defined(JUMP)
        {
            //Get high
        	DBOF << "Get high..." << std::endl;
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
            visManager.setText("jump", "Jump");

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

            DBOF << "Jump fitness: " << jumpFitness <<
                    " land fitness: " << landFitness <<
                    " total fitness: " << totalFitness << std::endl;
            fitness.push_back(totalFitness);
            Params::simulator->deleteTracker();
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: jump " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // JUMP


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
            visManager.setText("crouch", "Crouch");

            STEP_FUNCTION(Params::sim::get_down_steps);
            fitness.push_back(tracker->average());
            Params::simulator->deleteTracker();
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: get down " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // GETDOWN


#if defined(TURNTO)
        {
        	// Implement the turn to task by Dan Lessin
            DBOF << "Turn to..." << std::endl;
            size_t trials = Params::sim::turn_to_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){
                Params::simulator->reset();
                spider = init_spider();
                dnn->reset();
                spider->setNN(dnn);
                Params::simulator->setIndividual(spider);
                Params::simulator->addItem(new mod_robot::Control(neuron_index));
                mod_robot::Target* target = new mod_robot::Target();
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	AngleTracker* tracker = new AngleTracker(target->getPosition());
            	Params::simulator->setTracker(tracker);
            	visManager.setText("turnto", "Turn To");
                STEP_FUNCTION(Params::sim::turn_to_steps);
                angle += angle_increment;
                taskFitness += tracker->average();
                DBOF << "Turn To fit " << i << ": " << tracker->average() << std::endl;
            }
            if(trials) taskFitness /= float(trials);
            fitness.push_back(taskFitness);
            DBOF << "Turn To fitness: " << fitness.back() << std::endl;
            ++neuron_index;
            //Params::simulator->deleteTracker();
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: get down " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // TURNTO


#if defined(MOVETO)
        {
        	// Implement the move to task by Dan Lessin
            DBOF << "Move to..." << std::endl;
            size_t trials = Params::sim::move_to_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){
                Params::simulator->reset();
                spider = init_spider();
                dnn->reset();
                spider->setNN(dnn);
                Params::simulator->setIndividual(spider);
                Params::simulator->addItem(new mod_robot::Control(neuron_index));
                mod_robot::Target* target = new mod_robot::Target();
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	TargetTracker* tracker = new TargetTracker(target->getPosition());
            	Params::simulator->setTracker(tracker);
            	visManager.setText("moveto", "Move To " + boost::lexical_cast<std::string>(i));
                STEP_FUNCTION(Params::sim::move_to_steps);
                angle += angle_increment;
                taskFitness += tracker->average();
            }
            if(trials) taskFitness /= float(trials);
            fitness.push_back(taskFitness);
            DBOF << "Move To fitness: " << fitness.back() << std::endl;
            ++neuron_index;
            //Params::simulator->deleteTracker();
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: get down " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // MOVETO


#if defined(STRIKE)
        {
        	// Implement the strike task by Dan Lessin
            DBOF << "Strike..." << std::endl;
            size_t trials = Params::sim::strike_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
#if not defined(TRIALS0)
            Params::simulator->reset();
            spider = init_spider();
            dnn->reset();
            spider->setNN(dnn);
            Params::simulator->setIndividual(spider);
            Params::simulator->addItem(new mod_robot::Control(neuron_index));
            mod_robot::Target* target = new mod_robot::Target();
            Params::simulator->addItem(target, 0, 0, 0);
            visManager.setText("strike", "Strike");
            STEP_FUNCTION(Params::sim::strike_steps);
            taskFitness = target->getMaxImpulse()/20;
#endif
            fitness.push_back(taskFitness);
            DBOF << "Strike fitness: " << fitness.back() << std::endl;
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: strike " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // STRIKE


#if defined(ATTACK)
        {
        	// Implement the attack task by Dan Lessin
            DBOF << "Attack..." << std::endl;
            size_t trials = Params::sim::attack_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){
            	Params::simulator->reset();
            	spider = init_spider();
            	dnn->reset();
            	spider->setNN(dnn);
            	Params::simulator->setIndividual(spider);
            	Params::simulator->addItem(new mod_robot::Control(neuron_index));
            	mod_robot::Target* target = new mod_robot::Target();
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	visManager.setText("attack", "Attack");
            	STEP_FUNCTION(Params::sim::attack_steps);
            	taskFitness += target->getMaxImpulse()/20;
            	angle += angle_increment;
            }
            if(trials) taskFitness /= float(trials);
            fitness.push_back(taskFitness);
            _ff_fitness = taskFitness;
            DBOF << "Attack fitness: " << fitness.back() << std::endl;
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: attack " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // ATTACK


#if defined(TURNFROM)
        {
        	// Implement the turn from task by Dan Lessin
            DBOF << "Turn from..." << std::endl;
            size_t trials = Params::sim::turn_from_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){
                Params::simulator->reset();
                spider = init_spider();
                dnn->reset();
                spider->setNN(dnn);
                Params::simulator->setIndividual(spider);
                Params::simulator->addItem(new mod_robot::Control(neuron_index));
                mod_robot::Target* target = new mod_robot::Target(true);
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	AngleTracker* tracker = new AngleTracker(target->getPosition());
            	tracker->setGoal(AngleTracker::FROM);
            	Params::simulator->setTracker(tracker);
            	visManager.setText("turnfrom", "Turn From");
                STEP_FUNCTION(Params::sim::turn_from_steps);
                angle += angle_increment;
                taskFitness += tracker->average();
            }
            if(trials) taskFitness /= float(trials);
            fitness.push_back(taskFitness);
            ++neuron_index;
            //Params::simulator->deleteTracker();
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: turn from " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // TURNFROM


#if defined(RETREAT)
        {
        	// Implement the retreat task by Dan Lessin
            DBOF << "Retreat..." << std::endl;
            size_t trials = Params::sim::retreat_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){
                Params::simulator->reset();
                spider = init_spider();
                dnn->reset();
                spider->setNN(dnn);
                Params::simulator->setIndividual(spider);
                Params::simulator->addItem(new mod_robot::Control(neuron_index));
                mod_robot::Target* target = new mod_robot::Target(true);
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	visManager.setText("retreat", "Retreat");
                STEP_FUNCTION(Params::sim::retreat_steps);
                angle += angle_increment;
                taskFitness += calRetreatFitness(Params::simulator,
                		target->getPosition());
            }
            if(trials) taskFitness /= float(trials);
            fitness.push_back(taskFitness);
            _ff_fitness *= taskFitness;
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: retreat " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // RETREAT


#if defined(FIGHTFLIGHT)
        {
        	// Implement the retreat task by Dan Lessin
            size_t trials = Params::sim::fight_flight_trails;
            double angle = 0;
            double angle_increment = 360/double(trials);
            taskFitness = 0;
            for(size_t i=0; i<trials; ++i){

            	// Fight
            	Params::simulator->reset();
            	spider = init_spider();
            	dnn->reset();
            	spider->setNN(dnn);
            	Params::simulator->setIndividual(spider);
            	Params::simulator->addItem(new mod_robot::Control(neuron_index));
            	mod_robot::Target* target = new mod_robot::Target();
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	visManager.setText("attack", "Fight");
            	STEP_FUNCTION(Params::sim::fight_flight_steps);
            	taskFitness += target->getMaxImpulse()/20;

            	// Flight
                Params::simulator->reset();
                spider = init_spider();
                dnn->reset();
                spider->setNN(dnn);
                Params::simulator->setIndividual(spider);
                Params::simulator->addItem(new mod_robot::Control(neuron_index));
                target = new mod_robot::Target(true);
            	Params::simulator->addItemAngleDistance(target, angle, 5.0);
            	visManager.setText("fightflight", "Flight");
                STEP_FUNCTION(Params::sim::fight_flight_steps);
                angle += angle_increment;
                taskFitness += calRetreatFitness(Params::simulator,
                		target->getPosition());
            }
            if(trials) taskFitness /= float(trials*2);
            fitness.push_back(taskFitness);
            ++neuron_index;
#if defined(NSGA_PUSH_ALL_TASKS)
            DBOO << "Obj pushed: fightflight " << fitness.back() << std::endl;
            this->_objs.push_back(fitness.back());
#endif
        }
#endif // FIGHTFLIGHT


#if defined(DUMFIT)
        DBOF << "Dummy fitness: 0" << std::endl;
        for(size_t i=0; i<Params::cmoea::nb_of_bins; ++i){
            this->_innov_categories.push_back(0);
            DBOF <<  this->_innov_categories.back() << std::endl;
        }
        fitness.push_back(0);
#endif // DUMFIT


#if defined(TESTFIT)
        DBOF << "Fitness: " << std::endl;
        for(size_t i=0; i<Params::cmoea::nb_of_bins; ++i){
            this->_innov_categories.push_back(misc::rand<float>());
            DBOF <<  this->_innov_categories.back() << std::endl;
        }
        DBOF << std::endl;

        dbg::assertion(DBG_ASSERTION(_innov_categories.size() == Params::cmoea::nb_of_bins));
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::cmoea::obj_index));
        fitness.push_back(_innov_categories.front());
#endif // TESTFIT


        /***********************
         * CONSOLIDATE FITNESS *
         ***********************/
        /* most significant bit -> least significant bit
         * 000000000 -> all objectives (0)
         * 111111111 -> no objectives (511)
		 * 010111111 -> Attack and Retreat (191)
		 * 011111111 -> Retreat (511 - 256 = 255)
		 * 101111111 -> Turn from (511 - 128 = 383)
		 * 110111111 -> Attack (511 - 64 = 447)
		 * 111011111 -> Strike (511 - 32 = 479)
		 * 111101111 -> Move to (511 - 16 = 495)
		 * 111110111 -> Turn to (511 - 8 = 503)
		 * 111111011 -> Turn right (511 - 4 = 507)
		 * 111111101 -> Turn left (511 - 2 = 509)
		 * 111111110 -> Forward (511 - 1 = 510)
         */

#if defined(XTASKS)
        _innov_categories.clear();
        std::vector<float> fitness_combination;
        for(size_t i=0; i<Params::cmoea::nb_of_bins; ++i){
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
            DBOF << "obj: " << i << " fitness " << local_fitness << std::endl;
            _innov_categories.push_back(local_fitness);
        }
        dbg::assertion(DBG_ASSERTION(_innov_categories.size() == Params::cmoea::nb_of_bins));

#if not defined(NSGA_PUSH_ALL_TASKS)
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::cmoea::obj_index));
#endif
        this->_value = _innov_categories.front();
#else // XTASKS
        this->_value = compare::average(fitness);
        DBOF << "Average fitness: " << this->_value << std::endl;
#endif // XTASKS


        /************************
         * BEHAVIORAL DIVERSITY *
         ************************/
#if defined(DIVREACT) and not defined(NONETWORK)
        dbg::out(dbg::info, "eval_behavior") << "##### STARTING BEHAVIOR #####" << vec_to_string(_behavior) << std::endl;
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            dnn->clear();
            Params::simulator->reaction(dnn, _behavior, i, 5);
        }
        dbg::out(dbg::info, "eval_behavior") << "behavior "<<":\n" << vec_to_string(_behavior) << std::endl;
#endif // DIVREACT


#if defined(DIVMINAVG) and not defined(NONETWORK)
        dbg::out(dbg::info, "eval_behavior") << "##### STARTING BEHAVIOR #####" << vec_to_string(_behavior) << std::endl;

        std::vector<double> count;
        count.assign(Params::fitness::nb_bits, 0);
        SFERES_CONST size_t time_steps = 5;

        dbg::out(dbg::info, "eval_behavior") << "Bits: " << Params::fitness::nb_bits
                << " inputs: " << Params::dnn::nb_inputs
                << " time-steps: " << time_steps << std::endl;
        for(size_t i=0; i<Params::dnn::nb_inputs; ++i){
            dbg::out(dbg::info, "eval_behavior") << "Input " << i << std::endl;
            dnn->clear();
            Params::simulator->reaction(dnn, _behavior, i, time_steps);
            for(size_t k=1; k <= time_steps; ++k){
                for(size_t j=0; j < Params::fitness::nb_bits; ++j){
                    if(_behavior[_behavior.size() - k][j]){
                        count[j] += 1.0;
                    }
                }
            }
        }

        dbg::out(dbg::info, "eval_behavior") << "count:\n" << vec_to_string(count) << std::endl;
        dbg::out(dbg::info, "eval_behavior") << "original behavior:\n" << vec_to_string(_behavior) << std::endl;

        std::bitset<Params::fitness::nb_bits> median_response;
        for(size_t j=0; j<Params::fitness::nb_bits; ++j){
            median_response[j] = (count[j]/_behavior.size() > 0.5);
        }

        for(size_t i=0; i<_behavior.size(); ++i){
            _behavior[i] ^= median_response;
        }
        dbg::out(dbg::info, "eval_behavior") << "median behavior "<<":\n" << median_response << std::endl;
        dbg::out(dbg::info, "eval_behavior") << "adjusted behavior "<<":\n" << vec_to_string(_behavior) << std::endl;
#endif // DIVMINAVG


        /**********************
         ******* CLEANUP ******
         **********************/
        _sim_clock.stop();

#if not defined(NONETWORK)
        delete dnn;
#endif

        //Set time
        _nn_time = Params::simulator->getNNTime();
        _physics_time = Params::simulator->getPhysicsTime();
        _reset_time = Params::simulator->getResetTime();


        /**********************
         ***** MAP-ELITES *****
         **********************/
#if defined(MAPELITE)
      size_t desc_index = 0;

#if defined(DESC_HMOD)
      dbg::out(dbg::info, "desc") << "Behavioral descriptor " << desc_index <<
    		  " is _hidden_mod: " << _hidden_mod << std::endl;
      _behavioral_descriptors[desc_index++] = _hidden_mod;
#endif

#if defined(DESC_CPPNMOD)
      dbg::out(dbg::info, "desc") << "Behavioral descriptor " << desc_index <<
    		  " is _cppn_mod: " << _cppn_mod << std::endl;
      _behavioral_descriptors[desc_index++] = _cppn_mod;
#endif

      dbg::assertion(DBG_ASSERTION(desc_index == 2));
#endif

        /**********************
         ***** OBJECTIVES *****
         **********************/

        //Push back the primary objective objective
#if defined(PREDOBJ)
        DBOO << "Objective pushed: _food_fitness " << _food_fitness << std::endl;
        this->_objs.push_back(_food_fitness);
#elif not defined(NSGA_PUSH_ALL_TASKS) or defined(NSGA_PUSH_COMBINED_OBJECTIVE)
        DBOO << "Objective pushed: this->_value " << this->_value << std::endl;
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
        DBOO << "Objective pushed: num_edges " << -(int)boost::num_edges(gsimp) << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_edges(gsimp));
#endif

#ifdef NNODES
        DBOO << "Objective pushed: num_vertices " << -(int)boost::num_vertices(gsimp) << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_vertices(gsimp));
#endif

#ifdef NCNODES
        DBOO << "Objective pushed: num_edges and num_vertices "
        		<< -(int)boost::num_edges(gsimp) - (int)boost::num_vertices(gsimp) << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-(int)boost::num_edges(gsimp) - (int)boost::num_vertices(gsimp));
#endif

#ifdef MOD
        DBOO << "Objective pushed: _mod " << _mod << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_mod);
#endif

#ifdef HMOD
        DBOO << "Objective pushed: _hidden_mod " << _hidden_mod << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_hidden_mod);
#endif

#ifdef CPPNMOD
        DBOO << "Objective pushed: _cppn_mod " << _cppn_mod << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(_cppn_mod);
#endif

#ifdef MAX_LENGTH
        DBOO << "Objective pushed: max_length " << -ind.gen().max_length() << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-ind.gen().max_length());
#endif

#ifdef LENGTH
        DBOO << "Objective pushed: LENGTH " << -_length << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
        this->_objs.push_back(-_length);
#endif

#ifdef CPPNLENGTH
        DBOO << "Objective pushed: cppn length " << -ind.cppn().get_nb_connections() << std::endl;
//        dbg::assertion(DBG_ASSERTION(this->_objs.size() == 1));
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
        DBOO << "Objective pushed: _predator_fitness " << _predator_fitness << std::endl;
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
        DBOO << "Objective pushed: diversity dummy -1" << std::endl;
        this->_objs.push_back(-1);
#endif

        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::nsga3::nb_of_objs));

        /*************************************
         ********** VISUALIZATIONS ***********
         *************************************/

        if (this->mode() == sferes::fit::mode::view ||
        		this->mode() == sferes::fit::mode::usr1)
        {
            //Do any calculation here
            std::cout << "Performing visualizations..." << std::endl;
            regularity_score = 0;

            if (this->mode() == sferes::fit::mode::view) {
            	visManager.setText("nn", "");
                visualizeNetwork(ind);
                std::ofstream nn_csv(("nn"+ _postFix +".csv").c_str());
                ind.nn().write_csv(nn_csv);
                nn_csv.close();
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
//        using namespace io;


        //Print the network to PNG if the right libraries are available
#if defined(PNGW)
        ind.nn().init();
        nn_t nn = ind.nn().simplified_nn();
        ctrnn_nn_t* dnn = buildCtrnn(nn);
        Params::simulator->reset();
        setNeuronVisData(ind);
        dnn->reset();
        robot_t* spider = init_spider();
        spider->setNN(dnn);
        Params::simulator->setIndividual(spider);
        Params::simulator->print_3d_network("nn3d" + _postFix + ".png");
        Params::simulator->stepsVisual(3);
#endif

//        SFERES_CONST size_t no_objs = 63;
        SFERES_CONST size_t obj_forward = 62;
        SFERES_CONST size_t obj_backward = 61;
        SFERES_CONST size_t obj_turnleft = 59;
        SFERES_CONST size_t obj_turnright = 55;
        SFERES_CONST size_t obj_jump = 47;
        SFERES_CONST size_t obj_down = 31;

        if(_postFix == "") _setPostfix();
        nn_t nnsimp = ind.nn();
        nnsimp.simplify();

        std::cout << "behavior:\n" << vec_to_string(_behavior) << std::endl;
        std::cout << "- Performance: " << this->_value << std::endl;

        std::cout << "- Forward   : " << this->cat(obj_forward) << std::endl;
        std::cout << "- Backward  : " << this->cat(obj_backward) << std::endl;
        std::cout << "- Turn left : " << this->cat(obj_turnleft) << std::endl;
        std::cout << "- Turn right: " << this->cat(obj_turnright) << std::endl;
        std::cout << "- Jump      : " << this->cat(obj_jump) << std::endl;
        std::cout << "- Get down  : " << this->cat(obj_down) << std::endl;


        std::cout << "- Modularity: " << _mod << std::endl;
        std::cout << "- length: " << _length << std::endl;
        std::cout << "- nb neurons: " << ind.nn().get_nb_neurons() << std::endl;
        std::cout << "- nb conns: "<<ind.nn().get_nb_connections() << std::endl;
        std::cout << "- reg: " << regularity_score << std::endl;

        std::ofstream ofs2(("nn"+ _postFix +".dot").c_str());
        ind.nn().write_dot(ofs2);

        std::ofstream ofs3(("nn"+ _postFix +".svg").c_str());
        SvgWriter normal_writer;
        normal_writer.switchAxis(1,2);
        normal_writer.setScale(100);
        normal_writer.setPlotBias();
        normal_writer.setRectangles();
        std::map<vertex_desc_t, gen::spatial::Pos> coords_map;
        coords_map = nnsimp.getVisualizationCoordsmap();
        nnsimp.write_svg(ofs3, normal_writer, coords_map);

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
                std::string key = hiddenToHiddenKey(Params::dnn::nb_of_hidden, node, k);
                dnn->setConnection((*Params::nodeConnectionMap)[key], 1.0);
            }
        }

        for(size_t i=0; i<Params::dnn::nb_of_hidden; ++i){
            std::string key = hiddenToHiddenKey(Params::dnn::nb_of_hidden, i, i);
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
        Params::simulator->resetVisData();

        typedef typename Indiv::nn_t nn_t;
        typedef typename Indiv::nn_t::graph_t graph_t;
        typedef typename Indiv::nn_t::neuron_t neuron_t;
        typedef typename Indiv::nn_t::neuron_t::af_t::params_t neu_bais_t;
        typedef typename Indiv::nn_t::weight_t weight_t;
        typedef typename Indiv::nn_t::vertex_desc_t vertex_desc_t;


        nn_t nnsimp = ind.nn();
        nnsimp.simplify();


        std::vector<std::string> mods;
        std::vector<SvgColor> colors;


        bool hidden_modules_only = false;
        if(hidden_modules_only){
            //Option 1
            std::vector<bool> removed;
            mod::split(extractHiddenLayer(nnsimp.get_graph(), removed), mods);
            std::vector<SvgColor> temp_colors = getColors(mods);

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
        }else{


            //Option 2
            mod::split(nnsimp.get_graph(), mods);
            colors = getColors(mods);
            //End option 2
        }
        dbg::out(dbg::info, "visualize") << "Colors: "<< colors.size() << std::endl;


        std::map<vertex_desc_t, gen::spatial::Pos> coords_map = nnsimp.getVisualizationCoordsmap();

        size_t i = 0;
        BGL_FORALL_VERTICES_T(v, nnsimp.get_graph(), graph_t){

            dbg::out(dbg::info, "visualize") <<
            		"Node: " << i <<
					" coordinate: " << coords_map[v].x() <<
					" "<< coords_map[v].y() <<
					" " << coords_map[v].z() << std::endl;

            neuron_t node = nnsimp.get_graph()[v];
            double r = (((double)colors[i].r)/255.0);
            double g = (((double)colors[i].g)/255.0);
            double b = (((double)colors[i].b)/255.0);

            Params::simulator->addNeuronVisData(
            		coords_map[v].x(),
					coords_map[v].y(),
					coords_map[v].z(),
					r, g, b,
					(*Params::neuronMap)[node.get_id()]);
            ++i;
        }
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
    inline double calculateHiddenHierarchy(const G& g){
        std::vector<bool> __i_dont_care__;
        G hiddenOnlyGraph = extractHiddenLayer(g, __i_dont_care__);
        return hierar::Get_GRC(hiddenOnlyGraph);
    }

    template<typename G>
    inline double calcHiddenHourHierarchy(const G& g){
        std::vector<bool> __i_dont_care__;
        G hiddenOnlyGraph = extractHiddenLayer(g, __i_dont_care__);
        return hierar::getHourglassHierarchy(hiddenOnlyGraph);
    }

    std::vector<SvgColor> getColors(const std::vector<std::string>& mods){
        //Assign color to modular split
        //We probably want to move these elsewhere
        std::vector<SvgColor> colors;
        std::vector<SvgColor> color_presets(8);
        color_presets[0] = (SvgColor(255, 0, 0));
        color_presets[1] = (SvgColor(0, 255, 0));
        color_presets[2] = (SvgColor(0, 255, 255));
        color_presets[3] = (SvgColor(255, 0, 255));
        color_presets[4] = (SvgColor(255, 255, 0));
        color_presets[5] = (SvgColor(0, 0, 255));
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

    double get_lrc_hier() const { return _lrc_hierarchy; }

    double get_hour_hier() const { return _hour_hierarchy; }

    double get_cppn_lrc_hier() const { return _cppn_lrc_hierarchy; }

    double get_cppn_hour_hier() const { return _cppn_hour_hierarchy; }

#if defined(MAPELITE)
    const boost::array<float, 2>& desc() const{return _behavioral_descriptors; }
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
        //std::cout << indivs_serialized << std::endl;
        //indivs_serialized+=1;
        dbg::out(dbg::info, "serialize") << "Serializing: individual parent parameters" << std::endl;
        sferes::fit::Fitness<Params, typename stc::FindExact<FitRobot<Params, Exact>, Exact>::ret>::serialize(ar, version);
        dbg::out(dbg::info, "serialize") << "Serializing: individual core parameters" << std::endl;
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
        dbg::out(dbg::info, "serialize") << "Serializing: _innov_categories" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_innov_categories);
#endif
#if defined(INNOV_DIVERSITY)
        dbg::out(dbg::info, "serialize") << "Serializing: _innov_diversity" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_innov_diversity);
#endif
#if defined(INNOV_CLOSEST_DIST)
        dbg::out(dbg::info, "serialize") << "Serializing: _closest_dist" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_closest_dist);
#endif
#if defined(HIDDEN_LAYER_MODULARITY)
        dbg::out(dbg::info, "serialize") << "Serializing: _hidden_mod" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_hidden_mod);
#endif
#if defined(CPPN_MODULARITY)
        dbg::out(dbg::info, "serialize") << "Serializing: _cppn_mod" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_cppn_mod);
#endif
#if defined(MAPELITE)
        dbg::out(dbg::info, "serialize") << "Serializing: _behavioral_descriptors" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_behavioral_descriptors);
#endif
#if defined(HIERARCHY)
        dbg::out(dbg::info, "serialize") << "Serializing: HIERARCHY" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_lrc_hierarchy);
        ar & BOOST_SERIALIZATION_NVP(_hour_hierarchy);
        ar & BOOST_SERIALIZATION_NVP(_cppn_lrc_hierarchy);
        ar & BOOST_SERIALIZATION_NVP(_cppn_hour_hierarchy);
#endif
#if defined(SERIALIZE_FOOD_PREDATOR)
        dbg::out(dbg::info, "serialize") << "Serializing: _fp_fitness" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_fp_fitness);
#endif
#if defined(SERIALIZE_FIGHT_FLIGHT)
        dbg::out(dbg::info, "serialize") << "Serializing: _ff_fitness" << std::endl;
        ar & BOOST_SERIALIZATION_NVP(_ff_fitness);
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

    float getFPFitness(){
    	return _fp_fitness;
    }

    float getFfFitness(){
    	return _ff_fitness;
    }

    float getDiv(){
        return this->obj(this->objs().size()-1);
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
// New style
    std::vector<float> &getBinFitnessVector(){return _innov_categories;}
    float getBinFitness(size_t index){ return _innov_categories[index];}

// Old style
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

public:
    void setCustomText(std::string custom_text){
    	_custom_text = custom_text;
    }
protected:

#if defined(INNOV_DIVERSITY)
public:

// New style
    std::vector<float> &getBinDiversityVector(){return _innov_diversity;}
    float getBinDiversity(size_t index){ return _innov_diversity[index];}
    void initBinDiversity(){
        if(_innov_diversity.size() != Params::cmoea::nb_of_bins){
            _innov_diversity.resize(Params::cmoea::nb_of_bins);
        }
    }
    void setBinDiversity(size_t index, float div){
        dbg::check_bounds(dbg::error, 0, index, _innov_diversity.size(), DBG_HERE);
        _innov_diversity[index] = div;
    }


// Old style
    void initDiv(){
        if(_innov_diversity.size() != Params::cmoea::nb_of_bins){
            _innov_diversity.resize(Params::cmoea::nb_of_bins);
        }
    }

//    std::vector<float>& getDiv(){
//        return _innov_diversity;
//    }

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
    float _fp_fitness;
    float _ff_fitness;

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

    double _lrc_hierarchy;
    double _hour_hierarchy;
    double _cppn_lrc_hierarchy;
    double _cppn_hour_hierarchy;

    std::string _custom_text;

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
size_t Params::cmoea::nb_of_bins = 0;

size_t Params::nsga3::nb_of_objs = 0;
float Params::nsga3::ref_points_delta = 0.0;

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

    //The fitness function is defined by the FitRetina class defined above
    //Using the parameters also defined above

    std::cout << "Setting slave callback" <<std::endl;
    Params::mpi::slaveInit = &slave_init;
    std::cout << "Setting master callback" <<std::endl;
    Params::mpi::masterInit = &master_init;

    typedef FitRobot<Params> fit_t;
    typedef fit::FitDummy<> fd_t;

    //This determines the type of weights used for the substrate
    //Note that these weights are always evolvable even thought they will not be
    //evolved in the case of hnn.
    typedef gen::EvoFloat<1, Params::weights> w_gen_t;
    typedef phen::Parameters<w_gen_t, fd_t, Params::weights> substrate_weight_t;

    //This determines the attributes of the activation function which are the
    //bias and the time constant.
    typedef gen::EvoFloat<1, Params::bias> b_gen_t;
    typedef phen::Parameters<b_gen_t, fd_t, Params::bias> substrate_af_bias_t;
    typedef gen::EvoFloat<1, Params::time_constant> t_gen_t;
    typedef phen::Parameters<t_gen_t, fd_t, Params::time_constant> sub_af_tc_t;
    typedef gen::TwoParameters<substrate_af_bias_t, sub_af_tc_t> c_gen_t;
    typedef ctrnn_phen::ParametersCTRNN<c_gen_t, fd_t, Params> sub_af_params_t;

    //Defines the Potential function of our Neural Network as the Weighted Sum.
    typedef PfWSum<substrate_weight_t> pf_t;

    //Defines the Activation function of our Neural Network as a -1/+1 Sigmoid
    //function (defined above). It takes the parameter values defined in bias,
    //probably named this way because the bias is the only
    //Part that is actually evolved about the neurons.
    typedef Af<sub_af_params_t> af_t;

    // Use the Direct Neural Network as a genotype, were each neuron has the
    // PfWSum Potential function and the Sigmoid activation function. Each
    // connection uses the weights parameters, and the entire network uses
    // Params.
    typedef Neuron<pf_t, af_t> nue_t;
    typedef Connection<substrate_weight_t> con_t;
    typedef sferes::gen::DnnSpatial<nue_t, con_t, Params> substrate_t;


#ifdef HNN
    //Set weight type

#ifdef REGULATOR
    typedef phen::Parameters<gen::EvoFloat<1, Params::ParamsHnn::cppn>,fit::FitDummy<>,Params::ParamsHnn::cppn> cppn_weight_t;
    typedef nn::PfWReg<cppn_weight_t, nn::PfRegParams<Params::ParamsHnn::regulator> > cppn_pf_t;
    typedef nn::AfDummy<AfDummyParams<> > cppn_af_t;
#elif defined(REGULATORY_CONNECTION)
    typedef nn::RegConnWeight<Params::ParamsHnn::regulatory_conn::weight_param> cppn_weight_t;
    typedef nn::PfWRegConn<cppn_weight_t, nn::PfRegConnParams<Params::ParamsHnn::regulatory_conn::pf_param> > cppn_pf_t;
    typedef nn::AfDummy<> cppn_af_t;
#else
    typedef phen::Parameters<gen::EvoFloat<1, Params::ParamsHnn::cppn>,fit::FitDummy<>,Params::ParamsHnn::cppn> cppn_weight_t;
    typedef nn::PfWSum<cppn_weight_t> cppn_pf_t;
    typedef nn::AfCppn<nn::cppn::AfParams<Params::ParamsHnn::cppn> > cppn_af_t;
#endif

    typedef gen::DnnFF<nn::Neuron<cppn_pf_t, cppn_af_t>, nn::Connection<cppn_weight_t>, Params::ParamsHnn> cppn_t;

    //Set the genotype to HyperNEAT
    typedef gen::HnnWS<cppn_t, Params::ParamsHnn, Params::ParamsHnn::cppn> gen_t;
#endif


    //Set the pheontype
#if defined(CTRNN_WINNER_TAKES_ALL)
    typedef phen::hnn::HnnCtrnnWinnerTakesAll<substrate_t, gen_t, fit_t, Params::ParamsHnn> raw_phen_t;
#elif defined(CTRNN_MULTIPLE_OUTPUTSETS_SUM)
    typedef phen::hnn::HnnCtrnnSum<substrate_t, gen_t, fit_t, Params::ParamsHnn> raw_phen_t;
#elif defined(NOLEO)
    typedef phen::hnn::HnnCtrnn<substrate_t, gen_t, fit_t, Params::ParamsHnn> raw_phen_t;
#elif defined(HNN)
    typedef phen::hnn::HnnCtrnnLeo<substrate_t, gen_t, fit_t, Params::ParamsHnn> raw_phen_t;
#else
    //The phenotype is the direct Neural network, using the genotype, fitness function and parameters defined above.
    typedef phen::Dnn<substrate_t, fit_t, Params> raw_phen_t;
#endif

#if defined(MPCMOEA)
    typedef ea::crowd::Indiv<raw_phen_t> phen_t;
#else
    typedef raw_phen_t phen_t;
#endif

    typedef boost::fusion::vector<
#if defined(STAT_FIGHT_FLIGHT)
		stat::ModRobotFf<phen_t, Params>,
		stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(SEVENTASKS)
    	stat::ModRobotFood<phen_t, Params>,
		stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(LEXICASE) or defined(ELEXICASE)
    	stat::RenderVideoPopOnly<phen_t, Params>,
        stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(CMOEA)
        stat::ModRobotMapEliteInovDiv<phen_t, Params>,
        stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(MAPELITE) and (defined(XTASKS) || defined(TESTFIT))
        stat::Map<phen_t, Params>,
        stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(MPCMOEA)
    	stat::StorePop<phen_t, Params>,
    	stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(NSGA3)
    	stat::StorePop<phen_t, Params>,
        stat::StatNsga3<phen_t, Params>,
        stat::ModRobotInnovFirstBin<phen_t, Params>
#elif defined(XTASKS)
        stat::ModRobot<phen_t, Params>,
        stat::ModRobotInnovFirstBin<phen_t, Params>
#else
        //stat::ModRobot<phen_t, Params>
        stat::ModRobotInnovFirstBin<phen_t, Params>
#endif
#ifndef NORANDSERIALIZED
        ,stat::SerializeRand<phen_t, Params>
#endif
    >  stat_t;

#if defined(STAT_FIGHT_FLIGHT)
    stat::ModRobotFf<phen_t, Params>::initOptions();
#endif

    //Set modifiers
#if defined(DIV) && !defined(MAPELITEINOVDIV)
    //Behavioral distance based only on current population.
    typedef modif::BehaviorDiv<> modifier_t;
#elif defined(NOVMAP) 	//Novelty search map based on behavior.
    typedef modif::Novmap<> modifier_t;
#else					//No modifier
    typedef boost::fusion::vector<> modifier_t;
//    typedef modif::Dummy<> modifier_t;
#endif


    //typedef eval::Parallel<Params> eval_t;
#if defined(MPI_ENABLED) and !defined(NOMPI) and defined(CMOEA) and !defined(MPCMOEA)
    typedef ea::ArchiveTask<phen_t, modifier_t, Params> archive_task_t;
    typedef eval::EvalTask<phen_t> eval_task_t;
    typedef boost::fusion::vector<eval_task_t, archive_task_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
#elif defined(LEXICASE) or defined(ELEXICASE)
    // Do not define anything
#elif defined(MPI_ENABLED) and !defined(NOMPI) and !defined(MPCMOEA)
    typedef eval::EvalTask<phen_t> eval_task_t;
    typedef boost::fusion::vector<eval_task_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
#else
    typedef eval::EvalWorldTracking<Params> eval_t;
#endif


    //Set the ea
#if defined(PNSGA)
    //Use the probabilistic variant of NSGA 2
    typedef ea::Pnsga<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
    //#elif defined(MAPELITEINOVDIV)
    //typedef ea::MapEliteInovDiv<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    //typedef ea::ContinueMapEliteInovDiv<ea_t, Params> continue_ea_t;
#elif defined(ELEXICASE) and (!defined(MPI_ENABLED) or defined(NOMPI))
    std::cout << "Running e-Lexicase without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(ELEXICASE) and defined(MPI_ENABLED) and !defined(NOMPI)
    std::cout << "Running e-Lexicase with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_tasks_t;
    typedef boost::fusion::vector<eval_tasks_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(LEXICASE) and (!defined(MPI_ENABLED) or defined(NOMPI))
    std::cout << "Running Lexicase without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(LEXICASE) and defined(MPI_ENABLED) and !defined(NOMPI)
    std::cout << "Running Lexicase with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_tasks_t;
    typedef boost::fusion::vector<eval_tasks_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(MAPELITEINOVPNSGA)
#if defined(MPI_ENABLED) and !defined(NOMPI)
    typedef ea::CmoeaNsga2Mpi<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
#else
    typedef ea::CmoeaNsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
#endif
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(MAPELITE)
    typedef ea::MapElite<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueMapElite<ea_t, Params> continue_ea_t;
#elif defined(MPCMOEA)
    typedef ea::MPCmoeaNsga2Mpi<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(NSGA)
    //Use the multi-objective evolutionary algorithm NSGA 2
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(NSGA3)
    std::cout << "Running NSGA-III" << std::endl;
    typedef ea::nsga3::Nsga3<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef ea::ContinueNsga3<ea_t, Params> continue_ea_t;
#else
    std::cout << "ERROR: No EA defined!" << std::endl;
    typedef sferes::ea::Ea<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
    typedef sferes::ea::ContinueEa<ea_t, Params> continue_ea_t;
#endif

    //Add options
    namespace opt = boost::program_options;
    options::add()("seed", opt::value<int>(), "program seed");
    options::positionalAdd("seed", 1);
    options::add()("print-neuron-groups",
    		opt::value<bool>()->default_value(false)->implicit_value(true),
			"Print neuron groups.");
    options::add()("movie",
    		opt::value<std::string>()->default_value("movie"),
			"Dump all movie images in the folder with this name.");
    options::add()("rank",
    		opt::value<int>()->default_value(-1),
			"Run as a specific rank when not using mpi.");
    options::add()("pause",
    		opt::value<bool>()->default_value(true),
			"Determines whether the simulation starts paused.");


#if defined(CMOEA)
    stat::ModRobotMapEliteInovDiv<phen_t, Params>::initOptions();
#elif defined(XTASKS)
    stat::ModRobot<phen_t, Params>::initOptions();
#endif
    //Add the option to continue
    cont::init();
    options::parse_and_init(argc, argv, false);


    cont::Continuator<continue_ea_t, Params> continuator;

    //Create the evolutionary algorithm
    continue_ea_t ea;

    // The pathname for the current instance
    std::string root = ".";
#ifdef MPCMOEA
    //continuator.setDeleteCheckpoints(false);
    int rank;
    if(options::map.count("rank") && options::map["rank"].as<int>() > -1){
    	rank = options::map["rank"].as<int>();
    } else{
    	rank = ea.get_rank();
    }
    root = "inst_" + boost::lexical_cast<std::string>(rank);
    boost::filesystem::path inst_path(root);
    boost::filesystem::create_directory(inst_path);
    std::string res_path = root + "/" + misc::date() + "_" + misc::getpid();
    ea.set_res_dir(res_path);
    continuator.setSearchDepth(1);
    continuator.setSearchDir(root);

    set_seed(ea.get_rank());
//    if(ea.get_rank() == 0){
//        print_params();
//    }
#else
    set_seed();
#endif

    //This will cause all slaves to enter the slave loop if MPI is defined
    //Prevents a lot of useless work, and makes debugging easier.
    fit_t _proto_fit;
    std::vector<boost::shared_ptr<phen_t> > empty;
    ea.eval().eval(empty, 0, 0, _proto_fit);

#if defined(ADD_EQUAL_WEIGHT_REFERENCE_POINT)
    Eigen::VectorXd target(Params::nsga3::nb_of_objs);
    target.fill(1.0 / double(Params::nsga3::nb_of_objs));
    ea.preferred_reference_points().push_back(target);
#endif


#ifdef RENDERNETWORK
#ifdef FORCEDMODULES
    const bool artificially_split_phenotype = true;
#else
    const bool artificially_split_phenotype = false;
#endif

#ifdef FMODH
    const bool artificially_split_genotype = true;
#else
    const bool artificially_split_genotype = false;
#endif
    visualizeNetworkBullet<Params, robot_t, ctrnn_nn_t>(init_spider(),
    		artificially_split_phenotype,
			artificially_split_genotype);
    return 0;
#endif


    //Start the run
    continuator.run(ea);

#if defined(MAPELITE)
    std::string fname = ea.res_dir() + "/archive.dat";
    if(fname == "/archive.dat") fname = "archive.dat";
    boost::fusion::at_c<0>(ea.stat()).write_archive(fname, ea);
#endif

//#if defined(MPCMOEA)
    // Don't delete anything
//#else
    //Do some cleanup after the run if necessary
    if(continuator.performCleanup()){
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "modularity.dat");
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "first_bin.dat");
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "innov_all_bin.dat");
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "nsga3.dat");
        continuator.deleteCheckpoints();
        continuator.moveGenFiles(ea.res_dir());
        sferes::io::deleteEmptyDirectories(continuator.getSearchDir());
    }
//#endif
    return 0;
}
