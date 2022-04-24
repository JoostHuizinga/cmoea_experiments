// TODO: Try multiplicative fitness (implemented)


/***********
 * DEFINES *
 ***********/

// Currently, stat_cmoea tracks the population, so sferes doesn't have to.
#define SFERES_NO_STATE

//Since I expect to constantly keep adding information to the serialize function
//I think it useful to track versions, rather than adding a new pre-processor macro
//every time I add something. This will be version 1, the previous version will be 0,
//and the next version will be 2, and so on.
#if defined(MRV1)
// Intentionally left empty
#endif

#if defined(MRV2)
#define SERIALIZE_MAZES_SOLVED
#endif

// Eigen vectorization caused results on Mount Moran to be different from the
// results on my laptop. Disabling vectorization seems to have a really dramatic
// impact on performance. Let's try with vectorization this time.
//#define EIGEN_DONT_VECTORIZE

/* Aliases and shorthands */

// Shorthand for cmoea bin sampling
#if defined(CMOEASAMP)
#define BIN_SAMPLING
#endif

// Shorthand for behavioral diversity based on end points
#if defined(DIV)
#define BEHAVIORAL_DIVERSITY
#endif

// Shorthand for stop the evaluation when the goal is reached
#if defined(STPGOAL)
#define STOP_AT_GOAL
#endif

// Shorthand for multiplicative fitness within bins
#if defined(MULT)
#define MULTIPLICATIVE_FITNESS
#endif

// Shorthand for fixed starting location
#if defined(FSTART)
#define FIXED_START
#endif

// Shorthand for opaque walls, the goal sensor can not see through walls
#if defined(OPQWALL)
#define OPAQUE_WALLS
#endif

// Definitions for guided NSGA-II
#if defined(GNSGA)
#define NSGA
#define NSGA_PUSH_ALL_TASKS
#define NSGA_PUSH_COMBINED_OBJECTIVE
#endif

#if defined(GLEXICASE)
#define LEXICASE
#define NSGA_PUSH_ALL_TASKS
#define NSGA_PUSH_COMBINED_OBJECTIVE
#endif

//Setup 5, controls for C-MOEA
#if defined(NPAT)
#define NSGA_PUSH_ALL_TASKS
#endif

#if defined(NPCO)
#define NSGA_PUSH_COMBINED_OBJECTIVE
#endif

// Shorthand for normalizing fitness by dividing by the maximum possible
// distance to the goal (As opposed to diving by the distance from the starting
// location to the goal).
#if defined(MDIST)
#define MAXIMUM_DISTANCE
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

/* Derived defines */

// If manual control is requested, visualizations have to be turned on
#if defined(MANUAL)
#define VISUALIZE
#endif

// Don't include SDL if we don't want to visualize
#if not defined(VISUALIZE) and defined(USE_SDL)
#undef USE_SDL
#endif

// Behavioral diversity is a form of diversity, so all diversity 
// dependent pieces of code should be enabled
#if defined(BEHAVIORAL_DIVERSITY)
#define DIVERSITY
#endif

#if defined(NOMPI)
#undef MPI_ENABLED
#endif

// Special debug flag that disables all forms of parallelization
// and it disables the 'equal population' flag making NSGA-II
// run much faster
#if defined(JHDEBUG)
#define NO_PARALLEL
#undef MPI_ENABLED
#elif not defined(NOEQPOP)
#define EQPOP
#endif

// The number of MPI tasks required by our evolutionary algorithm.
// TODO: Find a better way of determining the number of tasks present
#if defined(MPI_ENABLED) and defined(CMOEA)
#define NR_OF_MPI_TASKS 2
#elif defined(MPI_ENABLED) and defined(BIN_SAMPLING)
#define NR_OF_MPI_TASKS 2
#elif defined(MPI_ENABLED)
#define NR_OF_MPI_TASKS 1
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


/* Checks and warnings */

// Make sure RLS and DBG are not defined simultaneously
#if defined(RLS) and defined(DBG)
#error "You can not define both DBG and RLS."
#endif

// Make sure the number of generations is defined
#ifndef GEN
#warning "Number of generations is not defined."
#define GEN 0
#endif

// Make sure the number of generations is defined
#ifndef POP
#warning "Population size is not defined."
#define POP 0
#endif

// The default number of simulation steps for this experiment is 2500
#ifndef STEPS
#define STEPS 2500
#endif

#ifndef MAZES
#define MAZES 10
#endif

#ifndef BIN
#define BIN 10
#endif

#ifndef BINS
#define BINS 1023
#endif

/************
 * INCLUDES *
 ************/

// Include standard
#include <iostream>
#include <fstream>

// Include boost
#include <boost/archive/binary_iarchive.hpp>
#include <boost/algorithm/string/replace.hpp>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/fusion/container.hpp>


// Include SDL
#ifdef USE_SDL
#include <SDL.h> //only include SDL if visualization is requested
#endif

#ifdef PNGW
#include <modules/pngwriter/pngwriter_util.hpp>
#endif

// Include sferes
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/best_fit.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>

// Include modules
#include <modules/fastsim2/simu_fastsim.hpp>
#include <modules/nn2/phen_dnn.hpp>
#include <modules/nn2/gen_dnn.hpp>
#include <modules/continue/continue_run.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>
#include <modules/continue/rand.hpp>
#include <modules/datatools/params.hpp>
#include <modules/datatools/io_convenience.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/stat_clock.hpp>
#include <modules/mpiworldtrack/core.hpp>
#include <modules/randmaze/recursive_split.hpp>
#include <modules/randmaze/chambers_to_map.hpp>
#include <modules/nsgaext/dom_sort_no_duplicates.hpp>
#include <modules/nsgaext/nsga2_custom.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/cmoea/cmoea_util.hpp>
#include <modules/cmoea/cmoea_nsga2.hpp>
#include <modules/cmoea/cmoea_bin_sampling.hpp>
#include <modules/cmoea/continue_cmoea_nsga2.hpp>
#include <modules/lexicase/lexicase.hpp>
#include <modules/lexicase/e_lexicase.hpp>
#include <modules/nsga3/nsga3.hpp>
#include <modules/nsga3/continue_ea_nsga3.hpp>
#include <modules/nsga3/stat_nsga3.hpp>
#if defined(MPI_ENABLED)
#include <modules/cmoea/cmoea_nsga2_mpi.hpp>
#include <modules/cmoea/cmoea_bin_sampling_mpi.hpp>
#endif

