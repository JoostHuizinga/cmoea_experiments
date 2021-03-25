//#define NO_PARALLEL
/**
 * The retina experiment where there are 8 inputs and the network has to tell whether there is a right AND/OR a left object present.
 */

#ifdef AGE
#define TRACK_FIT
#endif

// It turns out that the debugger is not thread safe, and will hang when
// multiple threads are running at the same time. Thus, if debug is enabled
// we disable parallelization. That said, the dbg library may need some work.
#if defined(DBG_ENABLED)
#define NO_PARALLEL
#endif

// Turn on debug mode
#if defined(DBG)
#define DBG_ENABLED
#undef NDEBUG
#endif

#include <iostream>
#include <bitset>
#include <boost/assign/list_of.hpp>
#include <boost/random.hpp>

#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>
#include <sferes/ea/nsga2.hpp>
#include <modules/nsgaext/nsga2_custom.hpp>

#if !defined(__APPLE__) || defined (VMDMESA)
#include <sferes/eval/mpi.hpp>
#endif
#include <sferes/eval/eval.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>
#include <modules/nn2/nn.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/phen_dnn.hpp>

#include <modules/modularity/modularity.hpp>
//#include <modules/hnn/gen_hnn.hpp>
#include <modules/hnn/gen_hnn_world_seed.hpp>
#include <modules/hnn/af_cppn.hpp>

//#if defined(NO_LEO)
#include <modules/hnn/phen_hnn.hpp>
#include <modules/hnn/phen_hnn_leo.hpp>
//#else
//#include <modules/hnn/phen_hnn_old.hpp>
//#endif

//#include <string.h>

#include "behavior_div.hpp"
#include "gen_spatial.hpp"
#include "stat_modularity.hpp"
#include "mvg.hpp"
#include "modularity.hpp"
#include "regularity.hpp"
#include "mod2.hpp"

#include "stat_logall.hpp"
#include "mut_sweep_macros.hpp"
#include "gradual_connections.hpp"
#include "retina_left_right.hpp"
#include "dom_sort_no_duplicates.hpp"

//Include the continue headers
#include <modules/continue/continue_run.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>

#include <modules/datatools/params.hpp>


#define LEFT_RIGHT_NR_OF_STEPS 20

//Age based modifier
#ifdef AGE
# include "modif_age.hpp"
#endif

#if defined(PNSGA)
# include "pnsga.hpp"
#elif defined(MPNSGA)
# include "pnsga_alt.hpp"
#endif

//Novelty search map based on behavior
#ifdef NOVMAP
# include "modif_novmap.hpp"
#endif

#include <boost/algorithm/string.hpp>

#if defined(MPI_ENABLED) && !defined(NOMPI)
#include <modules/mpiworldtrack/eval_mpi_world_tracking.hpp>
#else
#include <modules/mpiworldtrack/eval_world_tracking.hpp>
#include <modules/mpiworldtrack/eval_parallel_world_tracking.hpp>
//#include "eval_parallel_world_tracking.hpp"
#endif

//#define SFERES_PARAMS(P)                                                    \
//  struct Ps__ { Ps__() {                                                    \
//                  static std::string __params = # P;                        \
//                  boost::replace_all(__params, ";", ";\n");                 \
//                  boost::replace_all(__params, "{", "{\n");                 \
//                  boost::replace_all(__params, "SFERES_CONST", "");         \
//                  std::cout << "Parameters:" << __params << std::endl; } }; \
//  P;                                                                        \
//  static Ps__ ____p;

using namespace sferes;
using namespace nn;
using namespace sferes::gen::evo_float;
using namespace sferes::gen::dnn;


// -1 to +1 sigmoid
//Directly inherits from the general activation function class AF.
//
template<typename P>
struct AfRetina : public Af<P> {
	typedef P params_t;
	SFERES_CONST float lambda = 20.0f;
	AfRetina() {
		assert(trait<P>::size(this->_params) == 1);
	}
	float operator()(float p) const {
		return tanh(lambda * (p + trait<P>::single_value(this->_params)));
	}
protected:
};


#if not defined(DNN_CON_ADD_MUT_RATE)
#define DNN_CON_ADD_MUT_RATE 0.2f
#endif

#if not defined(DNN_CON_DEL_MUT_RATE)
#define DNN_CON_DEL_MUT_RATE 0.2f
#endif

#if not defined(CPPN_CON_ADD_MUT_RATE)
#define CPPN_CON_ADD_MUT_RATE 0.09f
#endif

#if not defined(CPPN_CON_DEL_MUT_RATE)
#define CPPN_CON_DEL_MUT_RATE 0.08f
#endif

#if not defined(CPPN_NEU_ADD_MUT_RATE)
#define CPPN_NEU_ADD_MUT_RATE 0.05f
#endif

#if not defined(CPPN_NEU_DEL_MUT_RATE)
#define CPPN_NEU_DEL_MUT_RATE 0.04f
#endif


SFERES_TYPE_VALUE_PARAM(DnnConAddMutRate, SFERES_CONST float v, DNN_CON_ADD_MUT_RATE)
SFERES_TYPE_VALUE_PARAM(DnnConDelMutRate, SFERES_CONST float v, DNN_CON_DEL_MUT_RATE)
SFERES_TYPE_VALUE_PARAM(CppnConAddMutRate, SFERES_CONST float v, CPPN_CON_ADD_MUT_RATE)
SFERES_TYPE_VALUE_PARAM(CppnConDelMutRate, SFERES_CONST float v, CPPN_CON_DEL_MUT_RATE)
SFERES_TYPE_VALUE_PARAM(CppnNeuAddMutRate, SFERES_CONST float v, CPPN_NEU_ADD_MUT_RATE)
SFERES_TYPE_VALUE_PARAM(CppnNeuDelMutRate, SFERES_CONST float v, CPPN_NEU_DEL_MUT_RATE)


