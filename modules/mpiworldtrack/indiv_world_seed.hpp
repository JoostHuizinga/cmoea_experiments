/*
 * indiv_world_seed.hpp
 *
 *  Created on: Sep 9, 2016
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MPIWORLDTRACK_INDIV_WORLD_SEED_HPP_
#define MODULES_MPIWORLDTRACK_INDIV_WORLD_SEED_HPP_

namespace sferes
{
namespace gen
{
template<typename Phen>
class WorldSeedIndiv : public Phen
{
public:
    void mutate(){
        _world_seeds.clear();
        Phen::mutate();
    }


    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        dbg::out(dbg::info, "serialize") << "Serializing: parent" << std::endl;
        Phen::serialize(ar, version);
        dbg::out(dbg::info, "serialize") << "Serializing: _world_seeds" << std::endl;
        ar& BOOST_SERIALIZATION_NVP(_world_seeds);
    }

    void add_world_seed(size_t world_seed){
        _world_seeds.push_back(world_seed);
    }

    size_t get_world_seed(){
        return _world_seeds.back();
    }

    bool world_seed_set(){
        return !_world_seeds.empty();
    }

private:
    std::vector<size_t> _world_seeds;

};

}
}
#endif /* MODULES_MPIWORLDTRACK_INDIV_WORLD_SEED_HPP_ */
