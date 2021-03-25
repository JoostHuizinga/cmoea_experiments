#ifndef STAT_NUM_TILES_IND_HPP
#define STAT_NUM_TILES_IND_HPP

#include <iostream>
#include <sstream>
#include <sferes/stat/stat.hpp>

namespace sferes {
  namespace stat {
    SFERES_STAT(NumTilesInd, Stat) {
    public:
      /* Writes the total number of tiles visited by each individual. Called every generation. */
      template<typename E>
      void refresh(const E& ea) {
          /* Create a file to record the individual. */
          this->_create_log_file(ea, "fitnessInd");

          /* Go through each individual in the population. */
          for (size_t individual = 0; individual < ea.pop().size(); individual++) {
            /* Write the number of tiles visited by the individual. */
            (*this->_log_file) << ea.gen() << " " << ea.pop()[individual]->fit().value() << std::endl;
          }
        }
    };
  }
}
#endif
