/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file wildcard_match.h
 * @author qb.wu@outlook.com
 * @date 2019/11/02 15:32:43
 * @brief
 *
 **/

#include <iostream>
#include <map>
#include <boost/regex.hpp>

namespace offline {
namespace sre {
namespace rm {

class Wildcard {
public:
    explicit Wildcard(const std::string &expr)
            : _expr(expr) {}

    bool match(const std::string &content) const {
        return _match(
            _expr.data(), _expr.size(), content.data(), content.size());
    }

    std::size_t seg_num() const {
        return std::count(_expr.begin(), _expr.end(), '/');
    }
private:
    bool _match(const char *expr, int expr_sz,
            const char *cont, int cont_sz) const;

    std::pair<int, int> _match_bracket(const char *expr, int expr_sz,
            const char *cont, int cont_sz) const;

    std::pair<int, int> _match_element(const char *expr, int expr_sz,
            const char *cont, int cont_sz) const;

    bool _equal(const char *str1, const char *str2,
            std::size_t len) const {
        for (auto i = 0U; i < len; ++i) {
            if (str1[i] != str2[i]) {
                return false;
            }
        }
        return true;
    }

    std::string _expr;
};

} // rm
} // sre
} // offline
