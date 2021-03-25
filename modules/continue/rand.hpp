/*
 * rand.hpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_RAND_HPP_
#define MODULES_MISC_RAND_HPP_

//Sferes includes
#include <sferes/dbg/dbg.hpp>
#include <sferes/misc/rand.hpp>

//Module includes
#include <modules/continue/global_options.hpp>

namespace modules
{
namespace rand
{

void init(){
    sferes::options::add()("seed", boost::program_options::value<int>(), "program seed");
}

void set_seed(){
    dbg::trace trace("init", DBG_HERE);
    if(sferes::options::map.count("seed")){
        int seed = sferes::options::map["seed"].as<int>();
        std::cout<<"seed: " << seed << std::endl;
        srand(seed);
        sferes::misc::seed(seed);
    } else {
        time_t t = time(0) + ::getpid();
        std::cout<<"seed: " << t << std::endl;
        srand(t);
        sferes::misc::seed(t);
    }
}

}
}

#endif /* MODULES_MISC_RAND_HPP_ */
