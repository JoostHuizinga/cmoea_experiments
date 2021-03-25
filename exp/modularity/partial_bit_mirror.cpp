//#define NO_PARALLEL
/*
 * This is the partial bit mirroring problem where the network will be evolved on only part of the possible inputs
 * But at the end we measure performance on all possible inputs
 */

#ifdef JHDEBUG
#define NO_PARALLEL
#endif

#include <iostream>
#include <bitset>

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

#include "behavior_div.hpp"
//#include "behavior_nov.hpp"
#ifdef PNSGA
#include "pnsga.hpp"
#endif

#ifdef NOVMAP
# include "modif_novmap.hpp"
#endif


#include "gen_spatial.hpp"
#include "phen_hnn.hpp"
#include "stat_modularity.hpp"
#include "stat_bit_mirror.hpp"
#include "mvg.hpp"

using namespace sferes;
using namespace nn;
using namespace sferes::gen::evo_float;
using namespace sferes::gen::dnn;

#include <boost/algorithm/string.hpp>

#define SFERES_PARAMS(P)                                              \
		struct Ps__ { Ps__() {                                        \
			static std::string __params = # P;                        \
			boost::replace_all(__params, ";", ";\n");                 \
			boost::replace_all(__params, "{", "{\n");                 \
			boost::replace_all(__params, "static const", "");         \
			std::cout << "Parameters:" << __params << std::endl; } }; \
			P;                                                        \
			static Ps__ ____p;

