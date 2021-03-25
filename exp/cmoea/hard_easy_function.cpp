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

// Definitions for guided NSGA-II
#if defined(GNSGA)
#define NSGA
#define PUSH_COMBINED_OBJECTIVE
#endif

// Definitions for guided Lexicase
#if defined(GLEXICASE)
#define LEXICASE
#define PUSH_COMBINED_OBJECTIVE
#endif

#if defined(NPCO)
#define PUSH_COMBINED_OBJECTIVE
#endif

#if defined(COO)
#define COMBINED_OBJECTIVE_ONLY
#endif

#if not defined(COMBINED_OBJECTIVE_ONLY)
#define PUSH_ALL_TASKS
#else
#define PUSH_COMBINED_OBJECTIVE
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

// Include standard
#include <iostream>
#include <fstream>
#include <cmath>

// Include sferes
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/best_fit.hpp>
#include <sferes/eval/parallel.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/modif/diversity.hpp>
#include <sferes/run.hpp>
#include <sferes/ea/dom_sort_basic.hpp>
#include <sferes/misc/rand.hpp>

// Include modules
#include <modules/cmoea/cmoea_util.hpp>
#include <modules/cmoea/cmoea_nsga2.hpp>
#include <modules/nsgaext/dom_sort_no_duplicates.hpp>
#include <modules/nsgaext/nsga2_custom.hpp>
#include <modules/nsga3/nsga3.hpp>
#include <modules/lexicase/lexicase.hpp>
#include <modules/lexicase/e_lexicase.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/params.hpp>
#include <modules/continue/rand.hpp>

// Include local
#include <stat_hard_easy_function.hpp>

/* NAMESPACES */
using namespace sferes;
using namespace sferes::gen::evo_float;


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
///////////* Parameters for the experiment *////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/* Parameters for Sferes */
struct Params {
    
    // Parameters for the vector we are evolving
    struct evo_float {
        SFERES_CONST float cross_rate = 0.1f;
        SFERES_CONST float mutation_rate = 0.1f;
        SFERES_CONST float eta_m = 15.0f;
        SFERES_CONST float eta_c = 10.0f;
        SFERES_CONST mutation_t mutation_type = polynomial;
        SFERES_CONST cross_over_t cross_over_type = sbx;
    };
    
    struct parameters {
        SFERES_CONST float min = 0.0f;
        SFERES_CONST float max = 1.0f;
    };
    
    struct cmoea_nsga {
        // Type of non-dominated comparison that will be used by CMOEA NSGA
        typedef sferes::ea::_dom_sort_basic::non_dominated_f non_dom_f;
    };

    struct cmoea {
        // Size of every CMOEA bin
        static const unsigned bin_size = 40;
        
        // Number of CMOEA bins
        static const unsigned nb_of_bins = 3; // = (2 ^ numberOfTasks - 1)
        
        // Where fitness objective will be stored before non-dominated sorting
        static const unsigned obj_index = 0;
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
    	SFERES_CONST size_t nb_of_objs = 2;
    	static float ref_points_delta;
    };

    /* Parameters for the population */
    struct pop {
        // Required by sferes, but ignored by CMOEA
        SFERES_CONST unsigned size = 120;
        
        // The number of individuals create to initialize the population
        SFERES_CONST unsigned init_size = 120;
        
        // The maximum number of individuals to try an create at the same time.
        SFERES_CONST unsigned max_batch_size = 1024;

        // The number of individuals created every generation
        SFERES_CONST unsigned select_size = 120;
        
        // The number of generations for which to run the algorithm
        SFERES_CONST unsigned nb_gen = 500;
        
        // A multiplier on the number of individuals to create a generation 1.
        SFERES_CONST int initial_aleat = 1;

        // Frequency at which to dump the archive
        static const int dump_period = 200;
        
        // Fequency at which to write a checkpoint
        static const int checkpoint_period = 200;
    };

    struct ea {
    	typedef sferes::ea::dom_sort_basic_f dom_sort_f;
    };
    
    struct stats {
        static const size_t period = 1;
    };
};


