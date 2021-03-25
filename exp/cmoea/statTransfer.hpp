#ifndef STAT_TRANSFER_HPP
#define STAT_TRANSFER_HPP

#include <iostream>
#include <sstream>
#include <sferes/stat/stat.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace sferes {
  namespace stat {
    SFERES_STAT(Transfer, Stat) {
    public:
      /* Writes the trajectory of neural commands for each individual in the population. Called every generation (but only runs every trajectory_dump_period). */
      template<typename E>
      void refresh(const E& ea) {
        /* Only run this function if trajectory_dump_period generations have elapsed. */
        if ((ea.gen() % Params::stats::transfer_dump_period) != (Params::stats::transfer_dump_period - 1)) { return; }

        /* Create empty roster of champions. */
        int individualIndex[Params::stats::max_champions];
        float individualFitness[Params::stats::max_champions];
        for (int i = 0; i < Params::stats::max_champions; i++) {
          individualIndex[i] = -1;
          individualFitness[i] = -1.0f;
        }

        /* Assign champions based on fitness score. */
        for (size_t individual = 0; individual < ea.pop().size(); individual++) {
          float fitness = ea.pop()[individual]->fit().value();
          for (size_t championSlot = 0; championSlot < Params::stats::max_champions; championSlot++) {
            if (fitness > individualFitness[championSlot]) {
              individualIndex[championSlot] = individual;
              individualFitness[championSlot] = fitness;
              break; //found a slot to place this individual; stop searching
            }
          }
        }

        /* Print all champions. */
        int highestFitnessIndex = 0;
        for (size_t championSlot = 0; championSlot < Params::stats::max_champions; championSlot++) {
          std::stringstream genomeChampion;
          genomeChampion << ea.res_dir() << "/gen" << ea.gen() << "_champ" << championSlot << "_" << (int) ea.pop()[individualIndex[championSlot]]->fit().value();
          std::ofstream championFile(genomeChampion.str().c_str(), std::ios::binary);
          boost::archive::binary_oarchive championFileSerialized(championFile);
          championFileSerialized << ea.pop()[individualIndex[championSlot]];
          if (individualFitness[championSlot] > individualFitness[highestFitnessIndex]) {
            highestFitnessIndex = championSlot; //keep track of individual with the highest fitness
          }
        }

        /* Print the winner (champion with the highest fitness). */
        std::stringstream genomeWinner;
        genomeWinner << ea.res_dir() << "/gen" << ea.gen() << "_winner_" << (int) ea.pop()[individualIndex[highestFitnessIndex]]->fit().value();
        std::ofstream winnerFile(genomeWinner.str().c_str(), std::ios::binary);
        boost::archive::binary_oarchive winnerFileSerialized(winnerFile);
        winnerFileSerialized << ea.pop()[individualIndex[highestFitnessIndex]];
      }
    };
  }
}
#endif
