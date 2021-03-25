/*
 * pnsga_params.hpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_PNSGA_PARAMS_HPP_
#define MODULES_MISC_PNSGA_PARAMS_HPP_

#include "params.hpp"

SFERES_VALUE_PARAM(PnsgaPressure,
        #if defined(PR100)
            SFERES_CONST float v = 1.00;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR95)
            SFERES_CONST float v = 0.95;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR75)
            SFERES_CONST float v = 0.75;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR50)
            SFERES_CONST float v = 0.50;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR25)
            SFERES_CONST float v = 0.25;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR10)
            SFERES_CONST float v = 0.10;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if defined(PR0)
            SFERES_CONST float v = 0.00;
        #define PNSGA_PRESSURE_DEFINED
        #endif

        #if not defined(PNSGA_PRESSURE_DEFINED)
            SFERES_CONST float v = 1.00;
        #endif
)


#endif /* MODULES_MISC_PNSGA_PARAMS_HPP_ */
