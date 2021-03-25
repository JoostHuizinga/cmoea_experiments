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


#ifndef MODULES_MAPELITE_MAP_ELITE_INNOVATION_DIVERSITY_HPP_
#define MODULES_MAPELITE_MAP_ELITE_INNOVATION_DIVERSITY_HPP_

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/spirit/include/karma.hpp>

namespace karma = boost::spirit::karma;

#include <sferes/stc.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/dbg/dbg.hpp>

namespace sferes
{
namespace ea
{

// Main class
SFERES_EA(MapEliteInovDiv, Ea){
public:
    //The number of different bins, niches, categories or objectives present for the innovation engine
    static const size_t nr_of_bins = Params::innov::categories;

    //The number of individuals that can fit in each bin
    static const size_t bin_size = Params::innov::bin_size;

    //The number of individuals that are randomly generated at the start of a run
    static const size_t init_size = Params::pop::init_size;

    //The number of individuals that are generated each generation (must be divisible by two)
    static const size_t batch_size = Params::pop::size;

    typedef Phen phen_t;
    typedef boost::shared_ptr<phen_t> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef boost::multi_array<indiv_t, 2> array_t;
    typedef boost::multi_array<float, 3> distances_t;
    typedef boost::array<float, bin_size> distance_array_t;

    MapEliteInovDiv():
        _array(boost::extents[nr_of_bins][bin_size]),
        _distances(boost::extents[nr_of_bins][bin_size][bin_size])
    {
        dbg::trace trace("ea", DBG_HERE);
        dbg::out(dbg::info, "ea") << "Objectives: " << nr_of_bins << " clusters: " << bin_size << std::endl;
    }

    void random_pop()
    {
        dbg::trace trace("ea", DBG_HERE);
        //Create and evaluate the initial random population
        parallel::init();
        this->_pop.resize(init_size);
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop)
        {
            indiv = boost::shared_ptr<Phen>(new Phen());
            indiv->random();
        }
        this->_eval.eval(this->_pop, 0, this->_pop.size());

        //Initialize the archive with a single individual
        for(size_t i=0; i<nr_of_bins; ++i){
            this->_pop[0]->fit().setClosestDist(i, 0.f);
            for(size_t j=0; j<bin_size; ++j){
                _array[i][j] = this->_pop[0];
                for(size_t k=0; k<bin_size; ++k){
                    _distances[i][j][k] = 0.0;
                }
            }
        }

        //Add everyone to the correct spots in the archive
        size_t i = 0;
        BOOST_FOREACH(boost::shared_ptr<Phen>&indiv, this->_pop){
            dbg::out(dbg::info, "ea") << "****** Adding initial individual " << i << " ******" <<std::endl;
            _add_to_archive(indiv);
            dbg::out(dbg::info, "ea") << "****** Adding initial individual " << i << " done ******" <<std::endl;
            ++i;
        }

        DBG_CONDITIONAL(dbg::info, "archive", _init_debug_array());
    }

    void epoch()
    {
        dbg::trace trace("ea", DBG_HERE);
        //Optional, make sure the population reflects the archive
        this->_pop.clear();
        for (size_t i = 0; i < nr_of_bins; ++i){
            for(size_t j = 0; j < bin_size; ++j){
                this->_pop.push_back(_array[i][j]);
            }
        }

        //We are creating and selecting a number of individuals equal to the population size
        //A simpler variant would only select and mutate one individual
        pop_t ptmp;
        for (size_t i = 0; i < (batch_size/2); ++i)
        {
            dbg::out(dbg::info, "ea") << "Creating individual: " << i <<std::endl;
            indiv_t p1 = _selection(this->_pop);
            indiv_t p2 = _selection(this->_pop);
            indiv_t i1, i2;
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
            dbg::out(dbg::info, "ea") << "****** Adding individual " << i << " ******" <<std::endl;
            _add_to_archive(ptmp[i]);
            dbg::out(dbg::info, "ea") << "****** Adding individual " << i << " done ******" <<std::endl;
        }

        //For writing statistics only from the first bin:
        _set_first_bin_to_current_pop();

        DBG_CONDITIONAL(dbg::info, "archive", _print_archive());
    }

    const array_t& archive() const { return _array; }

protected:
    array_t _array;
    distances_t _distances;
    array_t _debug_array;

    void _convert_pop(const pop_t& pop1,std::vector<boost::shared_ptr<Phen> >& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i)
            pop2[i] = pop1[i];
    }

