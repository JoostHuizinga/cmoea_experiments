
//#define NO_PARALLEL
/**
 * This is the n-xor experiment where we test the networks on multiple separate xor problems.
 * This file also contains the sum4 setup where the network has to answer the 4 xor problem in one go by outputting true if all 4 xors are true
 * Important options are:
 * Objectives (pick at least one):
 * FG - add the performance measure to the list of objectives
 * LENGTH - add connection-cost as an objective
 * DIV - add diversity as an objective
 *
 * Performance measures (pick one, always include FG as an objective):
 * NF4 - 4 xors
 * NF5 - 5 xors
 * NF8 - 8xors
 * SUM4 - 4 xors combined in a single answer
 *
 * Pnsga (Pick or don't pick):
 * PNSGA - enable probabilistic nsga2
 * Pnsga otions (pick one if PNSGA is enabled):
 * PR10 - 10 percent pressure on the second objective
 * PR25 - 25 percent pressure on the second objective
 * PR50 - 50 percent pressure on the second objective
 */

#include <iostream>
#include <bitset>
#include <map>

#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/gen/sampled.hpp>

#include <sferes/ea/nsga2.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/pareto_front.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/run.hpp>
#include <modules/nn2/nn.hpp>
#include <modules/nn2/gen_dnn_ff.hpp>
#include <modules/nn2/phen_dnn.hpp>
#include <modules/modularity/modularity.hpp>

#include "pnsga.hpp"
#include "behavior_div.hpp"
//#include "behavior_nov.hpp"
#include "gen_spatial.hpp"
#include "stat_modularity.hpp"
#include "mvg.hpp"
#include "opt_placement.hpp"
#include "pnsga.hpp"

#ifdef NOVMAP
# include "modif_novmap.hpp"
#endif

/******************************
 *** Added for hnn research ***
 ******************************/
#include <boost/assign/list_of.hpp>
#include "phen_hnn.hpp"
#include "modularity.hpp"
#include "regularity.hpp"
/******************************
 * End added for hnn research *
 ******************************/


using namespace sferes;
using namespace nn;
using namespace sferes::gen::evo_float;
using namespace sferes::gen::dnn;

#include <boost/algorithm/string.hpp>

#define SFERES_PARAMS(P)                                                \
		struct Ps__ { Ps__() {                                                    \
			static std::string __params = # P;                                   \
			boost::replace_all(__params, ";", ";\n");                           \
			boost::replace_all(__params, "{", "{\n");                           \
			boost::replace_all(__params, "static const", "");                   \
			std::cout << "Parameters:" << __params << std::endl; } };                 \
			P;                                                                    \
			static Ps__ ____p;

