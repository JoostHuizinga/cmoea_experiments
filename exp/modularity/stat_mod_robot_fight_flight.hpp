#ifndef STAT_MOD_ROBOT_FOOD_HPP_
#define STAT_MOD_ROBOT_FOOD_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/clock.hpp>
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
class ff_mod_cppnmod: public compare::compare<compare::performance_mod>
{
public:
    template<typename I>
    bool operator() (const boost::shared_ptr<I> i1,
    		const boost::shared_ptr<I> i2) const
    {
        // first, compare the main objective
        if (i1->fit().getFfFitness() < i2->fit().getFfFitness())
        	return this->_smaller;
        if (i1->fit().getFfFitness() > i2->fit().getFfFitness())
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

SFERES_STAT(ModRobotFf, Stat)
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
		_best_indiv = compare::max(pop, ff_mod_cppnmod());

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
        _printer.add(true, "ff_fit", _best_indiv->fit().getFfFitness());
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

        /* most significant bit -> least significant bit
         * 000000000 -> all objectives (0)
         * 111111111 -> no objectives (511)
		 * 010111111 -> Attack and Retreat (191)
		 * 011111111 -> Retreat (511 - 256 = 255)
		 * 101111111 -> Turn from (511 - 128 = 383)
		 * 110111111 -> Attack (511 - 64 = 447)
		 * 111011111 -> Strike (511 - 32 = 479)
		 * 111101111 -> Move to (511 - 16 = 495)
		 * 111110111 -> Turn to (511 - 8 = 503)
		 * 111111011 -> Turn right (511 - 4 = 507)
		 * 111111101 -> Turn left (511 - 2 = 509)
		 * 111111110 -> Forward (511 - 1 = 510)
         */

		os << indiv->fit().getId();
		os << " " << indiv->fit().value();
		os << " " << indiv->fit().length();
		os << " " << indiv->fit().mod();
		os << " " << indiv->fit().getBinFitness(191);
		os << " " << indiv->fit().getBinFitness(255);
		os << " " << indiv->fit().getBinFitness(383);
		os << " " << indiv->fit().getBinFitness(447);
		os << " " << indiv->fit().getBinFitness(479);
		os << " " << indiv->fit().getBinFitness(495);
		os << " " << indiv->fit().getBinFitness(503);
		os << " " << indiv->fit().getBinFitness(507);
		os << " " << indiv->fit().getBinFitness(509);
		os << " " << indiv->fit().getBinFitness(510);
		os << "\n";

		std::cout << "Fitness: "<< indiv->fit().value() << "\n";
		std::cout << "Length: " << indiv->fit().length() << "\n";
		std::cout << "Modularity: "<< indiv->fit().mod() << "\n";
		std::cout << "Fight-Flight: " << indiv->fit().getBinFitness(191) << "\n";
		std::cout << "Retreat: "<< indiv->fit().getBinFitness(255) << "\n";
		std::cout << "Turn-from: "<< indiv->fit().getBinFitness(383) << "\n";
		std::cout << "Attack: "<< indiv->fit().getBinFitness(447) << "\n";
		std::cout << "Strike: "<< indiv->fit().getBinFitness(479) << "\n";
		std::cout << "Move-to: "<< indiv->fit().getBinFitness(495) << "\n";
		std::cout << "Turn-to: "<< indiv->fit().getBinFitness(503) << "\n";
		std::cout << "Turn right: "<< indiv->fit().getBinFitness(507) << "\n";
		std::cout << "Turn left: "<< indiv->fit().getBinFitness(509) << "\n";
		std::cout << "Forward: "<< indiv->fit().getBinFitness(510) << "\n";

		indiv->develop();
		indiv->fit().set_mode(fit::mode::view);
		indiv->fit().eval(*indiv);
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

		// Write header
		os << "id ";
		os << "fit ";
		os << "len ";
		os << "mod ";
		os << "att_retr ";
		os << "retr ";
		os << "trn_frm ";
		os << "att ";
		os << "strk ";
		os << "mv_to ";
		os << "trn_to ";
		os << "trn_r ";
		os << "trn_l ";
		os << "frw ";
		os << "\n";

		if(options::map.count("task") > 0){
			std::cout << "Visualizing best individual for task: " <<
					options::map["task"].as<int>() << std::endl;
			int task = options::map["task"].as<int>();
			indiv_t best_indiv = _all[0];
			for(int i=0; i<_all.size(); i++){
				float performance = _all[i]->fit().getBinFitness(task);
				if(best_indiv->fit().getBinFitness(task) < performance){
					best_indiv = _all[i];
				}
			}
			log_individual(os, best_indiv);
		} else if(options::map["plot-best"].as<bool>()){
			std::cout << "Visualizing best individual." << std::endl;
			log_individual(os, _best_indiv);
		} else if(k < _all.size()){
			std::cout << "Visualizing individual: " << k << std::endl;
			_all[k]->develop();
			_all[k]->show(os);
			_all[k]->fit().set_mode(fit::mode::usr1);
			_all[k]->fit().eval(*_all[k]);
			_all[k]->fit().visualizeNetwork(*_all[k]);
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
        add()("task",
        		value<int>(),
				"Plot the best individual on the provided task");
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
