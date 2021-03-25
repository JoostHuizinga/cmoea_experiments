/*
 * map_elite_innovation_nsga2_mpi.hpp
 *
 *  Created on: Mar 12, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MAPELITE_MAP_ELITE_INNOVATION_NSGA2_HPP_
#define MODULES_MAPELITE_MAP_ELITE_INNOVATION_NSGA2_HPP_

#error "map_elite_innovation_nsga2_mpi.hpp is deprecated"

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/detail/point_to_point.hpp>


namespace karma = boost::spirit::karma;

#include <sferes/stc.hpp>
#include <sferes/ea/ea.hpp>
#include <sferes/fit/fitness.hpp>
#include <sferes/dbg/dbg.hpp>

#include <sferes/ea/dom_sort_basic.hpp>
#include <sferes/ea/common.hpp>
#include <sferes/ea/crowd.hpp>
#include <exp/modularity/common_compare.hpp>

namespace sferes
{
namespace ea
{
namespace innov_pnsga
{

//// Probabilistic domination sort
SFERES_CLASS(prob_dom_f){
public:
    template<typename Indiv>
    inline bool operator() (const Indiv &ind, const std::vector<Indiv>&pop) const
    {
        BOOST_FOREACH(Indiv i, pop){
            if (dominate_flag(i, ind) == 1)
                return false;
        }
        return true;
    }

    template<typename I1, typename I2>
    inline int dominate_flag(const I1& i1, const I2& i2) const
    {
        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size()));
        dbg::assertion(DBG_ASSERTION(i2->fit().objs().size()));
        dbg::assertion(DBG_ASSERTION(i1->fit().objs().size() == i2->fit().objs().size()));

        size_t nb_objs = i1->fit().objs().size();

        dbg::out(dbg::info, "prob_dom") << "Objectives: " << nb_objs << std::endl;
        bool flag1 = false, flag2 = false;
        for (size_t i = 0; i < nb_objs; ++i)
        {
            dbg::assertion(DBG_ASSERTION(i < Params::obj_pressure_size()));
            float pressure = Params::obj_pressure(i);;

            dbg::out(dbg::info, "prob_dom") << "Objective: " << i << " pressure: " << pressure << " i1 value: " << i1->fit().obj(i) << " i2 value: " << i2->fit().obj(i) << std::endl;
            if (misc::rand<float>() > pressure)
                continue;
            float fi1 = i1->fit().obj(i);
            float fi2 = i2->fit().obj(i);
            if (fi1 > fi2) {
                flag1 = true;
            } else if (fi2 > fi1){
                flag2 = true;
            }
        }

        if (flag1 && !flag2){
            return 1;
        } else if (!flag1 && flag2){
            return -1;
        } else {
            return 0;
        }
    }
};
}

static const size_t max_send_size = 500;

inline int get_half(const int& world_size){
    return (world_size/2) + 1;
}

inline int get_my_world_size(int rank, int world_size, int& source){
    int half;
    int next_source = 0;
    while(rank > 0){ 
        source = next_source;
        half = get_half(world_size);
        if(rank >= half){
            rank -= half;
            world_size = (world_size-1)/2;
            next_source += half;
        } else {
            rank -= 1;
            world_size = world_size/2;
            next_source += 1;
        }
    }
    return world_size;
}

template<typename Archive_t>
inline void broadcast_send(boost::shared_ptr<boost::mpi::communicator> comm, Archive_t& archive, int& my_world_size, int tag){
    using namespace boost::mpi;
    if(my_world_size > 1){
        std::vector<MPI_Request> requests(4);
        int num_requests = 0;
        int target_1 = comm->rank() + 1;
        dbg::out(dbg::info, "mpi") << "Rank: " << comm->rank() << " sending to rank: " << target_1 << std::endl;

        const void* size_1 = &archive.size();
        BOOST_MPI_CHECK_RESULT(MPI_Isend,
                (const_cast<void*>(size_1), 1, 
                 get_mpi_datatype<std::size_t>(archive.size()), 
                 target_1, tag, *comm, &requests[0]));
        //BOOST_MPI_CHECK_RESULT(MPI_Isend,
        //        (const_cast<void*>(size_1), 1, 
        //         get_mpi_datatype<std::size_t>(archive.size()), 
        //         target_1, tag, *comm, &requests[1]));
        BOOST_MPI_CHECK_RESULT(MPI_Isend,
                (const_cast<void*>(archive.address()), archive.size(),
                 MPI_PACKED,
                 target_1, tag, *comm, &requests[1]));
        //BOOST_MPI_CHECK_RESULT(MPI_Isend,
        //        (const_cast<void*>(archive.address()), archive.size(),
        //         MPI_PACKED,
        //         target_1, tag, *comm, &requests[0]));
        num_requests = 2;


        //num_requests += detail::packed_archive_isend(*comm, target_1, tag, archive, &requests[num_requests], 2);
        if(my_world_size > 2){
            int target_2 = comm->rank() + get_half(my_world_size);
            dbg::out(dbg::info, "mpi") << "Rank: " << comm->rank() << " sending to rank: " << target_2 << std::endl;
            const void* size_2 = &archive.size();
            BOOST_MPI_CHECK_RESULT(MPI_Isend,
                    (const_cast<void*>(size_2), 1, 
                     get_mpi_datatype<std::size_t>(archive.size()), 
                     target_2, tag, *comm, &requests[2]));
            BOOST_MPI_CHECK_RESULT(MPI_Isend,
                    (const_cast<void*>(archive.address()), archive.size(),
                     MPI_PACKED,
                     target_2, tag, *comm, &requests[3]));
            num_requests = 4;
            //num_requests += detail::packed_archive_isend(*comm, target_2, tag, archive, &requests[num_requests], 2);
        }
        BOOST_MPI_CHECK_RESULT(MPI_Waitall, (num_requests, &requests[0], MPI_STATUSES_IGNORE));
    }
}

template<typename Data_t>
void better_broadcast(boost::shared_ptr<boost::mpi::communicator> comm, Data_t& data){
    using namespace boost::mpi;
    int tag = environment::collectives_tag();
    int size = comm->size();

    if (comm->rank() == 0) {
        packed_oarchive oa(*comm);
        oa << data;
        //for (int i = 0; i < n; ++i)
        //oa << values[i];
        dbg::out(dbg::info, "mpi") << "Master " << comm->rank()  << " send archive: " <<  oa.size() << std::endl;
        broadcast_send(comm, oa, size, tag);

        // Broadcast data to all nodes
       // std::vector<MPI_Request> requests(size * 2);
       // int num_requests = 0;
       // dbg::out(dbg::info, "mpi") << "Data size: " << data.size() << " in archive: " << oa.size() << std::endl;
       // for (int dest = 1; dest < size; ++dest) {
       //     // Build up send requests for each child send.
       //     num_requests += detail::packed_archive_isend(*comm, dest, tag, oa, &requests[num_requests], 2);
       // }

       // // Complete all of the sends
       // BOOST_MPI_CHECK_RESULT(MPI_Waitall, (num_requests, &requests[0], MPI_STATUSES_IGNORE));

        //broadcast(comm, oa, root);
    } else {
        int source=0;
        int my_world_size = get_my_world_size(comm->rank(), size, source);
        packed_iarchive ia(*comm);
        //broadcast(comm, ia, root);

        // Receive data from the root.
        MPI_Status stat;
        std::size_t count;
        int number_amount = 0;

        BOOST_MPI_CHECK_RESULT(MPI_Recv,
                (&count, 1, get_mpi_datatype<std::size_t>(count),
                 source, tag, *comm, &stat));
        MPI_Get_count(&stat, get_mpi_datatype<std::size_t>(count), &number_amount);
        dbg::out(dbg::info, "mpi") << "Worker " << comm->rank()  << " received size_t of size: " << number_amount << " from " << stat.MPI_SOURCE << " with tag " << stat.MPI_TAG << std::endl;

        // Prepare input buffer and receive the message
        ia.resize(count);
        BOOST_MPI_CHECK_RESULT(MPI_Recv,
                (ia.address(), ia.size(), MPI_PACKED,
                 source, tag,
                 *comm, &stat));
        MPI_Get_count(&stat, MPI_PACKED, &number_amount);
        dbg::out(dbg::info, "mpi") << "Worker " << comm->rank()  << " received archive of size: " << number_amount << " from " << stat.MPI_SOURCE << " with tag " << stat.MPI_TAG << std::endl;

        //detail::packed_archive_recv(*comm, source, tag, ia, status);
        dbg::out(dbg::info, "mpi") << "Worker " << comm->rank()  << " Archive received: " <<  ia.size() << std::endl;
        broadcast_send(comm, ia, my_world_size, tag);

        ia >> data;
        //for (int i = 0; i < n; ++i)
        //    ia >> values[i];
    }
}

template<typename Phen, typename FitModifier, typename Params>                                                       \
class ArchiveTask{
public:
    //Params
    typedef ArchiveTask<Phen, FitModifier, Params> this_t;

    //The type of Pareto domination sort to use.
    //Currently available types are:
    // - sferes::ea::dom_sort_basic_f           (defined in sferes/ea/dom_sort_basic.hpp)
    //   Sorts according to pareto dominance and will add individuals from the highest to
    //   the lowest layer, with crowding as a tie-breaker in the last layer to be added.
    // - sferes::ea::dom_sort_no_duplicates_f   (defined in sferes/ea/dom_sort_no_duplicates.hpp)
    //   Same as dom_sort_basic, accept that, for each front, only one individual per pareto
    //   location is added. Other individuals at the same location will be bumped to the next layer.
    typedef typename Params::ea::dom_sort_f dom_sort_f;

    //The type non dominated comparator to use
    //Currently available types are:
    // - sferes::ea::_dom_sort_basic::non_dominated_f (defined in sferes/ea/dom_sort_basic.hpp)
    //   Regular comparisons based on dominance
    // - sferes::ea::innov_pnsga::prob_dom_f<Params>
    //   Comparisons based on probabilistic sorting, where some objectives can be stronger than others.
    typedef typename Params::innov_pnsga::non_dom_f non_dom_f;

    //The index used to temporarily store the category
    //This index should hold a dummy value, as it will be overwritten constantly.
    //The default would be 0.
    static const size_t obj_index = Params::innov::obj_index;

    //The number of objectives (bins) used in the map
    static const size_t nr_of_bins = Params::innov::categories;

    //The size of each bin
    static const size_t bin_size = Params::innov::bin_size;

    //The number of individuals initially generated to fill the archive
    //If equal to the bin_size, every initially generated individual is added
    //to every bin of every category.
    //The init_size has to be greater than or equal to the bin_size
    static const size_t init_size = Params::pop::init_size;

    //Not actually the size of the standing population
    //(which is bin_size*nr_of_bins)
    //but the number of individuals that are generated each epoch.
    //Because individuals are always produced in pairs,
    //pop_size has to be divisible by 2.
    static const size_t pop_size = Params::pop::size;

    //Modifier for calculating distance
    typedef typename boost::mpl::if_<boost::fusion::traits::is_sequence<FitModifier>,
                     FitModifier,
                     boost::fusion::vector<FitModifier> >::type modifier_t;

    typedef Phen phen_t;
    typedef crowd::Indiv<phen_t> crowd_t;
    typedef boost::shared_ptr<crowd_t> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef typename std::vector<boost::shared_ptr<phen_t> > ea_pop_t;
    typedef typename std::vector<std::vector<indiv_t> > front_t;

    modifier_t _fit_modifier;
    ea_pop_t _pop;

    void run(boost::shared_ptr<boost::mpi::communicator> _world, boost::mpi::status s,  boost::shared_ptr<boost::mpi::environment> env){
        dbg::trace trace("ea", DBG_HERE);
        pop_t pop;
        pop_t archive;
        pop_t new_bin;
        indiv_t temp;
        
        //Let's pretend the workers are slow to reach broadcast
        //std::cout << "Worker sleeping..." << std::endl;
        //sleep(10);
        //std::cout << "Worker waking..." << std::endl;

        dbg::out(dbg::info, "mpi") << "Worker " << _world->rank() << " receiving broadcast from world: " << _world << std::endl;
        //boost::mpi::broadcast(*_world, pop, 0);
        //for(size_t i=0; i<pop_size; ++i){
        //    dbg::out(dbg::info, "mpi") << "Worker receiving individual " << i << std::endl;
        //    _world->recv(0, 0, temp);
        //    pop.push_back(temp);
        //}
        better_broadcast(_world, pop);
        //pop.reserve(pop_size);
        //pop_t chunk;
        //while(pop.size() < pop_size){
        //    boost::mpi::broadcast(*_world, chunk, 0);
        //    for (size_t j=0; j<chunk.size(); ++j){
        //        pop.push_back(chunk[j]);
        //    }
        //}
         
        
        dbg::out(dbg::info, "mpi") << "Worker " << _world->rank() << " received pop of size: " << pop.size() << std::endl;
        //Check for null pointers
        //for (size_t i=0; i<pop.size(); ++i){
        //    if(!pop[i]){
        //        std::cout << "Null pointer: " << pop[i] << " at index: " << i << " with pop_size: " << pop.size() << std::endl;
        //    }
        //}
        //for (size_t i=0; i<pop.size(); ++i){
        //    std::cout << "Indiv " << i << ": ";
        //    for(size_t j=0; j<pop[i]->fit().objs().size();++j){
        //        std::cout << "obj(" <<j << ")=" << pop[i]->fit().obj(j) << " ";
        //    }
        //    std::cout << std::endl;
        //}

        while(true){
            dbg::out(dbg::info, "mpi") << "Worker " << _world->rank() << " waiting for message in map_elites_innovation_nsga_mpi.hpp" << std::endl;
            s = _world->probe();
            dbg::out(dbg::info, "mpi") << "Worker " << _world->rank() << " receveived message in map_elites_innovation_nsga_mpi.hpp tag: " << s.tag() << " source: " << s.source()  << std::endl;
            if (s.tag() == env->max_tag()){
                break;
            }

            _world->recv(0, s.tag(), archive);

            for(size_t j = 0; j < pop.size(); ++j){
                dbg::out(dbg::info, "ea") << "dereferencing j: " << j  << " : " << pop[j] << std::endl;
                archive.push_back(pop[j]);
            }
            _apply_modifier(archive);
            _cat_to_obj(archive, s.tag());
            _fill_nondominated_sort(archive, new_bin);
            dbg::out(dbg::info, "mpi") << "Worker " << _world->rank() << " sending bin: " << s.tag() << " size: " << new_bin.size() << std::endl;
            //for (size_t i=0; i<new_bin.size(); ++i){
            //    std::cout << "Indiv " << i << ": ";
            //    for(size_t j=0; j<new_bin[i]->fit().objs().size();++j){
            //            std::cout << "obj(" <<j << ")=" << new_bin[i]->fit().obj(j) << " ";
            //    }
            //    std::cout << std::endl;
            //}
            _world->send(0, s.tag(), new_bin);
        }
    }

    // modifiers
   void apply_modifier()
   { boost::fusion::for_each(_fit_modifier, ApplyModifier_f<this_t>(*this)); }

   const ea_pop_t& pop() const { return _pop; };
   ea_pop_t& pop() { return _pop; };

protected:
    /**
     * Converts a population from array individuals to regular individuals.
     */
    void _convert_pop(const pop_t& pop1, ea_pop_t& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i){
            dbg::out(dbg::info, "ea") << "dereferencing pop1: " << pop1[i]  << std::endl;
//            pop1[i]->set_rank(0);
            dbg::out(dbg::info, "ea") << "printing pop2: " << pop2[i]  << std::endl;

            pop2[i] = pop1[i];
        }
    }

    /**
     * Converts a population from regular individuals to crowd individuals.
     */
    void _convert_pop(const ea_pop_t& pop1, pop_t& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i){
            pop2[i] = boost::shared_ptr<crowd_t>(new crowd_t(*pop1[i]));
        }
    }

    /**
     * Does not actually convert anything, merely copies the content from one pop
     * to the other pop.
     */
    void _convert_pop(const pop_t& pop1, pop_t& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i){
            pop2[i] = pop1[i];
        }
    }
 

    /**
     * Applies the modifier to the supplied population (vector of individuals).
     *
     * Note that this overwrites the this->_pop population.
     */
    void _apply_modifier(pop_t pop){
        dbg::trace trace("ea", DBG_HERE);
        _convert_pop(pop, this->_pop);
        apply_modifier();
    }

    /**
     * Selects a random individual from the supplied population.
     */
    indiv_t _selection(const pop_t& pop){
        dbg::trace trace("ea", DBG_HERE);
        int x1 = misc::rand< int > (0, pop.size());
        dbg::check_bounds(dbg::error, 0, x1, pop.size(), DBG_HERE);
        return pop[x1];
    }

    /**
     * Takes a mixed population, sorts it according to Pareto dominance, and generates a new population
     * depending on the bin size.
     *
     * @Param mixed_pop The mixed population from which to select.
     *                  The mixed population must be larger than the bin_size for selection to occur.
     * @Param new_pop   Output parameter. After execution, should contain a number of individuals
     *                  equal to the bin_size, selected based on Pareto dominance first, crowding second.
     */
    void _fill_nondominated_sort(pop_t& mixed_pop, pop_t& new_pop)
    {
        dbg::trace trace("ea", DBG_HERE);
        dbg::out(dbg::info, "ea") << "Mixed pop size: " << mixed_pop.size() << " bin size: " << bin_size << std::endl;
        dbg::assertion(DBG_ASSERTION(mixed_pop.size()));
        dbg::assertion(DBG_ASSERTION(mixed_pop.size() >= bin_size));

        //Rank the population according to Pareto fronts
        front_t fronts;
        _rank_crowd(mixed_pop, fronts);

        //Add Pareto layers to the new population until the current layer no longer fits
        new_pop.clear();
        size_t front_index = 0;
        while(fronts[front_index].size() + new_pop.size() < bin_size){
            new_pop.insert(new_pop.end(), fronts[front_index].begin(), fronts[front_index].end());
            ++front_index;
        }

        // sort the last layer
        size_t size_remaining = bin_size - new_pop.size();
        if (size_remaining > 0){
            dbg::assertion(DBG_ASSERTION(front_index < fronts.size()));
            std::sort(fronts[front_index].begin(), fronts[front_index].end(), crowd::compare_crowd());
            for (size_t k = 0; k < size_remaining; ++k){
                new_pop.push_back(fronts[front_index][k]);
            }
        }
        dbg::assertion(DBG_ASSERTION(new_pop.size() == bin_size));
    }

    // --- rank & crowd ---

    /**
     * Ranks and crowds a population.
     *
     * Takes a population and divides it based on objectives.
     *
     * @param pop    The population to be ranked.
     * @param fronts The resulting Pareto fronts will be stored here.
     */
    void _rank_crowd(pop_t& pop, front_t& fronts)
    {
        dbg::trace trace("ea", DBG_HERE);
        //Execute ranking based on dominance
        std::vector<size_t> ranks;
        dom_sort_f()(pop, fronts, non_dom_f(), ranks);

        //Why are we assigning a crowd score to every individual?
        parallel::p_for(parallel::range_t(0, fronts.size()), crowd::assign_crowd<indiv_t >(fronts));
    }


    /**
     * For the specified category and array, copies the category score to the obj_index (usually 1).
     */
    void _cat_to_obj(pop_t& bin, size_t category){
        dbg::trace trace("ea", DBG_HERE);
        for(size_t i=0; i<bin.size(); ++i){
            bin[i]->fit().set_obj(obj_index, bin[i]->fit().cat(category));
        }
    }

    /**
     * Copies the stored diversity back to the relevant objective.
     *
     * Does nothing when DIV is not defined
     */
    void _div_to_obj(pop_t& bin, size_t category){
        dbg::trace trace("ea", DBG_HERE);
#if defined(DIV)
        for(size_t i=0; i<bin.size(); ++i){
            size_t div_index = bin[i]->fit().objs().size() - 1;
            bin[i]->fit().set_obj(div_index, bin[i]->fit().div(category));
        }
#endif
    }

    /**
     * Copies the calculated diversity to the individuals diversity array.
     *
     * Does nothing when DIV is not defined
     */
    void _obj_to_div(pop_t& bin, size_t category){
        dbg::trace trace("ea", DBG_HERE);
#if defined(DIV)
        for(size_t j = 0; j < bin.size(); ++j){
            bin[j]->fit().initDiv();
            size_t div_index = bin[j]->fit().objs().size() - 1;
            bin[j]->fit().setDiv(category, bin[j]->fit().obj(div_index));
        }
#endif
    }

