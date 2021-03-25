/*
 * stat_render_video_pop_only.hpp
 *
 *  Created on: Feb 23, 2015
 *      Author: joost
 */

#ifndef RENDER_VIDEO_POP_ONLY_HPP_
#define RENDER_VIDEO_POP_ONLY_HPP_

#include <numeric>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>

#include <sferes/stat/stat.hpp>
#include <sferes/dbg/dbg.hpp>

#include "stat_mod_robot_shared.hpp"

namespace sferes
{
namespace stat
{

/**
 * Statistics object that stores and serializes the entire population when dumped.
 *
 * Implements the getPopulation() function required for continuing runs.
 */
SFERES_STAT(RenderVideoPopOnly, StorePop)
{

public:
	typedef boost::shared_ptr<Phen> indiv_t;
	typedef std::vector<indiv_t> pop_t;

    void show(std::ostream& os, size_t k)
    {
    	dbg::trace trace("stat", DBG_HERE);
    	sharedShow<indiv_t, pop_t, Params>(os, k, this->_all);
    }

    std::string name(){
        return "RenderVideoPopOnly";
    }

    static void initOptions(){
    	sharedInitOptions();
    }
};
}
}


#endif /* RENDER_VIDEO_POP_ONLY_HPP_ */