    void _apply_modifier(pop_t pop){
        dbg::trace trace("ea", DBG_HERE);
        _convert_pop(pop, this->_pop);
        this->apply_modifier();
    }

    void _set_first_bin_to_current_pop(){
        dbg::trace trace("ea", DBG_HERE);
        this->_pop.clear();
        for(size_t j = 0; j < bin_size; ++j){
            this->_pop.push_back(_array[0][j]);
        }
    }

    void _add_to_archive(indiv_t i1){
        dbg::trace trace("ea", DBG_HERE);
        dbg::assertion(DBG_ASSERTION(i1->fit().getInnovCategories().size() == nr_of_bins));

        for(size_t bin=0; bin<nr_of_bins; ++bin){
            dbg::out(dbg::info, "ea") << "*** On objective " << bin << " ***" <<std::endl;
            //Find the shortest distance to the new individual
            distance_array_t distance_array;
            size_t shortest_dist_new_indiv_index = 0;
            distance_array[shortest_dist_new_indiv_index] = _array[bin][shortest_dist_new_indiv_index]->fit().dist(*i1);
            for(size_t j=1; j<bin_size; ++j){
                distance_array[j] = _array[bin][j]->fit().dist(*i1);
                if(distance_array[shortest_dist_new_indiv_index] > distance_array[j]){
                    shortest_dist_new_indiv_index = j;
                }
            }

            //Find the shortest distance in the current distance array
            size_t first_dist_index = 0;
            size_t second_dist_index = 1;
            for(size_t j=0; j<bin_size; ++j){
                for(size_t k=j+1; k<bin_size; ++k){
                    if(_distances[bin][first_dist_index][second_dist_index] > _distances[bin][j][k]){
                        first_dist_index = j;
                        second_dist_index = k;
                    }
                }
            }


            dbg::out(dbg::info, "ea") << "New indiv " << i1
                    << " fitness " << i1->fit().cat(bin)
                    << " closest to old indiv " << _array[bin][shortest_dist_new_indiv_index]
                    << " at index " << shortest_dist_new_indiv_index
                    << " fitness " << _array[bin][shortest_dist_new_indiv_index]->fit().cat(bin)
                    << " dist " << distance_array[shortest_dist_new_indiv_index] << std::endl;

            dbg::out(dbg::info, "ea") << "Old indiv " << _array[bin][first_dist_index]
                    << " fitness " << _array[bin][first_dist_index]->fit().cat(bin)
                    << " at index " << first_dist_index
                    << " closest to old indiv " << _array[bin][second_dist_index]
                    << " at index " << second_dist_index
                    << " fitness " << _array[bin][second_dist_index]->fit().cat(bin)
                    << " dist " << _distances[bin][first_dist_index][second_dist_index] << std::endl;

            //Test for replacement
            if(distance_array[shortest_dist_new_indiv_index] < _distances[bin][first_dist_index][second_dist_index]){
                //Compete i1 against _array[i][new_dist_index]
                if(_array[bin][shortest_dist_new_indiv_index]->fit().cat(bin) < i1->fit().cat(bin)){
                    _replace(bin, i1, distance_array, shortest_dist_new_indiv_index);
                }
            } else {
                //Compete _array[i][first_dist_index] against _array[i][second_dist_index]
                if(_array[bin][first_dist_index]->fit().cat(bin) < _array[bin][second_dist_index]->fit().cat(bin)){
                    _replace(bin, i1, distance_array, first_dist_index);
                } else{
                    _replace(bin, i1, distance_array, second_dist_index);
                }

            }
        }
    }

    void _replace(size_t bin, indiv_t i1, distance_array_t &distance_array, size_t index_to_replace){
        dbg::trace trace("ea", DBG_HERE);
        dbg::out(dbg::info, "ea") << "Obj: " << bin
                << " fit new: " << i1->fit().cat(bin)
                << " replaced fit old: " << _array[bin][index_to_replace]->fit().cat(bin)
                << " on index " << index_to_replace << std::endl;
        _array[bin][index_to_replace] = i1;

        //Copy the distance array to the distance matrix
        for(size_t i=0; i<index_to_replace; ++i){
            _distances[bin][i][index_to_replace] = distance_array[i];
        }
        for(size_t i=index_to_replace+1; i<bin_size; ++i){
            _distances[bin][index_to_replace][i] = distance_array[i];
        }

        DBG_CONDITIONAL(dbg::info, "archive", _recalcute_distances(bin));
    }


