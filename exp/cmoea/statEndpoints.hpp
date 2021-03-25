#ifndef STAT_ENDPOINT_HPP
#define STAT_ENDPOINT_HPP

#include <iostream>
#include <sstream>
#include <sferes/stat/stat.hpp>

namespace sferes {
namespace stat {
SFERES_STAT(Endpoints, Stat) {
public:
    /* Writes the trajectory of neural commands for each individual in the population. Called every generation (but only runs every trajectory_dump_period). */
    template<typename E>
    void refresh(const E& ea) {
        /* Only run this function if trajectory_dump_period generations have elapsed. */
        if ((ea.gen() % Params::stats::num_tiles_dump_period) != (Params::stats::num_tiles_dump_period - 1)) { return; }

        /* Create a file to record the endpoints. */
        std::stringstream endpoints;
        endpoints << ea.res_dir() << "/points_gen" << ea.gen();
        std::ofstream endpointsFile(endpoints.str().c_str());

        /* Go through each individual in the population. */
        for (size_t individual = 0; individual < ea.pop().size(); individual++) {
            float x = ea.pop()[individual]->fit().historyPositions.at(ea.pop()[individual]->fit().historyPositions.size() - 2);
            float y = ea.pop()[individual]->fit().historyPositions.at(ea.pop()[individual]->fit().historyPositions.size() - 1);
            endpointsFile << x << " " << y << "\n";
        }

        /* Close the file. */
        endpointsFile.close();
    }
};
}
}
#endif