SFERES_PARAMS(
		struct Params
		{
	static long seed;
	struct pop
	{
		/******************************
		 *** Added for hnn research ***
		 ******************************/
#ifdef JHDEBUG
		static const unsigned size = 100;
		static const unsigned nb_gen = 100;
		static const int dump_period = 1;
#else //JHDEBUG
		/******************************
		 * End added for hnn research *
		 ******************************/

		static const unsigned size = 1000;
		//static const unsigned nb_gen = 5001;
#if defined(GEN5000)
		static const unsigned nb_gen = 5001;
		static const int dump_period = 5000;
#elif defined(GEN10000)
		static const unsigned nb_gen = 10001;
		static const int dump_period = 10000;
#elif defined(GEN25000)
		static const unsigned nb_gen = 25001;
		static const int dump_period = 25000;
#elif defined(GEN40000)
		static const unsigned nb_gen = 40001;
		static const int dump_period = 40000;
#elif defined(GEN50000)
		static const unsigned nb_gen = 50001;
		static const int dump_period = 25000;
#else //Default GEN is 40000
		static const unsigned nb_gen = 40001;
		static const int dump_period = nb_gen - 1;
#endif //defined(GEN5000)

#endif //JHDEBUG
		static const int initial_aleat = 1;

	};

	struct ea
	{
		static float pressure;
#ifdef PR10
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.1, 1.0);
#elif defined(PR25)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.25, 1.0);
#elif defined(PR50)
		SFERES_ARRAY(float, obj_pressure, 1.0, 0.5, 1.0);
#else
		SFERES_ARRAY(float, obj_pressure, 1.0, 1.0, 1.0);
#endif
	};
	struct rand_obj
	{
		static const float std_dev = 0.1;
	};
	struct rand_input
	{
		static const int nr_of_input_patterns = 1000;
	};
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
		static const size_t max_modularity_split = 3;
#ifdef NF4
			SFERES_ARRAY(int, layers, 8, 8, 4);
			SFERES_ARRAY(float, x,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3, -1, 1, 3
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0);
#elif defined(NF5)
			SFERES_ARRAY(int, layers, 10, 10, 5);
			SFERES_ARRAY(float, x,
					-4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5,
					-4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5,
					-4, -2, 0, 2, 4
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0);
#elif defined(NF8)
			SFERES_ARRAY(int, layers, 16, 16, 8);
			SFERES_ARRAY(float, x,
					-7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
					-7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
					-7, -5, -3, -1, 1, 3, 5, 7,
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 2, 2, 2, 2);

#elif defined(NF20)
			SFERES_ARRAY(int, layers, 40, 40, 20);
			SFERES_ARRAY(float, x,
					-19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5,
					-19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5,
					  -19,   -17,   -15,   -13,   -11,    -9,    -7,    -5,    -3,    -1,    1,    3,    5,    7,    9,   11,   13,   15,   17,   19);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#elif defined(NF50)
			SFERES_ARRAY(int, layers, 100, 100, 50);
			SFERES_ARRAY(float, x,
					-49.5, -48.5, -47.5, -46.5, -45.5, -44.5, -43.5, -42.5, -41.5, -40.5, -39.5, -38.5, -37.5, -36.5, -35.5, -34.5, -33.5, -32.5, -31.5, -30.5, -29.5, -28.5, -27.5, -26.5, -25.5,
					-24.5, -23.5, -22.5, -21.5, -20.5, -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5,  -8.5,   -7.5,  -6.5,  -5.5,  -4.5,  -3.5,  -2.5,  -1.5,  -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5,  10.5,  11.5,  12.5,  13.5,  14.5, 15.5,  16.5,   17.5,  18.5,  19.5,  20.5,  21.5,  22.5,  23.5,  24.5,
					 25.5, 26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5, 33.5, 34.5, 35.5, 36.5, 37.5, 38.5, 39.5, 40.5, 41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5, 49.5,
					-49.5, -48.5, -47.5, -46.5, -45.5, -44.5, -43.5, -42.5, -41.5, -40.5, -39.5, -38.5, -37.5, -36.5, -35.5, -34.5, -33.5, -32.5, -31.5, -30.5, -29.5, -28.5, -27.5, -26.5, -25.5,
					-24.5, -23.5, -22.5, -21.5, -20.5, -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5, 22.5, 23.5, 24.5,
					25.5, 26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5, 33.5, 34.5, 35.5, 36.5, 37.5, 38.5, 39.5, 40.5, 41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5, 49.5,
					-49, -47, -45, -43, -41, -39, -37, -35, -33, -31, -29, -27, -25, -23, -21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1,
					1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#elif defined(SUM4)
			SFERES_ARRAY(int, layers, 8, 8, 4, 1);
			SFERES_ARRAY(float, x,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3, -1, 1, 3,
					0
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2,
					3);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0,
					0);
#endif

	};

	struct dnn
	{

#ifdef NF4
		static const size_t nb_outputs = 4;
#elif defined(NF5)
		static const size_t nb_outputs = 5;
#elif defined(NF8)
		static const size_t nb_outputs = 8;
#elif defined(NF10)
		static const size_t nb_outputs = 10;
#elif defined(NF20)
		static const size_t nb_outputs = 20;
#elif defined(NF25)
		static const size_t nb_outputs = 25;
#elif defined(NF50)
		static const size_t nb_outputs = 50;
#elif defined(NF75)
		static const size_t nb_outputs = 75;
#elif defined(NF100)
		static const size_t nb_outputs = 100;
#elif defined(SUM4)
		static const size_t nb_outputs = 1;
#endif

		struct bias
		{
			struct evo_float
			{
				static const float cross_rate = 0.5f;
#ifdef NF4
				static const float mutation_rate = 1.0 / 20.0f;
#elif defined (NF5)
				static const float mutation_rate = 1.0 / 30.0f;
#elif defined (NF8)
				static const float mutation_rate = 1.0 / 80.0f;
#endif
				static const float eta_m = 10.0f;
				static const float eta_c = 10.0f;
				static const mutation_t mutation_type = polynomial;
				static const cross_over_t cross_over_type = sbx;
			};
			struct sampled
			{
				SFERES_ARRAY(float, values, -2, -1, 0, 1, 2);
				static const float mutation_rate = 1.0 / 15.0f;
				static const float cross_rate = 0.5f;
				static const bool ordered = true;
			};
		};
		struct weight
		{
			struct sampled
			{
				SFERES_ARRAY(float, values, -2, -1, 1, 2);
				static const float mutation_rate = 1.0f;
				static const float cross_rate = 0.5f;
				static const bool ordered = true;
			};
			struct evo_float
			{
				static const float cross_rate = 0.5f;
				static const float mutation_rate = 1.0f;
				static const float eta_m = 10.0f;
				static const float eta_c = 10.0f;
				static const mutation_t mutation_type = polynomial;
				static const cross_over_t cross_over_type = sbx;
			};
		};



#ifdef SUM4
		static const size_t nb_inputs = 8;
#else
		static const size_t nb_inputs = nb_outputs * 2;
#endif

		static const size_t min_nb_neurons = 10; //useless
		static const size_t max_nb_neurons = 30; //useless

		//      static const size_t min_nb_conns = 20; //default that works well
		static const size_t min_nb_conns = 0;
		static const size_t max_nb_conns = 100; //nb_outputs * 10;

		static const float m_rate_add_conn = 0.2f;
		static const float m_rate_del_conn = 0.2f;

		static const float m_rate_change_conn = 0.0f;
		static const float m_rate_add_neuron = 0.05f;
		static const float m_rate_del_neuron = 0.05f;
		static const float m_avg_weight = 2.0f;

		static const int io_param_evolving = true;
		static const init_t init = ff;
		struct spatial
		{
#ifdef NF4
			SFERES_ARRAY(int, layers, 8, 8, 4);
			SFERES_ARRAY(float, x,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3, -1, 1, 3
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0);
#elif defined(NF5)
			SFERES_ARRAY(int, layers, 10, 10, 5);
			SFERES_ARRAY(float, x,
					-4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5,
					-4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5,
					-4, -2, 0, 2, 4
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0);
#elif defined(NF8)
			SFERES_ARRAY(int, layers, 16, 16, 8);
			SFERES_ARRAY(float, x,
					-7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
					-7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
					-7, -5, -3, -1, 1, 3, 5, 7,
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 2, 2, 2, 2);

#elif defined(NF20)
			SFERES_ARRAY(int, layers, 40, 40, 20);
			SFERES_ARRAY(float, x,
					-19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5,
					-19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5,
					  -19,   -17,   -15,   -13,   -11,    -9,    -7,    -5,    -3,    -1,    1,    3,    5,    7,    9,   11,   13,   15,   17,   19);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

#elif defined(NF50)
			SFERES_ARRAY(int, layers, 100, 100, 50);
			SFERES_ARRAY(float, x,
					-49.5, -48.5, -47.5, -46.5, -45.5, -44.5, -43.5, -42.5, -41.5, -40.5, -39.5, -38.5, -37.5, -36.5, -35.5, -34.5, -33.5, -32.5, -31.5, -30.5, -29.5, -28.5, -27.5, -26.5, -25.5,
					-24.5, -23.5, -22.5, -21.5, -20.5, -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5,  -8.5,   -7.5,  -6.5,  -5.5,  -4.5,  -3.5,  -2.5,  -1.5,  -0.5,
					  0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5,  10.5,  11.5,  12.5,  13.5,  14.5, 15.5,  16.5,   17.5,  18.5,  19.5,  20.5,  21.5,  22.5,  23.5,  24.5,
					 25.5, 26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5, 33.5, 34.5, 35.5, 36.5, 37.5, 38.5, 39.5, 40.5, 41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5, 49.5,
					-49.5, -48.5, -47.5, -46.5, -45.5, -44.5, -43.5, -42.5, -41.5, -40.5, -39.5, -38.5, -37.5, -36.5, -35.5, -34.5, -33.5, -32.5, -31.5, -30.5, -29.5, -28.5, -27.5, -26.5, -25.5,
					-24.5, -23.5, -22.5, -21.5, -20.5, -19.5, -18.5, -17.5, -16.5, -15.5, -14.5, -13.5, -12.5, -11.5, -10.5, -9.5, -8.5, -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
					0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5, 22.5, 23.5, 24.5,
					25.5, 26.5, 27.5, 28.5, 29.5, 30.5, 31.5, 32.5, 33.5, 34.5, 35.5, 36.5, 37.5, 38.5, 39.5, 40.5, 41.5, 42.5, 43.5, 44.5, 45.5, 46.5, 47.5, 48.5, 49.5,
					-49, -47, -45, -43, -41, -39, -37, -35, -33, -31, -29, -27, -25, -23, -21, -19, -17, -15, -13, -11, -9, -7, -5, -3, -1,
					1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
					2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#elif defined(SUM4)
			SFERES_ARRAY(int, layers, 8, 8, 4, 1);
			SFERES_ARRAY(float, x,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3, -1, 1, 3,
					0
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
					2, 2, 2, 2,
					3);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0,
					0);
#endif
			static const float min_coord = -4.0f;
			static const float max_coord = 4.0f;
#ifdef RANDCOORDS
			static const bool rand_coords = true;
#else
			static const bool rand_coords = false;
#endif
		};
	};

	struct parameters
	{
		// maximum value of parameters (weights & bias)
		static const float min = 0;
		// minimum value
		static const float max = 1;
	};
	struct stats
	{
		static const size_t period = 1;
	};
	struct fitness
	{
#if defined(NF4)
		static const size_t b_size = 256 * Params::dnn::nb_outputs;
#elif defined(SUM4)
		static const size_t b_size = 256;
#elif defined(NF5)
		static const size_t b_size = 1024 * Params::dnn::nb_outputs;
#elif defined(NF8)
		static const size_t b_size = 65536 * Params::dnn::nb_outputs;
#else
		static const size_t b_size = Params::dnn::nb_outputs * 4 * Params::dnn::nb_outputs;
#endif
	};

	/******************************
	 *** Added for hnn research ***
	 ******************************/

	/***********************/
	/*** CPPN PARAMETERS ***/
	/***********************/
	struct ParamsHnn {
		struct cppn {
			// params of the CPPN
			// These values define the mutations on the activation function of the CPPN nodes
			// As such, they define which activation function can be chosen and the mutation rate for changing the activation function
			struct sampled {
				SFERES_ARRAY(float, values, nn::cppn::sine, nn::cppn::sigmoid, nn::cppn::gaussian, nn::cppn::linear); //Possible activation functions
				static const float mutation_rate = 0.1f;			//Mutation rate for the cppn nodes
				static const float cross_rate = 0.0f;				//Not actually used, even if non-zero
				static const bool ordered = false;					//Mutation will randomly pick an other activation function from the list
				//The alternative has bias towards adjacent values
			};

			// These value determine the mutation rate for the weights and biases of the cppn
			// Note that, in practice, the bias is determined by the weight of the connection between
			// the bias input node and the target node, so biases are not mutated 'directly'.
			struct evo_float {
				static const float mutation_rate = 0.1f;			//Mutation rate for the cppn nodes
				static const float cross_rate = 0.0f; 				//Not actually used, even if non-zero
				static const mutation_t mutation_type = polynomial;	//Mutation type
				static const cross_over_t cross_over_type = sbx;	//Not used
				static const float eta_m = 10.0f;					//Mutation eta (parameter for polynomial mutation.
				static const float eta_c = 10.0f;					//?
			};

			struct parameters
			{
				// minimum value
				static const float min = -3;
				// maximum value of parameters (weights & bias)
				static const float max = 3;
			};

			//This threshold determines at what value of the Link-Expression Output (LEO) a link will be expressed or not.
			//Before the threshold LEO values are scaled between 0 and 1 and only values equal to or above the threshold are expressed.
			//As a result a value of 0.0 means all links are expressed (effectively disabling the LEO node)
			//while a value greater than 1.0 means no values will ever be expressed (probably a bad idea).
			//A value of 0.5 is default
			static const float leo_threshold = 0.5f;
		};

		//These are the parameters used for setting the CPPN
		//The 7 inputs are the x, y, z coordinates for neuron 1
		//The x, y, z coordinates for neuron 2
		//And a bias.
		//The outputs are the weights and the link expression, and the bias.
		struct dnn
		{
			static const size_t nb_inputs = 7;
			typedef phen::hnn::ThreeDInput<Params::dnn> input_t;

			static const size_t nb_outputs = 3;
			static const init_t init = ff;

			static const float m_rate_add_conn = 0.09f;		//Add connection rate for the cppn
			static const float m_rate_del_conn = 0.08f;		//Delete connection rate for the cppn
			static const float m_rate_add_neuron = 0.05f;		//Add neuron rate for the cppn
			static const float m_rate_del_neuron = 0.04f;		//Delete neuron rate for the cppn
			static const float m_rate_change_conn = 0.0f;		//Change connection rate for the cppn
			//(e.g. tears down one side of a connection, then reconnects this side to an other neuron.
			//Not used for the cppn, even if value is non zero.)
			static const size_t min_nb_neurons = 7;
			static const size_t max_nb_neurons = 20;
			static const size_t min_nb_conns = 0;
			static const size_t max_nb_conns = 150;
		};
	};

	/******************************
	 * End added for hnn research *
	 ******************************/
		};
) // end of params