    indiv_t _selection(const pop_t& pop){
        dbg::trace trace("ea", DBG_HERE);
        size_t x1 = misc::randInt<size_t>(pop.size());
        dbg::assertion(DBG_ASSERTION(x1 < pop.size()));
        return pop[x1];
    }

    //Debug functions

    /**
     * Recalculate the distances that will be printed in the debug array.
     *
     * Note, requires an add closest distance
     */
    void _recalcute_distances(size_t objective_index){
        //Recalculate closest distances (for debugging only)
        for(size_t i=0; i<bin_size; ++i){
            _array[objective_index][i]->fit().setClosestDist(objective_index, std::numeric_limits<float>::max());
        }

        for(size_t i=0; i<bin_size; ++i){
            for(size_t j=i+1; j<bin_size; ++j){
                if(_distances[objective_index][i][j] < _array[objective_index][i]->fit().getClosestDist(objective_index)){
                    _array[objective_index][i]->fit().setClosestDist(objective_index, _distances[objective_index][i][j]);
                }
                if(_distances[objective_index][i][j] < _array[objective_index][j]->fit().getClosestDist(objective_index)){
                    _array[objective_index][j]->fit().setClosestDist(objective_index, _distances[objective_index][i][j]);
                }
            }
        }
    }

    /**
     * Prints the fitness and closest distance values for each position in the archive.
     *
     * Note: requires the _debug_array to be set, otherwise it will throw a segmentation fault
     */
    void _print_archive(){
        std::cout << "Old distance matrix:" << std::endl;
        for(size_t i=0; i<nr_of_bins; ++i){
            for(size_t j=0; j<bin_size; ++j){
                bool better_fit = _debug_array[i][j]->fit().cat(i) < _array[i][j]->fit().cat(i);
                bool better_dist = _debug_array[i][j]->fit().getClosestDist(i) < _array[i][j]->fit().getClosestDist(i);
                std::cout << "(";
                if(_debug_array[i][j] != _array[i][j] && !better_fit && !better_dist) std::cout << COL_MAGENTA;
                if(better_fit) std::cout << COL_BLUE;
                std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], _debug_array[i][j]->fit().cat(i));
                std::cout << END_COLOR << ":";
                if(_debug_array[i][j] != _array[i][j] && !better_fit && !better_dist) std::cout << COL_MAGENTA;
                if(better_dist) std::cout << COL_GREEN;
                std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], _debug_array[i][j]->fit().getClosestDist(i));
                std::cout << END_COLOR << ") ";
            }
            std::cout << std::endl;
        }

        std::cout << "Distance matrix:" << std::endl;
        for(size_t i=0; i<nr_of_bins; ++i){
            for(size_t j=0; j<bin_size; ++j){
                bool better_fit = _debug_array[i][j]->fit().cat(i) < _array[i][j]->fit().cat(i);
                bool better_dist = _debug_array[i][j]->fit().getClosestDist(i) < _array[i][j]->fit().getClosestDist(i);

                std::cout << "(";
                if(_debug_array[i][j] != _array[i][j] && !better_fit && !better_dist) std::cout << COL_MAGENTA;
                if(better_fit) std::cout << COL_BLUE;
                std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], _array[i][j]->fit().cat(i));
                std::cout << ":" << END_COLOR;
                if(_debug_array[i][j] != _array[i][j] && !better_fit && !better_dist) std::cout << COL_MAGENTA;
                if(better_dist) std::cout << COL_GREEN;
                std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], _array[i][j]->fit().getClosestDist(i));
                std::cout << END_COLOR << ") ";
                _debug_array[i][j] = _array[i][j];
            }
            std::cout << std::endl;
        }
    }

    /**
     * Sets the debug array.
     *
     * Required for debugging at the end of the random pop and load population functions.
     *
     * TODO: Make copies rather than copying pointers so the state of each individual during the
     * last generation is properly preserved.
     */
    void _init_debug_array(){
        boost::array<size_t,2> extents; //size of each dimension
        extents[0] = nr_of_bins;
        extents[1] = bin_size;
        _debug_array.resize(extents);
        for(size_t i=0; i<nr_of_bins; ++i){
            for(size_t j=0; j<bin_size; ++j){
               _debug_array[i][j] = _array[i][j];
            }
        }
    }
};
}
}
#endif //MODULES_MAPELITE_MAP_ELITE_INNOVATION_DIVERSITY_HPP_


