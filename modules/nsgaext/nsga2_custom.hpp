/*
 * nsga2_custom.hpp
 *
 *  Created on: Aug 18, 2014
 *      Author: joost
 */

#ifndef NSGA2_CUSTOM_HPP_
#define NSGA2_CUSTOM_HPP_

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

#ifdef ENABLE_TIMING
#include <exp/modularity/clock.hpp>
#endif


namespace sferes
{
namespace ea
{
// Main class
SFERES_EA(Nsga2Custom, Ea){

public:
	typedef boost::shared_ptr<crowd::Indiv<Phen> > indiv_t;
	typedef typename std::vector<indiv_t> pop_t;
	typedef typename pop_t::iterator it_t;
	typedef typename std::vector<std::vector<indiv_t> > front_t;
        SFERES_EA_FRIEND(Nsga2Custom);

	void random_pop()
	{
		dbg::out(dbg::info, "ea") << "NSGA2 Custom: random pop" << std::endl;
		parallel::init();

		_parent_pop.resize(Params::pop::size);
		dbg::assertion(DBG_ASSERTION(Params::pop::size % 4 == 0));

		pop_t init_pop = pop_t((size_t)(Params::pop::size * Params::pop::initial_aleat));;
		parallel::p_for(parallel::range_t(0, init_pop.size()), random<crowd::Indiv<Phen> >(init_pop));
		_init_pop(init_pop);
		dbg::out(dbg::info, "ea") << "NSGA2 Custom: random pop - done" << std::endl;
	}


	void epoch()
	{
	    dbg::out(dbg::info, "epoch") << "NSGA2 Custom: Clear populations" << std::endl;
#ifdef ENABLE_TIMING
		_epoch_clock.resetAndStart();
#endif
		_mixed_pop.clear();
		_child_pop.clear();
		this->_pop.clear();
		_pareto_front.clear();

		dbg::out(dbg::info, "epoch") << "NSGA2 Custom: Parent selection..." << std::endl;
#ifdef ENABLE_TIMING
		_parent_selection_clock.resetAndStart();
#endif
		_selection (_parent_pop, _child_pop, Params::pop::select_size);

#ifdef ENABLE_TIMING
		_parent_selection_clock.stop();
		_mutate_clock.resetAndStart();
#endif
		dbg::out(dbg::info, "epoch") << "NSGA2 Custom: Mutation..." << std::endl;

		parallel::p_for(parallel::range_t(0, _child_pop.size()),
				mutate<crowd::Indiv<Phen> >(_child_pop));
#ifdef ENABLE_TIMING
		_mutate_clock.stop();
		_eval_clock.resetAndStart();
#endif
		dbg::out(dbg::info, "epoch") << "NSGA2 Custom: Evaluation..." << std::endl;

#ifndef EA_EVAL_ALL
		_eval_pop(_child_pop);
		_merge(_parent_pop, _child_pop, _mixed_pop);
#else
		_merge(_parent_pop, _child_pop, _mixed_pop);
		_eval_pop(_mixed_pop);
#endif

#ifdef ENABLE_TIMING
		_eval_clock.stop();
        _mod_clock.resetAndStart();
#endif
		dbg::out(dbg::info, "epoch") << "NSGA2 Custom: Modifier..." << std::endl;
		_apply_modifier(_mixed_pop);

#ifdef ENABLE_TIMING
		_mod_clock.stop();
		_survivor_selection_clock.resetAndStart();
#endif

#ifndef NDEBUG
		BOOST_FOREACH(indiv_t& ind, _mixed_pop)
		for (size_t i = 0; i < ind->fit().objs().size(); ++i)
		{ assert(!std::isnan(ind->fit().objs()[i])); }
#endif

		dbg::out(dbg::info, "epoch") << "Survivor selection..." << std::endl;

		_fill_nondominated_sort(_mixed_pop, _parent_pop);
		_convert_pop(_parent_pop, this->_pop);

#ifdef ENABLE_TIMING
		_survivor_selection_clock.stop();
		_epoch_clock.stop();
#endif
		assert(_parent_pop.size() == Params::pop::size);
		assert(_pareto_front.size() <= Params::pop::size * 2);
		//	assert(_child_pop.size() == 0);
		assert(this->_pop.size() == Params::pop::size);

		dbg::out(dbg::info, "epoch") << "Epoch end" << std::endl;
	}

	const std::vector<boost::shared_ptr<Phen> >& pareto_front() const { return _pareto_front; }

	const pop_t& mixed_pop() const { return _mixed_pop; }

	const pop_t& parent_pop() const { return _parent_pop; }

	const pop_t& child_pop() const { return _child_pop; }



//    void best_pop(std::vector<boost::shared_ptr<Phen> >& population){
////        	std::cout << "Reading population: pnsga" << std::endl;
//    	_mixed_pop.clear();
//        for (size_t i = 0; i < Params::pop::size*2; ++i){
//        	indiv_t indiv (new crowd::Indiv<Phen>());
//        	indiv->gen() = population[0]->gen();
//        	_mixed_pop.push_back(indiv);
//        }
//        _init_pop(_mixed_pop);
//        _convert_pop(_parent_pop, this->_pop);
//    }

#ifdef ENABLE_TIMING
    double getEpochTime() const{
    	return _epoch_clock.time();
    }

    double getParentSelectionTime() const{
    	return _parent_selection_clock.time();
    }

    double getMutateTime() const{
    	return _mutate_clock.time();
    }

    double getEvalTime() const{
    	return _eval_clock.time();
    }

    double getModTime() const{
    	return _mod_clock.time();
    }

    double getSurvivorSelectionTime() const{
    	return _survivor_selection_clock.time();
    }

 protected:
	Clock _epoch_clock;
	Clock _parent_selection_clock;
	Clock _mutate_clock;
	Clock _eval_clock;
	Clock _mod_clock;
	Clock _survivor_selection_clock;
#endif

 protected:
    std::vector<boost::shared_ptr<Phen> > _pareto_front;
	pop_t _parent_pop;
	pop_t _child_pop;
	pop_t _mixed_pop;

      // for resuming
      void _set_pop(const std::vector<boost::shared_ptr<Phen> >& pop) {
        assert(!pop.empty());
        _parent_pop.resize(pop.size());
        for (size_t i = 0; i < pop.size(); ++i)
          _parent_pop[i] = boost::shared_ptr<crowd::Indiv<Phen> >(new crowd::Indiv<Phen>(*pop[i]));
      }

    void _init_pop(pop_t& init_pop)
    {
    	dbg::out(dbg::info, "ea") << "NSGA2 Custom: evaluating population" << std::endl;
      _eval_pop(init_pop);
      dbg::out(dbg::info, "ea") << "NSGA2 Custom: applying modifiers" << std::endl;
      _apply_modifier(init_pop);

      dbg::out(dbg::info, "ea") << "NSGA2 Custom: performing selection" << std::endl;
      front_t fronts;
      _rank_crowd(init_pop, fronts);
      _fill_nondominated_sort(init_pop, _parent_pop);
      _convert_pop(_parent_pop, this->_pop);
    }

	void _update_pareto_front(const front_t& fronts)
	{
		_convert_pop(fronts.front(), _pareto_front);
	}

	void _convert_pop(const pop_t& pop1,
			std::vector<boost::shared_ptr<Phen> > & pop2)
	{
		pop2.resize(pop1.size());
		for (size_t i = 0; i < pop1.size(); ++i)
			pop2[i] = pop1[i];
	}

	void _eval_pop(pop_t& pop)
	{
		this->_eval.eval(pop, 0, pop.size(), this->_fit_proto);
	}

	void _apply_modifier(pop_t& pop)
	{
		_convert_pop(pop, this->_pop);
		this->apply_modifier();
	}
	void _fill_nondominated_sort(pop_t& mixed_pop, pop_t& new_pop)
	{
		assert(mixed_pop.size());
		front_t fronts;
#ifndef NDEBUG
		BOOST_FOREACH(indiv_t& ind, mixed_pop)
		for (size_t i = 0; i < ind->fit().objs().size(); ++i)
		{ assert(!std::isnan(ind->fit().objs()[i])); }
#endif
		_rank_crowd(mixed_pop, fronts);
		new_pop.clear();

		// fill the i first layers
		size_t i;
		for (i = 0; i < fronts.size(); ++i)
			if (fronts[i].size() + new_pop.size() < Params::pop::size)
				new_pop.insert(new_pop.end(), fronts[i].begin(), fronts[i].end());
			else
				break;

		size_t size = Params::pop::size - new_pop.size();

		// sort the last layer
		if (new_pop.size() < Params::pop::size)
		{
			std::sort(fronts[i].begin(), fronts[i].end(), crowd::compare_crowd());
			for (size_t k = 0; k < size ; ++k)
			{
				assert(i < fronts.size());
#ifdef PAR_RAND
				if(fronts[i][k]->crowd() > 0){								//New
#endif
					new_pop.push_back(fronts[i][k]);
#ifdef PAR_RAND
				} else {													//New
					std::random_shuffle (fronts[i].begin() + k, fronts[i].end() );	//New
					for (; k < size ; ++k)									//New
					{														//New
						new_pop.push_back(fronts[i][k]);					//New
					}														//New
				}															//New
#endif
			}
		}
		assert(new_pop.size() == Params::pop::size);
	}

	//
	void _merge(const pop_t& pop1, const pop_t& pop2, pop_t& pop3)
	{
		assert(pop1.size());
		assert(pop2.size());
		pop3.clear();
		pop3.insert(pop3.end(), pop1.begin(), pop1.end());
		pop3.insert(pop3.end(), pop2.begin(), pop2.end());
		assert(pop3.size() == pop1.size() + pop2.size());
	}

	// --- tournament selection ---
	void _selection(pop_t& old_pop, pop_t& new_pop, size_t select_size)
	{
		new_pop.resize(select_size);
		std::vector<size_t> a1, a2;
		misc::rand_ind(a1, old_pop.size());
		misc::rand_ind(a2, old_pop.size());
		// todo : this loop could be parallelized
		for (size_t i = 0; i < select_size; i += 4)
		{
			const indiv_t& p1 = _tournament(old_pop[a1[i]], old_pop[a1[i + 1]]);
			const indiv_t& p2 = _tournament(old_pop[a1[i + 2]], old_pop[a1[i + 3]]);
			const indiv_t& p3 = _tournament(old_pop[a2[i]], old_pop[a2[i + 1]]);
			const indiv_t& p4 = _tournament(old_pop[a2[i + 2]], old_pop[a2[i + 3]]);
			assert(i + 3 < new_pop.size());
			p1->cross(p2, new_pop[i], new_pop[i + 1]);
			p3->cross(p4, new_pop[i + 2], new_pop[i + 3]);
		}
	}

	const indiv_t& _tournament(const indiv_t& i1, const indiv_t& i2)
	{
		// if (i1->rank() < i2->rank())
		//   return i1;
		// else if (i2->rank() > i1->rank())
		//   return i2;
		// else if (misc::flip_coin())
		//   return i1;
		// else
		//   return i2;

		int flag = fit::dominate_flag(i1, i2);
		if (flag == 1)
			return i1;
		if (flag == -1)
			return i2;
		if (i1->crowd() > i2->crowd())
			return i1;
		if (i1->crowd() < i2->crowd())
			return i2;
		if (misc::flip_coin())
			return i1;
		else
			return i2;
	}

	// --- rank & crowd ---

	void _rank_crowd(pop_t& pop, front_t& fronts){
		std::vector<size_t> ranks;
#ifndef NDEBUG
		BOOST_FOREACH(indiv_t& ind, pop)
		for (size_t i = 0; i < ind->fit().objs().size(); ++i)
		{ assert(!std::isnan(ind->fit().objs()[i])); }
#endif
		typename Params::ea::dom_sort_f()(pop, fronts, ranks);
		_update_pareto_front(fronts);
		parallel::p_for(parallel::range_t(0, fronts.size()),
				crowd::assign_crowd<indiv_t >(fronts));

		for (size_t i = 0; i < ranks.size(); ++i)
			pop[i]->set_rank(ranks[i]);
		parallel::sort(pop.begin(), pop.end(), crowd::compare_ranks());;
	}

//	void _assign_rank(pop_t& pop)
//	{
//		int rank = 0;
//		fit::compare_pareto comp;
//		assert(pop.size());
//		std::sort(pop.begin(), pop.end(), comp);
//		pop[0]->set_rank(0);
//		for (unsigned i = 1; i < pop.size(); ++i)
//		{
//			assert(comp(pop[i-1], pop[i]) || comp.eq(pop[i -1], pop[i]));
//			if (comp(pop[i-1], pop[i]))
//				++rank;
//			pop[i]->set_rank(rank);
//		}
//	}

};
}
}


#endif /* NSGA2_CUSTOM_HPP_ */
