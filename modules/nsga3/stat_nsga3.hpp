/*
 * stat_nsga3.hpp
 *
 *  Created on: Mar 7, 2019
 *      Author: Joost Huizinga
 */

#ifndef MODULES_NSGA3_STAT_NSGA3_HPP_
#define MODULES_NSGA3_STAT_NSGA3_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

#include <boost/graph/iteration_macros.hpp>

#include <Eigen/Core>

#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>

#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/clock.hpp>
#include <modules/datatools/stat_printer.hpp>
#include <modules/continue/global_options.hpp>

#define DBO dbg::out(dbg::info, "stat")

namespace sferes
{
namespace stat
{

/**
 * Convenience function for formatting Eigen vectors horizontally.
 */
std::string format_vec(Eigen::VectorXd vec){
    std::ostringstream ss;
    ss << std::setprecision(5);

	std::string result;
	for(size_t i=0; i<vec.size(); ++i){
		ss << vec[i] << " ";
//		result += boost::lexical_cast<std::string>(vec[i]) + " ";
	}
	return ss.str();
}

SFERES_STAT(StatNsga3, Stat)
{

public:
    static const size_t period = Params::stats::period;

    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);
        typedef typename E::indiv_t indiv_t;

        DBO << "Gen: " << ea.gen() << " period: " << period << std::endl;
        if(ea.gen() % period != 0) return;
        if(ea.pop().size()==0) return;

        //Write header
        DBO << "Creating log file" << std::endl;
        this->_create_log_file(ea, "nsga3.dat");
        DBO << "Log file: " << this->_log_file << std::endl;
        if(!this->_log_file) return;


        size_t associated = 0;
        for(size_t i=0; i<ea.associated_count().size(); ++i){
        	associated += ea.associated_count()[i];
        }

        size_t not_associated = 0;
        for(size_t i=0; i<ea.associated_count().size(); ++i){
        	if(ea.associated_count()[i] == 0){
        		not_associated += 1;
        	}
        }

        size_t candidates = 0;
        for(size_t i=0; i<ea.associated_candidates().size(); ++i){
        	candidates += ea.associated_candidates()[i].size();
        }

        size_t no_candidates = 0;
        for(size_t i=0; i<ea.associated_candidates().size(); ++i){
        	if(ea.associated_candidates()[i].size() == 0){
        		no_candidates += 1;
        	}
        }

//        std::cout << "Population vectors:" << std::endl;
//        for(size_t i=0; i<ea.mixed_pop().size(); ++i){
//
//        	for(size_t obj=0; obj<ea.mixed_pop()[i]->fit().objs().size(); ++obj){
//        		std::cout << ea.mixed_pop()[i]->fit().objs()[obj] << " ";
//        	}
//        	std::cout << std::endl;;
//        }


        dbg::out(dbg::info, "stat") << "Writing line" << std::endl;

        printer.dataStart();
        //Gen
        printer.add(true, "#gen", ea.gen());
        printer.add(true, "w_vec", ea.weight_vectors().size());
        printer.add(true, "s_ref", ea.structured_reference_points().size());
        printer.add(true, "p_ref", ea.preferred_reference_points().size());
        printer.add(true, "pop_s", ea.parent_pop().size());
        printer.add(true, "par_s", ea.pareto_front().size());
        printer.add(true, "mix_s", ea.mixed_pop().size());
        printer.add(true, "asso", associated);
        printer.add(true, "zero_asso", not_associated);
        printer.add(true, "cand", candidates);
        printer.add(true, "zero_cand", no_candidates);
        printer.add(true, "inter", format_vec(ea.intercepts()));


        //Print
        printer.dataPrint(this->_log_file);

    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::trace trace("stat", DBG_HERE);
    }

    std::string name(){
        return "StatNsga3";
    }

protected:
    StatPrinter printer;
};
}
}

#undef DBO

#endif /* MODULES_NSGA3_STAT_NSGA3_HPP_ */
