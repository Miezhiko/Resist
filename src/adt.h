#pragma once

#include "pattern_test.h"
#include "type.h"
#include "pattern_match.h"

#include <boost/preprocessor.hpp>

typedef std::tuple<> unit;

template<typename T>
struct remove_single_tuple {
  typedef T type;
};

template<typename T>
struct remove_single_tuple<std::tuple<T>> {
  typedef T type;
};

template<typename ... T>
struct make_algebraic_data_type {
  typedef algebraic_data_type<typename remove_single_tuple<T>::type ...> type;
};
