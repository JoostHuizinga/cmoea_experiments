/*
 * continue_ea_nsga.hpp
 *
 *  Created on: Nov 22, 2014
 *      Author: Joost Huizinga
 *
 * A wrapper for NSGA2 (and derived algorithms) that allows the EA to be resumed from a checkpoint.
 */

#ifndef MODULES_CONTINUE_CONTINUE_EA_HPP_
#define MODULES_CONTINUE_CONTINUE_EA_HPP_

//Sferes includes
#include <sferes/stc.hpp>
#include <sferes/dbg/dbg.hpp>
#include <sferes/ea/crowd.hpp>
#include <sferes/ea/ea.hpp>

//Boost includes
#include <boost/serialization/binary_object.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/assign/list_of.hpp>

namespace sferes{
namespace ea{

/**
 * A base EA continue class.
 *
 * Implements:
 * - write_gen_file(const std::string& prefix)
 *   Allows the EA to write a 'gen' file with an arbitrary name so we can distinguish
 *   checkpoints from proper gen files.
 *
 * - init_parent_pop(std::vector<boost::shared_ptr<LocalPhen> > population)
 *   Sets the initial population to the supplied population, rather than a random population.
 *
 * - _write(const std::string& fname, ConvertStatType& stat)
 *   If BINARYARCHIVE is defined, overwrites _write from ea.hpp, to write a binary string
 *   signaling the end of the file. This binary string is used to assert that the binary
 *   archive has been written properly and completely
 */
template<typename EA_t, typename Params>
class ContinueEa : public EA_t{
public:
#ifdef  SFERES_XML_WRITE
    typedef boost::archive::xml_oarchive oa_t;
#else
    typedef boost::archive::binary_oarchive oa_t;
#endif


    /**
     * Writes a gen file (a serialized version of the current state of the program) with the specified prefix.
     *
     * The full name of the file will be: prefix_<gen>
     * where <gen> is the current generation.
     *
     * @param prefix The prefix of the gen file.
     */
    std::string write_gen_file(const std::string& prefix){
        std::string fname = this->_res_dir + std::string("/") + prefix + boost::lexical_cast<std::string>(this->_gen);
        this->_write(fname, this->_stat);
        return fname;
    }

    /**
     * Sets the current parent population to be the supplied population.
     *
     * This function allows you to start with any arbitrary parent population,
     * instead of a random population.
     *
     * @param population The initial population.
     */
    template<typename LocalPhen>
    void init_parent_pop(std::vector<boost::shared_ptr<LocalPhen> > population){
        dbg::assertion(DBG_ASSERTION(population.size() <= Params::pop::size));
        this->pop.clear();
        for (size_t i = 0; i < Params::pop::size; ++i){
            this->pop.push_back(population[i]);
        }
    }

protected:

    /**
     * Writes all stats to the archive with the supplied name.
     *
     * If BINARYARCHIVE is defined, also writes a final string,
     * used to assert that the file has been written properly.
     *
     * @param fname The name if the output file.
     * @param stat  The boost fusion vector of statistics that should be written.
     */
    template<typename ConvertStatType>
    void _write(const std::string& fname, ConvertStatType& stat) const
    {
        dbg::trace trace("ea", DBG_HERE);
        dbg::out(dbg::info, "ea") << "Writing to file: " << fname << std::endl;
        std::ofstream ofs(fname.c_str());
        oa_t oa(ofs);
        boost::fusion::for_each(stat, WriteStat_f<oa_t>(oa));

#if !defined(SFERES_XML_WRITE)
        //If our archive is binary, explicitly set these characters at the end of the file
        //so we can check (or at least be reasonably certain) that the archive was written
        //successfully and entirely
        char end[25] = "\n</boost_serialization>\n";
        boost::serialization::binary_object object(end, 25);
        oa << object;
#endif

        std::cout << fname << " written" << std::endl;
    }

};

}
}
#endif /* MODULES_CONTINUE_CONTINUE_EA_HPP_ */
