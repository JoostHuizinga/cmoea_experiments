//| This file is a part of the sferes2 framework.
//| Copyright 2009, ISIR / Universite Pierre et Marie Curie (UPMC)
//| Main contributor(s): Jean-Baptiste Mouret, mouret@isir.fr
//|
//| This software is a computer program whose purpose is to facilitate
//| experiments in evolutionary computation and evolutionary robotics.
//|
//| This software is governed by the CeCILL license under French law
//| and abiding by the rules of distribution of free software.  You
//| can use, modify and/ or redistribute the software under the terms
//| of the CeCILL license as circulated by CEA, CNRS and INRIA at the
//| following URL "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and rights to
//| copy, modify and redistribute granted by the license, users are
//| provided only with a limited warranty and the software's author,
//| the holder of the economic rights, and the successive licensors
//| have only limited liability.
//|
//| In this respect, the user's attention is drawn to the risks
//| associated with loading, using, modifying and/or developing or
//| reproducing the software by the user in light of its specific
//| status of free software, that may mean that it is complicated to
//| manipulate, and that also therefore means that it is reserved for
//| developers and experienced professionals having in-depth computer
//| knowledge. Users are therefore encouraged to load and test the
//| software's suitability as regards their requirements in conditions
//| enabling the security of their systems and/or data to be ensured
//| and, more generally, to use and operate it in the same conditions
//| as regards security.
//|
//| The fact that you are presently reading this means that you have
//| had knowledge of the CeCILL license and that you accept its terms.




#ifndef AF_CPPN_UNSIGNED_HPP_
#define AF_CPPN_UNSIGNED_HPP_
#include <sferes/gen/sampled.hpp>
#include <sferes/gen/evo_float.hpp>
#include <iostream>
#include <fstream>

#include "gen_sampled_forbidden.hpp"


// classic activation functions
namespace nn
{
  namespace cppn
  {
    SFERES_CLASS(AfParamsFreezable){
      public:
        typedef AfParamsFreezable<Params, Exact> this_t;

        AfParamsFreezable():_frozen(false){}

        /**
         * Sets the activation function.
         *
         * @param t A float value representing the activation function.
         */
        void set(float t){dbg::trace trace("afparams", DBG_HERE); _type.data(0, t);}

        /**
         * Mutates the activation function.
         *
         * Randomly selects from:
         * - sine
         * - sigmoid
         * - gaussian
         * - linear
         */
        void mutate(){if(!_frozen) _type.mutate();}

        /**
         * Randomly selects an activation function.
         *
         * Effectively the same as mutate().
         */
        void random(){if(!_frozen) _type.random();}

        /**
         * Does nothing, the AfParams do not need to be developed.
         */
        void develop() {}

        /**
         * Returns the current activation function as an integer.
         *
         * @return An integer representing the activation function.
         */
        int type() const {return _type.data(0);}


        int data(int i) const {return _type.data(i);}

        /**
         * Returns 0.
         *
         * @return Returns 0.0,
         */
//        float param() const {return 0.0;}

        /**
         * Serializes the object, storing the activation function as a float.
         */
        template<typename A>
        void serialize(A& ar, unsigned int v){
          ar& BOOST_SERIALIZATION_NVP(_type);
#ifdef AF_HAS_PARAM
          sferes::gen::EvoFloat<1, Params> param;//todo What is its function?
          ar& BOOST_SERIALIZATION_NVP(param);
#endif

        }

        /**
         * Compares this object with an other object.
         *
         * Two objects are the same if, and only if, they represent the same activation function.
         *
         * @return True if the objects are the same, false otherwise.
         */
        bool operator==(const this_t &other) const {
          return _type == other._type;
        }


        void freeze(bool freeze=true){
            _frozen = freeze;
        }
      protected:
        sferes::gen::SampledForbidden<1, Params> _type;
        bool _frozen;
    };

  }
}


template<typename Params, typename Exact>
std::ostream& operator<< (std::ostream &out, nn::cppn::AfParamsFreezable<Params, Exact> &params)
{
    out << "type: " << params.type(); // << ", bias: " << params.param() ;
    return out;
}

#endif //AF_CPPN_UNSIGNED_HPP_
