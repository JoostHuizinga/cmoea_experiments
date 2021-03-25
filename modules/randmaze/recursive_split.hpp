/*
 * recursive_split.hpp
 *
 *  Created on: Sep 4, 2016
 *      Author: Joost Huizinga
 *
 * This maze generation algorithm is based on the one described in:
 * E. Meyerson, J. Lehman, and R. Miikkulainen.
 * Learning Behavior Characterizations for Novelty Search
 * Genetic and Evolutionary Computation Conference (GECCO), 2016
 *
 * Their implementation was apparently based on:
 * A. M. Reynolds.
 * Maze-solving by chemotaxis.
 * Physical Review E, 81(6), 2010.
 */

#ifndef MODULES_RANDMAZE_RECURSIVE_SPLIT_HPP_
#define MODULES_RANDMAZE_RECURSIVE_SPLIT_HPP_

#include <sferes/dbg/dbg.hpp>
#include <sferes/misc/rand.hpp>

namespace randmaze{

enum split_direction{
    none,
    horizontal,
    vertical,
};

class Chamber{
public:

    Chamber(int width=0, int height=0):
      width(width),
      height(height),
      split_loc(-1),
      door_loc(-1),
      split_dir(none),
      first(0),
      second(0),
      start_x(-1),
      start_y(-1),
      start_r(-1){
        // nix
    }

    ~Chamber(){
        if(first){
            delete first;
            first = 0;
        }
        if(second){
            delete second;
            second = 0;
        }
    }

    bool isSplit(){
        return split_dir != none;
    }

    bool split(){
        bool success = true;
        if(isSplit()){
            success = _split_child();
        } else{
            success = _split_self();
        }
        return success;
    }

    void addGoal(){
        goals.push_back(std::pair<int, int>(sferes::misc::randInt(width), sferes::misc::randInt(height)));
    }

    void generateStart(){
        start_x = sferes::misc::randInt(width);
        start_y = sferes::misc::randInt(height);
        start_r = sferes::misc::randReal<float>(2*M_PI);
    }

    /**
     * Splits this chamber horizontally at the indicated position.
     *
     * Returns true if successful and false otherwise.
     */
    void split_horizontal_at(int i, int j){
        dbg::assertion(DBG_ASSERTION(width > 1));
        dbg::assertion(DBG_ASSERTION(height > 1));
        dbg::assertion(DBG_ASSERTION(i > 0));
        dbg::assertion(DBG_ASSERTION(i < height));
        dbg::assertion(DBG_ASSERTION(j >= 0));
        dbg::assertion(DBG_ASSERTION(j < width));
        dbg::assertion(DBG_ASSERTION(split_dir == none));
        dbg::assertion(DBG_ASSERTION(split_loc == -1));
        dbg::assertion(DBG_ASSERTION(first == 0));
        dbg::assertion(DBG_ASSERTION(second == 0));
        split_dir = horizontal;
        split_loc = i;
        door_loc = j;
        first = new Chamber(width, split_loc);
        second = new Chamber(width, height - split_loc);
    }


    /**
     * Splits this chamber vertically at the indicated position.
     *
     * Returns true if successful and false otherwise.
     */
    void split_vertical_at(int i, int j){
        dbg::assertion(DBG_ASSERTION(width > 1));
        dbg::assertion(DBG_ASSERTION(height > 1));
        dbg::assertion(DBG_ASSERTION(i > 0));
        dbg::assertion(DBG_ASSERTION(i < width));
        dbg::assertion(DBG_ASSERTION(j >= 0));
        dbg::assertion(DBG_ASSERTION(j < height));
        dbg::assertion(DBG_ASSERTION(split_dir == none));
        dbg::assertion(DBG_ASSERTION(split_loc == -1));
        dbg::assertion(DBG_ASSERTION(first == 0));
        dbg::assertion(DBG_ASSERTION(second == 0));
        split_dir = vertical;
        split_loc = i;
        door_loc = j;
        first = new Chamber(split_loc, height);
        second = new Chamber(width- split_loc, height);
    }


    int width;
    int height;
    int split_loc;
    int door_loc;
    split_direction split_dir;

    Chamber* first;
    Chamber* second;

    std::vector<std::pair<int, int> > goals;

    int start_x;
    int start_y;
    float start_r;

private:
    /**
     * Randomly split one of our children.
     *
     * If one child fails, try the other.
     * If both children fail the split, return false.
     */
    bool _split_child(){
        dbg::assertion(DBG_ASSERTION(split_dir != none));
        dbg::assertion(DBG_ASSERTION(split_loc != -1));
        dbg::assertion(DBG_ASSERTION(first != 0));
        dbg::assertion(DBG_ASSERTION(second != 0));
        bool success = true;
        if(sferes::misc::flip_coin()){
            success = first->split();
            if(!success){
                success = second->split();
            }
        } else {
            success = second->split();
            if(!success){
                success = first->split();
            }
        }
        return success;
    }


    /**
     * Randomly split this node, and generate two children.
     */
    bool _split_self(){
        dbg::assertion(DBG_ASSERTION(split_dir == none));
        dbg::assertion(DBG_ASSERTION(split_loc == -1));
        dbg::assertion(DBG_ASSERTION(first == 0));
        dbg::assertion(DBG_ASSERTION(second == 0));
        if (height <= 1 || width <= 1) return false;
        if(sferes::misc::flip_coin()){
            _split_horizontal();
        } else {
            _split_vertical();
        }
        return true;
    }

    /**
     * Splits this chamber horizontally.
     *
     * Returns true if successful and false otherwise.
     */
    void _split_horizontal(){
        split_horizontal_at(sferes::misc::randInt(1, height), sferes::misc::randInt(width));
    }

    /**
     * Splits this chamber vertically.
     *
     * Returns true if successful and false otherwise.
     */
    void _split_vertical(){
        split_vertical_at(sferes::misc::randInt(1, width), sferes::misc::randInt(height));
    }
};

std::ostream& operator<<(std::ostream& is, const Chamber& obj){
    if(obj.split_dir == none){
        is << obj.width << ", " << obj.height;
    } else {
        is << obj.width << ", " << obj.height << ", " << obj.split_loc << ", " << obj.split_dir << " (" << *obj.first << " ; " << *obj.second << ")";
    }
    return is;
}

}



#endif /* MODULES_RANDMAZE_RECURSIVE_SPLIT_HPP_ */
