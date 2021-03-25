#ifndef STAT_NUM_TILES_POP_HPP
#define STAT_NUM_TILES_POP_HPP

#include <iostream>
#include <sstream>
#include <sferes/stat/stat.hpp>

namespace sferes {
  namespace stat {
    SFERES_STAT(NumTilesPop, Stat) {
    public:
      /* Writes the total number of tiles visited by the entire population. Called every generation (but only runs every num_tiles_dump_period). */
      template<typename E>
      void refresh(const E& ea) {
        /* Create the population grid. */
        std::vector<std::vector<bool> > tilesVisited;
        tilesVisited.resize(ea.pop()[0]->fit().width, std::vector<bool>(ea.pop()[0]->fit().height, false));

        /* Go through each individual in the population and add their grid to the population grid. */
        for (size_t individual = 0; individual < ea.pop().size(); individual++) {
          for (size_t x = 0; x < ea.pop()[0]->fit().width; x++) {
            for (size_t y = 0; y < ea.pop()[0]->fit().height; y++) {
              if (ea.pop()[individual]->fit().tilesVisited.at(x).at(y) == true) {
                tilesVisited.at(x).at(y) = true;
              }
            }
          }
        }

        /* Count the number of visited tiles. */
        int numTilesVisited = 0;
        for (size_t x = 0; x < ea.pop()[0]->fit().width; x++) {
          for (size_t y = 0; y < ea.pop()[0]->fit().height; y++) {
            if (tilesVisited.at(x).at(y)) { numTilesVisited++; }
          }
        }

        /* Write the visited tiles to a log file. */
        this->_create_log_file(ea, "fitnessPop");
        (*this->_log_file) << ea.gen() << " " << numTilesVisited << std::endl;
      }
    };
  }
}
#endif
