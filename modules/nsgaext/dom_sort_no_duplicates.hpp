/*
 * dom_sort_no_duplicates.hpp
 *
 *  Created on: Aug 18, 2014
 *      Author: joost
 */

#ifndef MODULES_NSGAEXT_NO_DUPLICATES_HPP_
#define MODULES_NSGAEXT_NO_DUPLICATES_HPP_

#include <set>
#include <map>

#include <boost/foreach.hpp>

#include <sferes/ea/dom_sort_basic.hpp>
#include <sferes/ea/dom_sort.hpp>
#include <sferes/dbg/dbg.hpp>

#include "dom_sort_fast.hpp"

namespace sferes
{
namespace ea
{

template<typename Indiv, typename ND>
inline void dom_sort_no_duplicates(const std::vector<Indiv>& pop,
        std::vector<std::vector<Indiv> >& fronts,
        const ND& nd,
        std::vector<size_t>& ranks)
{
    dbg::trace trace("dom-sort", DBG_HERE);
    //p[i] is set to i for all i in range(pop.size())
    std::vector<size_t> p(pop.size());
    for (size_t i = 0; i < p.size(); ++i)
        p[i] = i;

    ranks.resize(pop.size());
    int rank = 0;
    while (!p.empty())
    {
        //Create a temporay population
        std::vector<size_t> non_dominated;
        std::vector<Indiv> non_dominated_ind;
        std::set<std::vector<float> > unique_points;
        std::vector<Indiv> tmp_pop;
        for (size_t i = 0; i < p.size(); ++i){
            tmp_pop.push_back(pop[p[i]]);
        }
        assert(tmp_pop.size());

        //Find all individuals in the population who are not dominated in the temp population
        //Those individuals will get the current rank and they are added to non-dominated
        for (size_t i = 0; i < p.size(); ++i){
            if (nd(pop[p[i]], tmp_pop))
            {
                if(unique_points.insert(pop[p[i]]->fit().objs()).second){
                    non_dominated.push_back(p[i]);
                    ranks[p[i]] = rank;
                    non_dominated_ind.push_back(pop[p[i]]);
                }
            }
        }
        assert(non_dominated.size());

        //Put all individuals that were dominated into the np vector
        //And set this np vector as the new p vector
        std::vector<size_t> np;
        std::set_difference(p.begin(), p.end(),
                non_dominated.begin(), non_dominated.end(),
                std::back_insert_iterator<std::vector<size_t> >(np));
        assert(np.size() < p.size());
        p.swap(np);

        //Add all non-dominated individuals to the current front and fill the next rank
        fronts.push_back(non_dominated_ind);
        ++rank;
    }
}

template<typename Indiv>
inline void dom_sort_no_duplicates(const std::vector<Indiv>& pop,
        std::vector<std::vector<Indiv> >& fronts,
        std::vector<size_t>& ranks)
{
    dom_sort_no_duplicates(pop, fronts, _dom_sort_basic::non_dominated_f(), ranks);
}


namespace deb{
typedef std::vector<std::vector<size_t> > r_front_t;
}

//Find the rank of each individual according to the `max 1 unique point per layer' rule
template<typename Indiv>
inline size_t assign_ranks(const std::vector<Indiv>& pop, deb::r_front_t& raw_fronts, std::vector<size_t>& ranks){
    std::map<std::vector<float>, unsigned> unique_points;
    size_t rank;
    size_t proposed_rank;
    size_t max_rank = 0;
    size_t pop_index;

    for (unsigned i = 0; i < raw_fronts.size(); ++i){
        for (unsigned j = 0; j < raw_fronts[i].size(); ++j){
            pop_index = raw_fronts[i][j];
            if(unique_points.count(pop[pop_index]->fit().objs()) == 0){
                rank = (unique_points[pop[pop_index]->fit().objs()] = i);
            } else{
                rank = (unique_points[pop[pop_index]->fit().objs()] += 1);
                if(i > rank){
                    rank = (unique_points[pop[pop_index]->fit().objs()] = i);
                }
            }
            ranks[pop_index] = rank;
            if(rank > max_rank) max_rank = rank;
        }
    }
    return max_rank;
}

template<typename SizeType>
inline deb::r_front_t make_layers(deb::r_front_t& s, std::vector<SizeType>& n){
    deb::r_front_t f(1);
    for (size_t i = 0; i < n.size(); ++i){
        if (n[i] == 0){
            f[0].push_back(i);
        }
    }
    dbg::assertion(DBG_ASSERTION(!f[0].empty()));

    // second step : make layers
    size_t i = 0;
    while (!f[i].empty()){
        f.push_back(std::vector<size_t>());
        for (size_t pp = 0; pp < f[i].size(); ++pp){
            size_t p = f[i][pp];
            for (size_t k = 0; k < s[p].size(); ++k){
                size_t q = s[p][k];
                dbg::assertion(DBG_ASSERTION(q != p));
                dbg::assertion(DBG_ASSERTION(n[q] != 0));
                --n[q];
                if (n[q] == 0){
                    f.back().push_back(q);
                }
            }
        }
        ++i;
        dbg::assertion(DBG_ASSERTION(i < f.size()));
    }

    return f;
}

// cf deb's paper on NSGA-2 :
/// @article{deb2002nsga,
// title={{NSGA-II}},
//   author={Deb, K. and Pratap, A. and Agarwal, S. and Meyarivan, T. and Fast, A. and Algorithm, E.M.G.},
//  journal={IEEE transactions on evolutionary computation},
//   volume={6},
//  number={2},
//  year={2002}
// }
// this algorithm is in O(n^2)
template<typename Indiv, typename DC>
inline void dom_sort_no_duplicates_deb(const std::vector<Indiv>& pop,
        std::vector<std::vector<Indiv> >& fronts,
        const DC& dc,
        std::vector<size_t>& ranks)
{
    dbg::trace trace("dom_sort_trace" , DBG_HERE);
    typedef typename _dom_sort::count_dom_custom_comparator<Indiv, DC>::dom_by_t dom_by_t;
    //Ascertain that the population is good
    dbg::assertion(DBG_ASSERTION(!pop.empty()));
#ifndef NDEBUG
    BOOST_FOREACH(const Indiv& ind, pop)
    for (size_t i = 0; i < ind->fit().objs().size(); ++i)
    { dbg::assertion(DBG_ASSERTION(!std::isnan(ind->fit().objs()[i]))); }
#endif

    //Determine the number of individuals that each individual is dominated by.
    dbg::out(dbg::info, "dom_sort") << "Counting dominated..." << std::endl;
    deb::r_front_t s(pop.size());
    dom_by_t n(pop.size());
    std::fill(n.begin(), n.end(), 0);
    ranks.resize(pop.size());
    std::fill(ranks.begin(), ranks.end(), pop.size());
    parallel::p_for(parallel::range_t(0, pop.size()), _dom_sort::count_dom_custom_comparator<Indiv, DC>(pop, n, s, dc));
    dbg::out(dbg::info, "dom_sort") << "Counting dominated... done" << std::endl;

    //Ascertain that no individual is dominated by more than than the population size of individuals.
#ifndef NDEBUG
    BOOST_FOREACH(size_t k, n){ dbg::assertion(DBG_ASSERTION(k < pop.size())); }
#endif

    //Make layers
    dbg::out(dbg::info, "dom_sort") << "Making layers..." << std::endl;
    deb::r_front_t f = make_layers(s, n);
    dbg::out(dbg::info, "dom_sort") << "Making layers... done" << std::endl;

    //Ascertain that every individual in the population has been assigned a rank.
#ifndef NDEBUG
    size_t size = 0;
    BOOST_FOREACH(std::vector<size_t>& v, f)
    size += v.size();
    assert(size == pop.size());
#endif

    //Assign ranks
    dbg::out(dbg::info, "dom_sort") << "Assigning ranks..." << std::endl;
    size_t max_rank = assign_ranks(pop, f, ranks);
    dbg::out(dbg::info, "dom_sort") << "Assigning ranks... done" << std::endl;

    //Fill the fronts with individuals
    dbg::out(dbg::info, "dom_sort") << "Filling fronts..." << std::endl;
    fronts.clear();
    dbg::out(dbg::info, "dom_sort") << "Resizing fronts to: " << max_rank + 1 << std::endl;
    fronts.resize(max_rank + 1);
    dbg::out(dbg::info, "dom_sort") << "Pushing back individuals" << std::endl;
    for (size_t i = 0; i < pop.size(); ++i){
        dbg::assertion(DBG_ASSERTION(i < ranks.size()));
        dbg::assertion(DBG_ASSERTION(i < pop.size()));
        dbg::assertion(DBG_ASSERTION(ranks[i] < fronts.size()));
        fronts[ranks[i]].push_back(pop[i]);
    }
    dbg::out(dbg::info, "dom_sort") << "Filling fronts... done" << std::endl;

    dbg::assertion(DBG_ASSERTION(!fronts.empty()));
    dbg::assertion(DBG_ASSERTION(!fronts[0].empty()));
}



struct dom_sort_basic_f{
    template<typename Indiv, typename ND>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            const ND& nd,
            std::vector<size_t>& ranks) const
    {
        dom_sort_basic(pop, fronts, nd, ranks);
    }

    template<typename Indiv>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            std::vector<size_t>& ranks) const
    {
        dom_sort_basic(pop, fronts, _dom_sort_basic::non_dominated_f(), ranks);
    }
};


struct dom_sort_no_duplicates_f{
    template<typename Indiv, typename ND>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            const ND& nd,
            std::vector<size_t>& ranks) const
    {
        dom_sort_no_duplicates(pop, fronts, nd, ranks);
    }

    template<typename Indiv>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            std::vector<size_t>& ranks) const
    {
        dom_sort_no_duplicates(pop, fronts, _dom_sort_basic::non_dominated_f(), ranks);
    }
};


struct dom_sort_no_duplicates_fast_f{
    template<typename Indiv, typename ND>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            const ND& nd,
            std::vector<size_t>& ranks) const
    {
        dom_sort_no_duplicates_deb(pop, fronts, nd, ranks);
    }

    template<typename Indiv>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            std::vector<size_t>& ranks) const
    {
        dom_sort_no_duplicates_deb(pop, fronts, _dom_sort_basic::non_dominated_f(), ranks);
    }
};
}
}


#endif /* DOM_SORT_NO_DUPLICATES_HPP_ */
