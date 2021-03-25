/*
 * phen_hnn_ctrnn_two_params.hpp
 *
 *  Created on: Nov 6, 2013
 *      Author: joost
 */

#ifndef PHEN_HNN_CTRNN_TWO_PARAMS_HPP_
#define PHEN_HNN_CTRNN_TWO_PARAMS_HPP_

namespace sferes
{
namespace gen
{

/// in range [0;1]
template<typename Parameters1, typename Parameters2, typename Exact = stc::Itself>
class TwoParameters : public stc::Any<Exact>
{
public:
	typedef TwoParameters<Parameters1, Parameters2, Exact> this_t;
	Parameters1 parameters1;
	Parameters2 parameters2;

	TwoParameters(){

	}

    void mutate()
    {
    	parameters1.mutate();
    	parameters2.mutate();
    }

    void cross(const TwoParameters& o, TwoParameters& c1, TwoParameters& c2)
    {
    	Parameters1 parameters1c1;
    	Parameters1 parameters1c2;
    	parameters1.cross(o.paramters1, parameters1c1, parameters1c2);

    	Parameters2 parameters2c1;
    	Parameters2 parameters2c2;
    	parameters2.cross(o.paramters2, parameters2c1, parameters2c2);

    	//I think this should do the job, not sure though.
    	if (misc::flip_coin()){
    		c1.parameters1 = parameters1c1;
    		c1.parameters2 = parameters2c1;
    		c2.parameters1 = parameters1c2;
    		c2.parameters2 = parameters2c2;
    	} else {
    		c1.parameters1 = parameters1c1;
    		c1.parameters2 = parameters2c2;
    		c2.parameters1 = parameters1c2;
    		c2.parameters2 = parameters2c1;
    	}
    }

    void random()
    {
    	parameters1.random();
    	parameters2.random();
    }

    //Develop function
    void develop()
    {
    	parameters1.develop();
    	parameters2.develop();
    }

    float data(size_t i) const
    {
    	if(i < parameters1.size()){
    		return parameters1.data(i);
    	} else {
    		return parameters2.data(i-parameters1.size());
    	}
    }


    void  data(size_t i, float v)
    {
    	if(i < parameters1.size()){
    		parameters1.gen().data(i, v);
    	} else {
    		parameters2.gen().data(i-parameters1.size(), v);
    	}
    }

    size_t size() const { return parameters1.size() + parameters2.size(); }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	parameters1.serialize(ar, version);
    	parameters2.serialize(ar, version);
    }
};
}
}

#endif /* PHEN_HNN_CTRNN_TWO_PARAMS_HPP_ */