SFERES_PARAMS(
struct Params {
	static long seed;
    static bool init;
	SFERES_CONST size_t nr_of_input_patterns = 256;
	SFERES_CONST size_t nr_of_subtasks = 2;

	struct pop
	{
#ifdef JHDEBUG
		//Population size has to be devidable by four!
		SFERES_CONST unsigned size = 100;
		SFERES_CONST unsigned select_size = size;
		SFERES_CONST int dump_period = 101;
		SFERES_CONST int initial_aleat = 1;
		SFERES_CONST unsigned nb_gen = 100;
		SFERES_CONST int checkpoint_period = 1;
#else //JHDEBUG
		//Population size has to be devidable by four!
#if defined(POP100)
		SFERES_CONST unsigned size = 100;
		SFERES_CONST unsigned select_size = size;
#else
		SFERES_CONST unsigned size = 1000;
		SFERES_CONST unsigned select_size = size;
#endif

#if defined(GEN5000)
		SFERES_CONST unsigned nb_gen = 5001;
		SFERES_CONST int dump_period = 5000;
#elif defined(GEN100)
		SFERES_CONST unsigned nb_gen = 101;
		SFERES_CONST int dump_period = 101;
#elif defined(GEN10000)
		SFERES_CONST unsigned nb_gen = 10001;
		SFERES_CONST int dump_period = 10000;
#elif defined(GEN25000)
		SFERES_CONST unsigned nb_gen = 25001;
		SFERES_CONST int dump_period = 25000;
#elif defined(GEN40000)
		SFERES_CONST unsigned nb_gen = 40001;
		SFERES_CONST int dump_period = 40000;
#elif defined(GEN50000)
		SFERES_CONST unsigned nb_gen = 50001;
		SFERES_CONST int dump_period = 25000;
#else //Default GEN is 40000
		SFERES_CONST unsigned nb_gen = 40001;
		SFERES_CONST int dump_period = nb_gen - 1;
#endif //defined(GEN5000)
		SFERES_CONST int checkpoint_period = dump_period/10;
		SFERES_CONST int initial_aleat = 1;
#endif //JHDEBUG
	};

	struct ea
	{
		static float pressure;
#if defined(PNSGA) || defined(MPNSGA)
#if defined(PR00)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.0, 1.0);
#elif defined(PR10)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.1, 1.0);
#elif defined(PR25)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.25, 1.0);
#elif defined(PR50)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.5, 1.0);
#elif defined(PR75)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.75, 1.0);
#elif defined(PR95)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.95, 1.0);
#else //defined(PR100)
		SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0);
#endif

#elif defined(MPNSGA) //defined(PNSGA) || defined(MPNSGA)

#if defined(PR25_PR25)
		SFERES_ARRAY(float, obj_pressure, 0.25, 0.25, 1.0);
#elif defined(PR50_PR50)
		SFERES_ARRAY(float, obj_pressure, 0.50, 0.50, 1.0);
#elif defined(PR75_PR75)
		SFERES_ARRAY(float, obj_pressure, 0.75, 0.75, 1.0);
#elif defined(PR95_PR95)
		SFERES_ARRAY(float, obj_pressure, 0.95, 0.95, 1.0);
#elif defined(PR95_PR25)
		SFERES_ARRAY(float, obj_pressure, 0.95, 0.25, 1.0);
#elif defined(PR75_PR25)
		SFERES_ARRAY(float, obj_pressure, 0.75, 0.25, 1.0);
#else //defined(PR100_PR100)
		SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0);
#endif

#else //defined(PNSGA) || defined(MPNSGA)
		SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0);
#endif
	  typedef sferes::ea::dom_sort_basic_f dom_sort_f;
	  //typedef sferes::ea::pnsga::prob_dom_f dom_sort_f;
	};

    struct mpi
    {
        static mpi_init_ptr_t masterInit;
        static mpi_init_ptr_t slaveInit;
    };

	struct rand_obj
	{
		SFERES_CONST float std_dev = 0.1;
	};
	struct visualisation{
#if defined(LEO)
		SFERES_ARRAY(int, background, 255, 162, 162);
#elif defined(LENGTH) && defined(HNN)
		SFERES_ARRAY(int, background, 135, 197, 107);
#elif defined(HNN)
		SFERES_ARRAY(int, background, 170, 197, 225);
#elif defined(LENGTH)
		SFERES_ARRAY(int, background, 255, 255, 180);
#else
		SFERES_ARRAY(int, background, 255, 200, 200);
#endif
		SFERES_CONST size_t max_modularity_split = 1;

		SFERES_ARRAY(float, x,
		-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
		-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
		-2, -1, 1, 2,
		-1, 1,
		0);
		SFERES_ARRAY(float, y,
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2,
		3, 3,
		4);
		SFERES_ARRAY(float, z,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0,
		0);
	};

	struct dnn
	{
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
				// no 0 !
				SFERES_ARRAY(float, values, -2, -1, 1, 2); //???-2??
				SFERES_CONST float mutation_rate = 1.0f;
				SFERES_CONST float cross_rate = 0.5f;
				SFERES_CONST bool ordered = true;
			};
		};
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
		};

		SFERES_CONST size_t nb_inputs = 8;
# ifdef SEASONS
		SFERES_CONST size_t nb_outputs = 2;
# else
		SFERES_CONST size_t nb_outputs = 1;
# endif

		// slope of the sigmoid
		SFERES_CONST size_t lambda = 20;

		SFERES_CONST size_t min_nb_neurons = 10; //useless
		SFERES_CONST size_t max_nb_neurons = 30; //useless
		SFERES_CONST size_t min_nb_conns = 0; // ?? TODO
		SFERES_CONST size_t max_nb_conns = 120; // ?? TODO


	    SFERES_CONST float m_rate_add_conn = DnnConAddMutRate::v;		//Add Connection rate for the dnn
	    SFERES_CONST float m_rate_del_conn = DnnConDelMutRate::v;		//Delete connection rate for the dnn

		//SFERES_CONST float m_rate_change_conn = 0.15; //old default
		SFERES_CONST float m_rate_change_conn = 0.0; //new default?
		SFERES_CONST float m_rate_add_neuron = 0.0f;
		SFERES_CONST float m_rate_del_neuron = 0.0f;

		// average number of weigth changes
		SFERES_CONST float m_avg_weight = 2.0f;

		SFERES_CONST int io_param_evolving = true;
		SFERES_CONST init_t init = ff;
		struct spatial
		{
			SFERES_CONST float min_coord = -4.0f;
			SFERES_CONST float max_coord = 4.0f;
#ifdef RANDCOORDS
			SFERES_CONST bool rand_coords = true;
#else
			SFERES_CONST bool rand_coords = false;
#endif

#ifdef SEASONS
			SFERES_ARRAY(int, layers, 8, 8, 4, 2, 2);
			SFERES_ARRAY(float, x,
			-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
			-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
			-2, -1, 1, 2,
			-1, 1,
			-1, 1);
			SFERES_ARRAY(float, y,
			0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1, 1, 1,
			2, 2, 2, 2,
			3, 3,
			4, 4);
			SFERES_ARRAY(float, z,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0,
			0, 0);
#elif defined(MEDIUM_SEP)
			//wider separation (best with tlength of 3.5)
			SFERES_ARRAY(int, layers, 8, 8, 4, 2, 1);
			SFERES_ARRAY(float, x,
					-5, -4, -3, -2, 2, 3, 4, 5,
					-5, -4, -3, -2, 2, 3, 4, 5,
					-3, -2, 2, 3,
					-2, 2,
					0);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2,
					3, 3,
					4);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0,
					0, 0,
					0);
#elif defined(LARGE_SEP)
			SFERES_ARRAY(int, layers, 8, 8, 4, 2, 1);
			SFERES_ARRAY(float, x,
					-13, -12, -11, -10, 10, 11, 12, 13,
					-13, -12, -11, -10, 10, 11, 12, 13,
					-11, -10, 10, 11,
					-10, 10,
					0);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2,
					3, 3,
					4);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0,
					0, 0,
					0);
