// stat_testset.hpp
#ifndef STAT_TESTSET_HPP_
#define STAT_TESTSET_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>


#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>

#include <modules/datatools/stat_printer.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/formatting.hpp>
#include <modules/diversity/behavior_div.hpp>

using namespace modules::misc;

namespace sferes
{
namespace stat
{


SFERES_STAT(StatTestset, Stat)
{
    typedef boost::shared_ptr<Phen> indiv_t;
    typedef std::vector<indiv_t> pop_t;
    typedef typename Phen::gen_t::nn_t nn_t;
    typedef typename Phen::gen_t::graph_t graph_t;
    typedef typename nn_t::vertex_desc_t vertex_desc_t;

public:
    template<typename E>
    void refresh(const E& ea){
        dbg::trace trace("stat", DBG_HERE);
        typedef typename E::pop_t mixed_pop_t;

        //If we are not at the dumping period, don't dump
        if (ea.gen() % Params::stats::testset_period != 0) return;

        //Create log file
        dbg::out(dbg::info, "stat") << "Creating log file..." << std::endl;
        this->_create_log_file(ea, "testset.dat");

        //If there is no log-file, stop processing data after best and all are populated
        if(!this->_log_file){
            dbg::out(dbg::info, "stat") << "Log file is null, aborting..." << std::endl;
            return;
        }

        // Initialize our data structures
        pop_t pop = ea.pop();
        pop_t best;
        indiv_t best_individual;

        // Sort the population, performance first
        dbg::out(dbg::info, "stat") << "Sorting population..." << std::endl;
        std::sort(pop.begin(), pop.end(), compare::performance().descending());

        // The best individuals (all individuals that have the maximum fitness)
        dbg::out(dbg::info, "stat") << "Gathering best individuals..." << std::endl;
        compare::gatherBest(pop, best);
        best_individual = best[0];


        // Evaluate the best individual on the test set
        dbg::out(dbg::info, "stat") << "Evaluating best individual on test set..." << std::endl;
        best_individual->develop();
        best_individual->fit().evalTestset(*best_individual);

        // Write performance on the test set to our log file
        dbg::out(dbg::info, "stat") << "Writing..." << std::endl;
        printer.dataStart();
        printer.add(true, "#gen", ea.gen());
        printer.add(true, "b_fit", best_individual->fit().valueTestset());
        printer.dataPrint(this->_log_file);
        dbg::out(dbg::info, "stat") << "Refresh finished" << std::endl;
    }

    std::string name(){
        return "StatTestset";
    }

protected:
    StatPrinter printer;
};
}
}


#endif //STAT_TESTSET_HPP_
