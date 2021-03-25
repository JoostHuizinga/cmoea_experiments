/*
 * lexicase.hpp
 *
 *  Created on: Sep 25, 2017
 *      Author: Joost Huizinga
 */

#ifndef MODULES_E_LEXICASE_LEXICASE_HPP_
#define MODULES_E_LEXICASE_LEXICASE_HPP_

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <sferes/stc.hpp>
#include <sferes/parallel.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/ea/dom_sort.hpp>
#include <sferes/ea/common.hpp>
#include <sferes/ea/crowd.hpp>

#define DBO dbg::out(dbg::info, "lexicase")
#define DBOS dbg::out(dbg::info, "lexicase_select")


namespace sferes
{
namespace ea
{

// Main class
SFERES_EA(ELexicase, Ea){

public:
    typedef boost::shared_ptr<Phen> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    SFERES_EA_FRIEND(ELexicase);

    ELexicase(){
    	DBO << "Parallel initialized" << std::endl;
    	_order_initialized = false;
    	parallel::init();
    	dbg::assertion(DBG_ASSERTION(!(Params::e_lexicase::pool_mad && Params::e_lexicase::fixed_offset)));
    }

	void random_pop()
	{
		dbg::trace trace("ea", DBG_HERE);
        this->_pop.resize(Params::pop::size);
        BOOST_FOREACH(indiv_t& indiv, this->_pop) {
          indiv = indiv_t(new Phen());
          indiv->random();
        }
        this->_eval_pop(this->_pop);
        this->apply_modifier();
	}


	void epoch()
	{
		dbg::trace trace("ea", DBG_HERE);
		_child_pop.clear();
		_mixed_pop.clear();
#ifdef DBG_ENABLED
		DBO << "Pop. before parent selection." << std::endl;
		_print_pop(this->_pop);
#endif
		DBO << "Parents selected:" << std::endl;
		_parent_select (this->_pop, _child_pop, Params::pop::select_size);
		DBO << "Mutating children" << std::endl;
		parallel::p_for(parallel::range_t(0, _child_pop.size()),
				mutate<Phen>(_child_pop));
		DBO << "Evaluating children" << std::endl;
		_eval_pop(_child_pop);
#ifdef DBG_ENABLED
		DBO << "Children after mutation" << std::endl;
		_print_pop(_child_pop);
#endif
		_merge(this->_pop, _child_pop, _mixed_pop);
#ifdef DBG_ENABLED
		DBO << "Mixed population" << std::endl;
		_print_pop(_mixed_pop);
#endif
		_survivor_select (_mixed_pop, this->_pop, Params::pop::size);
#ifdef DBG_ENABLED
		DBO << "Final population" << std::endl;
		_print_pop(this->_pop);
#endif
		this->apply_modifier();
	}


 protected:
	pop_t _child_pop;
	pop_t _mixed_pop;
	std::vector<size_t> _order;
	bool _order_initialized;

	void _print_indiv(size_t i, const indiv_t& indiv){
		std::stringstream str;
		str << "Indiv " << i << ":";
		for(size_t j=0; j<indiv->fit().objs().size(); ++j){
			str << " " << indiv->fit().objs()[j];
		}
		dbg::out(dbg::info, "lexicase") << str.str() << std::endl;;
	}

	void _print_pop(const pop_t& pop){
		dbg::trace trace("ea", DBG_HERE);
		for(size_t i=0; i<pop.size(); ++i){
			_print_indiv(i, pop[i]);
		}
	}

	// for resuming
	void _set_pop(const pop_t& pop) {
		dbg::trace trace("ea", DBG_HERE);
		assert(!pop.empty());
		this->_pop = pop;
	}

  	void _merge(const pop_t& pop1, const pop_t& pop2, pop_t& pop3)
  	{
  		dbg::trace trace("ea", DBG_HERE);
  		assert(pop1.size());
  		assert(pop2.size());
  		pop3.clear();
  		pop3.insert(pop3.end(), pop1.begin(), pop1.end());
  		pop3.insert(pop3.end(), pop2.begin(), pop2.end());
  		assert(pop3.size() == pop1.size() + pop2.size());
  	}

	void _eval_pop(pop_t& pop)
	{
		dbg::trace trace("ea", DBG_HERE);
		this->_eval.eval(pop, 0, pop.size(), this->_fit_proto);
	}

	size_t _select_one(const pop_t& old_pop,
			const std::vector<float>& pop_offsets,
			const std::vector<float>& pop_max)
	{
		dbg::trace trace("ea", DBG_HERE);

		size_t nb_obj = old_pop[0]->fit().objs().size();
		sferes::misc::shuffle(_order);
		std::vector<size_t> old_pool;
		std::vector<size_t> new_pool(old_pop.size());
		for(size_t i=0; i<new_pool.size(); ++i){
			new_pool[i]=i;
		}

		size_t i = 0;
		size_t obj_i;
		size_t ind_i;
		float max_score;
		float offset;
		float threshold;
		DBOS << "Selection start" << std::endl;
		while(i < nb_obj && new_pool.size() > 1){
			obj_i = _order[i];
			old_pool = new_pool;
			new_pool.clear();

			// TODO: This is not actually the algorithm as described in the paper,
			// which uses the maximum score across the whole population
			max_score = old_pop[old_pool[0]]->fit().objs()[obj_i];
			for(size_t pool_i=1; pool_i<old_pool.size(); ++pool_i){
				ind_i = old_pool[pool_i];
				if(old_pop[ind_i]->fit().objs()[obj_i] > max_score){
					max_score = old_pop[ind_i]->fit().objs()[obj_i];
				}
			}

			if(Params::e_lexicase::pool_max){
				max_score = _get_max(old_pop, old_pool, obj_i);
			} else {
				max_score = pop_max[obj_i];
			}
			if(Params::e_lexicase::pool_mad){
				offset = _get_mad(old_pop, old_pool, obj_i);
			} else {
				offset = pop_offsets[obj_i];
			}
			threshold = max_score - offset;

			DBOS << "Obj: " << obj_i << " Max: " << max_score << " Offset: " << pop_offsets[obj_i] << " Thresh: " << threshold << std::endl;

			for(size_t pool_i=0; pool_i<old_pool.size(); ++pool_i){
				ind_i = old_pool[pool_i];
				if (old_pop[ind_i]->fit().objs()[obj_i] >= threshold){
					new_pool.push_back(ind_i);
//					_print_indiv(ind_i, old_pop[ind_i]);
				}
			}
			DBOS << "Passed: " << new_pool.size() << std::endl;
			++i;
		}
		if(new_pool.size() == 1){
			return new_pool[0];
		} else if (new_pool.size() > 1){
			return sferes::misc::rand_in_vector(new_pool);
		} else if (old_pool.size() > 0){
			return sferes::misc::rand_in_vector(old_pool);
		} else {
			throw std::runtime_error("Unexpected error when selecting individual.");
		}
	}

	void _init_order(size_t nb_objs){
		dbg::trace trace("ea", DBG_HERE);
		_order.resize(nb_objs);
		for(size_t i=0; i<nb_objs; ++i){
			_order[i]=i;
		}
		_order_initialized = true;
	}

	float _get_mad(
			const pop_t& pop,
			const std::vector<size_t>& pool,
			size_t obj_i)
	{
		std::vector<float> scores(pool.size());
		size_t ind_i;
		for(size_t pool_i=0; pool_i<pool.size(); ++pool_i){
			ind_i = pool[pool_i];
			scores[pool_i] = pop[ind_i]->fit().objs()[obj_i];
		}
		return compare::getMadDestructive(scores);
	}

	float _get_max(
			const pop_t& pop,
			const std::vector<size_t>& pool,
			size_t obj_i)
	{
		float max_score = pop[pool[0]]->fit().objs()[obj_i];
		size_t ind_i;
		for(size_t pool_i=1; pool_i<pool.size(); ++pool_i){
			ind_i = pool[pool_i];
			if(pop[ind_i]->fit().objs()[obj_i] > max_score){
				max_score = pop[ind_i]->fit().objs()[obj_i];
			}
		}
		return max_score;
	}

	std::vector<float> _get_offset(const pop_t& pop){
		dbg::trace trace("ea", DBG_HERE);
		size_t nb_obj = pop[0]->fit().objs().size();
		std::vector<float> mad(nb_obj);
		if(Params::e_lexicase::fixed_offset){
			dbg::assertion(DBG_ASSERTION(nb_obj <= Params::e_lexicase::offset_size()));
			for(int i=0; i<nb_obj; ++i){
				mad[i] = Params::e_lexicase::offset(i);
			}
		} else {
			std::vector<float> scores(pop.size());
			for(size_t obj_i=0; obj_i<nb_obj; ++obj_i){
				for(size_t ind_i=0; ind_i<pop.size(); ++ind_i){
					scores[ind_i] = pop[ind_i]->fit().objs()[obj_i];
				}
				mad[obj_i] = compare::getMadDestructive(scores);
				DBOS << "Select threshold obj " << obj_i << ": " << mad[obj_i] << std::endl;
			}
		}
		return mad;
	}

	std::vector<float> _get_max(const pop_t& pop){
		dbg::trace trace("ea", DBG_HERE);
		size_t nb_obj = pop[0]->fit().objs().size();
		std::vector<float> max_obj(nb_obj, std::numeric_limits<float>::min());
		for(size_t obj_i=0; obj_i<nb_obj; ++obj_i){
			for(size_t ind_i=0; ind_i<pop.size(); ++ind_i){
				float obj = pop[ind_i]->fit().objs()[obj_i];
				if(obj > max_obj[obj_i]){
					max_obj[obj_i] = obj;
				}
			}
			DBOS << "Max " << obj_i << ": " << max_obj[obj_i] << std::endl;
		}
		return max_obj;
	}

	void _parent_select(const pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		dbg::trace trace("ea", DBG_HERE);

		// Implement lexicase selection
		std::vector<float> offset = _get_offset(old_pop);
		std::vector<float> max_objs = _get_max(old_pop);
		if(!_order_initialized) _init_order(old_pop[0]->fit().objs().size());
		new_pop.resize(select_size);
		for(size_t i=0; i<select_size; i+=2){
			size_t ind_1_i = _select_one(old_pop, offset, max_objs);
			size_t ind_2_i = _select_one(old_pop, offset, max_objs);
#ifdef DBG_ENABLED
			_print_indiv(i, old_pop[ind_1_i]);
			_print_indiv(i+1, old_pop[ind_2_i]);
#endif
			old_pop[ind_1_i]->cross(old_pop[ind_2_i], new_pop[i], new_pop[i+1]);
		}
	}

	void _survivor_select(const pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		dbg::trace trace("ea", DBG_HERE);
		std::vector<float> offset = _get_offset(old_pop);
		std::vector<float> max_objs = _get_max(old_pop);
		if(!_order_initialized) _init_order(old_pop[0]->fit().objs().size());
		new_pop.resize(select_size);
		for(size_t i=0; i<select_size; ++i){
			size_t ind_i = _select_one(old_pop, offset, max_objs);
//			DBO << "indiv: " << i << " selected: " << ind_i << std::endl;
			new_pop[i] = old_pop[ind_i];
		}
//		DBO << "Selection finished" << std::endl;
	}
};
}
}

#undef DBO
#undef DBOS

#endif /* MODULES_E_LEXICASE_LEXICASE_HPP_ */