// Include local
#include "stat_cmoea.hpp"
#include "stat_testset.hpp"


/* NAMESPACES */
using namespace sferes;
using namespace sferes::gen::evo_float;


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
///////////* Parameters for the experiment *////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// Define based parameters
SFERES_TYPE_VALUE_PARAM(NbGenerations, static const size_t v, GEN)
SFERES_TYPE_VALUE_PARAM(PopSize, static const size_t v, POP)
SFERES_TYPE_VALUE_PARAM(NbSteps, static const size_t v, STEPS)
SFERES_TYPE_VALUE_PARAM(NbMazes, static const size_t v, MAZES)
SFERES_TYPE_VALUE_PARAM(BinSize, static const size_t v, BIN)
SFERES_TYPE_VALUE_PARAM(NrBins, static const size_t v, BINS)

/* Parameters for Sferes */
SFERES_PARAMS (
struct Params {
    // Bool to keep track of whether we have initialized yet
    static bool init;
    // This value is required by the sferes simu class,
    // but it is actually never used by the simulator.
    struct simu {
        SFERES_CONST float dt = 0.0f;
        // Linked to number of bins; if changed, change num_bins as well
        SFERES_CONST uint num_mazes = NbMazes::v;

#ifdef JHDEBUG
        SFERES_CONST uint num_testset_mazes = 40;
#else
        SFERES_CONST uint num_testset_mazes = 1000;
#endif
        // The seed for generating mazes
        SFERES_CONST uint maze_seed = 1;
        static std::vector<boost::shared_ptr<randmaze::Maze> > mazes;
        static std::vector<boost::shared_ptr<randmaze::Maze> > testset_mazes;

        // 6 laser range-finders
        SFERES_CONST uint num_lasers = 6;
        // 4 pie-slice goal sensors
        SFERES_CONST uint num_slices = 4;
        // The maximum motor value allowed on the robot
        SFERES_CONST float max_motor_clamp = 3.0f;
        // The maximum motor value allowed on the robot
        SFERES_CONST float min_motor_clamp = -3.0f;

        // The initial x-position of the robot
        SFERES_CONST uint robot_starting_x = 10;
        // The initial y-position of the robot
        SFERES_CONST uint robot_starting_y = 10;
        // The initial rotation of the robot
        SFERES_CONST uint robot_starting_r = 0;
        // The size of the robot
        SFERES_CONST uint robot_radius = 4;
        // The range of all range-finder, light, and tile sensors
        SFERES_CONST uint sensor_range = 400;
        // Width/height of each tile in the grid, in pixels
        SFERES_CONST uint grid_size = 20;
        SFERES_CONST uint num_steps = NbSteps::v;

        // If visualization enabled, whether goals should be drawn
        SFERES_CONST bool draw_goals = true;
        // If visualization enabled, whether light switches should be drawn
        SFERES_CONST bool draw_switches = true;
        // If visualization enabled, whether radars should be drawn
        SFERES_CONST bool draw_radars = true;
        // If visualization enabled, whether laser range-finders should be drawn
        SFERES_CONST bool draw_lazers = true;
        // If visualization is performed, whether light sensors should be drawn
        SFERES_CONST bool draw_light_sensors = true;
        // If visualization is performed, whether the tile grid should be drawn
        SFERES_CONST bool draw_grid = false;
        // If visualization is performed, whether visited tiles should be drawn
        SFERES_CONST bool draw_tiles = false;
        // If visualization is performed, whether tile sensors should be drawn
        SFERES_CONST bool draw_tile_sensors = true;

    };

    /* Parameters for the direct neural network */
    struct dnn {
    	 // Number of input neurons
        SFERES_CONST uint nb_inputs         = Params::simu::num_lasers +
        									  Params::simu::num_slices;
        // Number of output neurons
        SFERES_CONST uint nb_outputs        = 2;
        // Minimum number of neurons (presumably for initialization)
        SFERES_CONST uint min_nb_neurons    = 10;
        // Maximum number of neurons (presumably for initialization)
        SFERES_CONST uint max_nb_neurons    = 30;
        // Minimum number of connections (for initialization)
        SFERES_CONST uint min_nb_conns      = 50;
        // Maximum number of connections (for initialization)
        SFERES_CONST uint max_nb_conns      = 250;
        // Chance to add a single connection (percent of the time)
        SFERES_CONST float m_rate_add_conn    = 0.15f;
        // Chance to remove a single connection
        SFERES_CONST float m_rate_del_conn    = 0.05f;
        // Chance to alter a single connection's source or target
        SFERES_CONST float m_rate_change_conn = 0.15f;
        // Chance to add a neuron
        SFERES_CONST float m_rate_add_neuron  = 0.05f;
        // Chance to remove a random-selected neuron
        SFERES_CONST float m_rate_del_neuron  = 0.05f;
        // Type of network initialization (random_topology or ff)
        SFERES_CONST gen::dnn::init_t init = gen::dnn::random_topology;
    };

    /* Parameters for evolution */
    struct evo_float {
    	// Mutation rate (chance to alter connection weight)
        SFERES_CONST float mutation_rate = 0.1f;
        // Crossover rate (presumably unused)
        SFERES_CONST float cross_rate = 0.0f;
        // Mutation type
        SFERES_CONST mutation_t mutation_type = polynomial;
        // Crossover type
        SFERES_CONST cross_over_t cross_over_type = sbx;
        // Mutation parameter
        SFERES_CONST float eta_m = 15.0f;
        // Cross-over parameter (not actually used in current algorithms)
        SFERES_CONST float eta_c = 15.0f;
    };

    struct cmoea {
        // Size of every CMOEA bin
        static const unsigned bin_size = BinSize::v;
        // = 2^num_mazes - 1

#ifdef BIN_SAMPLING
        static const unsigned nb_of_bins = NrBins::v;
        static const unsigned nr_of_objs = NbMazes::v;
        SFERES_CONST double obj_change_rate = 0.001;
#else
        static const unsigned nb_of_bins = NrBins::v;
#endif
        // Where fitness objective will be stored before non-dominated sorting
        static const unsigned obj_index = 0;
    };

    /* Parameters for the population */
    struct pop {
#if defined(EQPOP)
    	// The standing and initial population size of NSGA-2
        SFERES_CONST unsigned size = cmoea::nb_of_bins * cmoea::bin_size;
        // Together with init_batch, determines the initial population for CMOEA
        SFERES_CONST unsigned init_size = size;
#else
        // The number of individuals maintained by the population
        SFERES_CONST unsigned size = PopSize::v;
        // The number of individuals create to initialize the population
        SFERES_CONST unsigned init_size = PopSize::v;
#endif
        // The maximum number of individuals to try an create at the same time.
        SFERES_CONST unsigned max_batch_size = 1024;

        // The number of individuals created every generation
        SFERES_CONST unsigned select_size = PopSize::v;
        SFERES_CONST unsigned nb_gen = NbGenerations::v;
        SFERES_CONST int initial_aleat = 1;
        //        static const int nr_of_dumps = NbGenerations::v/500;
        static int dump_period;
        static int checkpoint_period;
    };

    /* Additional parameters for neural networks */
    struct parameters {
    	// Maximum value of connection weights
        SFERES_CONST float min = -1.0f;
        // Minimum value of connection weights
        SFERES_CONST float max = 1.0f;
    };
    /* Parameters for novelty search */
    struct novelty {
    	// The threshold for adding individuals to the archive (novelty higher
    	// than this results in individuals being added)
        SFERES_CONST float rho_min_init = 1.0;
        // Number of nearest neighbors
        SFERES_CONST uint k = 15;
        // How many evaluations before rho_min should be decreased
        // (if no one added)
        SFERES_CONST uint stalled_tresh = 2500;
        // Reverse of stalled (number of organisms)
        SFERES_CONST uint adding_tresh = 4;
        // Chance to arbitrarily add an individual to the archive
        SFERES_CONST float add_to_archive_prob = 0;
    };
    /* Parameters for stat collection */
    struct stats {
    	// How often we record the population
        SFERES_CONST int period = 1;
#ifdef JHDEBUG
        // How often we test our best individual on the test set
        SFERES_CONST int testset_period = 2;
#else
        // How often we test our best individual on the test set
        SFERES_CONST int testset_period = 100;
#endif

    };

    // MPI pointers to callback functions
    struct mpi{
        static mpi_init_ptr_t masterInit;
        static mpi_init_ptr_t slaveInit;
    };

    struct ea {
    	// Type of non-dominated sorting that will be used by CMOEA NSGA
        typedef sferes::ea::dom_sort_basic_f dom_sort_f;
    };

    struct cmoea_nsga {
    	// Type of non-dominated comparison that will be used by CMOEA NSGA
        typedef sferes::ea::_dom_sort_basic::non_dominated_f non_dom_f;
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
#else // defined(ELEX_USE_OFFSET)
    	SFERES_CONST bool fixed_offset = false;
#endif // defined(ELEX_USE_OFFSET)
    	SFERES_NON_CONST_ARRAY(float, offset)
    };

    struct nsga3
    {
    	static size_t nb_of_objs;
    	static float ref_points_delta;
    };
};
)


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////* MPI callback functions *//////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

