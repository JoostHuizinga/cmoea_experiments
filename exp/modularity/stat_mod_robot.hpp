#ifndef STAT_MOD_ROBOT_HPP_
#define STAT_MOD_ROBOT_HPP_

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

#include "stat_mod_robot_shared.hpp"

#define DBO dbg::out(dbg::info, "stat")

namespace sferes
{
namespace stat
{

SFERES_STAT(ModRobot, Stat)
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
		pop_t pareto_front = ea.pareto_front();
		pop_t pop = ea.pop();
		mixed_pop_t mixed_pop = ea.mixed_pop();

		//Define temporary vectors and values
		std::vector<float> pareto_mod, pop_mod, mixed_mod, pareto_length;
		std::vector<float> pop_length, mixed_length, pareto_fit, pop_fit;
		std::vector<float> mixed_fit;
		std::vector<double> eval_time, sim_time, nn_time, physics_time;
		std::vector<double> reset_time, develop_time;
		indiv_t temp_indiv;

		//Best individual
		_best_indiv = compare::max(pareto_front, compare::performance_mod());

		//The mixed population (before selection)
		for (size_t i = 0; i < ea.mixed_pop().size(); ++i)
		{
			temp_indiv = mixed_pop[i];
			mixed_mod.push_back(temp_indiv->fit().mod());
			mixed_length.push_back(temp_indiv->fit().length());
			mixed_fit.push_back(temp_indiv->fit().value());
			eval_time.push_back(temp_indiv->fit().getEvalTime());
			sim_time.push_back(temp_indiv->fit().getSimTime());
			nn_time.push_back(temp_indiv->fit().getNNTime());
			physics_time.push_back(temp_indiv->fit().getPhysicsTime());
			reset_time.push_back(temp_indiv->fit().getResetTime());
			develop_time.push_back(temp_indiv->fit().getDevelopTime());
		}


		//The pareto front
		std::set<std::vector<float> > unique_points;
		int double_points = 0;

		for (size_t i = 0; i < pareto_front.size(); ++i)
		{
			temp_indiv = pareto_front[i];
			pareto_mod.push_back(temp_indiv->fit().mod());
			pareto_length.push_back(temp_indiv->fit().length());
			pareto_fit.push_back(temp_indiv->fit().value());
			if(!unique_points.insert(temp_indiv->fit().objs()).second){
				//Value already existed in set
				double_points++;
			}
		}

		//The current population (after selection)
		for (size_t i = 0; i < pop.size(); ++i)
		{
			temp_indiv = pop[i];
			temp_indiv->fit().setId(i);
			_all.push_back(temp_indiv);
			pop_mod.push_back(temp_indiv->fit().mod());
			pop_length.push_back(temp_indiv->fit().length());
			pop_fit.push_back(temp_indiv->fit().value());
		}

		//If there is no log-file, stop processing data after best and all are
		//populated
        DBO << "Creating log file" << std::endl;
		this->_create_log_file(ea, "modularity.dat");
		DBO << "Log file: " << this->_log_file << std::endl;
		if(!this->_log_file) return;


		//Calculate averages
		float pareto_avg_mod = compare::average(pareto_mod);
		float pareto_avg_length = compare::average(pareto_length);
		float pareto_avg_fit = compare::average(pareto_fit);

		float pareto_min_mod = compare::min(pareto_mod);
		float pareto_max_mod = compare::max(pareto_mod);
		float pareto_min_len = compare::min(pareto_length);
		float pareto_max_len = compare::max(pareto_length);
		float pareto_min_fit = compare::min(pareto_fit);
		float pareto_max_fit = compare::max(pareto_fit);

		float pop_avg_mod = compare::average(pop_mod);
		float pop_avg_length = compare::average(pop_length);
		float pop_avg_fit = compare::average(pop_fit);

		float mixed_avg_mod = compare::average(mixed_mod);
		float mixed_avg_length = compare::average(mixed_length);
		float mixed_avg_fit = compare::average(mixed_fit);

		double mixed_avg_eval_time = compare::average(eval_time);
		double mixed_avg_sim_time = compare::average(sim_time);
		double mixed_avg_nn_time = compare::average(nn_time);
		double mixed_avg_physics_time = compare::average(physics_time);
		double mixed_avg_reset_time = compare::average(reset_time);
		double mixed_avg_develop_time = compare::average(develop_time);

		//Sort vectors
		compare::sort(mixed_fit);
		compare::sort(mixed_mod);
		compare::sort(mixed_length);
		compare::sort(pop_fit);
		compare::sort(pop_mod);
		compare::sort(pop_length);
		compare::sort(pareto_fit);
		compare::sort(pareto_mod);
		compare::sort(pareto_length);

