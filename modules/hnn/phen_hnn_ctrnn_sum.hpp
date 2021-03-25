/*
 * phen_hnn_ctrnn_sum.hpp
 *
 *  Created on: Jul 19, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_HNN_PHEN_HNN_CTRNN_SUM_HPP_
#define MODULES_HNN_PHEN_HNN_CTRNN_SUM_HPP_

#include <boost/algorithm/clamp.hpp>

#include "phen_hnn_ctrnn_winner_takes_all.hpp"

namespace sferes
{
  namespace phen
  {
    namespace hnn
    {


     // hyperneat-inspired phenotype, based on a cppn
    template <typename Substrate, typename Gen, typename Fit, typename Params, typename Exact = stc::Itself>
    class HnnCtrnnSum : public HnnCtrnnWinnerTakesAll<Substrate, Gen, Fit, Params, typename stc::FindExact<HnnCtrnnSum<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>
    {
    public:
        typedef HnnCtrnnWinnerTakesAll<Substrate, Gen, Fit, Params, typename stc::FindExact<HnnCtrnnSum<Substrate, Gen, Fit, Params, Exact>, Exact>::ret> parent_t;
        typedef typename parent_t::const_t const_t;

        friend class HnnCtrnnWinnerTakesAll<Substrate, Gen, Fit, Params, typename stc::FindExact<HnnCtrnnSum<Substrate, Gen, Fit, Params, Exact>, Exact>::ret>;
    protected:
        float _getWeight(std::vector<float>& input){
            dbg::trace trace("develop", DBG_HERE);
            float weight_output = 0;
            size_t nb_of_outputsets = this->getNbOfOutputSets();
            float threshold = this->getExpressionThreshold();

            for(size_t output_set = 0; output_set < nb_of_outputsets; ++output_set){
                size_t offset = output_set * const_t::nb_of_cppn_outputs;
                dbg::out(dbg::info, "develop") << "Connection expression output queried: " << const_t::expression_output_index + offset << std::endl;
                float expression = this->gen().conn_value(const_t::expression_output_index + offset, input);
                dbg::out(dbg::info, "develop") << "Connection expression: " << expression << std::endl;
                if(expression > threshold){
                    dbg::out(dbg::info, "develop") << "Weight output queried: " << const_t::weight_output_index + offset << std::endl;
                    weight_output += this->gen().conn_value(const_t::weight_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Weight output: " << weight_output << std::endl;
#ifdef JHDEBUG
                    if (std::isnan(weight_output)){
                        std::cerr << "********* Warning: weight is nan *********" <<std::endl;
                        std::cerr << "********* Result: " << weight_output << "*********" << std::endl;
                    }
#endif
                }
            }

            return boost::algorithm::clamp(weight_output, -1.0, 1.0);
        }

        std::pair<float, float> _getBiasAndTimeconstant(std::vector<float>& input){
            dbg::trace trace("develop", DBG_HERE);
            std::pair<float, float> result(0.0, 0.0);
            size_t nb_of_outputsets = this->getNbOfOutputSets();
            float threshold = this->getExpressionThreshold();

            for(size_t output_set = 0; output_set < nb_of_outputsets; ++output_set){
                size_t offset = output_set * const_t::nb_of_cppn_outputs;
                dbg::out(dbg::info, "develop") << "Node expression output queried: " << const_t::expression_output_index + offset << std::endl;
                float expression = this->gen().conn_value(const_t::expression_output_index + offset, input);
                dbg::out(dbg::info, "develop") << "Node expression: " << expression << std::endl;
                if(expression > threshold){
                    dbg::out(dbg::info, "develop") << "Bias output queried: " << const_t::bias_output_index + offset << std::endl;
                    result.first += this->gen().conn_value(const_t::bias_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Bias output: " << result.first << std::endl;

                    dbg::out(dbg::info, "develop") << "Time constant output queried: " << const_t::time_constant_output_index + offset << std::endl;
                    result.second += this->gen().conn_value(const_t::time_constant_output_index + offset, input);
                    dbg::out(dbg::info, "develop") << "Time constant output: " << result.second << std::endl;
                }
            }

            result.first = boost::algorithm::clamp(result.first, -1.0, 1.0);
            result.second = boost::algorithm::clamp(result.second, -1.0, 1.0);

            return result;
        }
    };

    }
  }
}

#endif /* MODULES_HNN_PHEN_HNN_CTRNN_SUM_HPP_ */