void joint_init(){
    dbg::trace trace("init", DBG_HERE);
    if(Params::init) return;
    Params::init = true;

    // Type definitions
    typedef boost::shared_ptr<randmaze::Maze> maze_ptr;

    // Count number of objectives
    int nb_of_objs = 0;
#if defined(NSGA_PUSH_ALL_TASKS)
    nb_of_objs += Params::simu::num_mazes;
#endif
#if not defined(NSGA_PUSH_ALL_TASKS) or defined(NSGA_PUSH_COMBINED_OBJECTIVE)
    nb_of_objs += 1;
#endif
#if defined(DIVERSITY)
    nb_of_objs += 1;
#endif
    dbg::out(dbg::info, "obj") << "Total objectives: " << nb_of_objs << std::endl;
    std::cout << "Number of objectives: " << nb_of_objs << std::endl;

    // Set the number of objectives for NSGA3
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

    // Fill offset array
    float epsilon = 0;
#if defined(ELEX_USE_OFFSET)
    SFERES_INIT_NON_CONST_ARRAY(Params::e_lexicase, offset, nb_of_objs);
#if defined(ELEX_OFFSET_0_20)
    epsilon = 0.2;
#elif defined(ELEX_OFFSET_0_15)
    epsilon = 0.15;
#elif defined(ELEX_OFFSET_0_10)
    epsilon = 0.10;
#elif defined(ELEX_OFFSET_0_05)
    epsilon = 0.05;
#elif defined(ELEX_OFFSET_0_01)
    epsilon = 0.01;
#else
#warning "No valid offset defined!"
#endif
    for(size_t i=0; i<nb_of_objs; ++i){
    	Params::e_lexicase::offset_set(i, epsilon);
    }
    Params::e_lexicase::offset_set(nb_of_objs, pow(epsilon, nb_of_objs));
#endif

    // Create 'mazes'
    misc::pushRand();
    misc::seed(Params::simu::maze_seed);
    for(uint i=0; i< Params::simu::num_mazes; ++i){
        randmaze::Chamber root(20, 20);
        for(int j=1; j<=5; ++j){
            root.split();
        }
        root.addGoal();
#if not defined(FIXED_START)
        do{
            root.generateStart();
            // Make sure the start and the goal are not on top of each other
        } while(root.start_x == root.goals[0].first and
        		root.start_y == root.goals[0].second);
#endif
        maze_ptr map (generate_map(root, Params::simu::grid_size));
        Params::simu::mazes.push_back(map);

#if defined(FIXED_START)
        // Define the starting position directly on the maze object, because
        // setting it on the root will transform the starting position by the
        // tile-size
        Params::simu::mazes.back()->start_x = Params::simu::robot_starting_x;
        Params::simu::mazes.back()->start_y = Params::simu::robot_starting_y;
        Params::simu::mazes.back()->start_r = Params::simu::robot_starting_r;
#endif
    }
    for(uint i=0; i< Params::simu::num_testset_mazes; ++i){
        randmaze::Chamber root(20, 20);
        for(int j=1; j<=5; ++j){
            root.split();
        }
        root.addGoal();
        // Make sure the start and the goal are not on top of each other
        do{
            root.generateStart();
        } while(root.start_x == root.goals[0].first and
        		root.start_y == root.goals[0].second);
        maze_ptr map (generate_map(root, Params::simu::grid_size));
        Params::simu::testset_mazes.push_back(map);
    }
    misc::popRand();
}

