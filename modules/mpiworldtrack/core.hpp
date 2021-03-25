/*
 * core.hpp
 *
 *  Created on: Sep 2, 2016
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MPIWORLDTRACK_CORE_HPP_
#define MODULES_MPIWORLDTRACK_CORE_HPP_

#include "indiv_world_seed.hpp"
#if defined(MPI_ENABLED) && !defined(NOMPI)
#include "eval_mpi_world_tracking.hpp"
#else
#include "eval_world_tracking.hpp"
#include "eval_parallel_world_tracking.hpp"
#endif



#endif /* MODULES_MPIWORLDTRACK_CORE_HPP_ */
