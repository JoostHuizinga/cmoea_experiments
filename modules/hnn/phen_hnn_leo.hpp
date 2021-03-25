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

#ifndef PHEN_HNN_LEO_HPP
#define PHEN_HNN_LEO_HPP

#include "phen_hnn.hpp"

namespace sferes
{
  namespace phen
  {
    namespace hnn
    {

    struct HnnLeoConstants{
        static const size_t weight_output_index = 0;
        static const size_t leo_output_index = 1;
        static const size_t nb_of_outputs_connection = 2;

        static const size_t bias_output_index = 0;
        static const size_t nb_of_outputs_neuron = 1;
    };

     // hyperneat-inspired phenotype, based on a cppn
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself>
    class HnnLeo : public Hnn<Substrate, Gen, Fit, Params, typename stc::FindExact<HnnLeo<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>
    {
      public:
        //Inherit parent type definition (unfortunately doesn't happen automatically)
        typedef Hnn<Substrate, Gen, Fit, Params, typename stc::FindExact<HnnLeo<Substrate, Gen, Fit, Params, Exact>, Exact>::ret> parent_t;

        typedef typename parent_t::gen_t gen_t;
        typedef typename parent_t::cppn_graph_t cppn_graph_t;
        typedef typename parent_t::cppn_nn_t cppn_nn_t;
        typedef typename parent_t::af_t af_t;

        typedef typename parent_t::nn_t nn_t;
        typedef typename parent_t::neuron_t neuron_t;
        typedef typename parent_t::neu_bais_t neu_bais_t;
        typedef typename parent_t::connection_t connection_t;
        typedef typename parent_t::con_weight_t con_weight_t;
        typedef typename parent_t::weight_t weight_t;
        typedef typename parent_t::vertex_desc_t vertex_desc_t;
        typedef typename parent_t::layer_t layer_t;
        typedef typename parent_t::graph_t graph_t;

        typedef typename parent_t::input_t input_t;
        typedef typename parent_t::pos_t pos_t;


        void _connectVertices(const size_t& offset, vertex_desc_t& source, vertex_desc_t& target, std::vector<float>& in){
            dbg::trace trace("hnn", DBG_HERE);
            float leo_float = this->_getLinkExpression(offset, in);
            if(leo_float > Params::cppn::leo_threshold){
                float weight_float = this->_getWeight(offset, in);
                weight_t weight;
                weight.gen().data(0, this->_scale(weight_float));
                weight.develop();

                dbg::out(dbg::info, "develop") << " weight: " << weight_float << " (raw) " <<
                        nn::trait<weight_t>::single_value(weight) << " (developed)" << std::endl;

                if(fabs(nn::trait<weight_t>::single_value(weight)) > Params::cppn::con_threshold){
                    this->_nn.add_connection(source, target, weight);
                }
            }
        }

        float _getLinkExpression(const size_t& offset, std::vector<float>& in){
            dbg::trace trace("hnn", DBG_HERE);
            size_t wo_index = offset + HnnLeoConstants::leo_output_index;
            float leo_output = this->gen().conn_value(wo_index, in);
#ifdef JHDEBUG
            if (std::isnan(leo_output)){
                std::cerr << "********* Warning: weight is nan *********" <<std::endl;
                std::cerr << "********* Result: " << leo_output << "*********" << std::endl;
            }
#endif
            return leo_output;
        }

    };
  }
}
}
#endif //PHEN_HNN_LEO_HPP
