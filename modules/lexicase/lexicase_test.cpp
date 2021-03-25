// This example relies on the following other modules:
// - datatools

#define DBG_ENABLED
#undef NDEBUG
#define NO_PARALLEL

// Include standard
#include <iostream>
#include <fstream>

// Include sferes
#include <sferes/phen/parameters.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/stat/stat.hpp>
#include <sferes/eval/parallel.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/modif/diversity.hpp>
#include <sferes/run.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/ea/dom_sort_basic.hpp>

// Include modules
#include <modules/cmoea/cmoea_util.hpp>
#include <modules/cmoea/cmoea_nsga2.hpp>
#include <modules/nsgaext/dom_sort_no_duplicates.hpp>
#include <modules/datatools/common_compare.hpp>

// Include local
#include "e_lexicase.hpp"
#include "lexicase.hpp"

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
    
    /* Parameters for the population */
    struct pop {
        // Population size
        SFERES_CONST unsigned size = 100;

        // The number of individuals created every generation
        SFERES_CONST unsigned select_size = 100;
        
        // The number of generations for which to run the algorithm
        SFERES_CONST unsigned nb_gen = 50;

        // Frequency at which to dump the archive
        SFERES_CONST int dump_period = 200;
    };

//    struct ea {
//    	typedef sferes::ea::dom_sort_basic_f dom_sort_f;
//    };

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
    
    struct stats {
        static const size_t period = 1;
    };
};


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////* Fitness Function *////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/**
 * Object to calculate the fitness of an individual over a set of randomly
 * generated mazes.
 */
SFERES_FITNESS(LexicaseTestFit, sferes::fit::Fitness) {
public:
	LexicaseTestFit(){}

    // Evaluates the performance of an individual on all tasks
    template<typename Indiv>
    void eval(Indiv& ind) {
        float hard = ind.data(0) * ind.data(1) * ind.data(2);
        float easy = ind.data(3);
        float task0 = 0;
        float task1 = 0;
        float task2 = 0;
        if(easy > hard){
            task0 = easy;
            task1 = (1-hard)/2.0;
        } else {
            task0 = easy;
            task1 = hard;
        }
         if(hard > 0.9){
             task2 = ind.data(4);
         }
         dbg::out(dbg::info, "fit") << task0 << " " << task1 << " " << task2 << std::endl;

        this->_objs.push_back(task0);
        this->_objs.push_back(task1);
        this->_objs.push_back(task2);
    }
};

SFERES_STAT(LexicaseTestStat, sferes::stat::Stat)
{

public:
	LexicaseTestStat(){
        _first_line = true;
    }

    static const size_t period = Params::stats::period;

    typedef boost::shared_ptr<Phen> indiv_t;
    typedef std::vector<indiv_t> pop_t;

    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);
        typedef typename E::pop_t mixed_pop_t;
        dbg::out(dbg::info, "stat") << "Gen: " << ea.gen() << " period: " << period << std::endl;
        if (ea.gen() % period != 0) return;

        std::vector<float> obj1, obj2, obj3;
        indiv_t temp;
        mixed_pop_t pop = ea.pop();

        dbg::out(dbg::info, "stat") << "Sorting population..." << std::endl;
        std::sort(pop.begin(), pop.end(), compare::pareto_objs().descending());

        for (size_t i = 0; i < pop.size(); ++i){
            temp = pop[i];
            obj1.push_back(temp->fit().objs()[0]);
            obj2.push_back(temp->fit().objs()[1]);
            obj3.push_back(temp->fit().objs()[2]);
        }

        compare::stats stats_obj1 = compare::sortAndCalcStats(obj1);
        compare::stats stats_obj2 = compare::sortAndCalcStats(obj2);
        compare::stats stats_obj3 = compare::sortAndCalcStats(obj3);


        std::cout << "gen: " << ea.gen() << "  ";
        std::cout << "obj1: " << stats_obj1.max << " (" << stats_obj1.avg << ") ";
        std::cout << "obj2: " << stats_obj2.max << " (" << stats_obj2.avg << ") ";
        std::cout << "obj3: " << stats_obj3.max << " (" << stats_obj3.avg << ") ";
        std::cout << std::endl;
    }

    bool firstLine(){
      return _first_line;
    }

    pop_t& getPopulation(){
        dbg::trace trace("stat", DBG_HERE);
        return _all;
    }

    std::string name(){
        return "StatCmoea";
    }

protected:
    indiv_t _best_individual;
    pop_t _all;
    bool _first_line;
};

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//////////////////* Main Program *//////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/* Sets up and runs the experiment. */
int main(int argc, char **argv) {
    std::cout << "Entering main: " << std::endl;
    
    time_t t = time(0) + ::getpid();
//    size_t t = 15;
    std::cout << "seed: " << t << std::endl;
    srand(t);
    sferes::misc::seed(t);

    /* Fitness function to use (a class, defined above), which makes use of the
     * Params struct. */
    typedef LexicaseTestFit<Params> fit_t;

    // The genotype.
    typedef gen::EvoFloat<5, Params> gen_t;
    
    // The phenotype
    typedef phen::Parameters<gen_t, fit_t, Params> phen_t;

    //Behavioral distance based only on current population.
    typedef modif::Dummy<> mod_t;

    // What statistics should be gathered
    typedef boost::fusion::vector<LexicaseTestStat<phen_t, Params> > stat_t;

    // The evaluator for the network
    typedef eval::Eval<Params> eval_t;
    
    // CMOEA with NSGA-II non-dominated selection within each bin
#ifdef ELEXICASE
    std::cout << "Running with E-Lexicase" << std::endl;
    typedef ea::ELexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#else
    std::cout << "Running with Lexicase" << std::endl;
    typedef ea::Lexicase<phen_t, eval_t, stat_t, mod_t, Params> ea_t;
#endif

    ea_t ea;
    
    run_ea(argc, argv, ea, boost::program_options::options_description(), false);

    /* Record completion (makes it easy to check if the job was preempted). */
    std::cout << "\n==================================" << \
            "\n====Evolutionary Run Complete!====" << \
            "\n==================================\n";

    return 0;
}