//    //Debug functions
//#ifdef DBG_ENABLED
//    array_t _debug_array;
//
//    enum array_type{
//        current_array,
//        debug_array
//    };
//
//    /**
//     * Prints the fitness and closest distance values for each position in the archive.
//     *
//     * Note: requires the _debug_array to be set, otherwise it will throw a segmentation fault
//     */
//    void _print_archive(){
//        dbg::trace trace("ea", DBG_HERE);
//        std::cout << "Old archive:" << std::endl;
//        _print_array(current_array);
//        std::cout << "New archive:" << std::endl;
//        _print_array(debug_array);
//        _init_debug_array();
//    }
//
//    /**
//     * Print the debug array.
//     */
//    void _print_array(array_type type){
//        for(size_t i=0; i<nr_of_bins; ++i){
//            _cat_to_obj(_array, i);
//            _div_to_obj(_array, i);
//            _cat_to_obj(_debug_array, i);
//            _div_to_obj(_debug_array, i);
//
//            pop_t temp_current = _array[i];
//            pop_t temp_debug = _debug_array[i];
//
//            compare::sort(temp_current, compare::pareto_objs().descending());
//            compare::sort(temp_debug, compare::pareto_objs().descending());
//
//            for(size_t j=0; j<bin_size; ++j){
//                std::cout << "(";
//                for(size_t k=0; k<temp_current[j]->fit().objs().size(); ++k){
//                    bool better = temp_debug[j]->fit().obj(k) < temp_current[j]->fit().obj(k);
//                    bool worse = temp_debug[j]->fit().obj(k) > temp_current[j]->fit().obj(k);
//                    if(better) std::cout << COL_GREEN;
//                    if(worse) std::cout << COL_MAGENTA;
//                    float value = temp_current[j]->fit().obj(k);;
//                    if(type == debug_array) value = temp_debug[j]->fit().obj(k);
//                    std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], value);
//                    std::cout << END_COLOR;
//                    if(k+1 != _array[i][j]->fit().objs().size()) std::cout << ":";
//                }
//                std::cout << ") ";
//            }
//            std::cout << std::endl;
//        }
//    }
//
//    /**
//     * Sets the debug array.
//     *
//     * Required for debugging at the end of the random pop and load population functions.
//     */
//    void _init_debug_array(){
//        dbg::trace trace("ea", DBG_HERE);
//        for(size_t i=0; i<nr_of_bins; ++i){
//            _debug_array[i].resize(bin_size);
//            for(size_t j=0; j<bin_size; ++j){
//                _debug_array[i][j] = indiv_t(new crowd_t(*_array[i][j]));
//            }
//        }
//    }
//
//#endif //DBG_ENABLED
};

