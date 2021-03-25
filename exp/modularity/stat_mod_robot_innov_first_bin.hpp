/*
 * stat_mod_robot_innov_first_bin.hpp
 *
 *  Created on: May 3, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_STAT_MOD_ROBOT_INNOV_FIRST_BIN_HPP_
#define EXP_MODULARITY_STAT_MOD_ROBOT_INNOV_FIRST_BIN_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/clock.hpp>
#include <modules/datatools/stat_printer.hpp>
#include <modules/continue/global_options.hpp>
#include <sferes/dbg/dbg.hpp>

#define DBO dbg::out(dbg::info, "stat")

namespace sferes
{
namespace stat
{

SFERES_STAT(ModRobotInnovFirstBin, Stat)
{

public:
    static const size_t period = Params::stats::period;

    static const size_t no_objs = 63;
    static const size_t obj_forward = 62;
    static const size_t obj_backward = 61;
    static const size_t obj_turnleft = 59;
    static const size_t obj_turnright = 55;
    static const size_t obj_jump = 47;
    static const size_t obj_down = 31;

    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);
        typedef typename E::indiv_t indiv_t;

        DBO << "Gen: " << ea.gen() <<
        		" period: " << period << std::endl;
        if(ea.gen() % period != 0) return;
        if(ea.pop().size()==0) return;

        //Write header
        DBO << "Creating log file" << std::endl;
        this->_create_log_file(ea, "first_bin.dat");
        DBO << "Log file: " << this->_log_file << std::endl;
        if(!this->_log_file) return;


        const std::vector<boost::shared_ptr<Phen> >& pop = ea.pop();

        //Define temporary vectors and values
        std::vector<float> pop_mod, pop_length, pop_fit;

        const boost::shared_ptr<Phen> best_individual = compare::max(pop, compare::performance_mod_cppnmod());

        //The current population (after selection)
        for (size_t i = 0; i < pop.size(); ++i)
        {
            pop_mod.push_back(pop[i]->fit().mod());
            pop_length.push_back(pop[i]->fit().length());
            pop_fit.push_back(pop[i]->fit().value());
#ifdef DBG_ENABLED
            for(size_t j=0; j<pop[i]->fit().objs().size(); ++j){
                dbg::out(dbg::info, "fitness") << "Obj: " << j << " value: " << pop[i]->fit().objs()[j] <<std::endl;
            }
#endif
        }


        //Calculate averages
        float pop_avg_mod = compare::average(pop_mod);
        float pop_avg_length = compare::average(pop_length);
        float pop_avg_fit = compare::average(pop_fit);

        dbg::out(dbg::info, "stat") << "Writing line" << std::endl;

        printer.dataStart();
        //Gen
        printer.add(true, "#gen", ea.gen());
        printer.add(true, "fit", best_individual->fit().value());
#if defined(SIXTASKS)
        printer.add(true, "forwd", best_individual->fit().cat(obj_forward));
        printer.add(true, "backwd", best_individual->fit().cat(obj_backward));
        printer.add(true, "left", best_individual->fit().cat(obj_turnleft));
        printer.add(true, "right", best_individual->fit().cat(obj_turnright));
        printer.add(true, "jump", best_individual->fit().cat(obj_jump));
        printer.add(true, "down", best_individual->fit().cat(obj_down));
#endif
        printer.add(true, "p_mod", best_individual->fit().mod());
        printer.add(true, "p_len", best_individual->fit().length());
        printer.add(true, "div", best_individual->fit().getDiv());
        printer.add(true, "h_mod", best_individual->fit().hidden_mod());
        printer.add(true, "g_mod", best_individual->fit().cppn_mod());
        printer.add(true, "g_len", best_individual->cppn().get_nb_connections());
        printer.add(true, "l_hie", best_individual->fit().get_lrc_hier());
        printer.add(true, "h_hie", best_individual->fit().get_hour_hier());
        printer.add(true, "gl_hie", best_individual->fit().get_cppn_lrc_hier());
        printer.add(true, "gh_hie", best_individual->fit().get_cppn_hour_hier());

        //Pop stats
        printer.add(true, "av_s", pop_mod.size());
        printer.add(true, "av_fit", pop_avg_fit);
        printer.add(true, "av_mod", pop_avg_mod);
        printer.add(true, "av_len", pop_avg_length);

        //Print
        printer.dataPrint(this->_log_file);

    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::trace trace("stat", DBG_HERE);
//        std::cout << "stat_mod_robot_innov_first_bin.hpp: Nothing to be serialized" << std::endl;
    }

    std::string name(){
        return "ModRobotInnovFirstBin";
    }

protected:
    StatPrinter printer;
};
}
}

#undef DBO

#endif /* EXP_MODULARITY_STAT_MOD_ROBOT_INNOV_FIRST_BIN_HPP_ */