void master_init(){
    dbg::trace trace("init", DBG_HERE);
    PRINT_PARAMS;
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

//#if defined(VISUALIZE)
//void saveFrame(SDL_Surface* screen, int frameCounter,
//		std::string movieFolder, std::string framePrefix){
//#if defined(PNGW)
//
//	std::string frame_name = movieFolder + "/" + framePrefix + "_" +  boost::lexical_cast<std::string>(frameCounter) + ".png";
//	std::cout << "Writing png to: " << frame_name << std::endl;
//	int width = screen->w;
//	int height = screen->h;
//	unsigned char *image = (unsigned char*)screen->pixels;
//
//	pngwriter PNG(width, height, 1.0, frame_name.c_str());
//	nPixels = width * height * 3;
//	size_t x = 1;   // start the top and leftmost point of the window
//	size_t y = 1;
//	double R, G, B;
//	for(size_t i=0; i<nPixels; i++){
//		switch(i%3){
//		case 2:{
//			B = (double) image[i];
//			PNG.plot(x, y, R, G, B);
//			if( x == width ){
//				x=1;
//				y++;
//			}
//			else{
//				x++;
//			}
//			break;
//		}
//		case 1:{
//			G = (double) image[i];
//			break;
//		}
//		case 0:{
//			R = (double) image[i];
//			break;
//		}
//		}
//	}
//	PNG.close();
//	//
//	//	    delete[] image;
//	//++frameCounter;
//#endif
//}
//#endif

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////* Fitness Function *////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/**
 * Object to calculate the fitness of an individual over a set of randomly
 * generated mazes.
 */
SFERES_FITNESS(RandMazesFitness, sferes::fit::Fitness) {
public:
    // Set integer and float values to 'uninitialized' values of -1.
    RandMazesFitness(){
        _numTilesVisited = -1;
        _divIndex = -1;
        _valueTestset = -1.f;
        _trainMazesSolved = -1.f;
        _testMazesSolved = -1.f;
        _id = 0;
        _frameCounter = 0;
#if defined(VISUALIZE)
        pause = options::map["pause"].as<bool>();;
        frame_skip = options::map["frame-skip"].as<int>();
        max_frames = options::map["max-frames"].as<int>();
#endif
    }

    /* Determines the behavioral distance between two robots
     * (used only by novelty search). */
    template<typename Indiv>
    float dist(Indiv& ind) const {
        return compare::l1diff(_behavior, ind.fit().getBehavior());
    }

    /* Prepares the given simulator for use by adding sensors to the robot and
     * configuring the simulated world. */
    template<typename Simu>
    void initSimulator(Simu& simulator) {
        /* Add range sensors. */
        simulator.getRobot().addLaser(fastsim::Laser(-M_PI / 2.0, Params::simu::sensor_range));
        simulator.getRobot().addLaser(fastsim::Laser(-M_PI / 4.0, Params::simu::sensor_range));
        simulator.getRobot().addLaser(fastsim::Laser(0.0, Params::simu::sensor_range));
        simulator.getRobot().addLaser(fastsim::Laser(M_PI / 4.0, Params::simu::sensor_range));
        simulator.getRobot().addLaser(fastsim::Laser(M_PI / 2.0, Params::simu::sensor_range));
        simulator.getRobot().addLaser(fastsim::Laser(M_PI, Params::simu::sensor_range));
        dbg::out(dbg::info, "sensors") << "Laser expected: " <<
        		Params::simu::num_lasers << " present: " <<
				simulator.getRobot().getLasers().size() << std::endl;
        dbg::assertion(DBG_ASSERTION(simulator.getRobot().getLasers().size() ==  Params::simu::num_lasers));

        // Add radar
#if defined(OPAQUE_WALLS)
        simulator.getRobot().addRadar(fastsim::Radar(0, Params::simu::num_slices, false));
#else
        simulator.getRobot().addRadar(fastsim::Radar(0, Params::simu::num_slices));
#endif

        // sensors are only updated when calling moveRobot(); this obtains sensor
        // values without actually changing the robot's position
        simulator.moveRobot(0.0, 0.0);
    }


    template<typename Indiv>
    void inline evalMaze(Indiv& ind, simu::Fastsim<Params>& simulator,
    		bool debug=false)
    {
#if defined(VISUALIZE)
    	// Initialize variables associated with visualization
    	if(this->_mode == fit::mode::view){
    		simulator.initDisplay();
    		simulator.adjustVisibility(Params::simu::draw_goals,
    				Params::simu::draw_switches,
					Params::simu::draw_radars,
					Params::simu::draw_lazers,
					Params::simu::draw_light_sensors,
					Params::simu::draw_grid,
					Params::simu::draw_tiles,
					Params::simu::draw_tile_sensors);
//    		simulator.getDisplay().setMovieFolder("setMe");
//    		simulator.getDisplay().setFramePrefix("setMeAsWell");
    	}
#endif

    	std::string movie_folder = options::map["movie"].as<std::string>();

        dbg::out(dbg::info, "sim") << "Initializing simulator" << std::endl;
        initSimulator(simulator);

        /* Initialize the neural network. */
        dbg::out(dbg::info, "sim") << "Initializing neural network" << std::endl;
        ind.nn().init();

        /* Used for manual motor commands only, to allow gentle acceleration. */
        float lastMotorLeft = 0.0;
        float lastMotorRight = 0.0;

        /* Prepare neural network for sensor readings and output. */
        dbg::out(dbg::info, "sim") << "Creating vectors" << std::endl;
        std::vector<float> requestedActions(Params::dnn::nb_outputs, 0.0);
        std::vector<float> neuralInputs(Params::dnn::nb_inputs, 0.0);
        size_t nn_input_index;

        /* Run the simulation. */
        dbg::out(dbg::info, "sim") << "Running simulator" << std::endl;
        for (size_t timeStep = 0; timeStep < Params::simu::num_steps; timeStep++) {
            dbg::out(dbg::info, "sim") << "processing step: " << timeStep <<
            		" out of: " << Params::simu::num_steps << std::endl;
#if defined(VISUALIZE)
            //update the display if visualization is enabled
            if(this->_mode == fit::mode::view){
            	if(timeStep % frame_skip == 0){
            		simulator.updateView();

#ifdef PNGW
            		if(!pause){
            			saveFrameSDL(simulator.getDisplay().getScreen(),
            					_frameCounter, movie_folder, _frame_prefix);
            			++_frameCounter;
            			if(max_frames > 0 && _frameCounter >= max_frames){
            				SDL_Quit();
            				exit(0);
            			}
            		}
#endif

            	}

            	// Enable some rudimentary control over the simulation
            	keyCode = SDL_GetKeyState(NULL);
            	while(SDL_PollEvent(&event)){
            		switch( event.type ){
            		case SDL_KEYDOWN:
            			switch( event.key.keysym.sym ){
            			case SDLK_s:
            				timeStep = Params::simu::num_steps;
            				break;
            			case SDLK_r:
            				simulator.getDisplay().dRobot = !simulator.getDisplay().dRobot;
            				break;
            			case SDLK_p:
            				pause = !pause;
            				break;
            			case SDLK_n:
            				if(frame_skip > 1){
            					frame_skip--;
            				}
            				break;
            			case SDLK_m:
            				frame_skip++;
            				break;
            			case SDLK_ESCAPE:
            				SDL_Quit();
            				exit(0);
            				break;
            			default:
            				break;
            			}
            			break;
            			case SDL_QUIT:
            				SDL_Quit();
            				exit(0);
            				break;
            			default:
            				break;
            		}
            	}
            	if(pause){
            		--timeStep;
            		continue;
            	}
            }
#endif


            dbg::out(dbg::info, "sim") << "Running neural network" << std::endl;
            nn_input_index = 0;
            /* Set inputs for the laser range-finders. */
            for (size_t i = 0; i < Params::simu::num_lasers; i++) {
            	//the distance from this laser to the wall, or -1 if no wall was detected
                neuralInputs[nn_input_index++] = simulator.getRobot().getLasers()[i].getDist();
            }

            /* Set inputs for the goal sensor. */
            for (size_t i = 0; i < Params::simu::num_slices; i++) {
                if(simulator.getRobot().getRadars()[0].getActivatedSlice() == i){
                    neuralInputs[nn_input_index++] = 1;
                } else {
                    neuralInputs[nn_input_index++] = 0;
                }
            }

            /* Run the neural network. */
            ind.nn().step(neuralInputs);

            /* Obtain neural outputs (motors). */
            for (size_t i = 0; i < requestedActions.size(); i++) {
                //if(debug){
                //    std::cout << "Output: " << ind.nn().get_outf()[i] << std::endl;
                //}
                //scaled from [0, 1] to [-3, 3]
                requestedActions[i] = ind.nn().get_outf()[i] *
                		(Params::simu::max_motor_clamp -
                				Params::simu::min_motor_clamp) +
								Params::simu::min_motor_clamp;
            }
            dbg::out(dbg::info, "nnio") << "NN input: " << vec_to_string(neuralInputs) << std::endl;
            dbg::out(dbg::info, "nnio") << "NN input: " << vec_to_hex(neuralInputs) << std::endl;
            dbg::out(dbg::info, "nnio") << "NN output: " << vec_to_string(requestedActions) << std::endl;
            dbg::out(dbg::info, "nnio") << "NN output: " << vec_to_hex(requestedActions) << std::endl;
#if defined(MANUAL)
            if(this->_mode == fit::mode::view){
            	dbg::out(dbg::info, "sim") << "Processing user input" << std::endl;
            	lastMotorLeft = 0.0;
            	lastMotorRight = 0.0;
            	keyCode = SDL_GetKeyState(NULL);
            	if(keyCode[SDLK_UP]){
            		lastMotorLeft = Params::simu::max_motor_clamp;
            		lastMotorRight = Params::simu::max_motor_clamp;
            	}
            	if(keyCode[SDLK_DOWN]){
            		lastMotorLeft = Params::simu::min_motor_clamp;
            		lastMotorRight = Params::simu::min_motor_clamp;
            	}
            	if(keyCode[SDLK_LEFT]){
            		lastMotorLeft += Params::simu::max_motor_clamp/4;
            		lastMotorRight += Params::simu::min_motor_clamp/4;
            	}
            	if(keyCode[SDLK_RIGHT]){
            		lastMotorLeft += Params::simu::min_motor_clamp/4;
            		lastMotorRight += Params::simu::max_motor_clamp/4;
            	}

            	requestedActions[0] = lastMotorLeft;
            	requestedActions[1] = lastMotorRight;
            }
#endif

            /* Clamp robot movement. */
            dbg::out(dbg::info, "sim") << "Clamping speeds" << std::endl;
            if (requestedActions[0] > Params::simu::max_motor_clamp) {
            	requestedActions[0] = Params::simu::max_motor_clamp; }
            if (requestedActions[0] < Params::simu::min_motor_clamp) {
            	requestedActions[0] = Params::simu::min_motor_clamp; }
            if (requestedActions[1] > Params::simu::max_motor_clamp) {
            	requestedActions[1] = Params::simu::max_motor_clamp; }
            if (requestedActions[1] < Params::simu::min_motor_clamp) {
            	requestedActions[1] = Params::simu::min_motor_clamp; }


            dbg::out(dbg::info, "control") << vec_to_string(requestedActions) << std::endl;

            /* Move the robot and update sensor values. */
            dbg::out(dbg::info, "sim") << "Move the robot by: (" <<
            		requestedActions[0] << ", " << requestedActions[1] <<
					")" << std::endl;
            simulator.moveRobot(requestedActions[0], requestedActions[1]);

            /* Record the robot's positions */
#if defined(STOP_AT_GOAL)
            float goalX = simulator.getMap()->getGoals()[0].getX();
            float goalY = simulator.getMap()->getGoals()[0].getY();
            float dist = simulator.getRobot().getPosture().distTo(goalX, goalY);
            if(dist < Params::simu::robot_radius){
                break;
            }
#endif
        }
        dbg::out(dbg::info, "sim") << "Simulation finished" << std::endl;
    }

    /* Calculates the fitness of the robot in the simulator */
    float inline calcFitness(simu::Fastsim<Params>& simulator,
    		boost::shared_ptr<randmaze::Maze> mapCopy)
    {
        float fitness = 0;
        float goalX = simulator.getMap()->getGoals()[0].getX();
        float goalY = simulator.getMap()->getGoals()[0].getY();

#if defined(MAXIMUM_DISTANCE)
        float mapWidth = simulator.getMap()->getRealWidth();
        float mapHeight = simulator.getMap()->getRealHeight();
        float maxDist = sqrt(pow(0-goalX,2) + pow(0-goalY,2));
        float candidateDist = sqrt(pow(0-goalX,2.0) + pow(mapHeight-goalY,2));
        maxDist = candidateDist > maxDist? candidateDist : maxDist;
        candidateDist = sqrt(pow(mapWidth-goalX,2) + pow(0-goalY,2));
        maxDist = candidateDist > maxDist? candidateDist : maxDist;
        candidateDist = sqrt(pow(mapWidth-goalX,2) + pow(mapHeight-goalY,2));
        maxDist = candidateDist > maxDist? candidateDist : maxDist;
        float distance = simulator.getRobot().getPosture().distTo(goalX, goalY);
        fitness = 1 - (distance / maxDist);
#else
        float distance = simulator.getRobot().getPosture().distTo(goalX, goalY);
        float maxDist = sqrt(pow(mapCopy->start_x-goalX,2) +
        		pow(mapCopy->start_y-goalY,2));
        fitness = 1 - (distance / maxDist);
#endif
        if(fitness < 0) fitness=0;
        if(distance < Params::simu::robot_radius) fitness = 1.0;
        return fitness;
    }


    /* Evaluates the performance of an individual robot on the test set. */
    template<typename Indiv>
    void evalTestset(Indiv& ind) {
    	//std::cout << "Running test set..." << std::endl;
        // Initialize test set value
        _valueTestset = 0;
        _testMazesSolved = 0.f;
        _mazeSolved.resize(Params::simu::testset_mazes.size(), false);

        _frameCounter = 0;
        _frame_prefix = "testMaze";

        // Evaluate test set
        for(size_t mapI=0; mapI < Params::simu::testset_mazes.size(); ++mapI){
        	// A bit of hacky code to show only the cases in which a maze
        	// can not be solved.
        	if(_mazeSolved[mapI]) continue;

            // Create simulator
            dbg::out(dbg::info, "sim") << "Creating simulator" << std::endl;
            boost::shared_ptr<randmaze::Maze> mapCopy(
            		new randmaze::Maze(*Params::simu::testset_mazes[mapI])
            );
            simu::Fastsim<Params> simulator(mapCopy,
            		Params::simu::grid_size, false, Params::simu::robot_radius);

            // Positions the robot
            dbg::out(dbg::info, "sim") << "Positioning robot " << std::endl;
            simulator.getRobot().setPosture(fastsim::Posture(mapCopy->start_x,
            		mapCopy->start_y,
					mapCopy->start_r));

            // Evaluate individual on maze
            evalMaze(ind, simulator, true);

            // Calculate fitness
            float fitness = calcFitness(simulator, mapCopy);
//            if(fitness <= 0.99999){
//            	std::cout << "Failed on maze: " << mapI <<
//            			" with fitness: " << fitness << std::endl;
//            }
            _valueTestset += fitness;
            if(fitness == 1.0){
            	_testMazesSolved += 1;
            	_mazeSolved[mapI] = true;
            }
            dbg::out(dbg::info, "fit") << "Fitness map " << mapI <<
            		": " << fitness << std::endl;
        }


        // Normalize performance
        _valueTestset /= float(Params::simu::testset_mazes.size());

        dbg::out(dbg::info, "fitness") << " Test set performance fitness: " <<
        		_valueTestset << std::endl;
    }




    /* Evaluates the performance of an individual robot. */
    template<typename Indiv>
    void eval(Indiv& ind) {
        // Initialize performance values
        this->_value = 0;
        _numTilesVisited = 0;
        _trainMazesSolved = 0.f;

        _frameCounter = 0;
        _frame_prefix = "trainMaze";

        dbg::out(dbg::info, "sim") << "Creating simulator" << std::endl;
        /* Create the simulator. */
        for(size_t mapI=0; mapI < Params::simu::mazes.size(); ++mapI){
            boost::shared_ptr<randmaze::Maze> mapCopy(
            		new randmaze::Maze(*Params::simu::mazes[mapI]));
            simu::Fastsim<Params> simulator(mapCopy,
            		Params::simu::grid_size, false, Params::simu::robot_radius);

            // Positions the robot
            dbg::out(dbg::info, "sim") << "Positioning robot" << std::endl;
            simulator.getRobot().setPosture(fastsim::Posture(mapCopy->start_x,
            		mapCopy->start_y,  mapCopy->start_r));
            evalMaze(ind, simulator);

            // Record the number of tiles visited by this robot
            _numTilesVisited += simulator.getMap()->getNumTilesVisited();

            // Set data for the behavioral metric (only used by novelty search)
            _behavior.push_back(simulator.getRobot().getPosture().getX());
            _behavior.push_back(simulator.getRobot().getPosture().getY());

            // Calculate fitness
            float fitness = calcFitness(simulator, mapCopy);
            this->_value += fitness;
            if(fitness == 1.0){
            	_trainMazesSolved += 1;
            }
            _cmoea_task_performance.push_back(fitness);
#if defined(NSGA_PUSH_ALL_TASKS)
            this->_objs.push_back(fitness);
#endif
            dbg::out(dbg::info, "fit") << "Fitness map " << mapI <<
            		": " << fitness << std::endl;
        }
#if defined(BIN_SAMPLING)

#elif defined(MULTIPLICATIVE_FITNESS)
        cmoea::calculate_bin_fitness_mult(_cmoea_task_performance,
        		_cmoea_bin_fitness);
#else
        cmoea::calculate_bin_fitness(_cmoea_task_performance,
        		_cmoea_bin_fitness);
#endif
        this->_value /= float(Params::simu::mazes.size());

#if not defined(NSGA_PUSH_ALL_TASKS) or defined(NSGA_PUSH_COMBINED_OBJECTIVE)
        this->_objs.push_back(this->_value);
        dbg::out(dbg::info, "fitness") << " Pushed back fitness: " <<
        		this->_objs.back() << std::endl;
#endif
        // If curiosity search is enabled, add an extra objective
        // (will be replaced by the novelty metric).
#if defined(DIVERSITY)
        _divIndex = this->_objs.size();
        this->_objs.push_back(-2);
        dbg::out(dbg::info, "fitness") << " Pushed back dummy diversity: " <<
        		this->_objs.back()  << std::endl;
#else
        _divIndex = -1;
#endif
        dbg::assertion(DBG_ASSERTION(this->_objs.size() == Params::nsga3::nb_of_objs));
    }

    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    //////////////* Fitness attributes *////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
        dbg::trace trace("fit", DBG_HERE);
        sferes::fit::Fitness<Params,
			typename stc::FindExact<RandMazesFitness<Params, Exact>,
			Exact>::ret>::serialize(ar, version);
        ar & BOOST_SERIALIZATION_NVP(_historyPositions);
        ar & BOOST_SERIALIZATION_NVP(_behavior);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_task_performance);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_bin_fitness);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_bin_diversity);
        ar & BOOST_SERIALIZATION_NVP(_numTilesVisited);
        ar & BOOST_SERIALIZATION_NVP(_divIndex);
        ar & BOOST_SERIALIZATION_NVP(_valueTestset);
