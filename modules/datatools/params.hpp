/*
 * params.hpp
 *
 *  Created on: Sep 4, 2015
 *      Author: Joost Huizinga
 */

#ifndef MODULES_MISC_PARAMS_HPP_
#define MODULES_MISC_PARAMS_HPP_

#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#define SFERES_PARAMS(P)                                                    \
  struct Ps__ { Ps__() {                                                    \
                  static std::string __params = # P;                        \
                  boost::replace_all(__params, ";", ";\n");                 \
                  boost::replace_all(__params, "{", "{\n");                 \
                  boost::replace_all(__params, "static const", "");         \
                  std::cout << "Parameters:" << __params << std::endl; } }; \
  P;                                                                        

#define SFERES_VALUE_PARAM(NAME, VALUE)                                     \
  struct NAME {                                                             \
      NAME() {                                                              \
                  static std::string __params = # NAME # VALUE;             \
                  boost::replace_all(__params, ";", ";\n");                 \
                  boost::replace_all(__params, "{", "{\n");                 \
                  boost::replace_all(__params, "static const", "");         \
                  std::cout << "Parameters:" << __params;                   \
      }                                                                     \
      VALUE                                                                 \
  };                                                                        \
  static NAME __##NAME;

#define SFERES_TYPE_VALUE_PARAM(NAME, TYPE, VALUE)                          \
  struct NAME {                                                             \
      NAME() {                                                              \
                  static std::string __params = std::string(# NAME) + " " + \
                      std::string(# TYPE) + std::string("=") +              \
                      boost::lexical_cast<std::string>(VALUE) + ";\n";      \
                  std::cout << "Parameters:" << __params;                   \
      }                                                                     \
      TYPE = VALUE;                                                         \
  };                                                                        \
  static NAME __##NAME;

#define PRINT_PARAMS Ps__ ____p

//This goes right before main
//Struct should point to where the array is defined (e.g Params::dnn::spatial)
//Name should be the name of the array
//Doesn't actually do anything, but it is required for compilation (otherwise you get linking errors)
#define SFERES_DECLARE_NON_CONST_ARRAY(STRUCT, NAME) \
        STRUCT::NAME##_t* STRUCT::_##NAME = 0; \
        size_t STRUCT::_##NAME##_size = 0;

//This goes where you actually build the array
//Struct should point to where the array is defined (e.g Params::dnn::spatial)
//Name should be the name of the array
//Size should be the size of the array
//
//Note: because I did not implement a set function yet,
//you'll have to initialize the values of the array through the _NAME array pointer
//E.g. Params::dnn::spatial::_x[0] = 1
//Or an other example
//E.g. (size_t i=0; i<Params::dnn::spatial::x_size();++i) Params::dnn::spatial::_x[i] = i
#define SFERES_INIT_NON_CONST_ARRAY(STRUCT, NAME, SIZE)    \
        STRUCT::_##NAME = new STRUCT::NAME##_t[SIZE]; \
        STRUCT::_##NAME##_size = SIZE;

#define SFERES_INIT_NON_CONST_ARRAY_TEMPLATE(STRUCT, NAME, SIZE)    \
        STRUCT::_##NAME = new typename STRUCT::NAME##_t[SIZE]; \
        STRUCT::_##NAME##_size = SIZE;


//This goes where you would define your SFERES_ARRAY
//It only requires name and type
#define SFERES_NON_CONST_ARRAY(T, A)             \
        static T* _##A;                                \
        static size_t _##A##_size;                     \
        \
        static T A(size_t i)                   \
        { assert(i < A##_size()); return _##A[i]; }    \
        \
        static size_t A##_size()                 \
        { return _##A##_size; }                        \
        \
        static size_t A##_set(size_t i, T value) \
        { return _##A[i] = value; }                    \
        \
        typedef T A##_t;


#define SFERES_DECLARE_NON_CONST_MATRIX(STRUCT, NAME) \
        STRUCT::NAME##_t* STRUCT::_##NAME = 0;            \
        size_t STRUCT::_##NAME##_rows = 0;                \
        size_t STRUCT::_##NAME##_columns = 0;

#define SFERES_INIT_NON_CONST_MATRIX(STRUCT, NAME, COLUMNS, ROWS)  \
        STRUCT::_##NAME = new STRUCT::NAME##_t[COLUMNS*ROWS];          \
        STRUCT::_##NAME##_rows = ROWS;                                 \
        STRUCT::_##NAME##_columns = COLUMNS;

#define SFERES_INIT_NON_CONST_MATRIX_TEMPLATE(STRUCT, NAME, COLUMNS, ROWS)  \
        STRUCT::_##NAME = new typename STRUCT::NAME##_t[COLUMNS*ROWS];          \
        STRUCT::_##NAME##_rows = ROWS;                                 \
        STRUCT::_##NAME##_columns = COLUMNS;

#define SFERES_NON_CONST_MATRIX(T, A)                                 \
        static T* _##A;                                                     \
        static size_t _##A##_rows;                                          \
        static size_t _##A##_columns;                                       \
        \
        static T A(size_t i)                                        \
        { assert(i < A##_size()); return _##A[i]; }                         \
        \
        static T A(size_t column_index, size_t row_index)             \
        { assert(column_index < A##_columns());                             \
        assert(row_index < A##_rows());                                   \
        return A(column_index + row_index*A##_columns()); }               \
        \
        static void A##_set(size_t column_index, size_t row_index, T value) \
        { assert(column_index < A##_columns());                             \
        assert(row_index < A##_rows());                                   \
        _##A[column_index + row_index*A##_columns()] = value; }           \
        \
        static size_t A##_size()                                      \
        { return _##A##_rows*_##A##_columns; }                                \
        \
        static size_t A##_rows()                                      \
        { return _##A##_rows; }                                           \
        \
        static size_t A##_columns()                                   \
        { return _##A##_columns; }                                            \
        \
        typedef T A##_t;

// TODO: Move these declarations to somewhere else, such that other files including params.hpp
// Do not need to define the number of generations in this way
//SFERES_VALUE_PARAM(NbGenerations,
//#if defined(GEN50000)
//    static const size_t v = 50001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN25000)
//    static const size_t v = 25001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN10000)
//    static const size_t v = 10001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN5000)
//    static const size_t v = 5001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN4000)
//    static const size_t v = 4001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN2000)
//    static const size_t v = 2001;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN100)
//    static const size_t v = 101;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN25)
//        static const size_t v = 26;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN20)
//        static const size_t v = 21;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN10)
//    static const size_t v = 11;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN2)
//    static const size_t v = 2;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN1)
//    static const size_t v = 1;
//#define GEN_DEFINED
//#endif
//
//#if defined(GEN0)
//    static const size_t v = 0;
//#define GEN_DEFINED
//#endif
//
//#if not defined(GEN_DEFINED)
////#error "The number of generations is not defined."
//    static const size_t v = 0;
//#endif
//)

//IMPORTANT: Population size has to be divisible by 4
//SFERES_VALUE_PARAM(PopSize,
//#if defined(POP1000)
//    static const size_t v = 1000;
//#define POP_DEFINED
//#endif
//
//#if defined(POP100)
//    static const size_t v = 100;
//#define POP_DEFINED
//#endif
//
//#if defined(POP20)
//    static const size_t v = 20;
//#define POP_DEFINED
//#endif
//
//#if defined(POP12)
//    static const size_t v = 12;
//#define POP_DEFINED
//#endif
//
//#if defined(POP4)
//    static const size_t v = 4;
//#define POP_DEFINED
//#endif
//
//#if defined(POP0)
//    static const size_t v = 0;
//#define POP_DEFINED
//#endif
//
//#if not defined(POP_DEFINED)
////#error "The population size is not defined."
//    static const size_t v = 0;
//#endif
//)

#endif /* MODULES_MISC_PARAMS_HPP_ */
