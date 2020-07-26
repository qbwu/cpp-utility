/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file wildcard_match.cpp
 * @author qb.wu@outlook.com
 * @date 2019/11/02 15:43:57
 * @brief
 *
 **/

#include <cassert>
#include "wildcard_match.h"

namespace offline {
namespace sre {
namespace rm {

bool Wildcard::_match(const char *expr, int expr_sz, const char *cont,
        int cont_sz) const {
    int i = 0;
    int j = 0;
    while (i < expr_sz && j < cont_sz) {
        switch (expr[i]) {
        case '*':
        {
            // Skip continuous '*'-s
            while (i < expr_sz && expr[i] == '*') { ++i; }
            auto k = j;
            for (; k < cont_sz && cont[k] != '/'; ++k) {
                // If '*' matches cont[j: k]
                if (_match(expr + i, expr_sz - i, cont + k, cont_sz - k)) {
                    return true;
                }
            }
            // If '*' matches cont[j: cont_sz] or cont[j: next_slash]
            return _match(expr + i, expr_sz - i, cont + k, cont_sz - k);
        }
        case '{':
        {
            auto match_sz = _match_bracket(expr + i, expr_sz - i, cont + j, cont_sz - j);
            if (match_sz.first > 0 && match_sz.second > 0) {
                i += match_sz.first;
                j += match_sz.second;
                break;
            } else if (match_sz.second <= 0) {
                return false;
            } else {
                // Not a close bracket structure
                [[fallthrough]];
            }
        }
        default: /* general characters */
            if (expr[i++] != cont[j++]) {
                return false;
            }
        }
    }
    // For the case 'abc*' <-> 'abc'
    while (i < expr_sz && (expr[i] == '*' || expr[i] == '/')) { ++i; }
    return i == expr_sz && j == cont_sz;
}

// return value: (length of the bracket patten expression,
//                length of the matched portion in the content)
std::pair<int, int> Wildcard::_match_bracket(
        const char *expr, int expr_sz, const char *cont, int cont_sz) const {
    assert(expr_sz != 0 && expr[0] == '{');

    int i = 0;
    int j = 0;
    do {
        ++i;
        auto tk_sz = _match_element(expr + i, expr_sz - i, cont, cont_sz);
        if (tk_sz.first >= 0) {
            i += tk_sz.first;
        } else {
            return {-1, -1};
        }
        if (tk_sz.second != 0) {
            j = tk_sz.second;

            assert(j <= cont_sz);
            cont_sz = 0;
        }
    } while (expr[i] == ',');

    assert(i < expr_sz && expr[i] == '}');
    return {i + 1, j};
}

// return value: (length of the current matching element in the bracket patten expression,
//                length of the matched portion in the content)
std::pair<int, int> Wildcard::_match_element(const char *expr, int expr_sz,
        const char *cont, int cont_sz) const {
    int i = 0;
    int j = 0;
    while (i < expr_sz) {
        switch (expr[i]) {
        // Invalid token
        case '/':
            return {-1, -1};

        // Terminal character
        case ',':
        case '}':
            assert(i < expr_sz && j <= cont_sz);
            return {i, j};

        case '{':
        {
            auto res = _match_bracket(expr + i, expr_sz - i, cont + j, cont_sz - j);
            if (res.first != 0) {
                i += res.first;
            } else {
                return {-1, -1};
            }

            if (res.second != 0) {
                j += res.second;
            } else {
                j = 0;
                cont_sz = 0;
            }
            break;
        }
        default:
            if (j < cont_sz && cont[j] == expr[i]) {
                ++j;
            } else {
                j = 0;
                cont_sz = 0;
            }
            ++i;
        }
    }
    // Missing terminal character.
    return {-1, -1};
}

} // rm
} // sre
} // offline