// Choose a number of reference points closest to the population size
// Probably not the most efficient way to calculate the reference point delta,
// but given that it only needs to be calculated once, it doesn't seem necessary to
// optimize at this point
void joint_init(){
    dbg::trace trace("init", DBG_HERE);

    size_t partitions = 1;
	size_t nb_of_ref_points = compare::nCr(Params::nsga3::nb_of_objs + partitions - 1, partitions);
	size_t prev_nb_of_ref_points = 0;
	while(nb_of_ref_points < Params::pop::size){
		prev_nb_of_ref_points = nb_of_ref_points;
		partitions += 1;
		nb_of_ref_points = compare::nCr(Params::nsga3::nb_of_objs + partitions - 1, partitions);
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
    SFERES_INIT_NON_CONST_ARRAY(Params::e_lexicase, offset, Params::nsga3::nb_of_objs + 1);
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
    for(size_t i=0; i<Params::nsga3::nb_of_objs; ++i){
    	Params::e_lexicase::offset_set(i, epsilon);
    }
    Params::e_lexicase::offset_set(Params::nsga3::nb_of_objs, pow(epsilon, Params::nsga3::nb_of_objs));
#endif
}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////* Fitness Function *////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

template<typename Indiv>
float rastrigin(Indiv& x, size_t start, size_t end){
	size_t n = end  - start;
	float fn = (float) n;
	float result = 10 * fn;
	for(size_t i=start; i<end; ++i){
		result += pow(x.data(i), 2.0f) - 10 * cos(2 * M_PI * x.data(i));
	}
	return result;
}

template<typename Indiv>
float mod_rastrigin(Indiv& x, size_t start, size_t end){
	float n = (float) (end  - start);
	float result = 0;
	for(size_t i=start; i<end; ++i){
		result += 0.25 + 0.5 * x.data(i) - 0.25 * cos(3 * M_PI * x.data(i));
	}
	return result / n;
}

template<typename Indiv>
float mean(Indiv& x, size_t start, size_t end){
	float n = (float) (end  - start);
	float result = 0;
	for(size_t i=start; i<end; ++i){
		result += x.data(i);
	}
	return result / n;
}

/**
 * Object to calculate the fitness of an individual over a set of randomly
 * generated mazes.
 */
SFERES_FITNESS(ExampleCmoeaFit, sferes::fit::Fitness) {
public:
    ExampleCmoeaFit(){
        
    }

    // Calculates behavioral distance between individuals
//    template<typename Indiv>
//    float dist(Indiv& ind) const {
//    	return 0.0;
////        return compare::l1diff(_behavior, ind.fit().getBehavior());
//    }


    // Evaluates the performance of an individual on all tasks
    template<typename Indiv>
    void eval(Indiv& ind) {
    	dbg::trace trace("fit", DBG_HERE);
    	size_t nb_hard = 20;
    	size_t nb_shared = 10;
    	size_t nb_easy = 10;
    	size_t shared_start = nb_hard+nb_easy;
    	float hard_task_thresh = 0.9;

#if defined(MEAN)
    	float hard_bits = mean(ind, 0, nb_hard);
    	float easy_bits = mean(ind, nb_hard, nb_hard+nb_easy);
    	float shared_bits = mean(ind, shared_start, shared_start + nb_shared);
#else
    	float hard_bits = mod_rastrigin(ind, 0, nb_hard);
    	float easy_bits = mod_rastrigin(ind, nb_hard, nb_hard+nb_easy);
    	float shared_bits = mod_rastrigin(ind, shared_start, shared_start + nb_shared);
#endif

    	_easy_bits = easy_bits;
    	_hard_bits = hard_bits;
    	_shared_bits = shared_bits;

    	float hard_task = (hard_bits + shared_bits) / 2;
    	float easy_task = 1 - (easy_bits + shared_bits) / 2;
        if(hard_task > hard_task_thresh){
        	easy_task += easy_bits;
        }
//
        _cmoea_task_performance.push_back(easy_task);
        _cmoea_task_performance.push_back(hard_task);
        
        cmoea::calculate_bin_fitness_mult(_cmoea_task_performance, _cmoea_bin_fitness);

#if defined(PUSH_ALL_TASKS)
        this->_objs.push_back(easy_task);
        this->_objs.push_back(hard_task);
#endif

        this->_value = easy_task * hard_task;
#if defined(PUSH_COMBINED_OBJECTIVE)
        this->_objs.push_back(this->_value);
#endif
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
			typename stc::FindExact<ExampleCmoeaFit<Params, Exact>,
			Exact>::ret>::serialize(ar, version);
        ar & BOOST_SERIALIZATION_NVP(_behavior);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_task_performance);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_bin_fitness);
        ar & BOOST_SERIALIZATION_NVP(_cmoea_bin_diversity);
        ar & BOOST_SERIALIZATION_NVP(_divIndex);
    }

    // Getters and setters for CMOEA
    std::vector<float> getBehavior() const { return _behavior;}
    std::vector<float> &getBinFitnessVector(){return _cmoea_bin_fitness;}
    float getBinFitness(size_t index){ return _cmoea_bin_fitness[index];}
    std::vector<float> &getBinDiversityVector(){return _cmoea_bin_diversity;}
    float getBinDiversity(size_t index){ return _cmoea_bin_diversity[index];}
    float getCmoeaObj(size_t index){ return _cmoea_task_performance[index];}
    
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

    void printBits(){
    	std::cout << "h:" << _hard_bits << " e: " << _easy_bits << " s: " << _shared_bits << std::endl;
    }

