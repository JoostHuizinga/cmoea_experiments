/*
 * formatting_test.cpp
 *
 *  Created on: Sep 23, 2016
 *      Author: Joost Huizinga
 */

#ifndef STATIC_BOOST
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE misc_formatting
#include <boost/test/unit_test.hpp>
#include "formatting.hpp"
using namespace modules::misc;

BOOST_AUTO_TEST_CASE(misc_formatting_test)
{
    std::cout << "Pad 0.4 to 3 chars: " << formatting::pad(0.4, 3) << std::endl;
    std::cout << "Pad 0.4 to 5 chars: " << formatting::pad(0.4, 5) << std::endl;
    std::cout << "Pad 12.4 to 5 chars: " << formatting::pad(12.4, 5) << std::endl;
    BOOST_CHECK_EQUAL(formatting::pad(0.4, 3), "0.4");
    BOOST_CHECK_EQUAL(formatting::pad(0.4, 5), "0.400");
    BOOST_CHECK_EQUAL(formatting::pad(12.4, 5), "12.40");
}
