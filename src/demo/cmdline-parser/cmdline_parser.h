/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file cmdline_parser.h
 * @author qb.wu@outlook.com
 * @date 2019/10/14 14:14:55
 * @brief
 *
 **/

#ifndef CMDLINE_PARSER_H
#define CMDLINE_PARSER_H

#include <iostream>
#include <vector>
#include <typeinfo>
#include <unordered_set>
#include <boost/program_options.hpp>

namespace offline {
namespace sre {
namespace rm {

namespace po = boost::program_options;

class ParsedCommand {
public:
    bool parse(int argc, char** argv);

    const std::unordered_set<std::string>& shell_opt_keys() const {
        return _shell_opts;
    }
    const std::unordered_set<std::string>& command_opt_keys() const {
        return _command_opts;
    }
    const std::unordered_set<std::string>& generic_opt_keys() const {
        return _generic_opts;
    }
    // These flag values always exist, guarenteed by the option description
    // defined in the parse method. In case they were not to exist (by mistake),
    // the functions could throw std::out_of_range exeption.
    std::string config() const { return opt_values(CMD_KEY_CONFIG)[0]; }
    std::string func() const { return opt_values(CMD_KEY_COMMAND)[0]; }
    std::string resource_id() const { return opt_values(CMD_KEY_RESOURCE_ID)[0]; }
    std::string token() const { return opt_values(CMD_KEY_TOKEN)[0]; }

    bool check_opt(const std::string &key) const {
        return _vm.find(key) != _vm.end();
    }

    std::vector<std::string> opt_values(const std::string &key) const;

    std::string flags(const std::string &prefix, const std::string &key,
            const std::string &multitoken_sep) const {
        return flags(prefix, key, opt_values(key), multitoken_sep);
    }

    template <typename T>
    bool set_opt(const std::string &key, const T &val);

    template <typename T>
    bool cmp_set_opt(const std::string &key, const T &comp, const T &val);

    static std::string flags(const std::string &prefix, const std::string &key,
            std::vector<std::string> values, const std::string &multitoken_sep);

    void debug(std::ostream &os) const;

private:
    bool _parse_streaming_cmd(const std::vector<std::string> &args);
    bool _parse_fs_cmd(const std::vector<std::string> &args);
    bool _parse_job_cmd(const std::vector<std::string> &args);
    bool _parse_info_cmd(const std::vector<std::string> &args);

    bool _check_opt_eq_values(const std::vector<std::string> &values) const;
    bool _check_opt_mv_values(const std::vector<std::string> &values) const;
    bool _check_opt_getmerge_values(const std::vector<std::string> &values) const;
    bool _check_opt_put_values(const std::vector<std::string> &values) const;

    bool _check_opt_group_order(const std::vector<po::option> &parsed_opts,
            const po::options_description &shell_opts,
            const po::options_description &generic_opts) const;

    bool _extract_opt_keys(const std::vector<po::option> &parsed_opts,
            const po::options_description &desc,
            std::unordered_set<std::string> &res) const;

    std::unordered_set<std::string> _shell_opts;
    std::unordered_set<std::string> _generic_opts;
    std::unordered_set<std::string> _command_opts;

    po::variables_map _vm;

    static const std::string CMD_KEY_CONFIG;
    static const std::string CMD_KEY_COMMAND;
    static const std::string CMD_KEY_RESOURCE_ID;
    static const std::string CMD_KEY_TOKEN;
};

template <typename T>
bool ParsedCommand::cmp_set_opt(const std::string &key, const T &comp, const T &val) {
    auto it = _vm.find(key);
    if (it == _vm.end()) {
        return false;
    }
    auto &v = it->second;
    return v.as<T>() == comp ? (v.value() = val, true) : false;
}

template <typename T>
bool ParsedCommand::set_opt(const std::string &key, const T &val) {
    auto it = _vm.find(key);
    if (it == _vm.end()) {
        return false;
    }
    it->second.value() = val;
    return true;
}

} // rm
} // sre
} // offline

#endif // CMDLINE_PARSER_H
