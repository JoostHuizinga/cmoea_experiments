// stat_multi_modal_logic.hpp
#ifndef STAT_MULTI_MODAL_LOGIC_HPP_
#define STAT_MULTI_MODAL_LOGIC_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>
#include <modules/misc/common_compare.hpp>
#include "behavior_div.hpp"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_real.hpp>

template <typename T>
struct real6_policy : boost::spirit::karma::real_policies<T> {
  static unsigned int precision(T) { return 6; }
  static int floatfield(T n) { return boost::spirit::karma::real_policies<T>::fmtflags::fixed; }
};

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

SFERES_STAT(MultiModalLogic, Stat)
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
		pop_t pop = ea.pop();
		pop_t best;

		//Sort the population, performance first, modularity second
		std::sort(pop.begin(), pop.end(), compare::performance_mod().descending());

		//Clear the populations that we dump
		dbg::out(dbg::info, "stat") << "Clearing populations..." << std::endl;
		_pop.clear();

		//Define our data structures
		std::vector<float> pop_mod, pop_length, pop_fit;
		boost::shared_ptr<Phen> b;

		//The best individuals (all individuals that have the maximum fitness)
		dbg::out(dbg::info, "stat") << "Gathering best individuals..." << std::endl;
		for (size_t i = 0; i < pop.size() && pop[i]->fit().obj(0) == pop[0]->fit().obj(0); ++i){
		    best.push_back(pop[i]);
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

#ifdef MIXED_POP
        std::vector<float> mixed_mod, mixed_length, mixed_fit;
		//The mixed population (before selection)
		dbg::out(dbg::info, "stat") << "Gathering mixed individuals..." << std::endl;
        mixed_pop_t mixed_pop = ea.mixed_pop();
		for (size_t i = 0; i < ea.mixed_pop().size(); ++i){
			b = mixed_pop[i];
			mixed_mod.push_back(b->fit().mod());
			mixed_length.push_back(b->fit().length());
			mixed_fit.push_back(b->fit().value());
		}

        float mixed_avg_mod = std::accumulate(mixed_mod.begin(), mixed_mod.end(), 0.0f) / mixed_mod.size();
        float mixed_avg_length = std::accumulate(mixed_length.begin(), mixed_length.end(), 0.0f) / mixed_length.size();
        float mixed_avg_fit = std::accumulate(mixed_fit.begin(), mixed_fit.end(), 0.0f) / mixed_fit.size();

        std::sort(mixed_fit.begin(), mixed_fit.end());
        std::sort(mixed_mod.begin(), mixed_mod.end());
        std::sort(mixed_length.begin(), mixed_length.end());
#endif


#ifdef PARETO
		std::vector<float> pareto_mod, pareto_length, pareto_fit;
        //The pareto front
        dbg::out(dbg::info, "stat") << "Gathering pareto front data..." << std::endl;
		pop_t p = ea.pareto_front();
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

        std::sort(pareto_fit.begin(), pareto_fit.end());
        std::sort(pareto_mod.begin(), pareto_mod.end());
        std::sort(pareto_length.begin(), pareto_length.end());
#endif
		float pop_avg_mod = std::accumulate(pop_mod.begin(), pop_mod.end(), 0.0f) / pop_mod.size();
		float pop_avg_length = std::accumulate(pop_length.begin(), pop_length.end(), 0.0f) / pop_length.size();
		float pop_avg_fit = std::accumulate(pop_fit.begin(), pop_fit.end(), 0.0f) / pop_fit.size();

		dbg::out(dbg::info, "stat") << "Sorting..." << std::endl;

		std::sort(pop_fit.begin(), pop_fit.end());
		std::sort(pop_mod.begin(), pop_mod.end());
		std::sort(pop_length.begin(), pop_length.end());


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
		printer.add(true, "b_lmod", _best_individual->fit().lmod());
		printer.add(true, "b_cppn_mod", _best_individual->fit().cppn_mod());

		//Pareto stats
#ifdef PARETO
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
#endif

        //Pop stats
        printer.add(true, "po_siz", pop_mod.size());
        printer.add(true, "po_av_fit", pop_avg_fit);
        printer.add(true, "po_av_mod", pop_avg_mod);
        printer.add(true, "po_av_len", pop_avg_length);
        printer.add(false, "po_me_fit", pop_fit[pop_mod.size() / 2]);
        printer.add(false, "po_me_mod", pop_mod[pop_mod.size() / 2]);
        printer.add(false, "po_me_len", pop_length[pop_mod.size() / 2]);

#ifdef MIXED_POP
        //Mixed pop stats
        printer.add(true, "mx_siz", mixed_mod.size());
        printer.add(true, "mx_av_fit", mixed_avg_fit);
        printer.add(true, "mx_av_mod", mixed_avg_mod);
        printer.add(true, "mx_av_len", mixed_avg_length);
        printer.add(false, "mx_me_fit", mixed_fit[mixed_fit.size() / 2]);
        printer.add(false, "mx_me_mod", mixed_mod[mixed_mod.size() / 2]);
        printer.add(false, "mx_me_len", mixed_length[mixed_length.size() / 2]);
#endif

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


	void evaluate_individual(boost::shared_ptr<Phen> individual){
	    dbg::trace trace("stat", DBG_HERE);
	    individual->fit().set_mode(fit::mode::view);
	    individual->develop();
	    individual->fit().eval(*individual);
	}

	void log_individual(std::ostream& os, boost::shared_ptr<Phen> individual){
	    dbg::trace trace("stat", DBG_HERE);

	    //Add a prefix to generated images.
        namespace sp = boost::spirit::karma;
        std::cout << "Fitness before evaluation: " << individual->fit().value() << std::endl;
        //individual->fit().setCustomText(boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]],(individual->fit().value()))));

        std::cout << "Individual ID: " << individual->fit().getId() << std::endl;
        float fitness = individual->fit().value();
        float modularity = individual->fit().mod();
        size_t id = individual->fit().getId();
        sp::real_generator<float, real6_policy<float> > pl;
        individual->fit().setPostFix("_fit_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]], fitness)) +
                "_mod_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(8, '0')[sp::maxwidth(8)[pl]], modularity)) +
                "_id_" + boost::lexical_cast<std::string>(sp::format(sp::left_align(4, '0')[sp::maxwidth(4)[sp::int_]], id)) +
                "_" + options::map["nn-postfix"].as<std::string>());

	    dbg::out(dbg::info, "stat") << "Logging individual: " << individual->fit().getId() << std::endl;
	    if(options::map["analyze"].as<bool>()){
	        individual->fit().analyze(*individual);
	    }
	    if(options::map["visualize"].as<bool>()){
	        individual->fit().dump(*individual);
	    }
		os << individual->fit().getId();
		os << " " << individual->fit().value();
		os << " " << individual->fit().length();
		os << " " << individual->fit().mod();
		os << " " << individual->fit().cppn_mod();
		os << " " << individual->fit().get_left_right_cppn_score();
		for(int i=0; i<individual->fit().objs().size(); ++i){
		    os << " " << individual->fit().obj(i);
		}

		os << " " << individual->fit().get_left_right_mod();
		os << " " << individual->fit().get_regularity_score();
		os << " " << individual->fit().get_original_length(*individual);
		os << " " << individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual);
		os << " " << 1.0-(individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual));
		os << " " << individual->gen().get_nn().get_nb_connections();
		os << "\n";
	}

	void show(std::ostream& os, size_t k){
		//std::cout << "Visualizing for k: " << k << " from: " << _pop.size() << std::endl;

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

        if(k < _pop.size()){
            std::cout << "Visualizing individual: " << k << std::endl;
            evaluate_individual(_pop[k]);
            log_individual(os, _pop[k]);
        } else if(options::map["plot-all"].as<bool>()){
            std::cout << "Visualizing all " << _pop.size() << " individuals." << std::endl;
            for(int i=0; i<_pop.size(); i++){
                std::cout << "- evaluating individual: " << i << std::endl;
                evaluate_individual(_pop[i]);
            }
            modif::BehaviorDiv<> dif;
            dif.apply(_pop);

            for(int i=0; i<_pop.size(); i++){
                std::cout << "- visualizing individual: " << i << std::endl;
                log_individual(os, _pop[i]);
            }
        } else if(options::map["plot-best"].as<bool>()){
            std::cout << "Visualizing best individual." << std::endl;
            evaluate_individual(_best_individual);
            log_individual(os, _best_individual);
        } else if(options::map["test-dom-sort"].as<bool>()){
            std::vector<std::vector<boost::shared_ptr<Phen> > > fronts;
            std::vector<size_t> ranks;
            ea::dom_sort_no_duplicates(_pop, fronts, ranks);
            for(int i=0; i<fronts.size(); ++i){
                std::cout << fronts[i].size() << std::endl;
            }
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

    static void initOptions(){
        using namespace options;
        using namespace boost::program_options;
        add()("plot-best", value<bool>()->default_value(true)->implicit_value(true), "Plot the best individual when loading");
        add()("plot-all", value<bool>()->default_value(false)->implicit_value(true), "Plot all individuals when loading");
        add()("recalc-best", value<bool>()->default_value(false)->implicit_value(true), "Always recalculate the best individual upon loading");
        add()("nn-postfix", boost::program_options::value<std::string>()->default_value(""), "Print the specified postfix after images created.");
        add()("visualize", value<bool>()->default_value(false)->implicit_value(true), "Write a network for all individuals.");
        add()("analyze", value<bool>()->default_value(false)->implicit_value(true), "Perform additional analysis.");
    }

protected:
    pop_t _pop;
    indiv_t _best_individual;
	StatPrinter printer;
};
}
}


#endif //STAT_MULTI_MODAL_LOGIC_HPP_
