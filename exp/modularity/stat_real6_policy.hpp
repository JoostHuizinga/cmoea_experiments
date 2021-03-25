/*
 * stat_real6_policy.hpp
 *
 *  Created on: Jul 4, 2018
 *      Author: joost
 */

#ifndef EXP_MODULARITY_STAT_REAL6_POLICY_HPP_
#define EXP_MODULARITY_STAT_REAL6_POLICY_HPP_

template <typename T>
struct real6_policy : boost::spirit::karma::real_policies<T> {
  static unsigned int precision(T) { return 6; }
  static int floatfield(T n) { return boost::spirit::karma::real_policies<T>::fmtflags::fixed; }
};



#endif /* EXP_MODULARITY_STAT_REAL6_POLICY_HPP_ */