#if defined(SERIALIZE_MAZES_SOLVED)
        ar & BOOST_SERIALIZATION_NVP(_trainMazesSolved);
        ar & BOOST_SERIALIZATION_NVP(_testMazesSolved);
#endif
    }

    // Provide write access to the value parameter of the parent
    void set_value(float v) { this->_value = v; }
    std::vector<float> getPositionHistory() const { return _historyPositions;}
    std::vector<float> getBehavior() const { return _behavior;}
    int getNumTilesVisited() const {return _numTilesVisited;}
    float exp() const {
    	return float(_numTilesVisited) /
    			float(Params::simu::num_mazes*
    					Params::simu::grid_size*
						Params::simu::grid_size);
    }
    float div() const {return _divIndex >= 0 ? this->_objs[_divIndex] : -1.0f;}
    float valueTestset() const {return _valueTestset;}
    int getTrainMazesSolved() const {return _trainMazesSolved;}
    int getTestMazesSolved() const {return _testMazesSolved;}

    //ID related functions, such that we know which individual of the population
    //we are talking about
    void setId(size_t id){_id = id;}
    size_t getId(){ return _id;}
    void setPostFix(std::string postFix){_postFix = postFix;}

    // Getters and setters for CMOEA
    std::vector<float> &getBinFitnessVector(){return _cmoea_bin_fitness;}
    float getBinFitness(size_t index){ return _cmoea_bin_fitness[index];}
    std::vector<float> &getBinDiversityVector(){return _cmoea_bin_diversity;}
    float getBinDiversity(size_t index){ return _cmoea_bin_diversity[index];}
    void initBinDiversity(){
        if(_cmoea_bin_diversity.size() != Params::cmoea::nb_of_bins){
            _cmoea_bin_diversity.resize(Params::cmoea::nb_of_bins);
        }
    }
    void setBinDiversity(size_t index, float div){
        dbg::check_bounds(dbg::error, 0, index, _cmoea_bin_diversity.size(),
        		DBG_HERE);
        _cmoea_bin_diversity[index] = div;
    }

    float getCmoeaObj(size_t index){
    	return _cmoea_task_performance[index];
    }