#else
			//Current default
			//used in original experiments
			SFERES_ARRAY(int, layers, 8, 8, 4, 2, 1);
			SFERES_ARRAY(float, x,
			-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
			-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
			-2, -1, 1, 2,
			-1, 1,
			0);
			SFERES_ARRAY(float, y,
			0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1, 1, 1,
			2, 2, 2, 2,
			3, 3,
			4);
			SFERES_ARRAY(float, z,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0,
			0);

#endif
		};
	};

	struct parameters
	{
#ifdef DISCREET
		// maximum value of parameters (weights & bias)
		SFERES_CONST float min = 0;
		// minimum value
		SFERES_CONST float max = 1;
#else
		// maximum value of parameters (weights & bias)
		SFERES_CONST float min = -2;
		// minimum value
		SFERES_CONST float max = 2;
#endif
	};
	struct stats
	{
		//SFERES_CONST size_t period = 1; //print stats every N generations
		SFERES_CONST size_t period = 1; //print stats every N generations
		SFERES_CONST size_t logall_period = 100;
	};



	/***********************/
	/*** CPPN PARAMETERS ***/
	/***********************/
	struct ParamsHnn {

        struct multi_spatial{
            static NeuronGroups* substrate_structure;
        };

#ifdef NO_LEO
        typedef phen::hnn::HnnConstants hnn_const_t;
#else
        typedef phen::hnn::HnnLeoConstants hnn_const_t;
#endif

#ifdef LEO
#ifdef LEO_0_66
		struct leo
		{
			SFERES_CONST float input_to_seed1 = -0.66;
			SFERES_CONST float input_to_seed2 = 0.66;
#ifdef LEO_BIAS_1
			SFERES_CONST float bias_to_leo = -1;
#else
			SFERES_CONST float bias_to_leo = -2;
#endif
		};
#else
		struct leo
		{
			SFERES_CONST float input_to_seed1 = -1.0;
			SFERES_CONST float input_to_seed2 = 1.0;
#ifdef LEO_BIAS_1
			SFERES_CONST float bias_to_leo = -1;
#else
			SFERES_CONST float bias_to_leo = -2;
#endif
		};
#endif



#endif


		struct cppn {
			// params of the CPPN
			// These values define the mutations on the activation function of the CPPN nodes
			// As such, they define which activation function can be chosen and the mutation rate for changing the activation function
			struct sampled {
				SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid, nn::cppn::gaussian, nn::cppn::linear); //Possible activation functions
				SFERES_CONST float mutation_rate = 0.1f;			//Mutation rate for the cppn nodes
				SFERES_CONST float cross_rate = 0.0f;				//Not actually used, even if non-zero
				SFERES_CONST bool ordered = false;					//Mutation will randomly pick an other activation function from the list
				//The alternative has bias towards adjacent values
			};

			// These value determine the mutation rate for the weights and biases of the cppn
			// Note that, in practice, the bias is determined by the weight of the connection between
			// the bias input node and the target node, so biases are not mutated 'directly'.
			struct evo_float {
				SFERES_CONST float mutation_rate = 0.1f;			//Mutation rate for the cppn nodes
				SFERES_CONST float cross_rate = 0.0f; 				//Not actually used, even if non-zero
				SFERES_CONST mutation_t mutation_type = polynomial;	//Mutation type
				SFERES_CONST cross_over_t cross_over_type = sbx;	//Not used
				SFERES_CONST float eta_m = 10.0f;					//Mutation eta (parameter for polynomial mutation.
				SFERES_CONST float eta_c = 10.0f;					//?
			};

			struct parameters
			{
				// minimum value
				SFERES_CONST float min = -3;
				// maximum value of parameters (weights & bias)
				SFERES_CONST float max = 3;
			};

//#if defined(NO_LEO)
            SFERES_CONST float leo_threshold = 0.0f;
//#else
			//This threshold determines at what value of the Link-Expression Output (LEO) a link will be expressed or not.
			//Before the threshold LEO values are scaled between 0 and 1 and only values equal to or above the threshold are expressed.
			//As a result a value of 0.0 means all links are expressed (effectively disabling the LEO node)
			//while a value greater than 1.0 means no values will ever be expressed (probably a bad idea).
			//A value of 0.5 is default
//			SFERES_CONST float leo_threshold = 0.5f;
//#endif

            //In the absence of LEO, or to prevent extremely weak connections, this thresholds determines above
            //what strength a connection is added. Note: the connections below this value become impossible.
			SFERES_CONST float con_threshold = 0.0f;

			typedef gen::RandomSeed seed_t;
		};

		//Properties of the substrate
		//struct hnn {
		//	SFERES_CONST float min = -3; // maximum value of parameters (weights & bias)
		//	SFERES_CONST float max = 3;  // minimum value
		//	SFERES_CONST size_t nb_rand_mutations = 10; //Number of random mutations applied to the first generation
		//};

		//These are the parameters used for setting the CPPN
		//The 7 inputs are the x, y, z coordinates for neuron 1
		//The x, y, z coordinates for neuron 2
		//And a bias.
		//The outputs are the weights and the link expression, and the bias.
		struct dnn
		{
#ifdef DISTI
			SFERES_CONST size_t nb_inputs = 8;
			typedef phen::hnn::DistanceInput<Params::dnn> input_t;
#else
			SFERES_CONST size_t nb_inputs = 7;
			typedef phen::hnn::ThreeDInput<Params::dnn> input_t;
#endif

			SFERES_CONST size_t nb_outputs = 3;
			SFERES_CONST init_t init = ff;

			SFERES_CONST float m_rate_add_conn = CppnConAddMutRate::v;		//Add Connection rate for the cppn
			SFERES_CONST float m_rate_del_conn = CppnConDelMutRate::v;		//Delete connection rate for the cppn
			SFERES_CONST float m_rate_add_neuron = CppnNeuAddMutRate::v;		//Add neuron rate for the cppn
			SFERES_CONST float m_rate_del_neuron = CppnNeuDelMutRate::v;		//Delete neuron rate for the cppn

			SFERES_CONST float m_rate_change_conn = 0.0f;		//Change connection rate for the cppn
			//(e.g. tears down one side of a connection, then reconnects this side to an other neuron.
			//Not used for the cppn, even if value is non zero.)
			SFERES_CONST size_t min_nb_neurons = 7;
			SFERES_CONST size_t max_nb_neurons = 20;
			SFERES_CONST size_t min_nb_conns = 0;
			SFERES_CONST size_t max_nb_conns = 150;
		};
	};
};
)

mpi_init_ptr_t Params::mpi::masterInit = 0;
mpi_init_ptr_t Params::mpi::slaveInit = 0;
NeuronGroups* Params::ParamsHnn::multi_spatial::substrate_structure = 0;
bool Params::init = false;


/********************************
 *       INITIALIZATION         *
 ********************************/

