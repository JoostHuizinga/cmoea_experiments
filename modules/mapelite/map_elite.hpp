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

/**
 * This is an implementation of map-elites without crossover.
 *
 * Crossover can obfuscate parent-child relationships, and give the impression of large jumps
 * that did not actually happen.
 */

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
#include <sferes/dbg/dbg.hpp>


namespace sferes
{
namespace ea
{

// Main class
SFERES_EA(MapElite, Ea)
{
public:
    typedef MapElite<Phen, Eval, Stat, FitModifier, Params, Exact> this_t;
    typedef Ea < Phen, Eval, Stat, FitModifier, Params, typename stc::FindExact<this_t, Exact>::ret > paren_t;

    //Inherited types
    typedef typename this_t::phen_t phen_t;
    typedef typename this_t::pop_t pop_t;
    typedef typename pop_t::iterator it_t;

    //New types
    typedef boost::shared_ptr<Phen> indiv_t;
    typedef std::vector<std::vector<indiv_t> > front_t;
    typedef boost::array<float, 2> point_t;
    typedef boost::multi_array<indiv_t, 2> array_t;

    //The x resolution of the map
    static const size_t res_x = Params::ea::res_x;

    //The y resolution of the map
    static const size_t res_y = Params::ea::res_y;

    //The number of individuals initially generated to fill the archive
    //If equal to the bin_size, every initially generated individual is added
    //to every bin of every category.
    //The init_size has to be greater than or equal to the bin_size
    static const size_t init_size = Params::pop::init_size;

    //Not actually the size of the standing population
    //(which is bin_size*nr_of_bins)
    //but the number of individuals that are generated each epoch.
    static const size_t pop_size = Params::pop::size;

    MapElite() :
        _array(boost::extents[res_x][res_y]),
        _array_parents(boost::extents[res_x][res_y])
    {
        dbg::trace trace("ea", DBG_HERE);
    }

    MapElite(const this_t& other) :
        paren_t(other),
        _array(other._array),
        _array_parents(other._array_parents)
    {
        dbg::trace trace("ea", DBG_HERE);
    }

    /**
     * Generate init_size individuals,
     * evaluate them and add them to the archive
     */
    void random_pop()
    {
        dbg::trace trace("ea", DBG_HERE);
        parallel::init();
        this->_pop.resize(init_size);
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop)
        {
            indiv = boost::shared_ptr<Phen>(new Phen());
            indiv->random();
            for(size_t i=0; i<Params::pop::initial_mutations; i++){
                indiv->mutate();
            }
        }
        this->_eval.eval(this->_pop, 0, this->_pop.size());
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop){
            _add_to_archive(indiv, indiv);
        }
    }

    /**
     * Execute one epoch of map-elite, generating pop_size new individuals,
     * and adding them to the archive if they are good enough.
     */
    void epoch()
    {
        dbg::trace trace("ea", DBG_HERE);
        //Add the entire archive to the population
        //Used for selection
        _archive_to_pop();


        //Generate pop_size individuals
        pop_t ptmp, p_parents;
        for (size_t i = 0; i < pop_size; ++i)
        {
            indiv_t p1 = _selection(this->_pop);
            boost::shared_ptr<Phen> i1 = boost::shared_ptr<Phen>(new Phen());
            i1->gen() = p1->gen();
            i1->mutate();
            ptmp.push_back(i1);
            p_parents.push_back(p1);
        }

        //Evaluate all new individuals
        this->_eval.eval(ptmp, 0, ptmp.size());

        //Apply the modifier to the new individuals
        //Note that this overwrites the current population (this->pop),
        //and that it only applies the modifier with respect the new individuals.
        //Diversity, for example, would only measure the diversity within the new individuals.
        _apply_modifier(ptmp);

        //Add all new individuals to the archive, if they are fit enough
        dbg::assertion(DBG_ASSERTION(ptmp.size() == p_parents.size()));
        for (size_t i = 0; i < ptmp.size(); ++i){
            _add_to_archive(ptmp[i], p_parents[i]);
        }

        //Add the entire archive to the population for statistics
        _archive_to_pop();
    }