protected:
    // A vector of all robot positions
    std::vector<float> _historyPositions;
    // Vector of robot (end) points over all mazes
    std::vector<float> _behavior;
    // Performance on each task, as required for cmoea
    std::vector<float> _cmoea_task_performance;
    // Performance on each combination of tasks, as required for cmoea
    std::vector<float> _cmoea_bin_fitness;
    // The diversity score of an individual, as calculated by cmoea
    std::vector<float> _cmoea_bin_diversity;
    // The total number of tiles visited across all mazes
    int _numTilesVisited;
    // The index of the diversity objective in the objective array
    int _divIndex;
    // Performance on the test set of mazes
    float _valueTestset;
    // Number of training-set mazes solved
    int _trainMazesSolved;
    // Number of test-set mazes solved
    int _testMazesSolved;

    std::vector<bool> _mazeSolved;

    //Set by stat (not serialized)
    size_t _id;
    std::string _postFix;
    int _frameCounter;
    std::string _frame_prefix;

#if defined(VISUALIZE)
    uint8_t* keyCode;
    SDL_Event event;
    bool pause;
    size_t frame_skip;
    size_t max_frames;
#endif
};

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////* Declare static globals *//////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

bool Params::init = false;
mpi_init_ptr_t Params::mpi::masterInit = 0;
mpi_init_ptr_t Params::mpi::slaveInit = 0;
int Params::pop::dump_period = -1;
int Params::pop::checkpoint_period = -1;
std::vector<boost::shared_ptr<randmaze::Maze> > Params::simu::mazes =
		std::vector<boost::shared_ptr<randmaze::Maze> >();
