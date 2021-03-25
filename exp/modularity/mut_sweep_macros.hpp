/*
 * mut_sweep_macros.hpp
 *
 *  Created on: Nov 22, 2013
 *      Author: joost
 */

#ifndef MUT_SWEEP_MACROS_HPP_
#define MUT_SWEEP_MACROS_HPP_


#ifdef NEUDELMUTRATE01
#define NEUDELMUTRATE(DEFAULT) 0.01f 		//Delete neuron rate for the cppn
#elif defined NEUDELMUTRATE001
#define NEUDELMUTRATE(DEFAULT) 0.001f		//Add connection rate for the cppn
#elif defined NEUDELMUTRATE005
#define NEUDELMUTRATE(DEFAULT) 0.005f		//Add connection rate for the cppn
#elif defined NEUDELMUTRATE03
#define NEUDELMUTRATE(DEFAULT) 0.03f		//Delete neuron rate for the cppn
#elif defined NEUDELMUTRATE05
#define NEUDELMUTRATE(DEFAULT) 0.05f		//Delete neuron rate for the cppn
#elif defined NEUDELMUTRATE07
#define NEUDELMUTRATE(DEFAULT) 0.07f		//Delete neuron rate for the cppn
#elif defined NEUDELMUTRATE09
#define NEUDELMUTRATE(DEFAULT) 0.09f		//Delete neuron rate for the cppn
#elif defined NEUDELMUTRATE11
#define NEUDELMUTRATE(DEFAULT) 0.11f		//Delete neuron rate for the cppn
#else
#define NEUDELMUTRATE(DEFAULT) DEFAULT		//Delete neuron rate for the cppn
#endif


#ifdef NEUADDMUTRATE01
      #define NEUADDMUTRATE(DEFAULT) 0.01f		//Add neuron rate for the cppn
#elif defined NEUADDMUTRATE001
      #define NEUADDMUTRATE(DEFAULT) 0.001f		//Add connection rate for the cppn
#elif defined NEUADDMUTRATE005
      #define NEUADDMUTRATE(DEFAULT) 0.005f		//Add connection rate for the cppn
#elif defined NEUADDMUTRATE03
      #define NEUADDMUTRATE(DEFAULT) 0.03f		//Add neuron rate for the cppn
#elif defined NEUADDMUTRATE05
      #define NEUADDMUTRATE(DEFAULT) 0.05f		//Add neuron rate for the cppn
#elif defined NEUADDMUTRATE07
      #define NEUADDMUTRATE(DEFAULT) 0.07f		//Add neuron rate for the cppn
#elif defined NEUADDMUTRATE09
      #define NEUADDMUTRATE(DEFAULT) 0.09f		//Add neuron rate for the cppn
#elif defined NEUADDMUTRATE11
      #define NEUADDMUTRATE(DEFAULT) 0.11f		//Add neuron rate for the cppn
#else
	#define NEUADDMUTRATE(DEFAULT) DEFAULT
#endif


#ifdef CONADDMUTRATE01
#define CONADDMUTRATE(DEFAULT) 0.01f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE001
#define CONADDMUTRATE(DEFAULT) 0.001f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE005
#define CONADDMUTRATE(DEFAULT) 0.005f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE03
#define CONADDMUTRATE(DEFAULT) 0.03f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE05
#define CONADDMUTRATE(DEFAULT) 0.05f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE07
#define CONADDMUTRATE(DEFAULT) 0.07f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE09
#define CONADDMUTRATE(DEFAULT) 0.09f		//Add connection rate for the cppn
#elif defined CONADDMUTRATE11
#define CONADDMUTRATE(DEFAULT) 0.11f		//Add connection rate for the cppn
#else
#define CONADDMUTRATE(DEFAULT) DEFAULT		//Add connection rate for the cppn
#endif


#ifdef CONDELMUTRATE01
#define CONDELMUTRATE(DEFAULT) 0.01f		//Delete connection rate for the cppn
#elif defined CONDELMUTRATE001
#define CONDELMUTRATE(DEFAULT) 0.001f		//Add connection rate for the cppn
#elif defined CONDELMUTRATE005
#define CONDELMUTRATE(DEFAULT) 0.005f		//Add connection rate for the cppn
#elif defined CONDELMUTRATE03
#define CONDELMUTRATE(DEFAULT) 0.03f		//Delete connection rate for the cppn
#elif defined CONDELMUTRATE05
#define CONDELMUTRATE(DEFAULT) 0.05f		//Delete connection rate for the cppn
#elif defined CONDELMUTRATE07
#define CONDELMUTRATE(DEFAULT) 0.07f		//Delete connection rate for the cppn
#elif defined CONDELMUTRATE09
#define CONDELMUTRATE(DEFAULT) 0.09f		//Delete connection rate for the cppn
#elif defined CONDELMUTRATE11
#define CONDELMUTRATE(DEFAULT) 0.11f		//Delete connection rate for the cppn
#else
#define CONDELMUTRATE(DEFAULT) DEFAULT		//Delete connection rate for the cppn
#endif


#ifdef MUTRATE05
#define CONCHANGEMUTRATE(DEFAULT) 0.05f			//Mutation rate of the cppn network weights and biases
#elif defined MUTRATE10
#define CONCHANGEMUTRATE(DEFAULT) 0.1f			//Mutation rate for the cppn nodes
#elif defined MUTRATE15
#define CONCHANGEMUTRATE(DEFAULT) 0.15f			//Mutation rate for the cppn nodes
#elif defined MUTRATE20
#define CONCHANGEMUTRATE(DEFAULT) 0.2f			//Mutation rate for the cppn nodes
#elif defined MUTRATE25
#define CONCHANGEMUTRATE(DEFAULT) 0.25f			//Mutation rate for the cppn nodes
#elif defined MUTRATE30
#define CONCHANGEMUTRATE(DEFAULT) 0.30f			//Mutation rate for the cppn nodes
#else
#define CONCHANGEMUTRATE(DEFAULT) DEFAULT			//Mutation rate for the cppn nodes
#endif


#if defined(PR00)
#define PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.0, 1.0, 1.0);
#elif defined(PR10)
#define PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.1, 1.0, 1.0);
#elif defined(PR25)
#define	PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.25, 1.0, 1.0);
#elif defined(PR50)
#define PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.5, 1.0, 1.0);
#elif defined(PR75)
#define PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.75, 1.0, 1.0);
#elif defined(PR95)
#define PNSGA_PROB(DEFAULT) SFERES_ARRAY(float, obj_pressure, 1.0, 0.95, 1.0, 1.0);
#else //defined(PR100)
#define PNSGA_PROB(DEFAULT) DEFAULT
#endif

#endif /* MUT_SWEEP_MACROS_HPP_ */
