/*
 * stat_mod_robot_shared.hpp
 *
 *  Created on: Jul 5, 2018
 *      Author: Joost Huizinga
 *
 * Functions and methods shared between the stat file for the mod_robot
 * experiment.
 */

#ifndef EXP_MODULARITY_STAT_MOD_ROBOT_SHARED_HPP_
#define EXP_MODULARITY_STAT_MOD_ROBOT_SHARED_HPP_

#include <sferes/dbg/dbg.hpp>
#include "stat_real6_policy.hpp"

namespace sferes
{
namespace stat
{

template<typename indiv_t, typename pop_t>
indiv_t calculateBest(pop_t& _all){
    size_t best_index = 0;
    float best_fitness = _all[0]->fit().cat(0);

    for(size_t i=0; i<_all.size(); ++i){
        float fitness = _all[i]->fit().cat(0);
        if(fitness > best_fitness){
            best_fitness = fitness;
            best_index = i;
        }
    }

    return _all[best_index];
}

template<typename pop_t>
void setId(pop_t& _all){
    for(size_t i=0; i<_all.size(); ++i){
        _all[i]->fit().setId(i);
    }
}

template<typename indiv_t>
void log_individual(std::ostream& os, indiv_t indiv){
    dbg::trace trace("stat", DBG_HERE);
    namespace sp = boost::spirit::karma;
    std::cout << "Fitness before evaluation: " << indiv->fit().value() << std::endl;
    std::cout << "Individual ID: " << indiv->fit().getId() << std::endl;
    float fitness = indiv->fit().value();
    float modularity = indiv->fit().mod();
    size_t id = indiv->fit().getId();

    sp::real_generator<float, real6_policy<float> > pl;

    indiv->develop();
    indiv->fit().set_mode(fit::mode::view);
    indiv->fit().setCustomText("Fitness: " + boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]],(indiv->fit().value()))));
    indiv->fit().setPostFix("_fit_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]], fitness)) +
            "_mod_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]], modularity)) +
            "_id_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(4, '0')[sp::maxwidth(4)[sp::int_]], id)) +
            "_" + options::map["nn-postfix"].as<std::string>());
    indiv->fit().eval(*indiv);

    os << indiv->fit().getId();
    os << " " << indiv->fit().value();
    os << " " << indiv->fit().length();
    os << " " << indiv->fit().mod();
    os << " " << indiv->fit().get_regularity_score();
    os << " " << indiv->fit().get_original_length(*indiv);
    os << " " << indiv->fit().get_regularity_score()/indiv->fit().get_original_length(*indiv);
    os << " " << 1.0-(indiv->fit().get_regularity_score()/indiv->fit().get_original_length(*indiv));
    os << "\n";
}


template<typename indiv_t, typename pop_t, typename Params>
void sharedShow(std::ostream& os, size_t k, pop_t& _all, indiv_t best_individual)
{
    dbg::trace trace("stat", DBG_HERE);
    std::cout << "Visualizing for k: " << k <<
    		" from: " << _all.size() << std::endl;

    // We are responsible for evaluating networks,
    // so we have to do the work of the slaves
    if(Params::mpi::slaveInit != 0){
        (*Params::mpi::slaveInit)();
    }

    if(options::map["plot-best"].as<bool>()){
        std::cout << "Visualizing best individual." << std::endl;
        setId(_all);
        if(!best_individual || options::map["recalc-best"].as<bool>()){
        	best_individual = calculateBest<indiv_t, pop_t>(_all);
        }

        log_individual(os, best_individual);
    } else if(k < _all.size()){
        std::cout << "Visualizing individual: " << k << std::endl;
        _all[k]->develop();
        _all[k]->show(os);
        _all[k]->fit().set_mode(fit::mode::usr1);
        _all[k]->fit().eval(*_all[k]);
        _all[k]->fit().visualizeNetwork(*_all[k]);
    } else {
        //TODO: Error message
    }
}

template<typename indiv_t, typename pop_t, typename Params>
void sharedShow(std::ostream& os, size_t k, pop_t& _all){
	indiv_t indiv = calculateBest<indiv_t, pop_t>(_all);
	sharedShow<indiv_t, pop_t, Params>(os, k, _all, indiv);
}

 void sharedInitOptions(){
    using namespace options;
    using namespace boost::program_options;
    add()("plot-best",
    		value<bool>()->default_value(true)->implicit_value(true),
    		"Plot the best individual when loading");
    add()("recalc-best",
    		value<bool>()->default_value(false)->implicit_value(true),
    		"Always recalculate the best individual upon loading");
    add()("nn-postfix",
    		boost::program_options::value<std::string>()->default_value(""),
    		"Print the specified postfix after images created.");
}


}
}

#endif /* EXP_MODULARITY_STAT_MOD_ROBOT_SHARED_HPP_ */
