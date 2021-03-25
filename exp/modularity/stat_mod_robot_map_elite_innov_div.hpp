/*
 * stat_mod_robot_map_elite_innov_div.hpp
 *
 *  Created on: Feb 26, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_STAT_MOD_ROBOT_MAP_ELITE_INOV_DIV_HPP_
#define EXP_MODULARITY_STAT_MOD_ROBOT_MAP_ELITE_INOV_DIV_HPP_


#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <boost/graph/iteration_macros.hpp>
#include "clock.hpp"
#include <modules/continue/global_options.hpp>
#include <modules/datatools/common_compare.hpp>
#include <sferes/dbg/dbg.hpp>
#include <boost/spirit/include/karma_real.hpp>
#include "stat_mod_robot_shared.hpp"



namespace sferes
{
namespace stat
{

SFERES_STAT(ModRobotMapEliteInovDiv, Stat)
{

public:
  ModRobotMapEliteInovDiv(){
    _first_line = true;
  }

    static const size_t pop_size = Params::pop::size;
    //static const size_t bin_size = Params::cmoea::bin_size;
    static const size_t period = Params::stats::period;

    typedef boost::shared_ptr<Phen> indiv_t;
    typedef std::vector<indiv_t> pop_t;
    typedef typename Phen::gen_t::nn_t nn_t;
    typedef typename Phen::gen_t::graph_t graph_t;
    typedef typename nn_t::vertex_desc_t vertex_desc_t;


    template<typename E>
    void refresh(const E& ea)
    {
        dbg::trace trace("stat", DBG_HERE);
        typedef typename E::pop_t mixed_pop_t;

        size_t categories = Params::cmoea::nb_of_bins;
        size_t bin_size = Params::cmoea::bin_size;

        dbg::out(dbg::info, "stat") << "Gen: " << ea.gen() << " period: " << period << std::endl;
        if (ea.gen() % period != 0) return;

        //Write header
        dbg::out(dbg::info, "stat") << "Creating log file" << std::endl;
        this->_create_log_file(ea, "innov_all_bin.dat");
        dbg::out(dbg::info, "stat") << "Log file: " << this->_log_file << std::endl;
        if(!this->_log_file) return;

        if(this->firstLine()){
            dbg::out(dbg::info, "stat") << "Writing header" << std::endl;
            (*this->_log_file) << "#gen ";
            for(size_t i=0; i<categories; ++i){
                (*this->_log_file) << "obj_" << i << " ";
            }

            (*this->_log_file) << "\n";
	    _first_line = false;
        }

        std::vector<size_t> best_fit_index(categories, 0);
        std::vector<float> best_fit(categories, 0);
        dbg::assertion(DBG_ASSERTION(categories == ea.archive().size()));

        //Add them in this exact order so it is easy to put them back into the archive
        _all.clear();
        dbg::out(dbg::info, "stat") << "Archive size: " << ea.archive().size() << std::endl;
        for(size_t i=0; i<ea.archive().size(); ++i){
            dbg::check_bounds(dbg::error, 0, i, ea.archive().size(), DBG_HERE);
            dbg::out(dbg::info, "stat") << "Bin: " << i << " size: " << ea.archive()[i].size() << std::endl;

            best_fit[i]=ea.archive()[i][0]->fit().cat(i);
            for(size_t j=0; j<ea.archive()[i].size(); ++j){
                dbg::check_bounds(dbg::error, 0, j, ea.archive()[i].size(), DBG_HERE);
                float fitness = ea.archive()[i][j]->fit().cat(i);
                if(fitness > best_fit[i]){
                    dbg::check_bounds(dbg::error, 0, i, best_fit.size(), DBG_HERE);
                    dbg::check_bounds(dbg::error, 0, i, best_fit_index.size(), DBG_HERE);
                    best_fit[i] = fitness;
                    best_fit_index[i] = j;
                }
                _all.push_back(ea.archive()[i][j]);
            }
        }

        dbg::assertion(DBG_ASSERTION(_all.size() == bin_size*categories));
        dbg::check_bounds(dbg::error, 0, 0, ea.archive().size(), DBG_HERE);
        dbg::check_bounds(dbg::error, 0, 0, best_fit_index.size(), DBG_HERE);
        dbg::check_bounds(dbg::error, 0, best_fit_index[0], ea.archive()[0].size(), DBG_HERE);
        _best_individual = ea.archive()[0][best_fit_index[0]];


        dbg::out(dbg::info, "stat") << "Writing line" << std::endl;
        //Start writing
        (*this->_log_file) << ea.gen() << " ";                // generation

        for(size_t i=0; i<best_fit.size(); ++i){
            (*this->_log_file) << best_fit[i] << " ";
        }
        (*this->_log_file) << std::endl;
    }

    bool firstLine(){
      return _first_line;
    }


    void show(std::ostream& os, size_t k)
    {
        dbg::trace trace("stat", DBG_HERE);
        sharedShow<indiv_t, pop_t, Params>(os, k, _all, _best_individual);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::trace trace("stat", DBG_HERE);
        dbg::out(dbg::info, "serialize") << "Serializing: _best_individual" << std::endl;
        ar& BOOST_SERIALIZATION_NVP(_best_individual);
        dbg::out(dbg::info, "serialize") << "Serializing: _all" << std::endl;
        ar& BOOST_SERIALIZATION_NVP(_all);
    }

    pop_t& getPopulation(){
        dbg::trace trace("stat", DBG_HERE);
        return _all;
    }

    std::string name(){
        return "ModRobotMapEliteInovDiv";
    }

    static void initOptions(){
    	sharedInitOptions();
    }

protected:
    indiv_t _best_individual;
    pop_t _all;
    bool _first_line;
};
}
}



#endif /* EXP_MODULARITY_STAT_MOD_ROBOT_MAP_ELITE_INOV_DIV_HPP_ */
