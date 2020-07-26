/***************************************************************************
 *
 * Copyright (c) 2016 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file templ_utility.hpp
 * @author qb.wu@outlook.com
 * @date 2016/09/12
 * @brief Included by templ_utility.h
 **/

#ifndef TEMPL_UTILITY_HPP
#define TEMPL_UTILITY_HPP

#include <functional>
#include <type_traits>

namespace offline {
namespace ckv {
namespace util {

template <typename Func, typename T>
void parse_template_args(Func&& func, T&& arg) {
    func(std::forward<T>(arg));
}

template <typename Func, typename T, typename...Ts>
void parse_template_args(Func&& func, T&& arg1, Ts&&...args) {
    func(std::forward<T>(arg1));
    parse_template_args(std::forward<Func>(func), std::forward<Ts>(args)...);
}

} // util
} // ckv
} // offline

#endif // CKV_TEMPL_UTILITY_HPP

