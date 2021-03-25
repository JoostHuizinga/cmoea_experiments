/*
 * nn_conn_to_key.hpp
 *
 *  Created on: Mar 17, 2018
 *      Author: Joost Huizinga
 */

#ifndef EXP_MODULARITY_NN_CONN_TO_KEY_HPP_
#define EXP_MODULARITY_NN_CONN_TO_KEY_HPP_


inline std::string inputToHiddenKey(const uint& inputIndex,
		const uint& hiddenIndex){
    std::ostringstream stream;
    stream << "i" << inputIndex << '_' << hiddenIndex;
    return stream.str();
}

inline std::string hiddenToHiddenKey(
		const uint nb_of_hidden,
		const uint& hiddenIndex1,
		const uint& hiddenIndex2){
    std::ostringstream stream;
    if(hiddenIndex1 >= nb_of_hidden){
        stream << 'o' << (hiddenIndex1 - nb_of_hidden);
    } else {
        stream << hiddenIndex1;
    }
    stream << '_';

    if(hiddenIndex2 >= nb_of_hidden){
        stream << 'o' << (hiddenIndex2 - nb_of_hidden);
    } else {
        stream << hiddenIndex2;
    }
    return stream.str();
}


#endif /* EXP_MODULARITY_NN_CONN_TO_KEY_HPP_ */
