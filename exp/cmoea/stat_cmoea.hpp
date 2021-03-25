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


#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>

#include <modules/datatools/stat_printer.hpp>
#include <modules/datatools/common_compare.hpp>
#include <modules/datatools/formatting.hpp>
#include <modules/diversity/behavior_div.hpp>

using namespace modules::misc;

namespace sferes
{
namespace stat
{


SFERES_STAT(StatCmoea, Stat)
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
        this->_create_log_file(ea, "cmoea.dat");

        // Initialize our data structures
        pop_t pop = ea.pop();
        pop_t best;
        std::vector<float> pop_fit, pop_div, pop_exp;
        indiv_t temp;
        _pop.clear();

        // Populate the archive population before the local population gets sorted
        for (size_t i = 0; i < pop.size(); ++i){
            _pop.push_back(pop[i]);
            //std::cout << "Indiv: "<< i << " fitness: " << pop[i]->fit().value() << std::endl;
        }

        // Sort the population, performance first
        dbg::out(dbg::info, "stat") << "Sorting population..." << std::endl;
        std::sort(pop.begin(), pop.end(), compare::performance().descending());

        // The best individuals (all individuals that have the maximum fitness)
        dbg::out(dbg::info, "stat") << "Gathering best individuals..." << std::endl;
        compare::gatherBest(pop, best);
        _best_individual = best[0];

        // The current population (after selection)
        dbg::out(dbg::info, "stat") << "Gathering current population data..." << std::endl;
        for (size_t i = 0; i < pop.size(); ++i){
            temp = pop[i];
            pop_fit.push_back(temp->fit().value());
            pop_div.push_back(temp->fit().div());
            pop_exp.push_back(temp->fit().exp());
        }

        //If there is no log-file, stop processing data after best and all are populated
        if(!this->_log_file){
            dbg::out(dbg::info, "stat") << "Log file is null, aborting..." << std::endl;
            return;
        }

        compare::stats stats_fit = compare::sortAndCalcStats(pop_fit);
        compare::stats stats_div = compare::sortAndCalcStats(pop_div);
        compare::stats stats_exp = compare::sortAndCalcStats(pop_exp);

        dbg::out(dbg::info, "stat") << "Writing..." << std::endl;
        printer.dataStart();
        //Gen
        printer.add(true, "#gen", ea.gen());

        //Best stats
        printer.add(true, "b_siz", best.size());
        printer.add(true, "b_fit", _best_individual->fit().value());
        printer.add(true, "b_div", _best_individual->fit().div());
        printer.add(true, "b_exp", _best_individual->fit().exp());

        //Pop stats
        printer.add(true, "p_fit_avg", stats_fit.avg);
        printer.add(true, "p_fit_med", stats_fit.median);
        printer.add(true, "p_fit_min", stats_fit.min);
        printer.add(true, "p_fit_max", stats_fit.max);
        printer.add(true, "p_fit_qu1", stats_fit.first_q);
        printer.add(true, "p_fit_qu2", stats_fit.third_q);

        printer.add(true, "p_div_avg", stats_div.avg);
        printer.add(true, "p_div_med", stats_div.median);
        printer.add(true, "p_div_min", stats_div.min);
        printer.add(true, "p_div_max", stats_div.max);
        printer.add(true, "p_div_qu1", stats_div.first_q);
        printer.add(true, "p_div_qu2", stats_div.third_q);

        printer.add(true, "p_exp_avg", stats_exp.avg);
        printer.add(true, "p_exp_med", stats_exp.median);
        printer.add(true, "p_exp_min", stats_exp.min);
        printer.add(true, "p_exp_max", stats_exp.max);
        printer.add(true, "p_exp_qu1", stats_exp.first_q);
        printer.add(true, "p_exp_qu2", stats_exp.third_q);
#ifdef PARETO
        printer.add(true, "pa_siz", ea.pareto_front().size());
#endif

        printer.dataPrint(this->_log_file);

        dbg::out(dbg::info, "stat") << "Refresh finished" << std::endl;
    }


