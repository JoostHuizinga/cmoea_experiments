/*
 * formatting.hpp
 *
 *  Created on: Sep 23, 2016
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_FORMATTING_HPP_
#define MODULES_MISC_FORMATTING_HPP_

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-W#pragma-messages"
#elif __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#pragma-messages"
#endif
#endif

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_real.hpp>
#include <boost/lexical_cast.hpp>

namespace modules{
namespace misc{
namespace formatting{

using namespace boost::spirit::karma;

template <typename T>
struct real6_policy : real_policies<T> {
  static unsigned int precision(T) { return 6; }
  static int floatfield(T n) { return real_policies<T>::fmtflags::fixed; }
};

//template <typename RealT>
std::string pad(float value, int padding){
    real_generator<float, real6_policy<float> > pl;
    return boost::lexical_cast<std::string>(format(left_align(padding, '0')[maxwidth(padding)[pl]], value));
}

std::string pad(double value, int padding){
    real_generator<double, real6_policy<double> > pl;
    return boost::lexical_cast<std::string>(format(left_align(padding, '0')[maxwidth(padding)[pl]], value));
}

}
}
}

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#elif __clang__
#pragma clang diagnostic pop
#endif
#endif

#endif /* MODULES_MISC_FORMATTING_HPP_ */