SFERES_PARAMS(
struct Params
{
	static long int seed;

	struct pop
	{
#ifdef JHDEBUG
		static const unsigned nb_gen = 100;
		static const unsigned size = 100;
		static const int dump_period = 1;
		static const int initial_aleat = 1;
#else //JHDEBUG

		static const unsigned size = 100;
		static const unsigned nb_gen = 250;

		static const int dump_period = nb_gen - 1;
		static const int initial_aleat = 1;
#endif//JHDEBUG
	};

	struct ea
	{
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
	struct visualisation{
#ifdef LENGTH
		SFERES_ARRAY(int, background, 170, 197, 225);
#else
		SFERES_ARRAY(int, background, 130, 250, 120);
#endif
		static const int max_modularity_split = 3;
	};
	struct dnn
	{

		//Parameters for evolving the bias
		struct bias
		{
			struct evo_float
			{
				static const float cross_rate = 0.5f;
				static const float mutation_rate = 1.0 / 20.0f;
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

		//Parameters for evolving the weights
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


		static const size_t nb_outputs = 8;
		static const size_t nb_inputs = nb_outputs;

		static const size_t min_nb_neurons = 10; //useless
		static const size_t max_nb_neurons = 30; //useless

		static const size_t min_nb_conns = 20;
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
			SFERES_ARRAY(int, layers, 8, 8);
			SFERES_ARRAY(float, x,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5,
					-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5
			);
			SFERES_ARRAY(float, y,
					0, 0, 0, 0, 0, 0, 0, 0,
					1, 1, 1, 1, 1, 1, 1, 1,
			);
			SFERES_ARRAY(float, z,
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0,
			);
			static const float min_coord = -6.0f;
			static const float max_coord = 6.0f;
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
		static const float min = -3;
		// minimum value
		static const float max = 3;
	};
	struct stats
	{
		static const size_t period = 1;
	};
	struct fitness
	{
		static const size_t b_size = 256 * Params::dnn::nb_outputs;
	};

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
};
) // end of params


SFERES_FITNESS(FitPartialBitMirror, sferes::fit::Fitness)
{
public:
	FitPartialBitMirror()  {
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
		/******************************
		 *** Added for hnn research ***
		 ******************************/
		ind.nn().init();
		/******************************
		 * End added for hnn research *
		 ******************************/

		std::vector<float> inputs(Params::dnn::nb_inputs);
		int k = (int)pow(2, Params::dnn::nb_inputs);
		float nba = 0, err_even = 0, err_odd = 0, err_all = 0;
		for (size_t s = 0; s < k; ++s)
		{
			// set inputs
			std::bitset<Params::dnn::nb_inputs> in(s);
			for (size_t i = 0; i < in.size(); ++i)
				inputs[i] = in[i] ? 1 : -1;

			/********************************
			 *** Adapted for hnn research ***
			 ********************************/

//			std::cout << "Stepping through depth: " << ind.nn().get_depth() << std::endl;
			// step
			for (size_t i = 0; i < ind.nn().get_depth(); ++i)
				ind.nn().step(inputs);

			/********************************
			 * End adapted for hnn research *
			 ********************************/

			// behaviors
			std::vector<bool> xor_res;
			for (size_t i = 0; i < Params::dnn::nb_inputs; i++){
#ifdef REVERSE
				if((inputs[i] >= 0 && ind.nn().get_outf(Params::dnn::nb_inputs - (i+1)) <= 0) || (inputs[i] <= 0 && ind.nn().get_outf(Params::dnn::nb_inputs - (i+1)) >= 0)){
					if(s%2 == 0) err_even++; // Don't show odd problems
					if(s%2 != 0) err_odd++;  // Don't show even problems
				}
#else //MIRROR
				if((inputs[i] >= 0 && ind.nn().get_outf(i) <= 0) || (inputs[i] <= 0 && ind.nn().get_outf(i) >= 0)){
					if(s%2 == 0) err_even++; // Don't show odd problems
					if(s%2 != 0) err_odd++;  // Don't show even problems
				}
#endif //REVERSE
//				std::cout << "Output: " << ind.nn().get_outf(i) << std::endl;
				_behavior[s * Params::dnn::nb_outputs + i] = ind.nn().get_outf(i) > 0 ? 1 : 0;
			}
		}
		err_all = err_even + err_odd;

		this->_objs.clear();

#ifdef MVG
		this->_objs.push_back(-42); // this will be set later
#endif
		float norm = k * Params::dnn::nb_outputs;
		float fit_even = 1 - err_even / norm;
		float fit_odd = 1 - err_odd / norm;
		float fit_all = 1 - err_all / norm;

		this->_values.resize(2);
#ifdef FGODD
		this->_values[0] = fit_odd;
		this->_objs.push_back(fit_odd);
#elif defined(FGEVEN)
		this->_values[0] = fit_even;
		this->_objs.push_back(fit_even);
#endif
		this->_values[1] = fit_all;
		this->_value = this->_values[0];

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


#if defined(LENGTH) && !defined(ONP)
		this->_objs.push_back(-_length);
#endif

		/********************************
		 *** Adapted for hnn research ***
		 ********************************/
		// optimal length
		_optlength = ind.nn().compute_optimal_length(0, false);
		/********************************
		 * End adapted for hnn research *
		 ********************************/

#if defined(LENGTH) && defined(ONP)
		this->_objs.push_back(-_optlength);
#endif

#ifdef TLENGTH
		this->_objs.push_back(-ind.gen().compute_length(1.5)); //default
#endif


#if defined(DIV) || defined(NOV) || defined(NOVMAP)
		this->_objs.push_back(0);
#endif


#ifdef RANDOBJ
		this->_objs.push_back(rand() / (float)RAND_MAX);
#endif

		if (this->mode() == sferes::fit::mode::view)
		{
			std::cout << "behavior:" << _behavior << std::endl;
			std::cout << "- FIT:" << this->_value << std::endl;
			std::cout << "- MOD:" << _mod << std::endl;
			std::cout << "length:" << _length << std::endl;
			std::cout << "nb neurons:" << ind.nn().get_nb_neurons() << std::endl;
			/******************************
			 *** Added for hnn research ***
			 ******************************/
			std::ofstream ofs2(("nn"+ _postFix +".dot").c_str());
			ind.nn().write_dot(ofs2);
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
	int nb_conns() const { return _nb_conns; }
	int nb_nodes() const { return _nb_nodes; }

	/******************************
	 *** Added for hnn research ***
	 ******************************/
	void setPostFix(std::string postFix){_postFix = postFix;}
	bool get_left_right_mod(){ return left_right_mod;}
	float get_left_right_solve(){ return left_right_solve;}
	float get_dyn_mod_score(){ return dyn_mod_score;}
	float get_regularity_score(){ return regularity_score; }

	/******************************
	 * End added for hnn research *
	 ******************************/
protected:
	/******************************
	 *** Added for hnn research ***
	 ******************************/
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
};

long Params::seed = -1;
int main(int argc, char **argv)
{
	typedef FitPartialBitMirror<Params> fit_t;
	typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::weight>, fit::FitDummy<>, Params> weight_t;
	typedef phen::Parameters<gen::EvoFloat<1, Params::dnn::bias>, fit::FitDummy<>, Params> bias_t;
	typedef PfWSum<weight_t> pf_t;
	typedef AfTanh<bias_t> af_t;
	typedef sferes::gen::DnnSpatial<Neuron<pf_t, af_t>, Connection<weight_t>, Params> substrate_t;


#ifdef HNN
	//Set HyperNEAT
	typedef phen::Parameters<gen::EvoFloat<1, Params::ParamsHnn::cppn>,fit::FitDummy<>,Params::ParamsHnn::cppn> cppn_weight_t;
	typedef gen::Hnn<cppn_weight_t, Params::ParamsHnn, Params::ParamsHnn::cppn> gen_t;
	typedef phen::hnn::Hnn<substrate_t, gen_t, fit_t, Params::ParamsHnn> phen_t;
#else
	typedef phen::Dnn<substrate_t, fit_t, Params> phen_t;
#endif

	typedef eval::Parallel<Params> eval_t;
	typedef boost::fusion::vector
			<
			//  stat::ParetoFront<phen_t, Params>, // comment this line to remove pareto.dat
			stat::BitMirror<phen_t, Params>
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