    void evaluate_individual(boost::shared_ptr<Phen> individual){
        dbg::trace trace("stat", DBG_HERE);
        std::cout << "Individual ID: " << individual->fit().getId() << std::endl;
        std::cout << "Fitness before evaluation: " << individual->fit().value() << std::endl;
        std::cout << "Training mazes solved before evaluation: " << individual->fit().getTrainMazesSolved() << std::endl;
        std::cout << "Test mazes solved before evaluation: " << individual->fit().getTestMazesSolved() << std::endl;

        individual->fit().set_mode(fit::mode::view);
        individual->develop();
        if(options::map["test-and-train"].as<bool>()){
        	individual->fit().eval(*individual);
        	individual->fit().evalTestset(*individual);
        } else if(options::map["testset"].as<bool>()){
            individual->fit().evalTestset(*individual);
        } else if(options::map["testfail"].as<bool>()){
        	individual->fit().set_mode(fit::mode::eval);
            individual->fit().evalTestset(*individual);
            individual->fit().set_mode(fit::mode::view);
            individual->fit().evalTestset(*individual);
        } else if(options::map["no-eval"].as<bool>()){
        	// intentionally left blank
        } else {
            individual->fit().eval(*individual);
        }
    }

    void log_individual(std::ostream& os, boost::shared_ptr<Phen> individual){
        dbg::trace trace("stat", DBG_HERE);

        //Add a prefix to generated images.
        //        namespace sp = boost::spirit::karma;
        std::cout << "Individual ID: " << individual->fit().getId() << std::endl;
        std::cout << "Fitness after evaluation: " << individual->fit().value() << std::endl;
        std::cout << "Training mazes solved after evaluation: " << individual->fit().getTrainMazesSolved() << std::endl;
        std::cout << "Test mazes solved after evaluation: " << individual->fit().getTestMazesSolved() << std::endl;
        float fitness = individual->fit().value();
        size_t id = individual->fit().getId();
        //        sp::real_generator<float, real6_policy<float> > pl;
        individual->fit().setPostFix("_fit_" + formatting::pad(fitness, 8) +
                "_id_" + formatting::pad(fitness, 4) +
                "_" + options::map["nn-postfix"].as<std::string>());

        dbg::out(dbg::info, "stat") << "Logging individual: " << individual->fit().getId() << std::endl;
        //	    if(options::map["analyze"].as<bool>()){
        //	        individual->fit().analyze(*individual);
        //	    }
        //	    if(options::map["visualize"].as<bool>()){
        //	        individual->fit().dump(*individual);
        //	    }
        os << individual->fit().getId();
        os << " " << individual->fit().value();
        os << " " << individual->fit().getTrainMazesSolved();
        os << " " << individual->fit().getTestMazesSolved();
        //		os << " " << individual->fit().value();
        //		os << " " << individual->fit().length();
        //		os << " " << individual->fit().mod();
        //		os << " " << individual->fit().cppn_mod();
        //		os << " " << individual->fit().get_left_right_cppn_score();
        //		for(int i=0; i<individual->fit().objs().size(); ++i){
        //		    os << " " << individual->fit().obj(i);
        //		}
        //
        //		os << " " << individual->fit().get_left_right_mod();
        //		os << " " << individual->fit().get_regularity_score();
        //		os << " " << individual->fit().get_original_length(*individual);
        //		os << " " << individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual);
        //		os << " " << 1.0-(individual->fit().get_regularity_score()/individual->fit().get_original_length(*individual));
        //		os << " " << individual->gen().get_nn().get_nb_connections();
        os << "\n";
    }

    void show(std::ostream& os, size_t k){
        dbg::trace trace("stat", DBG_HERE);
        if(!checkOptions()){
            std::cerr << "ERROR: StatCmoea options not initialized." << std::endl;
            return;
        }
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

        if(options::map["recalc-best"].as<bool>()){
            float best_fit = _pop[0]->fit().value();
            _best_individual = _pop[0];
            for(size_t i=0; i<_pop.size(); i++){
                if(_pop[i]->fit().value() > best_fit){
                    best_fit = _pop[i]->fit().value();
                    _best_individual = _pop[i];
                }
            }
            std::cout << "Best individual fitness: " << _best_individual->fit().value() << std::endl;
        }

        if(options::map["plot-indiv"].as<int>() >= 0){
            int indiv_id = options::map["plot-indiv"].as<int>();
            std::cout << "Visualizing individual: " << indiv_id << std::endl;
            evaluate_individual(_pop[indiv_id]);
            log_individual(os, _pop[indiv_id]);
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
        }
    }


