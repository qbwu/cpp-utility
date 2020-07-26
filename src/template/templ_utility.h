/***************************************************************************
 *
 * Copyright (c) 2016 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file templ_utility.h
 * @author qb.wu@outlook.com
 * @date 2016/09/12
 * @brief tools for parsing variable template parameters.
 **/

#ifndef TEMPL_UTILITY_H
#define TEMPL_UTILITY_H

#include <functional>

namespace offline {
namespace ckv {
namespace util {

template <typename Func, typename T>
void parse_template_args(Func&& func, T&& arg);

template <typename Func, typename T, typename...Ts>
void parse_template_args(Func&& func, T&& arg1, Ts&&...args);

} // util
} // ckv
} // offline

#include "templ_utility.hpp"

#endif // TEMPL_UTILITY_H
