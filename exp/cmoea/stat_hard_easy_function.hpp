/*
 * stat_cmoea.hpp
 *
 *  Created on: Feb 26, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_CMOEA_STAT_HARD_EASY_FUNCTION_HPP_
#define EXP_CMOEA_STAT_HARD_EASY_FUNCTION_HPP_


#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>

#include <boost/graph/iteration_macros.hpp>
#include <boost/spirit/include/karma_real.hpp>

#include <sferes/dbg/dbg.hpp>

#include <modules/continue/global_options.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/cmoea/cmoea_util.hpp>
#include <modules/datatools/stat_printer.hpp>

namespace sferes
{
namespace stat
{

SFERES_STAT(StatHardEasyFunction, Stat)
{

public:
	StatHardEasyFunction(){}
    
    static const size_t period = Params::stats::period;

    typedef boost::shared_ptr<Phen> indiv_t;
    typedef std::vector<indiv_t> pop_t;

    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);

        dbg::out(dbg::info, "stat") << "Gen: " << ea.gen() << " period: " << period << std::endl;
        if (ea.gen() % period != 0) return;

        //Create log file
        dbg::out(dbg::info, "stat") << "Creating log file..." << std::endl;
        this->_create_log_file(ea, "result.dat");

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


        // Find the highest performance on each objective
        float max_easy = 0;
        float max_hard = 0;
        std::cout << "Printing population" << std::endl;
        for (size_t i = 0; i < pop.size(); ++i){
//        	pop[i]->fit().printBits();
            if(pop[i]->fit().objs()[0] > max_easy){
            	max_easy = pop[i]->fit().objs()[0];
            }
            if(pop[i]->fit().objs()[1] > max_hard){
            	max_hard = pop[i]->fit().objs()[1];
            }
        }
        std::cout << "Population printed" << std::endl;

        // The best individuals (all individuals that have the maximum fitness)
        dbg::out(dbg::info, "stat") << "Gathering best individuals..." << std::endl;
        compare::gatherBest(pop, best);
        best_individual = best[0];

        // Write performance on the test set to our log file
        dbg::out(dbg::info, "stat") << "Writing..." << std::endl;
        printer.dataStart();
        printer.add(true, "#gen", ea.gen());
        printer.add(true, "b_fit", best_individual->fit().value());
        printer.add(true, "b_easy", best_individual->fit().getCmoeaObj(0));
        printer.add(true, "b_hard", best_individual->fit().getCmoeaObj(1));
        printer.add(true, "max_easy", max_easy);
        printer.add(true, "max_hard", max_hard);
        for (size_t i = 0; i < pop.size(); ++i){
        	printer.add(true, "i" + std::to_string(i) + "_easy", pop[i]->fit().getCmoeaObj(0));
        	printer.add(true, "i" + std::to_string(i) + "_hard", pop[i]->fit().getCmoeaObj(1));
        	printer.add(true, "i" + std::to_string(i) + "_fit", pop[i]->fit().value());
        }
        printer.dataPrint(this->_log_file);
        dbg::out(dbg::info, "stat") << "Refresh finished" << std::endl;
        
        
        std::cout << "Generation: " << ea.gen()
        		<< " combined performance: " << best_individual->fit().value()
				<< " easy: " << best_individual->fit().objs()[0]
				<< " hard: " << best_individual->fit().objs()[1]
				<< " best easy " << max_easy
				<< " best hard " << max_hard << std::endl;
    }

    std::string name(){
        return "StatHardEasyFunction";
    }

protected:
    StatPrinter printer;
};
}
}



#endif /* EXP_CMOEA_STAT_HARD_EASY_FUNCTION_HPP_ */
