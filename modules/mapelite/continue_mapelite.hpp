/*
 * continue_mapelite.hpp
 *
 *  Created on: Jun 26, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MAPELITE_CONTINUE_MAPELITE_HPP_
#define MODULES_MAPELITE_CONTINUE_MAPELITE_HPP_

//Sferes includes
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

template<typename EA, typename Params>
class ContinueMapElite: public EA{
public:
    typedef typename EA::oa_t oa_t;

    void set_gen(size_t gen){
        this->_gen = gen;
    }

    void write_gen_file(const std::string& prefix){
        std::string fname = this->_res_dir + std::string("/") + prefix + boost::lexical_cast<std::string>(this->_gen);
        this->_write(fname, this->_stat);
    }

    template<typename LocalPhen>
    void init_parent_pop(std::vector<boost::shared_ptr<LocalPhen> > population){
        dbg::out(dbg::info, "continue") << "Adding: " << population.size() << std::endl;

        //Add everyone to the archive in the appropriate place
        BOOST_FOREACH(boost::shared_ptr<LocalPhen>& indiv, population){
            this->_add_to_archive(indiv, indiv);
        }
    }

protected:


    /**
     * Writes all stats to the archive with the supplied name.
     *
     * If BINARYARCHIVE is defines, also writes a final string,
     * used to assert that the file has been written properly.
     *
     * @param fname The name if the output file.
     * @param stat  The boost fusion vector of statistics that should be written.
     */
    template<typename ConvertStatType>
    void _write(const std::string& fname, ConvertStatType& stat) const
    {
      dbg::trace trace("ea", DBG_HERE);
      std::ofstream ofs(fname.c_str());
      oa_t oa(ofs);
      boost::fusion::for_each(stat, WriteStat_f<oa_t>(oa));

#ifdef BINARYARCHIVE
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

#endif /* MODULES_MAPELITE_CONTINUE_MAPELITE_HPP_ */