// Main class
SFERES_EA(MapEliteInovNSGA2Mpi, Ea){
public:
    //Params

    //The type of Pareto domination sort to use.
    //Currently available types are:
    // - sferes::ea::dom_sort_basic_f           (defined in sferes/ea/dom_sort_basic.hpp)
    //   Sorts according to pareto dominance and will add individuals from the highest to
    //   the lowest layer, with crowding as a tie-breaker in the last layer to be added.
    // - sferes::ea::dom_sort_no_duplicates_f   (defined in sferes/ea/dom_sort_no_duplicates.hpp)
    //   Same as dom_sort_basic, accept that, for each front, only one individual per pareto
    //   location is added. Other individuals at the same location will be bumped to the next layer.
    typedef typename Params::ea::dom_sort_f dom_sort_f;

    //The type non dominated comparator to use
    //Currently available types are:
    // - sferes::ea::_dom_sort_basic::non_dominated_f (defined in sferes/ea/dom_sort_basic.hpp)
    //   Regular comparisons based on dominance
    // - sferes::ea::innov_pnsga::prob_dom_f<Params>
    //   Comparisons based on probabilistic sorting, where some objectives can be stronger than others.
    typedef typename Params::innov_pnsga::non_dom_f non_dom_f;

    //The index used to temporarily store the category
    //This index should hold a dummy value, as it will be overwritten constantly.
    //The default would be 0.
    static const size_t obj_index = Params::innov::obj_index;

    //The number of objectives (bins) used in the map
    static const size_t nr_of_bins = Params::innov::categories;

    //The size of each bin
    static const size_t bin_size = Params::innov::bin_size;

    //The number of individuals initially generated to fill the archive
    //If equal to the bin_size, every initially generated individual is added
    //to every bin of every category.
    //The init_size has to be greater than or equal to the bin_size
    static const size_t init_size = Params::pop::init_size;

    //Not actually the size of the standing population
    //(which is bin_size*nr_of_bins)
    //but the number of individuals that are generated each epoch.
    //Because individuals are always produced in pairs,
    //pop_size has to be divisible by 2.
    static const size_t pop_size = Params::pop::size;

    typedef Phen phen_t;
    typedef crowd::Indiv<phen_t> crowd_t;
    typedef boost::shared_ptr<crowd_t> indiv_t;
    typedef typename std::vector<indiv_t> pop_t;
    typedef typename std::vector<std::vector<indiv_t> > front_t;
    typedef boost::array<pop_t, nr_of_bins> array_t;


    MapEliteInovNSGA2Mpi()
    {
        dbg::trace trace("ea", DBG_HERE);
        dbg::compile_assertion<pop_size%2 == 0>("Population size has to be divisible by 2.");
        dbg::out(dbg::info, "ea") << "Objectives: " << nr_of_bins << std::endl;
    }

    void random_pop()
    {
        dbg::trace trace("ea", DBG_HERE);
        //Create and evaluate the initial random population
        parallel::init();
        pop_t pop;
        pop.resize(init_size);
        BOOST_FOREACH(indiv_t& indiv, pop)
        {
            indiv = indiv_t(new crowd_t());
            indiv->random();
        }
        this->_eval.eval(pop, 0, pop.size());

        _apply_modifier(pop);
        _add_to_archive(pop);

        DBG_CONDITIONAL(dbg::info, "archive", _init_debug_array());
    }

    void epoch()
    {
        dbg::trace trace("ea", DBG_HERE);

        //We are creating and selecting a number of individuals equal to the population size
        //A simpler variant would only select and mutate one individual
        pop_t ptmp;
        for(size_t i=0; i<_array.size(); ++i){
            if(_array[i].size() != bin_size){
                std::cout << "Before bin " << i << " does contains only " << _array[i].size() << " indiv." <<std::endl;
            }
        }
        for (size_t i = 0; i < (pop_size/2); ++i)
        {
            dbg::out(dbg::info, "ea") << "Creating individual: " << i <<std::endl;
            indiv_t p1 = _selection(_array);
            indiv_t p2 = _selection(_array);
            indiv_t i1, i2;

            p1->cross(p2, i1, i2);
            dbg::out(dbg::info, "ea") << "Mutating i1 " << i1 << std::endl;
            i1->mutate();
            dbg::out(dbg::info, "ea") << "Mutating i1 " << i1 << " success " << std::endl;

            dbg::out(dbg::info, "ea") << "Mutating i2 " << i2 << std::endl;
            i2->mutate();
            dbg::out(dbg::info, "ea") << "Mutating i2 " << i2 << " success" << std::endl;
            ptmp.push_back(i1);
            ptmp.push_back(i2);
        }
        this->_eval.eval(ptmp, 0, ptmp.size());
        _add_to_archive(ptmp);

        for(size_t i=0; i<_array.size(); ++i){
            if(_array[i].size() != bin_size){
                std::cout << "Bin " << i << " does contains only " << _array[i].size() << " indiv." <<std::endl;
            }
        }

        //For writing statistics only from the first bin:
        _convert_pop(_array[0], this->_pop);

        DBG_CONDITIONAL(dbg::info, "archive", _print_archive());
    }

    const array_t& archive() const { return _array; }


protected:
    array_t _array;

    /**
     * Converts a population from array individuals to regular individuals.
     */
    void _convert_pop(const pop_t& pop1, std::vector<boost::shared_ptr<Phen> >& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i){
            dbg::out(dbg::info, "ea") << "dereferencing pop1: " << pop1[i]  << std::endl;
//            pop1[i]->set_rank(0);
            dbg::out(dbg::info, "ea") << "printing pop2: " << pop2[i]  << std::endl;

            pop2[i] = pop1[i];
        }
    }

    /**
     * Converts a population from regular individuals to crowd individuals.
     */
    void _convert_pop(const std::vector<boost::shared_ptr<Phen> >& pop1, pop_t& pop2){
        dbg::trace trace("ea", DBG_HERE);
        pop2.resize(pop1.size());
        for (size_t i = 0; i < pop1.size(); ++i){
            pop2[i] = boost::shared_ptr<crowd_t>(new crowd_t(*pop1[i]));
        }
    }

    /**
     * Applies the modifier to the supplied population (vector of individuals).
     *
     * Note that this overwrites the this->_pop population.
     */
    void _apply_modifier(pop_t pop){
        dbg::trace trace("ea", DBG_HERE);
        _convert_pop(pop, this->_pop);
        this->apply_modifier();
    }

    /**
     * Adds the new `population' (vector of individuals) to the archive by adding
     * every individual to every bin, and then running NSGA 2 (or pNSGA) selection on every bin.
     */
    void _add_to_archive(pop_t& pop){
        dbg::trace trace("ea", DBG_HERE);
        //Add everyone to the archive

        //Set new task
        for (size_t i = 1; i < this->eval().world()->size(); ++i){
            this->eval().world()->send(i, this->eval().env()->max_tag(), 1);
        }

        //std::cout << "Broadcasting population of size: " << pop.size() << std::endl;
        //for (size_t i=0; i<pop.size(); ++i){
        //    std::cout << "Indiv " << i << ": ";
        //    for(size_t j=0; j<pop[i]->fit().objs().size();++j){
        //        std::cout << "obj(" <<j << ")=" << pop[i]->fit().obj(j) << " ";
        //    }
        //    std::cout << std::endl;
        //}

        //for (size_t i=0; i<pop.size(); ++i){
        //    if(!pop[i]){
        //        std::cout << "Null pointer: " << pop[i] << " at index: " << i << " with pop_size: " << pop.size() << std::endl;
        //    }
        //}
        //Let's pretend the master is really slow here
        //std::cout << "Master sleeping..." << std::endl;
        //sleep(10);
        //std::cout << "Master waking..." << std::endl;

        //Broadcast the new individuals to all workers
        dbg::out(dbg::info, "mpi") << "Broadcasting population of size: " << pop.size() << " to world: " << this->eval().world() << std::endl;
        //broadcast(*this->eval().world(), pop, 0);
        //Manual broadcast for debugging (mpi broadcast currenlty fails)
        //static const size_t max_send_size = 500;
        better_broadcast(this->eval().world(), pop);
        //size_t current=0;
        //size_t still_to_send = pop.size();
        //pop_t chunk;
        //chunk.resize(max_send_size);
        //while(current < pop.size()){
        //    if(still_to_send < max_send_size){
        //        chunk.resize(still_to_send);
        //        still_to_send = 0;
        //    } else {
        //        still_to_send -= max_send_size;
        //    }
        //    for (size_t j=0; j<max_send_size && current<pop.size(); ++j, ++current){
        //        //dbg::out(dbg::info, "mpi") << "Sending indivdiual " << j << " to worker " << i << std::endl;
        //        //this->eval().world()->send(i, 0, pop[j]);
        //        chunk[j] = pop[current];
        //    }
        //    boost::mpi::broadcast(*this->eval().world(), chunk, 0);
        //}


        size_t current = 0;
        std::vector<bool> done(nr_of_bins);
        std::fill(done.begin(), done.end(), false);

        //While I seem unable to reproduce the race-condition in this particular case,
        //I may be able to prevent it this way
        //std::cout << "Master sleeping..." << std::endl;
        //sleep(10);
        //std::cout << "Master waking..." << std::endl;

        // first round
        for (size_t i = 1; i < this->eval().world()->size() && current < nr_of_bins; ++i){
            dbg::out(dbg::info, "mpi") << "Sending bin: " << current  << " to worker " << i << std::endl; 
            this->eval().world()->send(i, current, _array[current]);
            ++current;
        }

        // Subsequent rounds
        while (current < nr_of_bins){
            boost::mpi::status s = this->eval().world()->probe();
            dbg::out(dbg::info, "mpi") << "Receiving bin: " << s.tag() << std::endl;
            this->eval().world()->recv(s.source(), s.tag(), _array[s.tag()]);
            dbg::out(dbg::info, "mpi") << "Received bin: " << s.tag() << " size: " << _array[s.tag()].size()  << std::endl;
            //for (size_t i=0; i<_array[s.tag()].size(); ++i){
            //    std::cout << "Indiv " << i << ": ";
            //    for(size_t j=0; j<_array[s.tag()][i]->fit().objs().size();++j){
            //            std::cout << "obj(" <<j << ")=" << _array[s.tag()][i]->fit().obj(j) << " ";
            //    }
            //    std::cout << std::endl;
            //}
            done[s.tag()] = true;
            dbg::out(dbg::info, "mpi") << "Sending bin: " << current << std::endl;
            this->eval().world()->send(s.source(), current, _array[current]);
            ++current;
        }

        // Join
        bool all_done = true;
        do{
            dbg::out(dbg::info, "mpi")<<"joining..."<<std::endl;
            all_done = true;
            for (size_t i = 0; i < nr_of_bins; ++i){
                if (!done[i]){
                    boost::mpi::status s = this->eval().world()->probe();
                    dbg::out(dbg::info, "mpi") << "Receiving bin: " << s.tag() << std::endl;
                    this->eval().world()->recv(s.source(), s.tag(), _array[s.tag()]);
                    dbg::out(dbg::info, "mpi") << "Received bin: " << s.tag() << " size: " << _array[s.tag()].size()  << std::endl;
                    done[s.tag()] = true;
                    all_done = false;
                }
            }
        }
        while (!all_done);

        //DBG_CONDITIONAL(dbg::info, "archive", _obj_to_div(_array, i));
    }

    /**
     * Selects a random individual from the supplied population.
     */
    indiv_t _selection(const pop_t& pop){
        dbg::trace trace("ea", DBG_HERE);
        int x1 = misc::rand< int > (0, pop.size());
        dbg::check_bounds(dbg::error, 0, x1, pop.size(), DBG_HERE);
        return pop[x1];
    }

    /**
     * Selects a random individual from the supplied archive
     */
    indiv_t _selection(const array_t& archive){
        dbg::trace trace("ea", DBG_HERE);
        size_t category = misc::rand< size_t > (0, archive.size());
        dbg::check_bounds(dbg::error, 0, category, archive.size(), DBG_HERE);
        size_t individual_index = misc::rand< size_t > (0, archive[category].size());
        dbg::check_bounds(dbg::error, 0, individual_index, archive[category].size(), DBG_HERE);
        return archive[category][individual_index];
    }

    /**
     * Takes a mixed population, sorts it according to Pareto dominance, and generates a new population
     * depending on the bin size.
     *
     * @Param mixed_pop The mixed population from which to select.
     *                  The mixed population must be larger than the bin_size for selection to occur.
     * @Param new_pop   Output parameter. After execution, should contain a number of individuals
     *                  equal to the bin_size, selected based on Pareto dominance first, crowding second.
     */
    void _fill_nondominated_sort(pop_t& mixed_pop, pop_t& new_pop)
    {
        dbg::trace trace("ea", DBG_HERE);
        dbg::assertion(DBG_ASSERTION(mixed_pop.size()));

        //Rank the population according to Pareto fronts
        front_t fronts;
        _rank_crowd(mixed_pop, fronts);

        //Add Pareto layers to the new population until the current layer no longer fits
        new_pop.clear();
        size_t front_index = 0;
        while(fronts[front_index].size() + new_pop.size() < bin_size){
            new_pop.insert(new_pop.end(), fronts[front_index].begin(), fronts[front_index].end());
            ++front_index;
        }

        // sort the last layer
        size_t size_remaining = bin_size - new_pop.size();
        if (size_remaining > 0){
            dbg::assertion(DBG_ASSERTION(front_index < fronts.size()));
            std::sort(fronts[front_index].begin(), fronts[front_index].end(), crowd::compare_crowd());
            for (size_t k = 0; k < size_remaining; ++k){
                new_pop.push_back(fronts[front_index][k]);
            }
        }
        dbg::assertion(DBG_ASSERTION(new_pop.size() == bin_size));
    }

    // --- rank & crowd ---

    /**
     * Ranks and crowds a population.
     *
     * Takes a population and divides it based on objectives.
     *
     * @param pop    The population to be ranked.
     * @param fronts The resulting Pareto fronts will be stored here.
     */
    void _rank_crowd(pop_t& pop, front_t& fronts)
    {
        dbg::trace trace("ea", DBG_HERE);
        //Execute ranking based on dominance
        std::vector<size_t> ranks;
        dom_sort_f()(pop, fronts, non_dom_f(), ranks);

        //Why are we assigning a crowd score to every individual?
        parallel::p_for(parallel::range_t(0, fronts.size()), crowd::assign_crowd<indiv_t >(fronts));

        //Why are we sorting the population?
//        for (size_t i = 0; i < ranks.size(); ++i){
//            pop[i]->set_rank(ranks[i]);
//        }
//        parallel::sort(pop.begin(), pop.end(), crowd::compare_ranks());;
    }


    /**
     * For the specified category and array, copies the category score to the obj_index (usually 1).
     */
    void _cat_to_obj(array_t& array, size_t category){
        dbg::trace trace("ea", DBG_HERE);
        dbg::check_bounds(dbg::error, 0, category, array.size(), DBG_HERE);
        for(size_t i=0; i<array[category].size(); ++i){
            array[category][i]->fit().set_obj(obj_index, array[category][i]->fit().cat(category));
        }
    }

    /**
     * Copies the stored diversity back to the relevant objective.
     *
     * Does nothing when DIV is not defined
     */
    void _div_to_obj(array_t& array, size_t category){
        dbg::trace trace("ea", DBG_HERE);
#if defined(DIV)
        for(size_t i=0; i<bin_size; ++i){
            size_t div_index = _array[category][i]->fit().objs().size() - 1;
            array[category][i]->fit().set_obj(div_index, array[category][i]->fit().div(category));
        }
#endif
    }

    /**
     * Copies the calculated diversity to the individuals diversity array.
     *
     * Does nothing when DIV is not defined
     */
    void _obj_to_div(array_t& array, size_t category){
        dbg::trace trace("ea", DBG_HERE);
#if defined(DIV)
        for(size_t j = 0; j < array[category].size(); ++j){
            array[category][j]->fit().initDiv();
            size_t div_index = array[category][j]->fit().objs().size() - 1;
            array[category][j]->fit().setDiv(category, array[category][j]->fit().obj(div_index));
        }
#endif
    }

    //Debug functions