void joint_init(){
    dbg::trace trace("init", DBG_HERE);
    if(Params::init) return;

    //Bring some static variables into scope
//    SFERES_CONST unsigned nb_problems = Params::problems::nb_of_problems;
//    SFERES_CONST unsigned nb_layers = Params::problems::layers_size();

    //Set layers
//    size_t nb_of_neurons = 0;
//    SFERES_INIT_NON_CONST_ARRAY(Params::dnn::spatial, layers, Params::problems::layers_size());
//    for(size_t i=0; i<Params::problems::layers_size(); ++i){
//        Params::dnn::spatial::layers_set(i, Params::problems::layers(i) * nb_problems);
//        nb_of_neurons += Params::problems::layers(i) * nb_problems;
//    }

    //Create neuron groups object and set its properties
    NeuronGroups* neuronGroups = new NeuronGroups();
    neuronGroups->setSeparateIndexPerLayer(false);
    neuronGroups->setSeparateIndexPerLayerNeurons(false);
    neuronGroups->setHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setOutputCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setInputToHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setHiddenToHiddenCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setHiddenToOutputCppnIndex(NeuronGroupsEnum::staticIndex);
    neuronGroups->setNbOfCppnOutputsNeuron(Params::ParamsHnn::hnn_const_t::nb_of_outputs_neuron);
    neuronGroups->setNbOfCppnOutputsCon(Params::ParamsHnn::hnn_const_t::nb_of_outputs_connection);

    //Set groups and connect them
    neuronGroups->addInput(Params::dnn::spatial::layers(0), 0);
    for(size_t layer_index=1; layer_index<(Params::dnn::spatial::layers_size()-1); ++layer_index){
        neuronGroups->addHidden(Params::dnn::spatial::layers(layer_index), layer_index);
    }
    neuronGroups->addOutput(Params::dnn::spatial::layers(Params::dnn::spatial::layers_size()-1), Params::dnn::spatial::layers_size()-1);
    neuronGroups->connectLayeredFeedForwardFull();
    neuronGroups->build();

    if(options::map["print-neuron-groups"].as<bool>()){
        neuronGroups->print();
    }

    Params::ParamsHnn::multi_spatial::substrate_structure = neuronGroups;
    Params::init = true;
}


void master_init(){
    dbg::trace trace("init", DBG_HERE);
    joint_init();
}


void slave_init(){
    dbg::trace trace("init", DBG_HERE);
    joint_init();
}

