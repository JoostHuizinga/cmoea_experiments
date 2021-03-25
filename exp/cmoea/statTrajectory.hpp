#ifndef STAT_TRAJECTORY_HPP
#define STAT_TRAJECTORY_HPP

#include <iostream>
#include <sstream>
#include <sferes/stat/stat.hpp>

namespace sferes {
  namespace stat {
    SFERES_STAT(Trajectory, Stat) {
    public:
      /* Writes the trajectory of neural commands for each individual in the population. Called every generation (but only runs every trajectory_dump_period). */
      template<typename E>
      void refresh(const E& ea) {

        /* Only run this function if trajectory_dump_period generations have elapsed. */
        if ((ea.gen() % Params::stats::trajectory_dump_period) != (Params::stats::trajectory_dump_period - 1)) { return; }

        /* Go through each individual in the population. */
        for (size_t individual = 0; individual < ea.pop().size(); individual++) {
          /* Create a file to record the trajectory. */
          std::stringstream trajectory;
          trajectory << ea.res_dir() << "/gen" << ea.gen() << "_ind" << individual << "_history";
          std::ofstream trajectoryFile(trajectory.str().c_str());

          /* Write the trajectory. */
          trajectoryFile << "/* TRAJECTORY */" << std::endl;
          trajectoryFile << "/* POINT:";
          for (size_t i = 0; i < ea.pop()[individual]->fit().historyPositions.size(); i++) {
            trajectoryFile << " " << ea.pop()[individual]->fit().historyPositions[i];
          }
          trajectoryFile << " */";
          trajectoryFile.close();

          // /* Create a file to record the outputs. */
          // std::stringstream outputs;
          // outputs << ea.res_dir() << "/gen" << ea.gen() << "_ind" << individual << "_outputs";
          // std::ofstream outputsFile(outputs.str().c_str());

          // /* Go through each neural output of this individual. */
          // for (size_t historyValue = 0; historyValue < ea.pop()[individual]->fit().historyOutputs.size(); historyValue++) {
          //   /* Write the neural output to the file. */
          //   if (historyValue % Params::dnn::nb_outputs == 0) { outputsFile << "\n"; }
          //   outputsFile << ea.pop()[individual]->fit().historyOutputs[historyValue] << " ";
          // }

          /* Close the file. */
          // outputsFile.close();
        }
      }
    };
  }
}
#endif
