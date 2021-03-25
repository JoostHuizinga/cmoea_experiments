/*
 * dom_sort_fast.hpp
 *
 *  Created on: Apr 30, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_NSGAEXT_DOM_SORT_FAST_HPP_
#define MODULES_NSGAEXT_DOM_SORT_FAST_HPP_

//Boost includes
#include <boost/foreach.hpp>

//Sferes includes
#include <sferes/stc.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/eval/parallel.hpp>

#ifdef TBB
#include <tbb/atomic.h>
#endif



namespace sferes {
namespace ea {
namespace _dom_sort {

static const int a_dom_b = 1;
static const int b_dom_a = -1;
static const int non_dom = 0;

////// Probabilistic domination sort
//SFERES_CLASS(prob_dom_f){
//public:
//    template<typename Indiv>
//    inline bool operator() (const Indiv &ind, const std::vector<Indiv>&pop) const
//    {
//        BOOST_FOREACH(Indiv i, pop){
//            if (dominate_flag(i, ind) == 1)
//                return false;
//        }
//        return true;
//    }
//
//    template<typename I1, typename I2>
//    inline int dominate_flag(const I1& i1, const I2& i2) const
//    {
//        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size()));
//        dbg::assertion(DBG_ASSERTION(i2->fit().objs().size()));
//        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size() == i2->fit().objs().size()));
//
//        size_t nb_objs = i1->fit().objs().size();
//
//        bool flag1 = false, flag2 = false;
//        for (size_t i = 0; i < nb_objs; ++i)
//        {
//            dbg::assertion(DBG_ASSERTION(i < Params::obj_pressure_size()));
//            float pressure = Params::obj_pressure(i);;
//
//            if (misc::rand<float>() > pressure)
//                continue;
//            float fi1 = i1->fit().obj(i);
//            float fi2 = i2->fit().obj(i);
//            if (fi1 > fi2) {
//                flag1 = true;
//            } else if (fi2 > fi1){
//                flag2 = true;
//            }
//        }
//
//        if (flag1 && !flag2){
//            return a_dom_b;
//        } else if (!flag1 && flag2){
//            return b_dom_a;
//        } else {
//            return non_dom;
//        }
//    }
//};



template<typename Phen, typename DominateComparator>
struct count_dom_custom_comparator
{

#ifdef TBB
    typedef std::vector<tbb::atomic<size_t> > dom_by_t;

#else
    typedef std::vector<size_t> dom_by_t;
#endif
    const std::vector<Phen>& pop;
    dom_by_t& dominatedBy;
    std::vector<std::vector<size_t> >& dominating;
    const DominateComparator& dominateComparator;

    count_dom_custom_comparator(const std::vector<Phen>& pop_,
            dom_by_t& n_,
            std::vector<std::vector<size_t> >& s_,
            const DominateComparator& dominateComparator_) :
                pop(pop_), dominatedBy(n_), dominating(s_), dominateComparator(dominateComparator_)
    {}

    void operator() (const parallel::range_t& range) const{
        dbg::assertion(DBG_ASSERTION(dominatedBy.size() == pop.size()));
        dbg::assertion(DBG_ASSERTION(dominating.size() == pop.size()));
        for (size_t indiv_1_index = range.begin(); indiv_1_index != range.end(); ++indiv_1_index){
//            dbg::out(dbg::info, "dom_sort_op") << "Checking domination for individual: " << indiv_1_index << std::endl;
            dbg::assertion(DBG_ASSERTION(indiv_1_index < dominating.size()));
            dbg::assertion(DBG_ASSERTION(indiv_1_index < dominatedBy.size()));
            dbg::assertion(DBG_ASSERTION(indiv_1_index < pop.size()));
            dbg::assertion(DBG_ASSERTION(dominating[indiv_1_index].empty()));
//            dominatedBy[indiv_1_index] = 0;
            for (size_t indiv_2_index = 0; indiv_2_index < pop.size(); ++indiv_2_index)
            {
                dbg::assertion(DBG_ASSERTION(indiv_2_index < dominatedBy.size()));
                dbg::assertion(DBG_ASSERTION(indiv_2_index < dominating.size()));
                dbg::assertion(DBG_ASSERTION(indiv_2_index < pop.size()));

                int result = dominateComparator.dominate_flag(pop[indiv_1_index], pop[indiv_2_index]);
                if (result == a_dom_b){
#ifdef TBB
                    dominatedBy[indiv_2_index].fetch_and_add(1);
#else
                    ++dominatedBy[indiv_2_index];
#endif
                    dominating[indiv_1_index].push_back(indiv_2_index);
//                    dbg::out(dbg::info, "dom_sort_op") << "Individual: " << indiv_1_index << " dominates individual: " << indiv_2_index << std::endl;
                } else if (result == b_dom_a){
//                    ++dominatedBy[indiv_1_index];
//                    dbg::out(dbg::info, "dom_sort") << "Individual: " << indiv_1_index << " is now dominated by: " << dominatedBy[indiv_1_index] << " individuals." << std::endl;
//                    dominating[indiv_2_index].push_back(indiv_1_index);
                }
            }
        }
    }
};


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
template<typename Indiv, typename DominateComparator>
inline void sort_deb(const std::vector<Indiv>& pop,
        std::vector<std::vector<Indiv> >& fronts,
        const DominateComparator& dominateComparator,
        std::vector<size_t>& ranks)
{
    dbg::assertion(DBG_ASSERTION(!pop.empty()));
    std::vector<std::vector<size_t> > s(pop.size());
    std::vector<std::vector<size_t> > f(1);
    std::vector<size_t> n(pop.size());
    ranks.resize(pop.size());

    std::fill(ranks.begin(), ranks.end(), pop.size());

#ifndef NDEBUG
    BOOST_FOREACH(const Indiv& ind, pop)
    for (size_t i = 0; i < ind->fit().objs().size(); ++i)
    { dbg::assertion(DBG_ASSERTION(!std::isnan(ind->fit().objs()[i]))); }
#endif

    parallel::p_for(parallel::range_t(0, pop.size()),
            _dom_sort::count_dom_custom_comparator<Indiv, DominateComparator>(pop, n, s, ranks, dominateComparator));

    for (size_t i = 0; i < pop.size(); ++i)
        if (ranks[i] == 0)
            f[0].push_back(i);

#ifndef NDEBUG
    BOOST_FOREACH(size_t k, n)
    { dbg::assertion(DBG_ASSERTION(k < pop.size())); }
#endif
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
                    ranks[q] = i + 1;
                    f.back().push_back(q);
                }
            }
        }
        ++i;
        dbg::assertion(DBG_ASSERTION(i < f.size()));
    }

#ifndef NDEBUG
    size_t size = 0;
    BOOST_FOREACH(std::vector<size_t>& v, f)
    size += v.size();
    assert(size == pop.size());
#endif

    // copy indivs to the res
    fronts.clear();
    fronts.resize(f.size());
    for (unsigned i = 0; i < f.size(); ++i)
        for (unsigned j = 0; j < f[i].size(); ++j)
            fronts[i].push_back(pop[f[i][j]]);

    assert(fronts.back().size() == 0);
    fronts.pop_back();
    dbg::assertion(DBG_ASSERTION(!fronts.empty()));
    dbg::assertion(DBG_ASSERTION(!fronts[0].empty()));
}

template<typename Indiv, typename ND>
inline void dom_sort(const std::vector<Indiv>& pop,
        std::vector<std::vector<Indiv> >& fronts,
        const ND& nd,
        std::vector<size_t>& ranks)
{
        _dom_sort::sort_deb(pop, fronts, ranks);
}

} //_dom_sort

struct dom_sort_fast_f{
    template<typename Indiv, typename ND>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            const ND& nd,
            std::vector<size_t>& ranks) const
    {
        _dom_sort::dom_sort(pop, fronts, nd, ranks);
    }

    template<typename Indiv>
    inline void operator() (const std::vector<Indiv>& pop,
            std::vector<std::vector<Indiv> >& fronts,
            std::vector<size_t>& ranks) const
    {
        _dom_sort::dom_sort(pop, fronts, _dom_sort_basic::non_dominated_f(), ranks);
    }
};

} //ea
} //sferes


#endif /* MODULES_NSGAEXT_DOM_SORT_FAST_HPP_ */
