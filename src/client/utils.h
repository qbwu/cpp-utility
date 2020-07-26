/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.h
 * @author qb.wu@outlook.com
 * @date 2019/10/14 14:16:50
 * @brief
 *
 **/

#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <iostream>
#include <boost/filesystem.hpp>

namespace offline {
namespace sre {
namespace rm {

std::string strfdate(std::time_t t, const std::string &format);

std::string inverse_escape_string(const std::string &escp_str);

inline std::string get_bin_path() {
    return fs::canonical("/proc/self/exe").parent_path().string();
}

inline std::string get_lib_path() {
    return fs::canonical("/proc/self/exe").parent_path()
            .parent_path().string() + "/lib";
}

inline std::string get_default_conf_path() {
    return fs::canonical("/proc/self/exe").parent_path()
            .parent_path().string() + "/conf";
}

} // rm
} // sre
} // offline

#endif // UTILS_H