// -1 to +1 sigmoid
namespace nn
{
	template<typename P>
	struct AfTanhNxor : public Af<P>
	{
		typedef P params_t;
		static const float lambda = 20.0f;
		AfTanhNxor() {
			assert(trait<P>::size(this->_params) == 1);
		}
		float operator() (float p) const
		{
			return tanh(lambda * (p + trait<P>::single_value(this->_params)));
		}
	};
}

SFERES_FITNESS(FitXorXor, sferes::fit::Fitness)
{
public:
	FitXorXor()  {
	}

	template<typename Indiv>
	float dist(const Indiv& o) const
	{
		return (o.fit()._behavior ^ _behavior).count() / (float) _behavior.size();
	}

	// size of the behavior vector
	static const size_t b_size = Params::fitness::b_size;

	typedef std::bitset<b_size> behavior_t;

	template<typename Indiv>
	void eval(Indiv& ind)
	{
//		std::cout << "Evaluating" <<std::endl;
		/******************************
		 *** Added for hnn research ***
		 ******************************/
		ind.nn().init();
		/******************************
		 * End added for hnn research *
		 ******************************/

		std::vector<float> inputs(Params::dnn::nb_inputs);

#ifdef RANDOM
		int nr_of_input_patterns = Params::rand_input::nr_of_input_patterns;
#else
		int nr_of_input_patterns = (int)pow(2, Params::dnn::nb_inputs);
#endif
		float err1 = 0, err2 = 0, nba = 0;
		for (size_t s = 0; s < nr_of_input_patterns; ++s)
		{
#ifdef RANDOM
//			std::bitset<Params::dnn::nb_inputs> in = misc::randBitset<Params::dnn::nb_inputs>();
			for (size_t i = 0; i < inputs.size(); ++i)
				inputs[i] = misc::flip_coin() ? 1 : -1;
#else
			std::bitset<Params::dnn::nb_inputs> in(s);
			for (size_t i = 0; i < inputs.size(); ++i)
				inputs[i] = in[i] ? 1 : -1;
#endif


			/********************************
			 *** Adapted for hnn research ***
			 ********************************/
			// step
			ind.nn().stepAll(inputs);

//			for (size_t i = 0; i < ind.nn().get_depth(); ++i)
//				ind.nn().step(inputs);
			/********************************
			 * End adapted for hnn research *
			 ********************************/

			// errors
#ifdef SUM4
			float sum_wanted = 0, sum_observed = 0;
			for (size_t i = 0; i < Params::dnn::nb_inputs; i += 2)
			{
				bool wanted = ((bool)(inputs[i] > 0)) ^ ((bool)(inputs[i + 1] > 0));
				sum_wanted += wanted;
			}
			bool out = ind.nn().get_outf(0) > 0;
			_behavior[s] = out;
			bool wanted = (sum_wanted >= 2);
			nba += wanted;
			if (wanted != out)
				err1 += 1;
			if (this->mode() == sferes::fit::mode::view)
				std::cout << "wanted: " << wanted << " out:" << out << std::endl;
#else
			for (size_t i = 0; i < Params::dnn::nb_inputs; i += 2)
			{
				bool wanted = ((bool)(inputs[i] > 0)) ^ ((bool)(inputs[i + 1] > 0));
				bool out = ind.nn().get_outf(i / 2) > 0 ? true : false;
				if (wanted != out) err1++;

//				bool wanted2 = false;
//				if (i <= 2)
//					wanted2 = ((bool)(inputs[i] > 0)) ^ ((bool)(inputs[i + 1] > 0));
//				else
//					wanted2 = ((bool)(inputs[i] > 0)) && ((bool)(inputs[i + 1] > 0));
//				bool out2 = ind.nn().get_outf(i / 2) > 0 ? true : false;
//				if (wanted2 != out2)
//					err2++;
#ifndef RANDOM
				_behavior[s * Params::dnn::nb_outputs + i / 2] = out;
#endif
			}
#endif
		}

//		std::cout << "Evaluation done" <<std::endl;

#ifdef RANDOM
		for (size_t output_nr = 0; output_nr < Params::dnn::nb_outputs; output_nr++)
		{
			for (size_t s = 0; s < 4; ++s)
			{
				std::bitset<Params::dnn::nb_inputs> in(s);

				in <<= output_nr*2;

				for (size_t i = 0; i < in.size(); ++i)
					inputs[i] = in[i] ? 1 : -1;

				ind.nn().stepAll(inputs);
//				for (size_t i = 0; i < ind.nn().get_depth(); ++i)
//					ind.nn().step(inputs);

				// errors
				for (size_t i = 0; i < Params::dnn::nb_outputs; i++){
					bool out = ind.nn().get_outf(i) > 0 ? true : false;
					_behavior[output_nr * 4 * Params::dnn::nb_outputs + s * Params::dnn::nb_outputs + i] = out;
				}
			}
		}
#endif

//		std::cout << "Behavior done" <<std::endl;

		this->_objs.clear();

#ifdef MVG
		this->_objs.push_back(-42); // this will be set later
#endif
		float norm = nr_of_input_patterns * Params::dnn::nb_outputs;
		this->_values.resize(2);
		this->_values[0] = 1 - err1 / norm;
		this->_values[1] = 1 - err2 / norm;

#ifdef FG
		this->_objs.push_back(1 - err1 / norm);
#endif
		this->_value = 1 - err1 / norm;

		// compute the modularity
		/********************************
		 *** Adapted for hnn research ***
		 ********************************/
		typename Indiv::nn_t::graph_t gsimp = ind.nn().simplified_nn().get_graph();
		/********************************
		 * End adapted for hnn research *
		 ********************************/


		if (boost::num_edges(gsimp) == 0 || boost::num_vertices(gsimp) == 0)
			_mod = 0;
		else
			_mod = mod::modularity(gsimp);

#ifdef MOD
		this->_objs.push_back(_mod);
#endif
		/********************************
		 *** Adapted for hnn research ***
		 ********************************/
		// length
		_length = ind.nn().compute_length();
		_optlength = ind.nn().compute_optimal_length();
		/********************************
		 * End adapted for hnn research *
		 ********************************/

#ifdef MAX_LENGTH
		this->_objs.push_back(-ind.gen().max_length());
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



#if defined(TLENGTH) && !defined(ONP)
		this->_objs.push_back(-ind.gen().compute_length(1.5)); //default
#endif

#if defined(TLENGTH) && defined(ONP)
		bool write_svg = (this->mode() == fit::mode::view);
		this->_objs.push_back(-ind.gen().compute_optimal_length(1.5, write_svg));
#endif

#if defined(LENGTH) && !defined(ONP)
		this->_objs.push_back(-_length);
#endif

#if defined(LENGTH) && defined(ONP)
		bool write_svg = (this->mode() == fit::mode::view);
		this->_objs.push_back(-ind.gen().compute_optimal_length(0, write_svg));
#endif


#if defined(DIV) || defined(NOV) || defined(NOVMAP)
		this->_objs.push_back(0);
#endif

#ifdef RANDOBJ
		this->_objs.push_back(rand() / (float)RAND_MAX);
#endif

		if (this->mode() == sferes::fit::mode::view)
		{
			/******************************
			 *** Added for hnn research ***
			 ******************************/
			/*** Perform analysis ***/
			//Left-right analysis
			std::set<int> left;
			std::set<int> right;
			for (int i=0; i<Params::dnn::nb_inputs; i+=2){
				left.insert(i);
				right.insert(i+1);
			}

			left_right_mod = (analysis_io_mod(ind, left, right)==Params::dnn::nb_inputs);

			//Regularity analysis
			regularity_score = regularity::analysis_regularity(ind);
			/******************************
			 * End added for hnn research *
			 ******************************/

			std::cout << "behavior:" << _behavior << std::endl;
			std::cout << "- FIT:" << 1 - err1 / norm << std::endl;
			std::cout << "- nb conns:" << ind.nn().get_nb_connections() << std::endl;
			std::cout << "- MOD:" << _mod << std::endl;
			std::cout << "- length:" << _length << std::endl;
			std::cout << "- nb neurons:" << ind.nn().get_nb_neurons() << std::endl;

			/******************************
			 *** Added for hnn research ***
			 ******************************/
//			std::ofstream ofs2(("nn"+ _postFix +".dot").c_str());
//			ind.nn().write_dot(ofs2);

			std::ofstream ofs3(("nn"+ _postFix +".svg").c_str());
			ind.nn().write_svg(ofs3);

			ind.nn().compute_optimal_length(0, true, "onp" + _postFix + ".svg");
#ifdef HNN
			std::ofstream ofs(("cppn" + _postFix + ".dot").c_str());
			ind.write_dot_cppn(ofs);
#endif
			/******************************
			 * End added for hnn research *
			 ******************************/
		}
	}
	void set_value(float v) { this->_value = v; }
	float values(size_t k) const { return _values[k]; }
	size_t nb_values() const { return _values.size(); }
	const behavior_t& behavior() const { return _behavior; }
	float mod() const { return _mod; }
	float length() const { return _length; }
	float optlength() const { return _optlength; }
	int nb_nodes() const { return _nb_nodes; }
	int nb_conns() const { return _nb_conns; }

	/******************************
	 *** Added for hnn research ***
	 ******************************/
	void setPostFix(std::string postFix){_postFix = postFix;}
	bool get_left_right_mod(){ return left_right_mod;}
	float get_left_right_solve(){ return left_right_solve;}
	bool get_one_solved(int i){ return false;}
	bool get_two_solved(int i){ return false;}
	float get_dyn_mod_score(){ return dyn_mod_score;}
	float get_regularity_score(){ return regularity_score; }

	template<typename I>
	float get_original_length(I& indiv){
		std::vector<std::string> weightsVector = get_weights_vector(indiv);
		std::string weightsString = "";
		BOOST_FOREACH(std::string weight, weightsVector){
			weightsString.append(weight);
		}
		return (weightsString.size()+1)*8;
	}

	/******************************
	 * End added for hnn research *
	 ******************************/

	void setId(size_t id){_id = id;}
	size_t getId(){ return _id;}
protected:
	/******************************
	 *** Added for hnn research ***
	 ******************************/
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

	bool left_right_mod;
	float left_right_solve;
	float dyn_mod_score;
	float regularity_score;
	std::string _postFix;
	/******************************
	 * End added for hnn research *
	 ******************************/

	std::vector<float> _values;
	behavior_t _behavior;
	float _mod;
	float _length, _optlength;
	int _nb_nodes, _nb_conns;
	size_t _id;
};