		//Calculate bloat
		nn_t cppn = _best_indiv->gen().nn();
		nn_t simplified_cppn = _best_indiv->gen().nn();
		simplified_cppn.simplify();
		std::vector<vertex_desc_t> to_remove;
		BGL_FORALL_VERTICES_T(v, simplified_cppn.get_graph(), graph_t){
			int in_degree = boost::in_degree(v, simplified_cppn.get_graph());
			int out_degree = boost::out_degree(v, simplified_cppn.get_graph());
			if (in_degree + out_degree == 0){
				to_remove.push_back(v);
			}
		}
		for (size_t i = 0; i < to_remove.size(); ++i){
			remove_vertex(to_remove[i], simplified_cppn.get_graph());
		}

		// Calculate neuron ratios
		double sim_con = double(simplified_cppn.get_nb_connections());
		double org_con = double(cppn.get_nb_connections());
		double con_ratio = 1.0 - (sim_con / org_con);
		double sim_neur = double(simplified_cppn.get_nb_neurons());
		double org_neur = double(cppn.get_nb_neurons());
		double neur_ratio = 1.0 - (sim_neur / org_neur);

		// Draw random number
		unsigned rnd_ch = misc::rand(std::numeric_limits<unsigned int>::max());

		dbg::out(dbg::info, "stat") << "Writing line" << std::endl;

		_printer.dataStart();
		//Start writing
		_printer.add(true, "#gen", ea.gen());
		//Best stats
		_printer.add(true, "b_fit", _best_indiv->fit().value());
		_printer.add(true, "b_food", _best_indiv->fit().getFoodFitness());
		_printer.add(true, "b_pred",_best_indiv->fit().getPredatorFitness());
		_printer.add(true, "b_mod",_best_indiv->fit().mod());
		_printer.add(true, "b_len",_best_indiv->fit().length());
		_printer.add(true, "b_div",_best_indiv->fit().getDiv());

		//Pareto stats
		_printer.add(true, "p_size",pareto_mod.size());
		_printer.add(true, "p_identical",double_points);
		_printer.add(true, "p_max_fit", pareto_min_fit);
		_printer.add(true, "p_min_fit", pareto_max_fit);
		_printer.add(true, "p_max_mod", pareto_min_mod);
		_printer.add(true, "p_min_mod", pareto_max_mod);
		_printer.add(true, "p_max_len", pareto_min_len);
		_printer.add(true, "p_min_len", pareto_max_len);

		//Pop stats
		_printer.add(true, "pop_size", pop_mod.size() );
		_printer.add(true, "pop_av_fit", pop_avg_fit);
		_printer.add(true, "pop_av_mod", pop_avg_mod );
		_printer.add(true, "pop_av_len", pop_avg_length );

		//Mixed stats
		_printer.add(true, "mix_size",mixed_mod.size() );
		_printer.add(true, "mix_av_fit",mixed_avg_fit );
		_printer.add(true, "mix_av_mod",mixed_avg_mod );
		_printer.add(true, "mix_av_len",mixed_avg_length );

		//Network size
		_printer.add(true, "nb_con",cppn.get_nb_connections());
		_printer.add(true, "nb_neur",cppn.get_nb_neurons());
		_printer.add(true, "simp_nb_con",simplified_cppn.get_nb_connections());
		_printer.add(true, "simp_nb_neur",simplified_cppn.get_nb_neurons());
		_printer.add(true, "con_ratio", con_ratio);
		_printer.add(true, "neur_ratio", neur_ratio);

		//Slave time in mili-seconds
		_printer.add(false, "develop_time", mixed_avg_develop_time);
		_printer.add(false, "eval_time", mixed_avg_eval_time);
		_printer.add(false, "sim_time", mixed_avg_sim_time);
		_printer.add(false, "nn_time", mixed_avg_nn_time);
		_printer.add(false, "physic_time", mixed_avg_physics_time);
		_printer.add(false, "reset_time", mixed_avg_reset_time);

		//Master time  in mili-seconds
//		_printer.add(false, "gen_t", gen_time  );
//		_printer.add(false, "epoch_t", ea.getEpochTime());
//		_printer.add(false, "parent_select_t", ea.getParentSelectionTime());
//		_printer.add(false, "mutate_t", ea.getMutateTime());
//		_printer.add(false, "master_eval_t", ea.getEvalTime());
//		_printer.add(false, "mod_t", ea.getModTime());
//		_printer.add(false, "survivor_select_t", ea.getSurvivorSelectionTime());

		//Random number check
		_printer.add(true, "random_number", rnd_ch);

        //Print
		_printer.dataPrint(this->_log_file);

	}


	void show(std::ostream& os, size_t k)
	{
		dbg::trace trace("stat", DBG_HERE);
		sharedShow<indiv_t, pop_t, Params>(os, k, _all, _best_indiv);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		dbg::trace trace("stat", DBG_HERE);
		dbg::out(dbg::info, "serialize") << "Serializing: _best_individual" << std::endl;
		ar& BOOST_SERIALIZATION_NVP(_best_indiv);
		dbg::out(dbg::info, "serialize") << "Serializing: _all" << std::endl;
		ar& BOOST_SERIALIZATION_NVP(_all);
	}

	pop_t& getPopulation(){
		dbg::trace trace("stat", DBG_HERE);
		return _all;
	}

    static void initOptions(){
    	sharedInitOptions();
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
