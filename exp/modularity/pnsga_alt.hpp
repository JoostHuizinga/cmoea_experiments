//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
//|
//| This software is a computer program whose purpose is to facilitate
//| experiments in evolutionary computation and evolutionary robotics.
//|
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.




#ifndef PNSGA_MULT_HPP_
#define PNSGA_MULT_HPP_

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>

#include <sferes/stc.hpp>
#include <sferes/parallel.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/ea/dom_sort_basic.hpp>
#include <sferes/ea/common.hpp>
#include <sferes/ea/crowd.hpp>

namespace sferes
{
namespace ea
{
namespace pnsga_alt
{
//template<typename Indiv>
template<typename Indiv, typename Params = stc::_Params, typename Exact = stc::Itself> \
class prob_dom_f : public stc::Any<Exact>{
public:
	prob_dom_f(){

	}

	prob_dom_f(const std::vector<Indiv>&pop){
		fillCache(pop);
	}


	inline bool operator() (const Indiv &ind, const std::vector<Indiv>&pop) const
	{
		BOOST_FOREACH(Indiv i, pop)
		{
			if (dominate_flag(i, ind) == 1){
				return false;
			}
		}
		return true;
	}

	inline int dominate_flag(const Indiv& i1, const Indiv& i2) const
	{
		int result = 0;
		assert(i1->fit().objs().size());
		assert(i2->fit().objs().size());
		assert(i1->fit().objs().size() == i2->fit().objs().size());

		//std::cout << "Testing for: " << i1 << " and: " << i2 << std::endl;
		if(cache.count(std::pair<Indiv, Indiv>(i1, i2)) > 0){
			//std::cout << "Cache used for: " << i1 << " and: " << i2 << std::endl;
			return cache.at(std::pair<Indiv, Indiv>(i1, i2));
		}

		size_t nb_objs = i1->fit().objs().size();
		assert(nb_objs);

		bool flag1 = false, flag2 = false;
		for (unsigned i = 0; i < nb_objs; ++i)
		{
			assert(i < Params::ea::obj_pressure_size());
			if (misc::rand<float>() > Params::ea::obj_pressure(i))
				continue;
			float fi1 = i1->fit().obj(i);
			float fi2 = i2->fit().obj(i);
			if (fi1 > fi2)
				flag1 = true;
			else
				if (fi2 > fi1)
					flag2 = true;
		}
		if (flag1 && !flag2){
			result = 1;
		} else if (!flag1 && flag2){
			result = -1;
		} else {
			result = 0;
		}
		return result;
	}
private:
	void fillCache(const std::vector<Indiv>&pop){
		std::vector<std::set<size_t> > dominates (pop.size());

		for(size_t i=0; i<pop.size(); i++){
			for(size_t j=i; j<pop.size(); j++){
				int flag = dominate_flag(pop[i], pop[j]);

				if(flag == 1){
					dominates[i].insert(j);
				} else if(flag==-1){
					dominates[j].insert(i);
				}


			//	std::cout << "Added: " << pop[i] << " and: " << pop[j] << std::endl;
			}
		}

		for(size_t i=0; i<pop.size(); i++){
			std::set<size_t> currentNew = dominates[i];

			while(currentNew.size()>0){
				std::set<size_t> nextNew;
				for(std::set<size_t>::iterator it = currentNew.begin(); it != currentNew.end(); it++){
					for(std::set<size_t>::iterator it2 = dominates[*it].begin(); it2 != dominates[*it].end(); it2++){
						if(dominates[i].count(*it2)==0){
							dominates[i].insert(*it2);
							nextNew.insert(*it2);
						}
					}
				}
				currentNew = nextNew;
			}
		}


		for(size_t i=0; i<pop.size(); i++){
			for(size_t j=i; j<pop.size(); j++){
				if(dominates[i].count(j) > 0 && not(dominates[j].count(i) > 0)){
					cache[std::pair<Indiv, Indiv>(pop[i], pop[j])] = 1;
					cache[std::pair<Indiv, Indiv>(pop[j], pop[i])] = -1;
				} else if(dominates[j].count(i) > 0 && not(dominates[i].count(j) > 0)){
					cache[std::pair<Indiv, Indiv>(pop[i], pop[j])] = -1;
					cache[std::pair<Indiv, Indiv>(pop[j], pop[i])] = 1;
				} else{
					cache[std::pair<Indiv, Indiv>(pop[i], pop[j])] = 0;
					cache[std::pair<Indiv, Indiv>(pop[j], pop[i])] = 0;
				}
			}
		}

	}

