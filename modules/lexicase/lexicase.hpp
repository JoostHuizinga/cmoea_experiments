/*
 * lexicase.hpp
 *
 *  Created on: Sep 25, 2017
 *      Author: Joost Huizinga
 */

#ifndef MODULES_LEXICASE_LEXICASE_HPP_
#define MODULES_LEXICASE_LEXICASE_HPP_

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



namespace sferes
{
namespace ea
{

struct paralelTest {
	~paralelTest() { }
	paralelTest() { }
	void operator() (const parallel::range_t& r) const {
//		for (size_t i = r.begin(); i != r.end(); ++i)
//			nop();
	}
};


// Main class
SFERES_EA(Lexicase, Ea){

public:
    typedef boost::shared_ptr<Phen> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    SFERES_EA_FRIEND(Lexicase);

    Lexicase(){
    	DBO << "Parallel initialized" << std::endl;
    	_order_initialized = false;
    	parallel::init();
    }

	void random_pop()
	{
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
//		parallel::p_for(parallel::range_t(0, _child_pop.size()),
//				paralelTest());
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
		for(size_t i=0; i<pop.size(); ++i){
			_print_indiv(i, pop[i]);
		}
	}

	// for resuming
	void _set_pop(const pop_t& pop) {
		assert(!pop.empty());
		this->_pop = pop;
	}

  	void _merge(const pop_t& pop1, const pop_t& pop2, pop_t& pop3)
  	{
  		assert(pop1.size());
  		assert(pop2.size());
  		pop3.clear();
  		pop3.insert(pop3.end(), pop1.begin(), pop1.end());
  		pop3.insert(pop3.end(), pop2.begin(), pop2.end());
  		assert(pop3.size() == pop1.size() + pop2.size());
  	}

	void _eval_pop(pop_t& pop)
	{
		this->_eval.eval(pop, 0, pop.size(), this->_fit_proto);
	}

	int _lex_compare(indiv_t ind1, indiv_t ind2, std::vector<size_t>& order){
		float obj1;
		float obj2;
		for(size_t i=0; i<order.size(); ++i){
			obj1 = ind1->fit().objs()[order[i]];
			obj2 = ind2->fit().objs()[order[i]];
			if(obj1 > obj2){
				return 1;
			} else if(obj1 < obj2){
				return -1;
			}
		}
		return 0;
	}

	size_t _select_one(pop_t& old_pop){
		sferes::misc::shuffle(_order);
		size_t b_ind_i = 0;
		for(size_t ind_i=1; ind_i<old_pop.size(); ++ind_i){
			if(_lex_compare(old_pop[b_ind_i], old_pop[ind_i], _order) < 0){
				b_ind_i = ind_i;
			}
		}
		return b_ind_i;
	}

	void _init_order(size_t nb_objs){
		_order.resize(nb_objs);
		for(size_t i=0; i<nb_objs; ++i){
			_order[i]=i;
		}
		_order_initialized = true;
	}

	void _parent_select(pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		// Implement lexicase selection
		if(!_order_initialized) _init_order(old_pop[0]->fit().objs().size());
		new_pop.resize(select_size);
		for(size_t i=0; i<select_size; i+=2){
			size_t ind_1_i = _select_one(old_pop);
			size_t ind_2_i = _select_one(old_pop);
#ifdef DBG_ENABLED
			_print_indiv(i, old_pop[ind_1_i]);
			_print_indiv(i+1, old_pop[ind_2_i]);
#endif
			old_pop[ind_1_i]->cross(old_pop[ind_2_i], new_pop[i], new_pop[i+1]);
		}
	}

	void _survivor_select(pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		// Implement lexicase selection
		if(!_order_initialized) _init_order(old_pop[0]->fit().objs().size());
		new_pop.resize(select_size);
		for(size_t i=0; i<select_size; ++i){
			size_t ind_i = _select_one(old_pop);
			new_pop[i] = old_pop[ind_i];
		}
	}
};
}
}

#undef DBO

#endif /* MODULES_LEXICASE_LEXICASE_HPP_ */
