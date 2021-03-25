#ifndef STAT_MOD_ROBOT_FOOD_HPP_
#define STAT_MOD_ROBOT_FOOD_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <modules/misc/common_compare.hpp>
#include "clock.hpp"
#include <modules/continue/global_options.hpp>
#include <sferes/dbg/dbg.hpp>

#define DBO dbg::out(dbg::info, "stat")

namespace sferes
{
namespace stat
{

/**
 * Compare on performance (fit().value()) first, modularity second (fit().mod(),
 * and CPPN modularity third (fit.cppn_mod()). Implements the 'smaller than'
 * (<) operator, which is assumed by std::max_element and std::min_element.
 */
class food_mod_cppnmod: public compare::compare<compare::performance_mod>
{
public:
    template<typename I>
    bool operator() (const boost::shared_ptr<I> i1, const boost::shared_ptr<I> i2) const
    {
        // first, compare the main objective
        if (i1->fit().getFoodFitness() < i2->fit().getFoodFitness())
        	return this->_smaller;
        if (i1->fit().getFoodFitness() > i2->fit().getFoodFitness())
        	return this->_greater;
        if (i1->fit().mod() < i2->fit().mod())
        	return this->_smaller;
        if (i1->fit().mod() > i2->fit().mod())
        	return this->_greater;
        if (i1->fit().cppn_mod() < i2->fit().cppn_mod())
        	return this->_smaller;
        if (i1->fit().cppn_mod() > i2->fit().cppn_mod())
        	return this->_greater;
        return this->_equal;
    }
};

SFERES_STAT(ModRobotFood, Stat)
{

public:
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

		DBO << "Gen: " << ea.gen() <<
				" period: " << Params::stats::period << std::endl;
		if (ea.gen() % Params::stats::period != 0) return;
        if(ea.pop().size()==0) return;

		//Clear all individuals
		_all.clear();

		//Retrieve the pareto front, population and mixed population
		pop_t pop = ea.pop();

		//Define temporary vectors and values
		indiv_t temp_indiv;

		//Best individual
		_best_indiv = compare::max(pop, food_mod_cppnmod());

		//The current population (after selection)
		for (size_t i = 0; i < pop.size(); ++i){
//			temp_indiv = pop[i];
			pop[i]->fit().setId(i);
			_all.push_back(pop[i]);
		}

		//If there is no log-file, stop processing data after best and all are
		//populated
        DBO << "Creating log file" << std::endl;
		this->_create_log_file(ea, "modularity.dat");
		DBO << "Log file: " << this->_log_file << std::endl;
		if(!this->_log_file) return;


        dbg::out(dbg::info, "stat") << "Writing line" << std::endl;

        _printer.dataStart();
        _printer.add(true, "#gen", ea.gen());
        _printer.add(true, "food_fit", _best_indiv->fit().getFoodFitness());
        _printer.add(true, "p_mod", _best_indiv->fit().mod());
        _printer.add(true, "p_len", _best_indiv->fit().length());
        _printer.add(true, "div", _best_indiv->fit().getDiv());
        _printer.add(true, "h_mod", _best_indiv->fit().hidden_mod());
        _printer.add(true, "g_mod", _best_indiv->fit().cppn_mod());
        _printer.add(true, "g_len", _best_indiv->cppn().get_nb_connections());
        _printer.add(true, "l_hie", _best_indiv->fit().get_lrc_hier());
        _printer.add(true, "h_hie", _best_indiv->fit().get_hour_hier());
        _printer.add(true, "gl_hie", _best_indiv->fit().get_cppn_lrc_hier());
        _printer.add(true, "gh_hie", _best_indiv->fit().get_cppn_hour_hier());

        _printer.dataPrint(this->_log_file);

	}


	void log_individual(std::ostream& os, indiv_t indiv){
		dbg::trace trace("stat", DBG_HERE);

		indiv->develop();
		indiv->fit().set_mode(fit::mode::view);
		indiv->fit().eval(*indiv);
		double reg_score = indiv->fit().get_regularity_score();
		double org_length = indiv->fit().get_original_length(*indiv);

		os << indiv->fit().getId();
		os << " " << indiv->fit().value();
		os << " " << indiv->fit().length();
		os << " " << indiv->fit().mod();
		os << " " << indiv->fit().get_regularity_score();
		os << " " << indiv->fit().get_original_length(*indiv);
		os << " " << reg_score/org_length;
		os << " " << 1.0-(reg_score/org_length);
		os << "\n";
	}



	void show(std::ostream& os, size_t k)
	{
		dbg::trace trace("stat", DBG_HERE);
		std::cout << "Visualizing for k: " << k <<
				" from: " << _all.size() << std::endl;

		//We are responsible for evaluating networks, so we have to do the work
		//of the slaves
		if(Params::mpi::slaveInit != 0){
			(*Params::mpi::slaveInit)();
		}

		if(options::map["plot-best"].as<bool>()){
			std::cout << "Visualizing best individual." << std::endl;
			log_individual(os, _best_indiv);
//#ifdef VISUALIZE
//			_best_individual->fit().runSimulation(*_best_individual);
//#endif
		} else if(k < _all.size()){
			std::cout << "Visualizing individual: " << k << std::endl;
			_all[k]->develop();
			_all[k]->show(os);
			_all[k]->fit().set_mode(fit::mode::usr1);
			_all[k]->fit().eval(*_all[k]);
			_all[k]->fit().visualizeNetwork(*_all[k]);
//#ifdef VISUALIZE
//			_all[k]->fit().runSimulation(*_all[k]);
//#endif
		} else if(k == 13371337){
			std::cout << "Visualizing all " << _all.size() <<
					" individuals." << std::endl;

			for(int i=0; i<_all.size(); i++){
				log_individual(os, _all[i]);
			}

		} else {
			//TODO: Error message
		}

	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		dbg::trace trace("stat", DBG_HERE);
		ar& BOOST_SERIALIZATION_NVP(_best_indiv);
		ar& BOOST_SERIALIZATION_NVP(_all);
	}

	pop_t& getPopulation(){
		dbg::trace trace("stat", DBG_HERE);
		return _all;
	}

    static void initOptions(){
        using namespace options;
        using namespace boost::program_options;
        add()("plot-best",
        		value<bool>()->default_value(true)->implicit_value(true),
				"Plot the best individual when loading");
        //add()("recalc-best",
        //value<bool>()->default_value(false)->implicit_value(true),
        //"Always recalculate the best individual upon loading");
        //add()("nn-postfix",
        //boost::program_options::value<std::string>()->default_value(""),
        //"Print the specified postfix after images created.");
    }

    std::string name(){
        return "ModRobot";
    }

protected:
	indiv_t _best_indiv;
	pop_t _all;
//    Clock _gen_clock;
    StatPrinter _printer;
};
}
}

#undef DBO

#endif
