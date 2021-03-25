/*
 * stat_store_pop.hpp
 *
 *  Created on: Feb 23, 2015
 *      Author: joost
 */

#ifndef STORE_POP_HPP_
#define STORE_POP_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>

namespace sferes
{
namespace stat
{

/**
 * Statistics object that stores and serializes the entire population when dumped.
 *
 * Implements the getPopulation() function required for continuing runs.
 */
SFERES_STAT(StorePop, Stat)
{

public:
	typedef boost::shared_ptr<Phen> indiv_t;
	typedef std::vector<indiv_t> pop_t;

	/**
	 * Copies the current population to a local vector.
	 *
	 * Should be called at least once, directly before dumping.
	 *
	 * @param ea The ea which holds the population to be stored.
	 */
	template<typename E>
	void refresh(const E& ea)
	{
        dbg::trace trace("stat", DBG_HERE);
        setPopulation(ea);
	}

	/**
	 * Serialize the stored population.
	 *
	 * @ar      The archive to which the population should be written.
	 * @version The version number that should be written to the archive.
	 */
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		dbg::trace trace("stat", DBG_HERE);
		dbg::out(dbg::info, "serialize") << "Serializing: _all" << std::endl;
		ar& BOOST_SERIALIZATION_NVP(_all);
	}

	/**
	 * Sets the current population.
	 *
	 * @param ea The ea which holds the population to be stored.
	 */
	template<typename E>
	void setPopulation(const E& ea){
		dbg::trace trace("stat", DBG_HERE);
        const pop_t& pop = ea.pop();

        //Clear all individuals
        _all.clear();

        //The mixed population (before selection)
        for (size_t i = 0; i < pop.size(); ++i)
        {
            _all.push_back(pop[i]);
        }
	}

	/**
	 * Returns a reference to the stored population.
	 *
	 * The idea is to load the population from a serialized file
	 * (automatically done by boost serialize)
	 * and then call this function to retrieve the population.
	 */
	pop_t& getPopulation(){
		dbg::trace trace("stat", DBG_HERE);
		return _all;
	}

    std::string name(){
        return "StorePop";
    }

protected:
	pop_t _all;
};
}
}


#endif /* STORE_POP_HPP_ */