//IMPORTANT: Not choosing at least one fitness (for example FG_AND or FG_OR) will result in a segmentation fault.
SFERES_FITNESS(FitRetina, sferes::fit::Fitness) {
public:
	FitRetina() : _postFix(""), _age(0) { // @suppress("Class members should be properly initialized")
	}
	int age() const { return _age; }
	void set_age(int a) { _age = a; }

	template<typename Indiv>
	float dist(const Indiv& o) const {
		return (o.fit()._behavior ^ _behavior).count() / (float) _behavior.size();
	}
	// size of the behavior vector
#ifdef SEASONS
	SFERES_CONST size_t b_size = 512;
#else
	SFERES_CONST size_t b_size = 256;
#endif

	template<typename Indiv>
	void eval(Indiv& ind) {
	    dbg::trace trace("eval", DBG_HERE);
#ifdef TESTSUB
		ind.gen().mutate();
		ind.develop();
#endif
		this->_objs.clear();
		this->_age++;
		_create_objects();
		float err1 = 0, err2 = 0;
		// for seasons
		std::vector<float> errs_o1(4), errs_o2(4);
		std::fill(errs_o1.begin(), errs_o1.end(), 0.0f);
		std::fill(errs_o2.begin(), errs_o2.end(), 0.0f);

		float ex1 = 0, ex2 = 0;

		std::vector<float> inputs(8);
		size_t k = 0;


		//Create the neural network
		ind.nn().init();

		float totalOutput=0;
		//std::cout << "Calculating error" << std::endl;
		for (size_t s = 0; s < 256; ++s) {
			//std::cout << "...Checking object" << s << std::endl;
			std::bitset<8> in(s);
			for (size_t i = 0; i < in.size(); ++i)
				inputs[i] = in[i] ? 1 : -1;
			bool left = std::find(_left_objects.begin(),
			        _left_objects.end(),
			        (s & 0xF)) != _left_objects.end();
			bool right = std::find(_right_objects.begin(),
			        _right_objects.end(),
			        (s & 0xF0)) != _right_objects.end();

			// step

			for (size_t i = 0; i < ind.nn().get_depth(); ++i)
				ind.nn().step(inputs);

#ifdef SEASONS
			float o1 = ind.nn().get_outf(0);
			float o2 = ind.nn().get_outf(1);

			_seasons(left, right, o1, errs_o1);
			_seasons(left, right, o2, errs_o2);

			_behavior[k] = o1 > 0 ? 1 : 0;
			_behavior[k + 1] = o2 > 0 ? 1 : 0;

			k += 2;
#else
			//std::cout << "...get_outf" << std::endl;
			float out = ind.nn().get_outf(0);
			totalOutput+=out;

#ifdef ROBJECTS
			size_t nb_white = in.count();
			if (nb_white == 4)
				err1 += (out > 0 ? 0 : 1);
			else
				err1 += (out < 0 ? 0 : 1);
#else
			// left & right
			// FIXME This definition makes that returning zero is always wrong, is this intentional?
			if (left && right){
				//std::cout << "Left and right: " << out << std::endl;
				err1 += (out > 0 ? 0 : 1);
			} else {
				//std::cout << "Otherwise: " << out << std::endl;
				err1 += (out < 0 ? 0 : 1);
			}
#endif
			if (left || right)
				err2 += (out > 0 ? 0 : 1);
			else
				err2 += (out < 0 ? 0 : 1);

//			std::cout << out << " ";

			_behavior[s] = out > 0 ? 1 : 0;
#ifdef TESTSUB
			// test if there are two xor modules
			size_t k = 0;
			BGL_FORALL_VERTICES_T(v, ind.nn().get_graph(), typename Indiv::nn_t::graph_t)
			std::cout << "nn " << ind.nn().get_graph()[v].get_id() << " "
			        << s << " " << k++ << " "
			        << ind.nn().get_graph()[v].get_afparams().data(0) << " "
			        << ind.nn().get_graph()[v].get_current_output() << std::endl;
#endif
#endif
		}

		//std::cout << "Calculating error - done" <<std::endl;

#if defined(MVG) && !defined(SEASONS)
		this->_objs.push_back(-42);
#endif



#ifdef SEASONS

		if (this->mode() == fit::mode::view) {
			std::cout << "o1:" << std::endl;
			for (size_t i = 0; i < errs_o1.size(); ++i)
				std::cout << errs_o1[i] << " ";
			std::cout << std::endl;

			std::cout << "o2:" << std::endl;
			for (size_t i = 0; i < errs_o2.size(); ++i)
				std::cout << errs_o2[i] << " ";
			std::cout << std::endl;
		}

		this->_objs.push_back(-43);
		this->_values.resize(2);
		assert(errs_o1.size() == errs_o2.size());
		for (size_t i = 0; i < errs_o1.size(); ++i)
			errs_o1[i] = 0.5 - errs_o1[i] / 512.0f;
		for (size_t i = 0; i < errs_o2.size(); ++i)
			errs_o2[i] = 0.5 - errs_o2[i] / 512.0f;

		// L&R : 0
		// L|R : 1
		// L : 2
		// R : 3
		// SI1 : L&R + L&R
		// SI2 : L|R + L|R
		// SI3 : L|R + L&R
		// SI4 : L&R + R
		// SI5 : L|R + R
		// SI6 : L + R

#ifndef HFS
		this->_values.resize(2);
#if   defined(SI1) // L&R + L&R
		this->_values[0] = errs_o1[0] + errs_o2[0];
#elif defined(SI2) // L|R + L|R
		this->_values[0] = errs_o1[1] + errs_o2[1];
#elif defined(SI3) // L|R + L&R
		this->_values[0] = errs_o1[0] + errs_o2[1];
#elif defined(SI4) // L&R + R
		this->_values[0] = errs_o1[0] + errs_o2[3];
#elif defined(SI5) // L|R + R
		this->_values[0] = errs_o1[1] + errs_o2[3];
#elif defined(SI6) // L + R
		this->_values[0] = errs_o1[2] + errs_o2[3];
#endif

#if   defined(SP1) // L&R + L&R
		this->_values[1] = errs_o1[0] + errs_o2[0];
#elif defined(SP2) // L|R + L|R
		this->_values[1] = errs_o1[1] + errs_o2[1];
#elif defined(SP3) // L|R + L&R
		this->_values[1] = errs_o1[0] + errs_o2[1];
#elif defined(SP4) // L&R + R
		this->_values[1] = errs_o1[0] + errs_o2[3];
#elif defined(SP5) // L|R + R
		this->_values[1] = errs_o1[1] + errs_o2[3];
#elif defined(SP6) // L + R
		this->_values[1] = errs_o1[2] + errs_o2[3];
#endif
#else
		this->_values.resize(6);
		this->_values[0] = errs_o1[0] + errs_o2[0];
		this->_values[1] = errs_o1[1] + errs_o2[1];
		this->_values[2] = errs_o1[0] + errs_o2[1];
		this->_values[3] = errs_o1[0] + errs_o2[3];
		this->_values[4] = errs_o1[1] + errs_o2[3];
		this->_values[5] = errs_o1[2] + errs_o2[3];
#endif


		if (this->mode() == fit::mode::view)
			for (size_t i = 0; i < this->_values.size(); ++i)
				std::cout << "season " << i << " " << this->_values[i] << std::endl;

#else
		this->_values.resize(2);
		this->_values[0] = 1 - err1 / 256.0f;
		this->_values[1] = 1 - err2 / 256.0f;
#endif

		this->_value = this->_values[0];

		// compute the modularity
		typename Indiv::nn_t::graph_t gsimp = ind.nn().simplified_nn().get_graph();

		//std::cout << "Calculating modularity" << std::endl;
		if (boost::num_edges(gsimp) == 0 || boost::num_vertices(gsimp) == 0)
			_mod = 0;
		else
			_mod = mod::modularity(gsimp);
		//compute_mod_simplified(ind.gen());
		//std::cout << "Calculating modularity - done" << std::endl;


#if defined(FG_AND)  || defined(ROBJECTS)
		//std::cout << "Error: " << err1 << std::endl;
//		std::cout << "Fitness: " << (1 - err1 / 256.0f) << std::endl;
//		std::cout << "Summed output: " << totalOutput << std::endl;
		this->_objs.push_back(1 - err1 / 256.0f);
		this->_value = this->_values[0];
#endif

#if defined(FG_OR)
		this->_objs.push_back(1 - err2 / 256.0f);
		this->_value = this->_values[1];
#endif

		this->_nb_conns = boost::num_edges(gsimp);
#ifdef NCONNS
		this->_objs.push_back(-(int)boost::num_edges(gsimp));
#endif

		this->_nb_nodes = boost::num_vertices(gsimp);
#ifdef NNODES
		this->_objs.push_back(-(int)boost::num_vertices(gsimp));
#endif

#ifdef NCNODES
		this->_objs.push_back(-(int)boost::num_edges(gsimp) - (int)boost::num_vertices(gsimp));
#endif



#ifdef MOD
		this->_objs.push_back(_mod);
#endif


		_length = ind.nn().compute_length();
		_optlength = ind.nn().compute_optimal_length(0, false);


#ifdef MAX_LENGTH
		this->_objs.push_back(-ind.gen().max_length());
#endif

#if defined(TLENGTH) && !defined(ONP)
		this->_objs.push_back(-GET_SPATIAL_NN(ind).compute_length(3.5)); //use with wider geometric separation, which is the new default
		//this->_objs.push_back(-ind.gen().compute_length(1.5)); //used in original runs
#endif


#if defined(TLENGTH) && defined(ONP)
		bool write_svg = (this->mode() == fit::mode::view);
		this->_objs.push_back(-ind.gen().compute_optimal_length(3.5, write_svg));
#endif

#if defined(LENGTH) && !defined(ONP)
		this->_objs.push_back(-_length);
#endif

#if defined(LENGTH) && defined(ONP)
		bool write_svg = (this->mode() == fit::mode::view);
		this->_objs.push_back(-ind.gen().compute_optimal_length(0, write_svg));
#endif

#if defined(ONP_VIEW)
		ind.compute_optimal_length(0, true);
#endif

#if defined(SLENGTH) && defined(ONP)
		bool write_svg = (this->mode() == fit::mode::view);
		float l = ind.gen().compute_optimal_length(0, write_svg);
		this->_objs[0] = ((1 - err1 / 256.0f) - l / 2000.0f);
#endif


#if defined(DIV) || defined(AGE) || defined(NOVMAP)
		this->_objs.push_back(0);
#endif

#ifdef RANDOBJ
		this->_objs.push_back(ind.gen().rand_obj());
#endif

		if (this->mode() == sferes::fit::mode::view) {
			/*** Perform analysis ***/
			//Sub-problem analysis
			LeftRightAnalysis<Params, Indiv> left_right_analysis(_left_objects, _right_objects);
			left_right_solve = left_right_analysis.analysis_left_right_cases(ind) ? 1.0 : 0.0;
			one_left_right_solved.resize(4);
			two_left_right_solved.resize(4);

			for(int i=0; i<4; i++){
				one_left_right_solved[i] = left_right_analysis.one_solved(i+1);
				two_left_right_solved[i] = left_right_analysis.two_solved(i+1);
			}

			//Left-right analysis
			std::set<int> left = boost::assign::list_of(0)(1)(2)(3);
			std::set<int> right = boost::assign::list_of(4)(5)(6)(7);
			left_right_mod = (analysis_io_mod(ind, left, right)==8);

			//Regularity analysis
			regularity_score = regularity::analysis_regularity(ind);
			//regularity_score=0;

			//Dyn mod analysis
			analysis_dyn_mod(ind);

			/*** Write networks ***/
			std::ofstream ofs3(("nn"+ _postFix +".svg").c_str());
			ind.nn().write_svg(ofs3);
//			ind.nn().compute_optimal_length(0, true, "onp" + _postFix + ".svg", left_right_analysis.getSolvedMap());
			ind.nn().compute_optimal_length(0, true, "onp" + _postFix + ".svg");
#ifdef HNN
			std::ofstream ofs(("cppn" + _postFix + ".dot").c_str());
			ind.write_dot_cppn(ofs);
#endif

			/*** Print information ***/
//			std::cout << "behavior:" << _behavior << std::endl;
//			std::cout << "- AND: " << 1 - err1 / 256.0f << std::endl;
//			std::cout << "- OR: " << 1 - err2 / 256.0f << std::endl;
//			std::cout << "- MOD: " << _mod << std::endl;
//			std::cout << "- length: " << _length << std::endl;
//			std::cout << "- nb neurons: " << ind.nn().get_nb_neurons() << std::endl;
//			std::cout << "- nb conns: " << ind.nn().get_nb_connections() << std::endl;
//			std::cout << "- lr-solve: " << left_right_solve << std::endl;
//			std::cout << "- reg: " << regularity_score << std::endl;

		}
	}


    /**
     * Performs additional analysis of this individual, calculating metrics that are
     * too computational expensive to do every generation. Should guarantee that it,
     * does not write any additional files.
     */
    template<typename Indiv>
    void analyze(Indiv& ind){

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

    }

    /**
     * Synonym of dump.
     */
    template<typename Indiv>
    void visualizeNetwork(Indiv& ind){dump(ind);}

	float values(size_t k) const { return _values[k]; }

	size_t nb_values() const { return _values.size(); }

	void set_value(float v) { this->_value = v; }

	const std::bitset<b_size>& behavior() const { return _behavior; }

	float mod() const { return _mod; }

	float length() const { return _length; }

	float optlength() const { return _optlength; }

	int nb_nodes() const { return _nb_nodes; }

	int nb_conns() const { return _nb_conns; }

	void setPostFix(std::string postFix){_postFix = postFix;}

	bool get_left_right_mod(){ return left_right_mod;}

	float get_left_right_solve(){ return left_right_solve;}
	size_t get_solved_length(){return one_left_right_solved.size();}
	bool get_one_solved(int i){ return one_left_right_solved[i];}
	bool get_two_solved(int i){ return two_left_right_solved[i];}

	float get_dyn_mod_score(){ return dyn_mod_score;}

	float get_regularity_score(){ return regularity_score; }

	template<typename I>
	float get_original_length(I& indiv){
		return regularity::get_original_length(indiv);
		//return 1;
	}

	void setId(size_t id){_id = id;}
	size_t getId(){ return _id;}
protected:

	bool left_right_mod;
	float left_right_solve;
	std::vector<bool> one_left_right_solved;
	std::vector<bool> two_left_right_solved;
	float dyn_mod_score;
	float regularity_score;
	std::string _postFix;

	std::vector<float> _values;
	std::vector<size_t> _left_objects;
	std::vector<size_t> _right_objects;
	std::vector<size_t> _rand_objects;


	std::bitset<b_size> _behavior;
	int _age;

	float _mod, _mod2;
	float _length, _optlength;
	int _nb_nodes, _nb_conns;
	size_t _id;



	/**
	 * Dynamic modularity analysis.
	 *
	 * This function measures how many neurons that change on the inputs of one side
	 * do not change on the inputs of the other side of the retina.
	 */
	template<typename I>
	void analysis_dyn_mod(I& indiv) {
		std::vector<float> inputs(8);
		std::vector<std::vector<bool> > changed(2);
		changed[0].resize(indiv.nn().get_nb_neurons());
		changed[1].resize(indiv.nn().get_nb_neurons());

		//For 2
		for (size_t c = 0; c < changed.size(); ++c) {
			std::fill(changed[c].begin(), changed[c].end(), false); // @suppress("Ambiguous problem")
//			std::fill<std::vector<bool>::iterator>(changed[c].begin(), changed[c].end(), false);

			//For all possible numbers that can be made with 4 bits
			for (size_t i = 0; i < 16; ++i) {
				std::bitset<8> obj(i);
				std::vector<bool> prev_state(indiv.nn().get_nb_neurons());

				//Also for all possible numbers that can be made with 4 bits
				for (size_t s = 0; s < 16; ++s) {
					std::bitset<8> in(s);

					//If c==0 in will contain the in pattern left and the obj pattern right
					//If c==1 in will contain the obj pattern left and the obj pattern right
					if (c == 0)
						in = (in << 4) | obj;
					else
						in = (obj << 4) | in;

					//Activate the network with the supplied patterns
					for (size_t j = 0; j < in.size(); ++j)
						inputs[j] = in[j] ? 1 : -1;

					for (size_t j = 0; j < indiv.nn().get_depth(); ++j)
						indiv.nn().step(inputs);

					//Record the state of each neuron
					size_t k = 0;
					std::vector<bool> state(indiv.nn().get_nb_neurons());
					BGL_FORALL_VERTICES_T(v, indiv.nn().get_graph(), typename I::nn_t::graph_t){
						state[k++] = indiv.nn().get_graph()[v].get_current_output() > 0;
					}

					//Did this neuron change at any point between 0 <= s < 16 while i is i;
					if (s != 0)
						for (size_t j = 0; j < state.size(); ++j)
							if (state[j] != prev_state[j])
								changed[c][j] = true;
					prev_state = state;
				}
			}
		}

		float diffs = 0.0f;
		for (size_t j = 0; j < changed[0].size(); ++j)
			if (changed[0][j] != changed[1][j])
				diffs += 1;
		float score = diffs / changed[0].size();
//		std::cout << "- DM: " << score << std::endl;
		dyn_mod_score = score;
	}

	void _create_objects() {
// #ifdef ROBJECTS
//       typedef boost::mt19937 base_generator_t;
//       typedef boost::variate_generator<base_generator_t, boost::uniform_int<> > rand_t;
//       rand_t rand_gen(base_generator_t(42u), boost::uniform_int<>(0, 255));
//       for (size_t i = 0; i < 8; ++i)
// 	_left_objects.push_back(rand_gen());
//       for (size_t i = 0; i < 8; ++i)
// 	_right_objects.push_back(rand_gen());

// #else
		//(1, 2, 4, 8 - 16, 32, 64, 128)
		_left_objects.push_back(15);	//(1111 - 0000)
		_left_objects.push_back(7);		//(1110 - 0000)
		_left_objects.push_back(5);		//(1010 - 0000)
		_left_objects.push_back(13);	//(1011 - 0000)

		_left_objects.push_back(1);		//(1000 - 0000)
		_left_objects.push_back(11);	//(1101 - 0000)
		_left_objects.push_back(4);		//(0010 - 0000)
		_left_objects.push_back(14);	//(0111 - 0000)
		//
		_right_objects.push_back(240);	//(0000 - 1111)
		_right_objects.push_back(224);	//(0000 - 0111)
		_right_objects.push_back(160);	//(0000 - 0101)
		_right_objects.push_back(176);	//(0000 - 1101)

		_right_objects.push_back(128);	//(0000 - 0001)
		_right_objects.push_back(208);	//(0000 - 1011)
		_right_objects.push_back(32);	//(0000 - 0100)
		_right_objects.push_back(112);	//(0000 - 1110)

		//#endif

	}

	void _seasons(bool left, bool right, float out, std::vector<float>& errs) {
		assert(errs.size() == 4);
		if (left && right)
			errs[0] += (out > 0 ? 0 : 1);
		else
			errs[0] += (out < 0 ? 0 : 1);

		if (left || right)
			errs[1] += (out > 0 ? 0 : 1);
		else
			errs[1] += (out < 0 ? 0 : 1);

		if (left)
			errs[2] += (out > 0 ? 0 : 1);
		else
			errs[2] += (out < 0 ? 0 : 1);

		if (right)
			errs[3] += (out > 0 ? 0 : 1);
		else
			errs[3] += (out < 0 ? 0 : 1);

	}
};

