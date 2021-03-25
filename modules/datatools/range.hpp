/*
 * range.hpp
 *
 *  Created on: Sep 24, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_RANGE_HPP_
#define MODULES_MISC_RANGE_HPP_

/**
 * A range object storing a minimum, maximum and step-size.
 */
struct Range{
    Range(){
        min = 0;
        max = 0;
        step = 0;
    }
    float min;
    float max;
    float step;

    unsigned nb_of_bins(float tolerance = 0.00001){
        return unsigned(((max - min) / step) + tolerance) + 1;
    }

    unsigned get_bin(float value, float tolerance = 0.00001){
        return unsigned(((value - min) / step) + tolerance);
    }
};

std::ostream& operator<<(std::ostream& is, const Range& obj){
    is << "min: " << obj.min << " max: " << obj.max << " step: " << obj.step;
    return is;
}

/**
 * Takes a vector of floats and returns a range that includes the minimum,
 * maximum and smallest step in the vector.
 */
Range get_range(std::vector<float>& v, float tolerance = 0.00001){
    Range result;
    if(v.empty()){
        return result;
    }
    result.min = v.front();
    result.max = v.front();
    result.step = std::numeric_limits<float>::max();
    for(unsigned i=0; i<v.size(); ++i){
        for(unsigned j=i+1; j<v.size(); ++j){
            float difference = fabs(v[i] - v[j]);
            if(difference > tolerance){
                if(difference <  result.step){
                    result.step = difference;
                }

            }
        }
        if(v[i] <   result.min){
            result.min = v[i];
        }
        if(v[i] > result.max){
            result.max = v[i];
        }
    }
    return result;
}




#endif /* MODULES_MISC_RANGE_HPP_ */
