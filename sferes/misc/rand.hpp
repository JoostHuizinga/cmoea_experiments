

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




#ifndef RAND_HPP_
#define RAND_HPP_

// Standard libraries
#include <cstdlib>
#include <cmath>
#include <list>
#include <stdlib.h>
#include <bitset>
#include <limits>
#include <sstream>
//#pragma GCC diagnostic push


#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif

// Fix for boost 1.69
// https://github.com/boostorg/random/issues/49
#define BOOST_PENDING_INTEGER_LOG2_HPP
#include <boost/integer/integer_log2.hpp>
// End fix

#include <boost/swap.hpp>
#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/serialization/nvp.hpp>

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#endif
#endif

#include <sferes/dbg/dbg.hpp>
#include <sferes/stat/stat.hpp>

#ifndef NO_PARALLEL
#include <tbb/mutex.h>
#endif

// someday we will have a real thread-safe random number generator...
namespace sferes
{
namespace misc
{

#ifndef NO_PARALLEL
//Let's try to make this thread safe
tbb::mutex rngMutex;
#define TBB_SCOPED_LOCK tbb::mutex::scoped_lock lock(rngMutex)
#else
#define TBB_SCOPED_LOCK ((void)0)
#endif


//We store the randomness generator in an array to allow multiple generators to be created,
//Which can be used to simulate the existence of multiple generators in an MPI setup
std::vector<boost::mt19937> random_generators(1);

/**
 * Used to store the state of all generators and write them to a string that can then be serialized.
 */
inline std::string randStateToString(){
	dbg::trace trace("random", DBG_HERE);
	TBB_SCOPED_LOCK;
	std::stringstream ss;
	ss << random_generators.size() <<" ";
	for(size_t i=0; i<random_generators.size(); i++){
		ss << random_generators[i] << " ";
	}
	return ss.str();
}

/**
 * Used the reconstruct the state of all number generators based on a string produced by randStateToString()
 */
inline void randStateFromString(std::string rand_state){
	dbg::trace trace("random", DBG_HERE);
	TBB_SCOPED_LOCK;
	std::stringstream ss(rand_state);
	size_t nr_of_generators;
	ss >> nr_of_generators;
	for(size_t i=0; i<nr_of_generators; i++){
		boost::mt19937 randomness_generator;
		ss >> randomness_generator;
		random_generators.push_back(randomness_generator);
	}
}

/**
 * Pushes a new generator into the vector.
 * By default only the last pushed generator will be used.
 */
inline void pushRand(){
	dbg::trace trace("random", DBG_HERE);
	TBB_SCOPED_LOCK;
	random_generators.push_back(boost::mt19937());
}

/**
 * Pops the last generator from the vector,
 * effectively reverting the state of the generator back to right before pushRand() was called
 */
inline void popRand(){
	dbg::trace trace("random", DBG_HERE);
	TBB_SCOPED_LOCK;
	if(random_generators.size() > 1){
		random_generators.pop_back();
	}
}

/**
 * Sets the seed of the active generator.
 */
inline void seed(unsigned int seed){
	dbg::trace trace("random", DBG_HERE);
	dbg::out(dbg::info, "random") << "Seed set to: " << seed << std::endl;
	TBB_SCOPED_LOCK;
	random_generators.back().seed(seed);
}


/**
 * Returns an integer between 0 (inclusive) and max (exclusive).
 */
template<typename IntType>
inline IntType randInt(IntType max = 1.0)
{
	dbg::trace trace("random", DBG_HERE);
	assert(max > 0);
	boost::uniform_int<IntType> dist(0, max-1);
	TBB_SCOPED_LOCK;
	IntType v = dist(random_generators.back());
	dbg::out(dbg::info, "random") << "Integer generated: " << v << " max: " << max << std::endl;
	dbg::out(dbg::info, "scrand") << "Integer generated: " << v << " max: " << max << std::endl;
	assert(v < max);
	return v;
}

/**
 * Returns an integer between min (inclusive) and max (exclusive).
 */
template<typename IntType>
inline IntType randInt(IntType min, IntType max)
{
    dbg::trace trace("random", DBG_HERE);
    assert(max > min);
    boost::uniform_int<IntType> dist(min, max-1);
    TBB_SCOPED_LOCK;
    IntType v = dist(random_generators.back());
    dbg::out(dbg::info, "random") << "Integer generated: " << v << " max: " << max << std::endl;
    dbg::out(dbg::info, "scrand") << "Integer generated: " << v << " max: " << max << std::endl;
    assert(v < max);
    assert(v >= min);
    return v;
}

/**
 * Returns a real valued number between 0 and max.
 */
template<typename RealType>
inline RealType randReal(RealType max = 1.0)
{
	dbg::trace trace("random", DBG_HERE);
	assert(max > 0);
	boost::random::uniform_real_distribution<RealType> dist(0, max);
	TBB_SCOPED_LOCK;
	RealType v = dist(random_generators.back());
	dbg::out(dbg::info, "random") << "Real generated: " << v << std::endl;
	dbg::out(dbg::info, "scrand") << "Real generated: " << v << std::endl;
	assert(v < max);
	return v;
}

/**
 * Specializations of the rand function for integer number types (all other assumed to be real numbers).
 * Currently supporting int, unsigned int, long and unsigned long.
 */
template<typename T> inline T rand(T max = 1.0){dbg::trace trace("random", DBG_HERE); return randReal(max);}
template<> inline int rand<int>(int max) {dbg::trace trace("random", DBG_HERE); return randInt(max);}
template<> inline unsigned int rand<unsigned int>(unsigned int max) {dbg::trace trace("random", DBG_HERE); return randInt(max);}
template<> inline long rand<long>(long max) {dbg::trace trace("random", DBG_HERE); return randInt(max);}
template<> inline unsigned long rand<unsigned long>(unsigned long max) {dbg::trace trace("random", DBG_HERE); return randInt(max);}

//    template<float> inline float rand(float max = 1.0) {return randReal(max);}
//    template<unsigned float> inline unsigned float rand(unsigned float max = 1.0) {return randReal(max);}
//    template<double> inline double rand(double max = 1.0) {return randReal(max);}
//    template<unsigned double> inline unsigned double rand(unsigned double max = 1.0) {return randReal(max);}

/**
 * Rand function to produce a real number between min and max.
 */
template<typename T>
inline T rand(T min, T max)
{
	dbg::trace trace("random", DBG_HERE);
	assert(max != min);
	assert(max > min);
	T res = T(rand<double>() * ((long int) max - (long int) min) + min);
	assert(res >= min);
	assert(res < max);
	return res;
}

/**
 * Gaussian random function to produce a real number according to a Gaussian distribution.
 *
 * Original implementation came from the sferes2 framework:
 * https://github.com/jbmouret/sferes2
 *
 * Seems to be based on the method by Abramowitz and Stegun as described here:
 * http://c-faq.com/lib/gaussian.html, but without the 'phase'.
 * I'm not sure about the consequences...
 *
 * @param m The median of the Gaussian distribution.
 * @param v The deviation (the default is 1, the standard deviation).
 */
template<typename T>
inline T gaussian_rand(T m=0.0,T  v=1.0)
{
	dbg::trace trace("random", DBG_HERE);
	float facteur = sqrt(-2.0f * log(rand<float>()));
	float trigo  = 2.0f * M_PI * rand<float>();

	return T(m + v * facteur * cos(trigo));

}

/**
 * Used to produce a vector of size randomly ordered numbers between 0 and size.
 * Every number will exists exactly once in the vector.
 */
inline void rand_ind(std::vector<size_t>& a1, size_t size)
{
	dbg::trace trace("random", DBG_HERE);
	a1.resize(size);
	for (size_t i = 0; i < a1.size(); ++i)
		a1[i] = i;
	for (size_t i = 0; i < a1.size(); ++i)
	{
		size_t k = rand(i, a1.size());
		assert(k < a1.size());
		boost::swap(a1[i], a1[k]);
	}
}


/**
 * return a random it in the list
 */
template<typename T>
inline typename std::list<T>::iterator rand_in_list(std::list<T>& l)
{
	dbg::trace trace("random", DBG_HERE);
	int n = rand(l.size());
	typename std::list<T>::iterator it = l.begin();
	for (int i = 0; i < n; ++i)
		++it;
	return it;
}


template<typename T>
inline T& rand_in_vector(std::vector<T> candidates){
    int n = randInt(candidates.size());
    return candidates[n];
}

/**
 * Flip and unweighed coin.
 */
inline bool flip_coin() {dbg::trace trace("random", DBG_HERE); return rand<float>() < 0.5f; }

/**
 * Returns an iterator to a random element in the container.
 */
template<typename L>
inline typename L::iterator rand_l(L& l)
{
	dbg::trace trace("random", DBG_HERE);
	size_t k = rand(l.size());
	dbg::out(dbg::info, "random") << "Selected: " << k << std::endl;
	typename L::iterator it = l.begin();
	for (size_t i = 0; i < k; ++i)
		++it;
	return it;
}

/**
 * Creates a random bitset of the specified length.
 */
template<int bits> std::bitset<bits> randBitset() {
	dbg::trace trace("random", DBG_HERE);

	int modulus = (bits%16) - 1;
	int max = 2 << modulus;
	boost::uniform_int<> dist(0, max-1);
	boost::uniform_int<> dist_sixteen(0, 65535); //(2^16) - 1

	TBB_SCOPED_LOCK;
	std::bitset<bits> r(dist(random_generators.back()));
	for(int i = 0; i < (bits/16); i++) {
		r <<= 16;
		// "OR" together with a new lower 16 bits:
		r |= std::bitset<bits>(dist_sixteen(random_generators.back()));
	}
	return r;
}

struct _RandGen : std::unary_function<unsigned, unsigned> {
//    	boost::mt19937 &_state;
    unsigned operator()(unsigned i) {
    	//boost::uniform_int<> dist(0, i - 1);
        return randInt(i);
    }
//    bar(boost::mt19937 &state) : _state(state) {}
};


template<typename T>
inline void shuffle(std::vector<T>& vec){
	_RandGen rand_gen;
	std::random_shuffle(vec.begin(), vec.end(), rand_gen);
}

}

namespace stat
{

/**
 * Stat class used to serialize the random number generators and add their state to the gen file.
 */
SFERES_STAT(SerializeRand, Stat){
public:
	/**
	 * Not used, but required to extend Stat.
	 */
	template<typename E>
	void refresh(const E& ea)
	{
		//nop
	}

	/**
	 * Saves the random number generator state to the archive.
	 */
	template<typename Archive>
	void save(Archive& ar, const unsigned v) const
	{
		dbg::trace trace("random", DBG_HERE);
		std::string random_number_generator_state = misc::randStateToString();
		ar& BOOST_SERIALIZATION_NVP(random_number_generator_state);
	}

	/**
	 * Loads the random number generator state from the archive.
	 */
	template<typename Archive>
	void load(Archive& ar, const unsigned v)
	{
		dbg::trace trace("random", DBG_HERE);
		std::string random_number_generator_state;
		ar& BOOST_SERIALIZATION_NVP(random_number_generator_state);
		misc::randStateFromString(random_number_generator_state);
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER();

    std::string name(){
        return "SerializeRand";
    }
};
}
}
#endif