    /**
     * Returns the current archive.
     *
     * @return A constant reference to the current archive.
     */
    const array_t& archive() const {dbg::trace trace("ea", DBG_HERE); return _array; }

    /**
     * Returns an archive of parents, useful for keeping track of how individuals navigate the archive.
     *
     * If an individual is at location [x][y] in the archive, its parent will be at location [x][y] in the parent archive.
     *
     * @return A constant reference to an archive.
     */
    const array_t& parents() const {dbg::trace trace("ea", DBG_HERE); return _array_parents; }

    /**
     * Takes an individual, and returns the location it should have in the archive,
     * according to its behavioral descriptors desc()[0] and desc()[1].
     *
     * desc()[0] and desc()[1] should return number between 0.0 and 1.0.
     *
     * @param indiv The individual for which the point should be determined.
     * @return      A point (boost::array<float, 2>) indicating the location of the individual in the archive.
     */
    template<typename I>
    const point_t get_point (const I& indiv) const {dbg::trace trace("ea", DBG_HERE); return _get_point(indiv);}

protected:
    array_t _array;
    array_t _array_parents;

    void _convert_pop(const pop_t& pop1,std::vector<boost::shared_ptr<Phen> >& pop2)
    {
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i)
            pop2[i] = pop1[i];
    }

    void _apply_modifier(pop_t& pop)
    {
        dbg::trace trace("ea", DBG_HERE);
        if(boost::fusion::size(this->fit_modifier()) > 0){
            _convert_pop(pop, this->_pop);
            this->apply_modifier();
        }
    }


    bool _add_to_archive(indiv_t i1, indiv_t parent)
    {
        dbg::trace trace("ea", DBG_HERE);
        point_t p = _get_point(i1);
        size_t x = round(p[0] * res_x);
        size_t y = round(p[1] * res_y);
        x = std::min(x, res_x - 1);
        y = std::min(y, res_y - 1);
        dbg::assertion(DBG_ASSERTION(x < res_x));
        dbg::assertion(DBG_ASSERTION(y < res_y));

        dbg::out(dbg::info, "ea") << "Candidate for (" << x << "," << y << ") fitness: " << i1->fit().value() << std::endl;

        if (!_array[x][y] || i1->fit().compare(_array[x][y]))
        {
            _array[x][y] = i1;
            _array_parents[x][y] = parent;
            return true;
        }
        return false;
    }

    /**
     * Takes an individual, and returns the location it should have in the archive,
     * according to its behavioral descriptors desc()[0] and desc()[1].
     *
     * desc()[0] and desc()[1] should return number between 0.0 and 1.0.
     *
     * @param indiv The individual for which the point should be determined.
     * @return      A point (boost::array<float, 2>) indicating the location of the individual in the archive.
     */
    template<typename I>
    point_t _get_point(const I& indiv) const
    {
        dbg::trace trace("ea", DBG_HERE);
        point_t p;
        p[0] =  std::min(1.0f, indiv->fit().desc()[0]);
        p[1] =  std::min(1.0f, indiv->fit().desc()[1]);

        return p;
    }

    /**
     * Randomly selects an individual from the supplied population.
     *
     * @param pop A population, usually a vector of boost::shared_ptr pointing to individuals.
     * @return    An individual form the population, usually a boost::shared_ptr pointing to an individual.
     */
    indiv_t _selection(const pop_t& pop) const
    {
        dbg::trace trace("ea", DBG_HERE);
        int x1 = misc::rand< int > (0, pop.size());
        return pop[x1];
    }


    void _archive_to_pop(){
        this->_pop.clear();
        for (size_t i = 0; i < res_x; ++i){
            for (size_t j = 0; j < res_y; ++j){
                if (_array[i][j]){
                    this->_pop.push_back(_array[i][j]);
                }
            }
        }
    }
};
}
}
#endif


