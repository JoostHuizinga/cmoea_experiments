#ifndef STAT_MODULARITY_HPP_
#define STAT_MODULARITY_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>
#include <modules/continue/global_options.hpp>
#include <modules/datatools/common_compare.hpp>

namespace sferes
{
namespace stat
{

class StatPrinter{
public:
    void dataStart(){
        index = 0;
    }

    template<typename DataType>
    void add(bool active, std::string name, DataType value){
        if(!active) return;
        std::ostringstream ss;
        ss << value;
        std::string value_str = ss.str();
        if(index >= values.size()){
            headers.push_back(name);
            values.push_back(value_str);
        } else {
            values[index] = value_str;
        }
        ++index;
    }

    void dataPrint(boost::shared_ptr<std::ofstream> file_stream){
        if(file_stream->tellp() == 0){
            for(size_t i=0; i<headers.size(); ++i){
                (*file_stream) <<  headers[i] << " ";
            }
            (*file_stream) << "\n";
        }
        for(size_t i=0; i<values.size(); ++i){
            (*file_stream) <<  values[i] << " ";
        }
        (*file_stream) << std::endl;
    }

private:
    size_t index;
    std::vector<std::string> headers;
    std::vector<std::string> values;
};

SFERES_STAT(Mod, Stat)
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
        if (ea.gen() % Params::stats::period != 0) return;

        //Create log file
		dbg::out(dbg::info, "stat") << "Creating log file..." << std::endl;
		this->_create_log_file(ea, "modularity.dat");

		//Get references to the different populations
		pop_t p = ea.pareto_front();
		pop_t pop = ea.pop();
		pop_t best;
		mixed_pop_t mixed_pop = ea.mixed_pop();

		//Sort the pareto front, performance first, modularity second
		std::sort(p.begin(), p.end(), compare::performance_mod().descending());

		//Clear the populations that we dump
		dbg::out(dbg::info, "stat") << "Clearing populations..." << std::endl;
		_pop.clear();

		//Define our data structures
		std::vector<float> pareto_mod, pop_mod, mixed_mod, pareto_length, pop_length, mixed_length, pareto_fit, pop_fit, mixed_fit;
		boost::shared_ptr<Phen> b;

		//The best individuals (all individuals that have the maximum fitness)
		dbg::out(dbg::info, "stat") << "Gathering best individuals..." << std::endl;
		for (size_t i = 0; i < p.size() && p[i]->fit().obj(0) == p[0]->fit().obj(0); ++i){
		    best.push_back(p[i]);
		}
		_best_individual = best[0];

		//The current population (after selection)
        dbg::out(dbg::info, "stat") << "Gathering current population front data..." << std::endl;
        for (size_t i = 0; i < pop.size(); ++i){
            b = pop[i];
            _pop.push_back(b);
            pop_mod.push_back(b->fit().mod());
            pop_length.push_back(b->fit().length());
            pop_fit.push_back(b->fit().value());
        }

        //If there is no log-file, stop processing data after best and all are populated
        if(!this->_log_file){
            dbg::out(dbg::info, "stat") << "Log file is null, aborting..." << std::endl;
            return;
        }

		//The mixed population (before selection)
		dbg::out(dbg::info, "stat") << "Gathering mixed individuals..." << std::endl;
		for (size_t i = 0; i < ea.mixed_pop().size(); ++i){
			b = mixed_pop[i];
			mixed_mod.push_back(b->fit().mod());
			mixed_length.push_back(b->fit().length());
			mixed_fit.push_back(b->fit().value());
		}

		//The pareto front
		dbg::out(dbg::info, "stat") << "Gathering pareto front data..." << std::endl;
		for (size_t i = 0; i < p.size(); ++i)
		{
			b = p[i];
			pareto_mod.push_back(b->fit().mod());
			pareto_length.push_back(b->fit().length());
			pareto_fit.push_back(b->fit().value());
		}

		float pareto_avg_mod = std::accumulate(pareto_mod.begin(), pareto_mod.end(), 0.0f) / pareto_mod.size();
		float pareto_avg_length = std::accumulate(pareto_length.begin(), pareto_length.end(), 0.0f) / pareto_length.size();
		float pareto_avg_fit = std::accumulate(pareto_fit.begin(), pareto_fit.end(), 0.0f) / pareto_fit.size();

		float pop_avg_mod = std::accumulate(pop_mod.begin(), pop_mod.end(), 0.0f) / pop_mod.size();
		float pop_avg_length = std::accumulate(pop_length.begin(), pop_length.end(), 0.0f) / pop_length.size();
		float pop_avg_fit = std::accumulate(pop_fit.begin(), pop_fit.end(), 0.0f) / pop_fit.size();