namespace sferes {
namespace stat {
SFERES_CLASS(RetinaStop) {
public:
	template<typename E>
	void refresh(const E& ea) const {
		if (stop_criterion(ea)) {
			std::cout << "Stop criterion reached" << std::endl;
			ea.write();
			exit(0);
		}
	}
	void show(std::ostream& os, size_t k)
	{}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{}

	template<typename EA>
	bool stop_criterion(const EA& ea) const {
		for (size_t i = 0; i < ea.pop().size(); ++i)
			if (ea.pop()[i]->fit().value() == 1.0)
				return true;
		return false;

	}
};
}
}


namespace sferes {
namespace stat {
SFERES_CLASS(NoStop) {
public:
	template<typename E>
	void refresh(const E& ea) const {
		//Do nothing
	}

	void show(std::ostream& os, size_t k)
	{}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{}

	template<typename EA>
	bool stop_criterion(const EA& ea) const {
		return false;
	}
};
}
}


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


long Params::seed = -1;
float Params::ea::pressure = 0.1;

int main(int argc, char **argv) {

    Params::mpi::slaveInit = &slave_init;
    Params::mpi::masterInit = &master_init;
#ifdef TEST
	misc::seed(time(NULL));
	std::cout << "Bitset   1: " << misc::randBitset<1>() << std::endl;
	std::cout << "Bitset   2: " << misc::randBitset<2>() << std::endl;
	std::cout << "Bitset   3: " << misc::randBitset<3>() << std::endl;
	std::cout << "Bitset  15: " << misc::randBitset<15>() << std::endl;
	std::cout << "Bitset  16: " << misc::randBitset<16>() << std::endl;
	std::cout << "Bitset  17: " << misc::randBitset<17>() << std::endl;
	std::cout << "Bitset 100: " << misc::randBitset<100>() << std::endl;
	misc::seed(1);
	std::cout << "Seed 1, bitset 10: " << misc::randBitset<10>() << std::endl;
	misc::seed(2);
	std::cout << "Seed 2, bitset 10: " << misc::randBitset<10>() << std::endl;
	misc::seed(1);
	std::cout << "Seed 1, bitset 10: " << misc::randBitset<10>() << std::endl;
#else


	//The fitness function is defined by the FitRetina class defined above
	//Using the parameters also defined above
	typedef FitRetina<Params> fit_t;

#ifdef DISCREET
	//Creates the type definition for weights based on the class called Parameters, which is a subclass of Indiv.
	typedef phen::Parameters<
				gen::Sampled<1, Params::dnn::weights>, 	//The genotype of weights
				fit::FitDummy<>, 						//The fitness function of weights (Which is a dummy since the weights do not have there own fitness).
				Params									//The parameters for weights (defined above)
	> weight_t;

	//Creates the type definition for weights based on the class called Parameters, which is a subclass of Indiv.
	typedef phen::Parameters<
			gen::Sampled<1, Params::dnn::bias>,			//The genotype of the bias, the template parameters indicate that there will only be 1 parameter,
														//And this parameter will be of the type bias, which holds several other parameters.
			fit::FitDummy<>,							//The fitness function of bias (Which is a dummy since the bias do not have there own fitness).
			Params										//The parameters for bias (defined above)
	> bias_t;
#else
	typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::weights>,
	        fit::FitDummy<>, Params> weight_t;
	typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::bias>,
	        fit::FitDummy<>, Params> bias_t;
#endif

	//Defines the Potential function of our Neural Network as the Weighted Sum.
	typedef PfWSum<weight_t> pf_t;

	//Defines the Activation function of our Neural Network as a -1/+1 Sigmoid function (defined above).
	//It takes the parameter values defined in bias, probably named this why because the bias is the only
	//Part that is actually evolved about the neurons.
	typedef AfRetina<bias_t> af_t;

	//Use the Direct Neural Network as a genotype, were each neuron has the PfWSum Potential function and the Sigmoid activation function,
	//Each connection has uses the weights parameters, and the entire network uses Params.
	typedef sferes::gen::DnnSpatial<Neuron<pf_t, af_t>, Connection<weight_t>, Params> substrate_t;


#ifdef HNN
    typedef phen::Parameters<gen::EvoFloat<1, Params::ParamsHnn::cppn>,fit::FitDummy<>,Params::ParamsHnn::cppn> cppn_weight_t;
    typedef nn::PfWSum<cppn_weight_t> cppn_pf_t;
    typedef nn::AfCppn<nn::cppn::AfParams<Params::ParamsHnn::cppn> > cppn_af_t;
    typedef gen::DnnFF<nn::Neuron<cppn_pf_t, cppn_af_t>, nn::Connection<cppn_weight_t>, Params::ParamsHnn> cppn_t;
    typedef gen::HnnWS<cppn_t, Params::ParamsHnn, Params::ParamsHnn::cppn> gen_t;

//	//Set parameters
//	typedef phen::Parameters<
//			gen::EvoFloat<1, Params::ParamsHnn::cppn>, 	//Set the genotype to cppn
//			fit::FitDummy<>, 					//Set fitness to dummy
//			Params::ParamsHnn::cppn						//Pass the cppn parameters
//	> cppn_weight_t;
//
//	//Set the genotype to HyperNEAT
//	typedef gen::Hnn<cppn_weight_t, Params::ParamsHnn, Params::ParamsHnn::cppn> gen_t;

	//Set the pheontype to HyperNEAT
#if defined(NO_LEO)
	typedef phen::hnn::Hnn<substrate_t, gen_t, fit_t, Params::ParamsHnn> phen_t;
#else
	typedef phen::hnn::HnnLeo<substrate_t, gen_t, fit_t, Params::ParamsHnn> phen_t;
#endif
#else
	//The phenotype is the direct Neural network, using the genotype, fitness function and parameters defined above.
	typedef phen::Dnn<substrate_t, fit_t, Params> phen_t;
#endif

	//typedef eval::Parallel<Params> eval_t;
#if defined(MPI_ENABLED) && !defined(NOMPI)
	typedef eval::EvalTask<phen_t> eval_task_t;
	typedef boost::fusion::vector<eval_task_t> tasks_t;
	typedef eval::MpiWorldTracking<Params, tasks_t> eval_t;
#else
	typedef eval::ParallelWorldTracking<Params> eval_t;
#endif
//#if !defined(__APPLE__) || defined (VMDMESA)
//	typedef eval::Mpi<Params> eval_t;
//#else
//	typedef eval::Eval<Params> eval_t;
//#endif

	typedef boost::fusion::vector
	<
//		stat::ParetoFront<phen_t, Params>,
		stat::Mod<phen_t, Params>
	    //,stat::ParetoFront<phen_t, Params> // comment this line to remove pareto.dat
#ifdef LOGALL
	, stat::LogAll<phen_t, Params>
#endif
#ifdef STOP
	, stat::RetinaStop<Params>
#endif
	>  stat_t;

#ifdef UNTILPERFECT
	typedef stat::RetinaStop<Params> stop_t;
#else
	typedef stat::NoStop<Params> stop_t;
#endif

	// MVG cannot be used with DIV !

#if defined(DIV) and defined(MVG)
	std::cout << "MVG cannot be used with DIF!!!" << std::endl;
	System.exit(1);
#endif

#if defined(DIV) and not defined(GRADUAL)		//Behavioral distance based only on current population.
	typedef modif::BehaviorDiv<> modifier_t;
#elif defined(MVG)		//Multiple objectives that switch every period.
	typedef modif::Mvg<> modifier_t;
#elif defined(AGE)		//Age based modifier.
	typedef modif::Age<> modifier_t;
#elif defined(NOVMAP) 	//Novelty search map based on behavior.
	typedef modif::Novmap<> modifier_t;
#elif defined(GRADUAL)
	typedef modif::Gradual<Params> modifier_t;
#else					//No modifier
	typedef modif::Dummy<> modifier_t;
#endif

	typedef boost::fusion::vector< stat::Mod<phen_t, Params> > convert_t;
	convert_t convert;

#ifdef PNSGA
	//Use the probabilistic variant of NSGA 2
	typedef ea::Pnsga<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
	typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#elif defined(MPNSGA)
	//Use the probabilistic variant of NSGA 2 which supports multiple p-values being smaller than one.
	typedef ea::Pnsga_mult<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
	typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#else
	//Use the multi-objective evolutionary algorithm NSGA 2
	typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
	typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;
#endif



    //Add options
    options::add()("seed", boost::program_options::value<int>(), "program seed");
    options::add()("load_and_run", boost::program_options::value<std::string>(), "file to load and run");
    options::add()("plot-best", boost::program_options::value<bool>()->default_value(true)->implicit_value(true), "Plot the best individual when loading");
    options::add()("plot-all", boost::program_options::value<bool>()->default_value(false)->implicit_value(true), "Plot all individuals when loading");
    options::add()("print-neuron-groups", boost::program_options::value<bool>()->default_value(false)->implicit_value(true), "Print neuron groups.");
    options::positionalAdd("seed", 1);

    //Add the option to continue
    cont::init();
    options::parse_and_init(argc, argv, false);
    set_seed();

    //Create the evolutionary algorithm
    continue_ea_t ea;

    //Create the continuator
    cont::Continuator<continue_ea_t, Params> continuator;

    //Start the run
#ifdef UNTILPERFECT
    //Keep restarting until fitness is perfect
    stop_t stop_condition;
    bool not_perfect = true;
    do{
#endif
        if(options::map.count("load_and_run")){
            std::string file_to_load = options::map["load_and_run"].as<std::string>();
            ea.load(file_to_load);
            std::vector<boost::shared_ptr<phen_t> > vec;
            vec.push_back(boost::fusion::at_c<0>(ea.stat()).getBest());
            ea.set_pop(vec);
            continuator.startRun(ea);
        }
        continuator.run(ea);
#ifdef UNTILPERFECT
        not_perfect = !stop_condition.stop_criterion(ea);
        if(not_perfect){
            ea.reset_stat();
        }
    }while(not_perfect);
#endif


    //Do some cleanup after the run if necessary
    if(continuator.performCleanup()){
        stat::mergeOneLineGenFiles(".", ea.res_dir(), "modularity.dat");
        continuator.deleteCheckpoints();
        continuator.moveGenFiles(ea.res_dir());
        io::deleteEmptyDirectories();
    }

//	if(!option_present(argc, argv)){
//		if (argc == 2) { // if a number is provided on the command line
//			int randomSeed = atoi(argv[1]);
//			printf("randomSeed:%i\n", randomSeed);
//			srand(randomSeed);  //set it as the random seed
//			Params::seed = randomSeed;
//
//#ifdef UNTILPERFECT
//			//Keep restarting until fitness is perfect
//			stop_t stop_condition;
//			bool not_perfect = true;
//			do{
//#endif
//				run_ea(argc, argv, ea, false);
//#ifdef UNTILPERFECT
//				not_perfect = !stop_condition.stop_criterion(ea);
//				if(not_perfect){
//					ea.reset_stat();
//				}
//			}while(not_perfect);
//#endif
//		} else if (argc == 3) { // number + file
//			std::cout << "Performing load and run" << std::endl;
//			int randomSeed = atoi(argv[1]);
//			printf("randomSeed:%i\n", randomSeed);
//			srand(randomSeed);  //set it as the random seed
//			Params::seed = randomSeed;
//
//			ea.load(std::string(argv[2]));
//			ea.loadBest();
//			cont::Continuator<ea_t, Params> continuator;
//			continuator.startRun(ea);
//
////			ea.run_with_current_population();
////#ifdef PNSGA
////			ea.load_and_run(std::string(argv[2]));
////#else
////			run_ea(argc, argv, ea, false);
////#endif
//		} else {
//#ifdef TESTSUB
//			srand(atoi(argv[1]));  //set it as the random seed
//			run_ea(argc, argv, ea, false);
//#else
//			//Run the algorithm with a random seed if no arguments were provided
//			run_ea(argc, argv, ea, true);
//#endif
//		}
//	} else {
//		run_ea(argc, argv, ea, true, &convert);
//	}

#endif
	return 0;
}