	std::map<std::pair<Indiv, Indiv>, int > cache;
};
}
     // Main class
    SFERES_EA(Pnsga_mult, Ea)
    {
      public:
        typedef boost::shared_ptr<crowd::Indiv<Phen> > indiv_t;
        typedef typename std::vector<indiv_t> pop_t;
        typedef typename pop_t::iterator it_t;
        typedef typename std::vector<std::vector<indiv_t> > front_t;

        void random_pop()
        {
          parallel::init();

          _parent_pop.resize(Params::pop::size);
          assert(Params::pop::size % 4 == 0);

          pop_t init_pop((size_t)(Params::pop::size * Params::pop::initial_aleat));
          parallel::p_for(parallel::range_t(0, init_pop.size()), random<crowd::Indiv<Phen> >(init_pop));
          _init_pop(init_pop);
        }

        void load_and_run(const std::string& fname)
        {
           //load the individual
          std::ifstream ifs(fname.c_str());
          if (ifs.fail())
          {
            std::cerr << "Cannot open :" << fname << std::endl;
            exit(1);
          }
          typedef boost::archive::xml_iarchive ia_t;
          ia_t ia(ifs);
          Phen phen;
          ia >> boost::serialization::make_nvp("phen", phen);
          pop_t init_pop((size_t)(Params::pop::size));
          for (size_t i = 0; i < init_pop.size(); ++i)
          {
            init_pop[i] =
              boost::shared_ptr<crowd::Indiv<Phen> >(new crowd::Indiv<Phen>());
            init_pop[i]->gen() = phen.gen();
          }
          _init_pop(init_pop);
          for (this->_gen = 0; this->_gen < Params::pop::nb_gen; ++this->_gen)
          {
            this->epoch();
            this->update_stats();
            if (this->_gen % Params::pop::dump_period == 0)
              this->_write(this->_gen);
          }
        }

        void epoch()
        {
        	_mixed_pop.clear();
        	_child_pop.clear();
        	assert(_mixed_pop.size() == 0);
        	this->_pop.clear();
        	_pareto_front.clear();
        	_selection(_parent_pop, _child_pop);
        	parallel::p_for(parallel::range_t(0, _child_pop.size()),
        			mutate<crowd::Indiv<Phen> >(_child_pop));
#ifndef EA_EVAL_ALL
        	_eval_pop(_child_pop);
        	_merge(_parent_pop, _child_pop, _mixed_pop);
#else
        	_merge(_parent_pop, _child_pop, _mixed_pop);
        	_eval_pop(_mixed_pop);
#endif
        	_apply_modifier(_mixed_pop);
#ifndef NDEBUG
        	BOOST_FOREACH(indiv_t & ind, _mixed_pop)
        	for (size_t i = 0; i < ind->fit().objs().size(); ++i)
        	{
        		assert(!std::isnan(ind->fit().objs()[i]));
        	}
#endif
        	_fill_nondominated_sort(_mixed_pop, _parent_pop);

        	_convert_pop(_parent_pop, this->_pop);

        	assert(_parent_pop.size() == Params::pop::size);
        	assert(_pareto_front.size() <= Params::pop::size * 2);

        	//	assert(_child_pop.size() == 0);
        	assert(this->_pop.size() == Params::pop::size);
        }

        const std::vector<boost::shared_ptr<Phen> >& pareto_front() const {
          return _pareto_front;
        }
        const pop_t& mixed_pop() const{
          return _mixed_pop;
        }
        const pop_t& parent_pop() const{
          return _parent_pop;
        }
        const pop_t& child_pop() const{
          return _child_pop;
        }
      protected:

        std::vector<boost::shared_ptr<Phen> > _pareto_front;

        pop_t _parent_pop;
        pop_t _child_pop;
        pop_t _mixed_pop;

        void _init_pop(pop_t& init_pop)
        {
          _eval_pop(init_pop);
          _apply_modifier(init_pop);
          front_t fronts;
          _rank_crowd(init_pop, fronts);
          _fill_nondominated_sort(init_pop, _parent_pop);
        }

        void _update_pareto_front(const front_t& fronts)
        {
          _convert_pop(fronts.front(), _pareto_front);
        }

        void _convert_pop(const pop_t& pop1,
                          std::vector<boost::shared_ptr<Phen> >& pop2)
        {
          pop2.resize(pop1.size());
          for (size_t i = 0; i < pop1.size(); ++i)
            pop2[i] = pop1[i];
        }

        void _eval_pop(pop_t& pop)
        {
          this->_eval.eval(pop, 0, pop.size());
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
          BOOST_FOREACH(indiv_t & ind, mixed_pop)
          for (size_t i = 0; i < ind->fit().objs().size(); ++i)
          {
            assert(!std::isnan(ind->fit().objs()[i]));
          }
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
            for (size_t k = 0; k < size; ++k)
            {
              assert(i < fronts.size());
              new_pop.push_back(fronts[i][k]);
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
        void _selection(pop_t& old_pop, pop_t& new_pop)
        {
          new_pop.resize(old_pop.size());
          std::vector<size_t> a1, a2;
          misc::rand_ind(a1, old_pop.size());
          misc::rand_ind(a2, old_pop.size());
           // todo : this loop could be parallelized
          for (size_t i = 0; i < old_pop.size(); i += 4)
          {
            const indiv_t& p1 = _tournament(old_pop[a1[i]], old_pop[a1[i + 1]]);
            const indiv_t& p2 =
              _tournament(old_pop[a1[i + 2]], old_pop[a1[i + 3]]);
            const indiv_t& p3 = _tournament(old_pop[a2[i]], old_pop[a2[i + 1]]);
            const indiv_t& p4 =
              _tournament(old_pop[a2[i + 2]], old_pop[a2[i + 3]]);
            assert(i + 3 < new_pop.size());
            p1->cross(p2, new_pop[i], new_pop[i + 1]);
            p3->cross(p4, new_pop[i + 2], new_pop[i + 3]);
          }
        }

        const indiv_t& _tournament(const indiv_t& i1, const indiv_t& i2)
        {
          pnsga_alt::prob_dom_f<indiv_t, Params> p;
          int flag = p.dominate_flag(i1, i2);
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

        void _rank_crowd(pop_t& pop, front_t& fronts)
        {
          std::vector<size_t> ranks;
#ifndef NDEBUG
          BOOST_FOREACH(indiv_t & ind, pop)
          for (size_t i = 0; i < ind->fit().objs().size(); ++i)
          {
            assert(!std::isnan(ind->fit().objs()[i]));
          }
#endif
          dom_sort_basic(pop, fronts, pnsga_alt::prob_dom_f<indiv_t, Params>(pop), ranks);
          _update_pareto_front(fronts);
          parallel::p_for(parallel::range_t(0, fronts.size()),
                          crowd::assign_crowd<indiv_t >(fronts));

          for (size_t i = 0; i < ranks.size(); ++i)
            pop[i]->set_rank(ranks[i]);
          parallel::sort(pop.begin(), pop.end(), crowd::compare_ranks());;
        }

        void _assign_rank(pop_t& pop)
        {
          int rank = 0;
          fit::compare_pareto comp;
          assert(pop.size());
          std::sort(pop.begin(), pop.end(), comp);
          pop[0]->set_rank(0);
          for (unsigned i = 1; i < pop.size(); ++i)
          {
            assert(comp(pop[i - 1], pop[i]) || comp.eq(pop[i - 1], pop[i]));
            if (comp(pop[i - 1], pop[i]))
              ++rank;
            pop[i]->set_rank(rank);
          }
        }

    };
  }
}
#endif // PNSGA_MULT_HPP_


