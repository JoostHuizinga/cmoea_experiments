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


#ifndef MAP_ELITE_HPP_
#define MAP_ELITE_HPP_

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <sferes/stc.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>


namespace sferes
{
namespace ea
{

// Main class
SFERES_EA(MapElite, Ea){
public:
    typedef boost::shared_ptr<Phen> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef typename pop_t::iterator it_t;
    typedef boost::shared_ptr<Phen> phen_t;
    typedef boost::array<float, Params::ea::objs> array_t;

    static const size_t objs = Params::ea::objs;

    MapElite()
    {
    }

    void random_pop()
    {
        //Create and evaluate the initial random population
        parallel::init();
        this->_pop.resize(Params::pop::init_size);
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop)
        {
            indiv = boost::shared_ptr<Phen>(new Phen());
            indiv->random();
        }
        this->_eval.eval(this->_pop, 0, this->_pop.size());

        //Initialize the archive with a single individual
        for(size_t i=0; i<objs; ++i){
                _array[i] = this->_pop[0];
        }

        //Add everyone to the correct spot in the archive
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop){
            _add_to_archive(indiv);
        }
    }

    void epoch()
    {
        //Optional, make sure the population reflects the archive
        this->_pop.clear();
        for (size_t i = 0; i < objs; ++i){
            this->_pop.push_back(_array[i]);
        }

        //We are creating and selecting a number of individuals equal to the population size
        //A simpler variant would only select and mutate one individual
        pop_t ptmp;
        for (size_t i = 0; i < Params::pop::size; ++i)
        {
            indiv_t p1 = _selection(_array);
            indiv_t p2 = _selection(_array);
            boost::shared_ptr<Phen> i1, i2;
            p1->cross(p2, i1, i2);
            i1->mutate();
            i2->mutate();
            i1->develop();
            i2->develop();
            ptmp.push_back(i1);
            ptmp.push_back(i2);
        }
        this->_eval.eval(ptmp, 0, ptmp.size());
        _apply_modifier(ptmp);

        //Add everyone to the archive
        for (size_t i = 0; i < ptmp.size(); ++i){
            _add_to_archive(ptmp[i]);
        }
    }

    void _convert_pop(const pop_t& pop1,std::vector<boost::shared_ptr<Phen> >& pop2)
    {
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i)
            pop2[i] = pop1[i];
    }

    void _apply_modifier(pop_t pop)
    {
        _convert_pop(pop, this->_pop);
        this->apply_modifier();
    }

    const array_t& archive() const { return _array; }

protected:
    array_t _array;

    void _add_to_archive(indiv_t i1)
    {
        for(size_t i=0; i<objs; ++i){
            //If the relevant spot in the array is empty
            if (_array[i]->fit().obj(i) <= i1->fit().obj(i)){
                _array[i] = i1;
            }
        }
    }

    indiv_t _selection(const pop_t& pop)
    {
        int x1 = misc::rand< int > (0, pop.size());
        return pop[x1];
    }


};
}
}
#endif


