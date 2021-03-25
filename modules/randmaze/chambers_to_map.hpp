/*
 * chambers_to_map.hpp
 *
 *  Created on: Sep 4, 2016
 *      Author: Joost Huizinga
 */

#ifndef MODULES_RANDMAZE_CHAMBERS_TO_MAP_HPP_
#define MODULES_RANDMAZE_CHAMBERS_TO_MAP_HPP_

#include <modules/fastsim2/map.hpp>
#include <modules/randmaze/recursive_split.hpp>

#include <sferes/dbg/dbg.hpp>

namespace randmaze{

class Maze: public fastsim::Map{
public:
    Maze(int width_, int height_, int gridSize):
        fastsim::Map(width_, height_, gridSize),
        start_x(-1),
        start_y(-1),
        start_r(-1)
    {
        // nix
    }

    float start_x;
    float start_y;
    float start_r;
};

void _split_chamber(const Chamber& chamber, Maze* map, int grid_size, int x_offset, int y_offset){
    if(chamber.split_dir == horizontal){
        for (int x = 0; x < chamber.width*grid_size; x++) {
            if(x <= chamber.door_loc*grid_size || (x+1) >= (chamber.door_loc*grid_size + grid_size)){
                map->setPixel((x_offset + x), y_offset + chamber.split_loc*grid_size, Maze::collision) ;
                map->setPixel((x_offset + x), y_offset + chamber.split_loc*grid_size-1, Maze::collision) ;
            }
        }
        _split_chamber(*chamber.first, map, grid_size, x_offset, y_offset);
        _split_chamber(*chamber.second, map, grid_size, x_offset, y_offset + chamber.split_loc*grid_size);
    } else if (chamber.split_dir == vertical){
        for (int y = 0; y < chamber.height*grid_size; y++) {
            if(y <= chamber.door_loc*grid_size || (y+1) >= (chamber.door_loc*grid_size + grid_size)){
                map->setPixel(x_offset + chamber.split_loc*grid_size, (y_offset + y), Maze::collision);
                map->setPixel(x_offset + chamber.split_loc*grid_size-1, (y_offset + y), Maze::collision);
            }
        }
        _split_chamber(*chamber.first, map, grid_size, x_offset, y_offset);
        _split_chamber(*chamber.second, map, grid_size, x_offset+ chamber.split_loc*grid_size, y_offset);
    }

    for(int i=0; i<chamber.goals.size(); ++i){
        map->addGoal(fastsim::Goal(chamber.goals[i].first*grid_size + grid_size/2, chamber.goals[i].second*grid_size + grid_size/2, 4, 0));
    }
}

Maze* generate_map(const Chamber& chamber, int grid_size = 10){
    Maze* map = new Maze(chamber.width*grid_size, chamber.height*grid_size, grid_size);


    // Draw outer wall
    for (int y = 0; y < chamber.height*grid_size; y++) {
        map->setPixel(0, y, Maze::collision);
        map->setPixel(chamber.width*grid_size-1, y, Maze::collision);
    }
    for (int x = 0; x < chamber.width*grid_size; x++) {
        map->setPixel(x, 0, Maze::collision);
        map->setPixel(x, chamber.height*grid_size-1, Maze::collision);
    }

    _split_chamber(chamber, map, grid_size, 0, 0);

    map->start_x = chamber.start_x*grid_size + grid_size/2;
    map->start_y = chamber.start_y*grid_size + grid_size/2;
    map->start_r = chamber.start_r;
    return map;
}

}
#endif /* MODULES_RANDMAZE_CHAMBERS_TO_MAP_HPP_ */