		float mixed_avg_mod = std::accumulate(mixed_mod.begin(), mixed_mod.end(), 0.0f) / mixed_mod.size();
		float mixed_avg_length = std::accumulate(mixed_length.begin(), mixed_length.end(), 0.0f) / mixed_length.size();
		float mixed_avg_fit = std::accumulate(mixed_fit.begin(), mixed_fit.end(), 0.0f) / mixed_fit.size();

		dbg::out(dbg::info, "stat") << "Sorting..." << std::endl;
		std::sort(mixed_fit.begin(), mixed_fit.end());
		std::sort(mixed_mod.begin(), mixed_mod.end());
		std::sort(mixed_length.begin(), mixed_length.end());
		std::sort(pop_fit.begin(), pop_fit.end());
		std::sort(pop_mod.begin(), pop_mod.end());
		std::sort(pop_length.begin(), pop_length.end());
		std::sort(pareto_fit.begin(), pareto_fit.end());
		std::sort(pareto_mod.begin(), pareto_mod.end());
		std::sort(pareto_length.begin(), pareto_length.end());

		dbg::out(dbg::info, "stat") << "Calculating bloat..." << std::endl;
		nn_t cppn = _best_individual->gen().get_nn();
		nn_t simplified_cppn = _best_individual->gen().get_nn();
		simplified_cppn.simplify();

		float con_ratio = 1;
		if (cppn.get_nb_connections() > 0){
		    con_ratio = (1.0 - (double(simplified_cppn.get_nb_connections()) / double(cppn.get_nb_connections())));
		}

        float node_ratio = 1;
        if (cppn.get_nb_neurons() > 0){
            node_ratio = (1.0 - (double(simplified_cppn.get_nb_neurons()) / double(cppn.get_nb_neurons())));
        }

		std::vector<vertex_desc_t> to_remove;
		BGL_FORALL_VERTICES_T(v, simplified_cppn.get_graph(), graph_t){
			if (boost::in_degree(v, simplified_cppn.get_graph()) + boost::out_degree(v, simplified_cppn.get_graph()) == 0){
				to_remove.push_back(v);
			}
		}
		for (size_t i = 0; i < to_remove.size(); ++i){
			remove_vertex(to_remove[i], simplified_cppn.get_graph());
		}

		dbg::out(dbg::info, "stat") << "Writing..." << std::endl;
		printer.dataStart();
        //Gen
        printer.add(true, "#gen", ea.gen());

		//Best stats
		printer.add(true, "b_siz", best.size());
		printer.add(true, "b_fit", _best_individual->fit().value());
		printer.add(true, "b_mod", _best_individual->fit().mod());
		printer.add(true, "b_len", _best_individual->fit().length());

		//Pareto stats
        printer.add(true, "pa_siz", pareto_mod.size());
        printer.add(false, "pa_av_fit", pareto_avg_fit);
        printer.add(false, "pa_av_mod", pareto_avg_mod);
        printer.add(false, "pa_av_len", pareto_avg_length);
        printer.add(false, "pa_me_fit", pareto_fit[pareto_mod.size() / 2]);
        printer.add(false, "pa_me_mod", pareto_mod[pareto_mod.size() / 2]);
        printer.add(false, "pa_me_len", pareto_length[pareto_mod.size() / 2]);
        printer.add(true, "pa_mx_fit", *std::max_element(pareto_fit.begin(), pareto_fit.end()));
        printer.add(true, "pa_mn_fit", *std::min_element(pareto_fit.begin(), pareto_fit.end()));
        printer.add(true, "pa_mx_mod", *std::max_element(pareto_mod.begin(), pareto_mod.end()));
        printer.add(true, "pa_mn_mod", *std::min_element(pareto_mod.begin(), pareto_mod.end()));
        printer.add(true, "pa_mx_len", *std::max_element(pareto_length.begin(), pareto_length.end()) );
        printer.add(true, "pa_mn_len", *std::min_element(pareto_length.begin(), pareto_length.end()));

        //Pop stats
        printer.add(true, "po_siz", pop_mod.size());
        printer.add(true, "po_av_fit", pop_avg_fit);
        printer.add(true, "po_av_mod", pop_avg_mod);
        printer.add(true, "po_av_len", pop_avg_length);
        printer.add(false, "po_me_fit", pop_fit[pop_mod.size() / 2]);
        printer.add(false, "po_me_mod", pop_mod[pop_mod.size() / 2]);
        printer.add(false, "po_me_len", pop_length[pop_mod.size() / 2]);

        //Mixed pop stats
        printer.add(true, "mx_siz", mixed_mod.size());
        printer.add(true, "mx_av_fit", mixed_avg_fit);
        printer.add(true, "mx_av_mod", mixed_avg_mod);
        printer.add(true, "mx_av_len", mixed_avg_length);
        printer.add(false, "mx_me_fit", mixed_fit[mixed_fit.size() / 2]);
        printer.add(false, "mx_me_mod", mixed_mod[mixed_mod.size() / 2]);
        printer.add(false, "mx_me_len", mixed_length[mixed_length.size() / 2]);