#ifdef DBG_ENABLED
    array_t _debug_array;

    enum array_type{
        current_array,
        debug_array
    };

    /**
     * Prints the fitness and closest distance values for each position in the archive.
     *
     * Note: requires the _debug_array to be set, otherwise it will throw a segmentation fault
     */
    void _print_archive(){
        dbg::trace trace("ea", DBG_HERE);
        std::cout << "Old archive:" << std::endl;
        _print_array(current_array);
        std::cout << "New archive:" << std::endl;
        _print_array(debug_array);
        _init_debug_array();
    }

    /**
     * Print the debug array.
     */
    void _print_array(array_type type){
        for(size_t i=0; i<nr_of_bins; ++i){
            _cat_to_obj(_array, i);
            _div_to_obj(_array, i);
            _cat_to_obj(_debug_array, i);
            _div_to_obj(_debug_array, i);

            pop_t temp_current = _array[i];
            pop_t temp_debug = _debug_array[i];

            compare::sort(temp_current, compare::pareto_objs().descending());
            compare::sort(temp_debug, compare::pareto_objs().descending());

            for(size_t j=0; j<bin_size; ++j){
                std::cout << "(";
                for(size_t k=0; k<temp_current[j]->fit().objs().size(); ++k){
                    bool better = temp_debug[j]->fit().obj(k) < temp_current[j]->fit().obj(k);
                    bool worse = temp_debug[j]->fit().obj(k) > temp_current[j]->fit().obj(k);
                    if(better) std::cout << COL_GREEN;
                    if(worse) std::cout << COL_MAGENTA;
                    float value = temp_current[j]->fit().obj(k);;
                    if(type == debug_array) value = temp_debug[j]->fit().obj(k);
                    std::cout << karma::format(karma::left_align(5, '0')[karma::maxwidth(5)[karma::double_]], value);
                    std::cout << END_COLOR;
                    if(k+1 != _array[i][j]->fit().objs().size()) std::cout << ":";
                }
                std::cout << ") ";
            }
            std::cout << std::endl;
        }
    }

    /**
     * Sets the debug array.
     *
     * Required for debugging at the end of the random pop and load population functions.
     */
    void _init_debug_array(){
        dbg::trace trace("ea", DBG_HERE);
        for(size_t i=0; i<nr_of_bins; ++i){
            _debug_array[i].resize(bin_size);
            for(size_t j=0; j<bin_size; ++j){
                _debug_array[i][j] = indiv_t(new crowd_t(*_array[i][j]));
            }
        }
    }

#endif //DBG_ENABLED
};
}
}



#endif /* MODULES_MAPELITE_MAP_ELITE_INNOVATION_NSGA2_HPP_ */