long Params::seed = -1;
float Params::ea::pressure = 0.1;
int main(int argc, char **argv)
{

	typedef FitXorXor<Params> fit_t;
	typedef phen::Parameters<gen::Sampled<1, Params::dnn::weight>, fit::FitDummy<>, Params> weight_t;
	typedef phen::Parameters<gen::Sampled<1, Params::dnn::bias>, fit::FitDummy<>, Params> bias_t;
	typedef PfWSum<weight_t> pf_t;
	typedef AfTanhNxor<bias_t> af_t;

	/********************************
	 *** Adapted for hnn research ***
	 ********************************/

	typedef sferes::gen::DnnSpatial<Neuron<pf_t, af_t>, Connection<weight_t>, Params> substrate_t;

#ifdef HNN
	//Set HyperNEAT
	typedef phen::Parameters<gen::EvoFloat<1, Params::ParamsHnn::cppn>,fit::FitDummy<>,Params::ParamsHnn::cppn> cppn_weight_t;
	typedef gen::Hnn<cppn_weight_t, Params::ParamsHnn, Params::ParamsHnn::cppn> gen_t;
	typedef phen::hnn::Hnn<substrate_t, gen_t, fit_t, Params::ParamsHnn> phen_t;
#else
	typedef phen::Dnn<substrate_t, fit_t, Params> phen_t;
#endif

	/********************************
	 * End adapted for hnn research *
	 ********************************/

	typedef eval::Parallel<Params> eval_t;
	typedef boost::fusion::vector
			<
			//    stat::ParetoFront<phen_t, Params>, // comment this line to remove pareto.dat
			stat::Mod<phen_t, Params>
	>  stat_t;

	// MVG cannot be used with DIV !
#if defined(DIV)and !defined(MVG)
	typedef modif::BehaviorDiv<> modifier_t;
#elif defined(MVG)
	typedef modif::Mvg<> modifier_t;
#elif defined(NOVMAP)
	typedef modif::Novmap<> modifier_t;
#else
	typedef modif::Dummy<> modifier_t;
#endif

#ifdef PNSGA
	typedef ea::Pnsga<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
#else
	typedef ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;
#endif
	ea_t ea;

	if (argc > 1) // if a number is provided on the command line
	{
		int randomSeed = atoi(argv[1]);
		printf("randomSeed:%i\n", randomSeed);
		srand(randomSeed);  //set it as the random seed
		Params::seed = randomSeed;
		run_ea(argc, argv, ea, false);
	}
	else
	{
		run_ea(argc, argv, ea);
	}


	return 0;
}
