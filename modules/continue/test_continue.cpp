/*
 * continue_test.cpp
 *
 *  Created on: May 7, 2015
 *      Author: Joost Huizinga
 *
 * A test file, useful to ascertain that the continue module is working properly.
 * TODO: Turn this into a proper unit test.
 */

//Boost header files

//Fix for boost 1.57.0
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/fusion/container.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/shared_ptr.hpp>

//Sferes header files
#include <sferes/phen/parameters.hpp>
#include <sferes/eval/eval.hpp>
#include <sferes/phen/indiv.hpp>
#include <sferes/gen/evo_float.hpp>
#include <sferes/modif/dummy.hpp>
#include <sferes/ea/nsga2.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/stat/best_fit.hpp>
#include <sferes/dbg/dbg.hpp>

//1. Include continue_run.hpp and continue_ea_nsga.hpp in your cpp file:
#include <modules/continue/continue_run.hpp>
#include <modules/continue/continue_ea_nsga2.hpp>
#include <modules/continue/stat_store_pop.hpp>
#include <modules/continue/stat_merge_one_line_gen_files.hpp>

//Define namespaces
using namespace sferes;
using namespace sferes::gen::evo_float;

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
//  static Ps__ ____p;

#define PRINT_PARAMS Ps__ ____p

SFERES_PARAMS(
struct Params {
    static long seed;

    struct pop{
        SFERES_CONST unsigned size = 12;
        SFERES_CONST unsigned nb_gen = 101;
        SFERES_CONST int dump_period = nb_gen-1;
        SFERES_CONST int initial_aleat = 1;

        //2. Add the checkpoint period to the `struct pop' parameters:
        //   Note: a checkpoint period of 1 is not recommended for most experiments.
        SFERES_CONST int checkpoint_period = 1;
    };

    struct evo_float
    {
        SFERES_CONST float cross_rate = 0.0f;
        SFERES_CONST float mutation_rate = 0.05f;
        SFERES_CONST float eta_m = 10.0f;
        SFERES_CONST float eta_c = 10.0f;
        SFERES_CONST mutation_t mutation_type = polynomial;
        SFERES_CONST cross_over_t cross_over_type = sbx;
    };

    struct parameters
    {
        SFERES_CONST float min = 0;
        SFERES_CONST float max = 1;
    };

    struct stats
    {
        SFERES_CONST unsigned period = 1;
    };
};
)


//Fitness function
SFERES_FITNESS(TestFit, fit::Fitness) {
public:
    TestFit(){}
    ~TestFit(){}

    template<typename Indiv>
    void eval(Indiv& ind) {
        dbg::trace trace("fit", DBG_HERE);
        this->_value = ind.data(0);
        this->_objs.push_back(this->_value);
    }
};


int main(int argc, char **argv) {
    //3. Initialize the continue module at the start of your main function:
    cont::init();

    typedef TestFit<Params> fit_t;
    typedef phen::Parameters<gen::EvoFloat<1, Params>,fit_t, Params> phen_t;
    typedef eval::Eval<Params> eval_t;
    typedef modif::Dummy<> modifier_t;

    //4. Add stat::StorePop as the first element in the boost fusion vector:
    typedef boost::fusion::vector<
            stat::StorePop<phen_t, Params>,
            stat::BestFit<phen_t, Params>
    >  stat_t;
    typedef ea::Nsga2<phen_t, eval_t, stat_t, modifier_t, Params> ea_t;

    //5. Wrap your evolutionary algorithm into a variant that supports continuation:
    typedef ea::ContinueNsga2<ea_t, Params> continue_ea_t;

    //Add options
    options::add()("seed", boost::program_options::value<int>(), "program seed");
    options::positionalAdd("seed", 1);

    //6. Replace run_ea with:
    options::parse_and_init(argc, argv, false);

    if(options::map.count("seed")){
        int seed = options::map["seed"].as<int>();
        srand(seed);
    } else {
        time_t t = time(0) + ::getpid();
        srand(t);
    }

    typedef ea_t::params_t ea_params_t;
    cont::Continuator<continue_ea_t, ea_params_t> continuator;
    continue_ea_t cont_ea;
    continuator.run(cont_ea);

    //Do some cleanup after the run if necessary
    if(continuator.performCleanup()){
        //Merge the (hypothetical) output file "bestfit.dat".
        //The stat::mergeOneLineGenFiles function assumes that:
        // - There is one line per generation
        // - The first number on each line indicates the number of that generation
        //WARNING: DO NOT USE IT ON FILES FOR WHICH THESE ASSUMPTIONS DO NOT HOLD
        //         THIS FUNCTION WILL DELETE, OVERWRITE, AND MANGLE YOUR DATA!
        stat::mergeOneLineGenFiles(".", cont_ea.res_dir(), "bestfit.dat");

        //Delete all checkpoints
        continuator.deleteCheckpoints();

        //Move all gen files to the most recent directory
        continuator.moveGenFiles(cont_ea.res_dir());

        //Deletes any empty directories
        io::deleteEmptyDirectories();
    }

    return 0;
}