protected:
    // Vector of robot (end) points over all mazes
    std::vector<float> _behavior;
    // Performance on each task, as required for cmoea
    std::vector<float> _cmoea_task_performance;
    // Performance on each combination of tasks, as required for cmoea
    std::vector<float> _cmoea_bin_fitness;
    // The diversity score of an individual, as calculated by cmoea
    std::vector<float> _cmoea_bin_diversity;
    // The index of the diversity objective in the objective array
    int _divIndex;

    float _easy_bits;
    float _hard_bits;
    float _shared_bits;
};

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////* Declare static globals *//////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

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
    
    time_t t = time(0) + ::getpid();
    std::cout<<"seed: " << t << std::endl;
    srand(t);
    sferes::misc::seed(t);

    /* Fitness function to use (a class, defined above), which makes use of the
     * Params struct. */
    typedef ExampleCmoeaFit<Params> fit_t;

    // The genotype.
    typedef gen::EvoFloat<40, Params> gen_t;
    
    // The phenotype
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    //Behavioral distance based only on current population.
    typedef modif::Dummy<> mod_t;
//    typedef modif::Diversity<> mod_t;

    // What statistics should be gathered
    typedef boost::fusion::vector<stat::StatHardEasyFunction<phen_t, Params> > stat_t;

    // The evaluator for the network
    typedef eval::Eval<Params> eval_t;
#if defined(NSGA)
    std::cout << "Running with NSGA-II" << std::endl;
    typedef ea::Nsga2Custom<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#elif defined(CMOEA)
    std::cout << "Running with CMOEA" << std::endl;
    typedef ea::CmoeaNsga2<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#elif defined(LEXICASE)
    std::cout << "Running with Lexicase-selection" << std::endl;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#elif defined(ELEXICASE)
    std::cout << "Running with e-Lexicase-selection" << std::endl;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#elif defined(NSGA3)
    std::cout << "Running with NSGA-III" << std::endl;
    typedef ea::nsga3::Nsga3<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#else
    // Intentionally does not compile
#endif

    modules::rand::init();
    options::parse_and_init(argc, argv, false);
    modules::rand::set_seed();

    std::cout << "Running init" << std::endl;
    joint_init();

    ea_t ea;
    
    std::cout << "Running ea" << std::endl;
    options::run_ea(ea);

    /* Record completion (makes it easy to check if the job was preempted). */
    std::cout << "\n==================================" << \
            "\n====Evolutionary Run Complete!====" << \
            "\n==================================\n";
    return 0;
}
