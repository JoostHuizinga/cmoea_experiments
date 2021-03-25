/*
 * continue_mapelite_inov_div.hpp
 *
 *  Created on: Feb 24, 2015
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_CONTINUE_MAPELITE_INOV_DIV_HPP_
#define EXP_MODULARITY_CONTINUE_MAPELITE_INOV_DIV_HPP_

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
class ContinueMapEliteInovDiv : public EA{
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
        //Add everyone to the archive in the appropriate place
        size_t pop_index = 0;
        for(size_t i=0; i<this->_array.shape()[0]; ++i){
            for(size_t j=0; j<this->_array.shape()[1]; ++j){
                this->_array[i][j] = population[pop_index];
                ++pop_index;
            }
        }

        //Rebuild the distance matrix
        for(size_t i=0; i<this->nr_of_bins; ++i){
            for(size_t j=0; j<this->bin_size; ++j){
                for(size_t k=j+1; k<this->bin_size; ++k){
                    this->_distances[i][j][k] = this->_array[i][j]->fit().dist(*this->_array[i][k]);
                }
            }
        }

        DBG_CONDITIONAL(dbg::info, "archive", this->_init_debug_array());
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


#endif /* EXP_MODULARITY_CONTINUE_MAPELITE_INOV_DIV_HPP_ */
