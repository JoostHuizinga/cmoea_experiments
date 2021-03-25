/*
 * recursive_split_test.cpp
 *
 *  Created on: Sep 4, 2016
 *      Author: Joost Huizinga
 */

#include "recursive_split.hpp"
#include "chambers_to_map.hpp"

/* Sets up and runs the simulation. */
int main(int argc, char **argv) {
    dbg::init();
    attach_ostream(dbg::warning, std::cout);
    attach_ostream(dbg::error, std::cerr);
    attach_ostream(dbg::info, std::cout);

    time_t t = time(0) + ::getpid();
    sferes::misc::seed(t);
    randmaze::Chamber* root;
    fastsim::Map* map;

    root = new randmaze::Chamber(10, 10);
    std::cout << "Initial configuration: " << std::endl;
    std::cout << *root << std::endl;

    for(int i=1; i<=5; ++i){
        root->split();
        std::cout << "After split " << i << ":" << std::endl;
        std::cout << *root << std::endl;
    }

    map = generate_map(*root, 3);
    std::cout << *map << std::endl;
    delete map;
    delete root;


    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 2: splitting at the 1 index with door at index 0 with grid size 2." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 0);
    map = generate_map(*root, 3);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 3: splitting at the 1 index with door at index 1 with grid size 2." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 1);
    map = generate_map(*root, 3);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 4: splitting at the 1 index with door at index 1 with grid size 3." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 1);
    map = generate_map(*root, 3);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 5: splitting at the 1 index with door at index 1 with grid size 4." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 1);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 6: splitting at the 1 index with door at index 0 with grid size 3." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 0);
    map = generate_map(*root, 3);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Test 7: splitting at the 1 index with door at index 0 with grid size 4." << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_vertical_at(1, 0);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 8: splitting 2 by 2 chamber horizontal at the 1 index with door at index 0 with grid size 4." << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(2, 2);
    root->split_horizontal_at(1, 0);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 9: splitting 3 by 3 chamber horizontal at the 1 index with door at index 0 with grid size 4." << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(3, 3);
    root->split_horizontal_at(1, 0);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 10: splitting 4 by 4 chamber horizontal at the 1 index with door at index 0 with grid size 4." << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(4, 4);
    root->split_horizontal_at(1, 0);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 11: splitting 4 by 4 chamber horizontal at the 2 index with door at index 1 with grid size 4." << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(4, 4);
    root->split_horizontal_at(2, 1);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 12: splitting 4 by 4 chamber horizontal at the 3 index with door at index 2 with grid size 4." << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(4, 4);
    root->split_horizontal_at(3, 2);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Test 13: splitting 4 by 4 chamber horizontal and then vertical" << std::endl;
    std::cout << "-------------------------------------------------------------------------------------------------" << std::endl;
    root = new randmaze::Chamber(4, 4);
    root->split_horizontal_at(2, 2);
    root->first->split_vertical_at(1, 0);
    root->second->split_vertical_at(1, 0);
    map = generate_map(*root, 4);
    std::cout << *map << std::endl;
    delete map;
    delete root;

    return 0;
}