std::vector<boost::shared_ptr<randmaze::Maze> > Params::simu::testset_mazes =
		std::vector<boost::shared_ptr<randmaze::Maze> >();

size_t Params::nsga3::nb_of_objs = 0;
float Params::nsga3::ref_points_delta = 0.0;

SFERES_DECLARE_NON_CONST_ARRAY(Params::e_lexicase, offset);

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//////////////////* Main Program *//////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/* Sets up and runs the experiment. */
int main(int argc, char **argv) {
    std::cout << "Entering main: " << std::endl;

    // Set MPI master and slave callbacks
    Params::mpi::slaveInit = &slave_init;
    Params::mpi::masterInit = &master_init;

    /* Fitness function to use (a class, defined above), which makes use of the
     * Params struct. */
    typedef RandMazesFitness<Params> fit_t;

    /* The genotype (a neural network). */
    typedef gen::EvoFloat<1, Params> evo_float_t;
    typedef phen::Parameters<evo_float_t, fit::FitDummy<>, Params> weight_t;
    typedef phen::Parameters<evo_float_t, fit::FitDummy<>, Params> bias_t;
    typedef nn::PfWSum<weight_t> pf_t;
    typedef nn::AfSigmoidBias<bias_t> af_t;
    typedef nn::Neuron<pf_t, af_t> neuron_t;
    typedef nn::Connection<weight_t> con_t;
    typedef sferes::gen::Dnn<neuron_t, con_t, Params> real_gen_t;
    typedef sferes::gen::WorldSeedIndiv<real_gen_t> gen_t;

    /* The phenotype (a directly-encoded neural network). */
    typedef phen::Dnn<gen_t, fit_t, Params> phen_t;

#if defined(BEHAVIORAL_DIVERSITY)
    //Behavioral distance based only on current population.
    typedef modif::BehaviorDiv<> mod_t;
#else
    //No modifier
    typedef boost::fusion::vector<> mod_t;
#endif



    /* What statistics should be gathered
     * (can provide as many statistics as desired). */
    typedef boost::fusion::vector<
            stat::StatCmoea<phen_t, Params>,
            stat::StatTestset<phen_t, Params>
#if defined(BIN_SAMPLING)
			, stat::StatClock<phen_t, Params>
#endif
    > stat_t;


    // Define the evolutionary algorithm
#if defined(NSGA) and !defined(MPI_ENABLED)
    std::cout << "Running NSGA-II without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(NSGA) and defined(MPI_ENABLED)
    std::cout << "Running NSGA-II with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_taks_t;
    typedef boost::fusion::vector<eval_taks_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(ELEXICASE) and (!defined(MPI_ENABLED) or defined(NOMPI))
    std::cout << "Running e-Lexicase without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(ELEXICASE) and defined(MPI_ENABLED) and !defined(NOMPI)
    std::cout << "Running e-Lexicase with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_tasks_t;
    typedef boost::fusion::vector<eval_tasks_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(LEXICASE) and !defined(MPI_ENABLED)
    std::cout << "Running Lexicase without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(LEXICASE) and defined(MPI_ENABLED)
    std::cout << "Running Lexicase with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_taks_t;
    typedef boost::fusion::vector<eval_taks_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(BIN_SAMPLING) and !defined(MPI_ENABLED)
    std::cout << "Running CMOEA with bin-sampling without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::CmoeaBinSampling<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(BIN_SAMPLING) and defined(MPI_ENABLED)
    std::cout << "Running CMOEA with bin-sampling and MPI" << std::endl;
    typedef ea::CmoeaSelectSampleTask<phen_t, mod_t, Params> archive_task_t;
    typedef eval::EvalTask<phen_t> eval_tasks_t;
    typedef boost::fusion::vector<eval_tasks_t, archive_task_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::CmoeaBinSamplingMpi<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(CMOEA) and !defined(MPI_ENABLED)
    std::cout << "Running CMOEA without bin-sampling without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::CmoeaNsga2<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(CMOEA) and defined(MPI_ENABLED)
    std::cout << "Running CMOEA without bin-sampling with MPI" << std::endl;
    typedef ea::ArchiveTask<phen_t, mod_t, Params> archive_task_t;
    typedef eval::EvalTask<phen_t> eval_taks_t;
    typedef boost::fusion::vector<eval_taks_t, archive_task_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::CmoeaNsga2Mpi<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueCmoeaNsga2<ea_t, Params> continue_ea_t;
#elif defined(NSGA3) and defined(MPI_ENABLED)
    std::cout << "Running NSGA-III with MPI" << std::endl;
    typedef eval::EvalTask<phen_t> eval_task_t;
    typedef boost::fusion::vector<eval_task_t> tasks_t;
    typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
    typedef ea::nsga3::Nsga3<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga3<ea_t, Params> continue_ea_t;
#elif defined(NSGA3) and !defined(MPI_ENABLED)
    std::cout << "Running NSGA-III without MPI" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::nsga3::Nsga3<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga3<ea_t, Params> continue_ea_t;
#else
    std::cout << "Running NSGA-II" << std::endl;
    typedef eval::EvalWorldTracking<Params> eval_t;
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#endif

    // Custom options
    namespace opt = boost::program_options;
    options::add()("movie",
    		opt::value<std::string>()->default_value("movie"),
			"Dump all movie images in the folder with this name.");
    options::add()("pause",
    		opt::value<bool>()->default_value(true),
			"Determines whether the simulation starts paused.");
    options::add()("max-frames",
    		opt::value<int>()->default_value(0),
			"How many frames should be recorded.");


    // Process command line options (debugger will not be active until this
    // code is completed)
    cont::init();
    modules::rand::init();
    stat::StatCmoea<phen_t, Params>::initOptions();
    options::parse_and_init(argc, argv, false);
    modules::rand::set_seed();

#ifdef PNGW
    std::string movie_folder = options::map["movie"].as<std::string>();
    boost::filesystem::path full_path( boost::filesystem::current_path() );
    boost::filesystem::path dir(full_path/movie_folder);
    boost::filesystem::create_directory(dir);
#endif

    /* Create the evolutionary algorithm as an object. */
    continue_ea_t ea;

#if defined(ADD_EQUAL_WEIGHT_REFERENCE_POINT)
    Eigen::VectorXd target(Params::nsga3::nb_of_objs);
    target.fill(1.0 / double(Params::nsga3::nb_of_objs));
    ea.preferred_reference_points().push_back(target);
#endif

    // Create the continuator
    cont::Continuator<continue_ea_t, Params> continuator;

    // This will cause all slaves to enter the slave loop if MPI is defined
    // Prevents a lot of useless work, and makes debugging easier.
    fit_t _proto_fit;
    std::vector<boost::shared_ptr<phen_t> > empty;
    ea.eval().eval(empty, 0, 0, _proto_fit);


    // Start the run
    continuator.run(ea);

    if(continuator.endOfRun()){
        // Store the best individual in a separate gen file, so we can load it
        std::vector<boost::shared_ptr<phen_t> > best_pop;
        stat_t stat = ea.stat();
        best_pop.push_back(boost::fusion::at_c<0>(stat).getBest());
        boost::fusion::at_c<0>(stat).setPopulationVector(best_pop);
        ea.write_gen_file("best_");
    }

    //Do some cleanup after the run if necessary
    if(continuator.performCleanup()){
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "cmoea.dat");
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "testset.dat",
        		stat::oneLineMultiGen);
        stat::mergeOneLineGenFiles(continuator.getSearchDir(), ea.res_dir(), "nsga3.dat");
        continuator.deleteCheckpoints();
        continuator.moveGenFiles(ea.res_dir());
        io::deleteEmptyDirectories();
    }


    /* Record completion (makes it easy to check if the job was preempted). */
    std::cout << "\n==================================" << \
            "\n====Evolutionary Run Complete!====" << \
            "\n==================================\n";
    return 0;
}