        //Network size
        printer.add(true, "con", cppn.get_nb_connections());
        printer.add(true, "nue", cppn.get_nb_neurons());
        printer.add(true, "con_s", simplified_cppn.get_nb_connections());
        printer.add(true, "nue_s", simplified_cppn.get_nb_neurons());
        printer.add(true, "con_r", con_ratio);
        printer.add(true, "nue_r", node_ratio);
        printer.dataPrint(this->_log_file);

		dbg::out(dbg::info, "stat") << "Refresh finished" << std::endl;
	}

	void log_individual(std::ostream& os, boost::shared_ptr<Phen> individual){
	    dbg::trace trace("stat", DBG_HERE);
	    dbg::out(dbg::info, "stat") << "Logging individual: " << individual->fit().getId() << std::endl;
	    individual->fit().set_mode(fit::mode::view);
	    individual->develop();
	    individual->fit().eval(*individual);
	    individual->fit().analyze(*individual);
	    individual->fit().dump(*individual);

	    dbg::out(dbg::info, "stat") << " - printing id" << std::endl;
		os << individual->fit().getId();
		dbg::out(dbg::info, "stat") << " - printing performance" << std::endl;
		os << " " << individual->fit().value();
		dbg::out(dbg::info, "stat") << " - printing length" << std::endl;
		os << " " << individual->fit().length();
		dbg::out(dbg::info, "stat") << " - printing mod" << std::endl;
		os << " " << individual->fit().mod();
		dbg::out(dbg::info, "stat") << " - printing left-right mod" << std::endl;
		os << " " << individual->fit().get_left_right_mod();
		dbg::out(dbg::info, "stat") << " - printing left-right solve" << std::endl;
		os << " " << individual->fit().get_left_right_solve();
		dbg::out(dbg::info, "stat") << " - printing dyn mod" << std::endl;
		os << " " << individual->fit().get_dyn_mod_score();
		dbg::out(dbg::info, "stat") << " - printing regularity" << std::endl;
		os << " " << individual->fit().get_regularity_score();
		dbg::out(dbg::info, "stat") << " - printing original length" << std::endl;
		os << " " << individual->fit().get_original_length(*individual);
		dbg::out(dbg::info, "stat") << " - printing regularity/original length" << std::endl;
		os << " " << individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual);
		dbg::out(dbg::info, "stat") << " - printing 1 - regularity/original length" << std::endl;
		os << " " << 1.0-(individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual));

		for(int j=0; j<individual->fit().get_solved_length();j++){
		    dbg::out(dbg::info, "stat") << " - one solved: " << j  << std::endl;
			os << " " << individual->fit().get_one_solved(j);
			dbg::out(dbg::info, "stat") << " - two solved: " << j  << std::endl;
			os << " " << individual->fit().get_two_solved(j);
		}

		os << "\n";
	}

	void show(std::ostream& os, size_t k){
		std::cout << "Visualizing for k: " << k << " from: " << _pop.size() << std::endl;

		//We are responsible for evaluating networks, so we have to do the work of the slaves
        if(Params::mpi::slaveInit != 0){
            (*Params::mpi::slaveInit)();
        }

        //Assign every individual its id
        for(size_t i=0; i<_pop.size(); i++){
            std::ostringstream ss;
            ss << std::setw( 4 ) << std::setfill( '0' ) << i;
            _pop[i]->fit().setPostFix(ss.str());
            _pop[i]->fit().setId(i);
        }

        if(options::map["plot-best"].as<bool>()){
            std::cout << "Visualizing best individual." << std::endl;
            log_individual(os, _best_individual);

        } else if(options::map["plot-all"].as<bool>()){
            std::cout << "Visualizing all " << _pop.size() << " individuals." << std::endl;
            for(int i=0; i<_pop.size(); i++){
                log_individual(os, _pop[i]);
            }

        } else if(k < _pop.size()){
			std::cout << "Visualizing individual: " << k << std::endl;
			log_individual(os, _pop[k]);
		}
	}


	template<class Archive>
	void serialize(Archive& ar, const unsigned int version){
		ar& BOOST_SERIALIZATION_NVP(_pop);
		ar& BOOST_SERIALIZATION_NVP(_best_individual);
	}


	std::vector<boost::shared_ptr<Phen> >& getPopulation(){
		return _pop;
	}

	indiv_t getBest(){
	    return _best_individual;
	}

protected:
    pop_t _pop;
    indiv_t _best_individual;
	StatPrinter printer;
};
}
}


#endif
