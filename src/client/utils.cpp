/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.cpp
 * @author qb.wu@outlook.com
 * @date 2019/10/16 15:52:30
 * @brief
 *
 **/

#include "utils.h"

namespace offline {
namespace sre {
namespace rm {

std::string strfdate(time_t t, const std::string &format) {
    std::tm t2;
    localtime_r(&t, &t2);
    char buf[128];
    if (std::strftime(buf, sizeof(buf), format.c_str(), &t2) >= sizeof(buf)) {
        LOG(ERROR) << "The date string may be truncated, format: " << format;
    }
    return buf;
}

std::string inverse_escape_string(const std::string &escp_str) {
    // Refer to:
    // https://www.gnu.org/software/bash/manual/html_node/Quoting.html#Quoting
    static const auto SINGLE_Q = R"(')";

    static const auto DOUBLE_Q = R"(")";
    static const auto RAW_DOUBLE_Q = R"(\")";

    static const auto BACK_Q = R"(`)";
    static const auto RAW_BACK_Q = R"(\`)";

    static const auto DOLLAR = R"($)";
    static const auto RAW_DOLLAR = R"(\$)";

    static const auto DOUBLE_SLASH = R"(\\)";
    static const auto RAW_DOUBLE_SLASH = R"(\\\\)";

    if (escp_str.find(SINGLE_Q) == std::string::npos) {
        return SINGLE_Q + escp_str + SINGLE_Q;
    }
    auto res = escp_str;
    algo::replace_all(res, DOUBLE_Q, RAW_DOUBLE_Q);
    algo::replace_all(res, BACK_Q, RAW_BACK_Q);
    algo::replace_all(res, DOLLAR, RAW_DOLLAR);
    algo::replace_all(res, DOUBLE_SLASH, RAW_DOUBLE_SLASH);
    return DOUBLE_Q + res + DOUBLE_Q;
}

} // rm
} // sre
} // offline