    template<class Archive>
    void serialize(Archive& ar, const unsigned int version){
        dbg::trace trace("stat", DBG_HERE);
        ar& BOOST_SERIALIZATION_NVP(_pop);
        ar& BOOST_SERIALIZATION_NVP(_best_individual);
    }


    std::vector<boost::shared_ptr<Phen> >& getPopulation(){
        dbg::trace trace("stat", DBG_HERE);
        return _pop;
    }

    /**
     * Sets the current population.
     *
     * @param ea The ea which holds the population to be stored.
     */
    template<typename E>
    void setPopulation(const E& ea){
        dbg::trace trace("stat", DBG_HERE);
        const pop_t& pop = ea.pop();
        _pop.clear();
        for (size_t i = 0; i < pop.size(); ++i){
            _pop.push_back(pop[i]);
        }
    }

    /**
     * Sets the current population.
     *
     * @param ea The ea which holds the population to be stored.
     */
    void setPopulationVector(const std::vector<boost::shared_ptr<Phen> >& pop){
        dbg::trace trace("stat", DBG_HERE);
        _pop.clear();
        for (size_t i = 0; i < pop.size(); ++i){
            _pop.push_back(pop[i]);
        }
    }

    indiv_t getBest(){
        dbg::trace trace("stat", DBG_HERE);
        return _best_individual;
    }

    static void initOptions(){
        dbg::trace trace("stat", DBG_HERE);
        using namespace options;
        using namespace boost::program_options;
        add()("plot-best", value<bool>()->default_value(true)->implicit_value(true),
        		"Plot the best individual when loading.");
        add()("plot-all", value<bool>()->default_value(false)->implicit_value(true),
        		"Plot all individuals when loading.");
        add()("plot-indiv", value<int>()->default_value(-1),
        		"Plot individual with specified ID (e.i. index in population).");
        add()("nn-postfix", boost::program_options::value<std::string>()->default_value(""),
        		"Print the specified postfix after images created.");
        add()("visualize", value<bool>()->default_value(false)->implicit_value(true),
        		"Write a network for all individuals.");
        add()("analyze", value<bool>()->default_value(false)->implicit_value(true),
        		"Perform additional analysis.");
        add()("recalc-best", value<bool>()->default_value(true)->implicit_value(true),
        		"Iterate over all individuals an check their fitness before visualizing.");
        add()("testset", value<bool>()->default_value(false)->implicit_value(true),
        		"Visualizes the individual on the testset, rather than the training set.");
        add()("testfail", value<bool>()->default_value(false)->implicit_value(true),
        		"Visualizes only the failures an individual experienced on the test set.");
        add()("test-and-train", value<bool>()->default_value(false)->implicit_value(true),
        		"Visualizes the individual on both the testset and the training set.");
        add()("no-eval", value<bool>()->default_value(false)->implicit_value(true),
        		"Do not load or visualize the individual on anything (for logging pre-stored information).");
        add()("frame-skip", value<int>()->default_value(1),
        		"Initial frameskip for recording videos.");
    }

    static bool checkOptions(){
        dbg::trace trace("stat", DBG_HERE);
        if(options::map.count("plot-best")==0) return false;
        if(options::map.count("plot-all")==0) return false;
        if(options::map.count("plot-indiv")==0) return false;
        if(options::map.count("nn-postfix")==0) return false;
        if(options::map.count("visualize")==0) return false;
        if(options::map.count("analyze")==0) return false;
        if(options::map.count("recalc-best")==0) return false;
        if(options::map.count("no-eval")==0) return false;
        return true;
    }

    std::string name(){
        return "StatCmoea";
    }

protected:
    pop_t _pop;
    indiv_t _best_individual;
    StatPrinter printer;
};
}
}


#endif //STAT_MULTI_MODAL_LOGIC_HPP_
