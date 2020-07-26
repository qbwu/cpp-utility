/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file cmdline_parser.cpp
 * @author qb.wu@outlook.com
 * @date 2019/10/14 14:14:55
 * @brief
 *
 **/

#include "cmdline_parser.h"
#include <cassert>
#include <boost/type_index.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <str/str_utils.h>

namespace {

namespace po = boost::program_options;
namespace sty = po::command_line_style;

po::typed_value<std::string>*(*value)() = &po::value<std::string>;
po::typed_value<std::vector<std::string>>*(*value_list)()
        = &po::value<std::vector<std::string>>;

const auto GLOBAL_CMDLINE_STYLE = sty::allow_short | sty::allow_dash_for_short // -k
                                | sty::short_allow_next // -k v
                                | sty::allow_long // --key
                                | sty::long_allow_next // --key v
                                | sty::long_allow_adjacent // --key=v
                                | sty::allow_long_disguise; // -key v

void print_help_msg(std::ostream &os, const po::options_description &desc) {
    std::ostringstream oss;
    oss << desc;
    auto desc_str = oss.str();
    boost::algorithm::replace_all(desc_str, "--", "-");
    os << "\nUsage: rm-client [shell options] command [generic options] [command options]\n\n"
       << desc_str
       << "\nFor more information: qb.wu@outlook.com\n";
}

} // anonymous

namespace offline {
namespace sre {
namespace rm {

const std::string ParsedCommand::CMD_KEY_COMMAND = "command";
const std::string ParsedCommand::CMD_KEY_RESOURCE_ID = "resource_id";
const std::string ParsedCommand::CMD_KEY_CONFIG = "config";
const std::string ParsedCommand::CMD_KEY_TOKEN = "token";

bool ParsedCommand::parse(int argc, char **argv) {
    // Clear the members at first
    *this = ParsedCommand{};

    auto value_error = false;

    po::options_description visible_opts("Allowed options");
    // Define the optioin descriptions
    po::options_description shell_opts("Shell options");
    shell_opts.add_options()
        ("help,h", "print this help message")
        (CMD_KEY_CONFIG.c_str(), value()->value_name("<path>")->default_value(
            get_default_conf_path()), "path of the config directory")
        (CMD_KEY_RESOURCE_ID.c_str(), value()->value_name("<id>")->default_value(""),
            "resource id")
        (CMD_KEY_TOKEN.c_str(), value()->value_name("<RM token string>")->default_value(""),
            "only necessary for a few functions");

    po::options_description generic_opts("Generic options");
    generic_opts.add_options()
        (HADOOP_CMD_ARCHIVES.c_str(), value()->value_name("<comma separated list of archives>"),
            "specify comma separated archives to be unarchived on the compute machines")
        (HADOOP_CMD_FILES.c_str(), value()->value_name("<comma separated list of files>"),
            "specify comma separated files to be copied to the map reduce cluster")
        (HADOOP_CMD_LIBJARS.c_str(), value()->value_name("<comma separated list of files>"),
            "Specify comma-separated jar files to include in th e classpath")
        (HADOOP_CMD_CONF.c_str(), value()->value_name("<configuration file>"),
            "specify an application configuration file")
        (HADOOP_CMD_D.c_str(), value_list()->value_name("<property=value>")->notifier(
            [this, &value_error](const std::vector<std::string> &v) {
                value_error |= !this->_check_opt_eq_values(v);
            }), "use value for given property");

    po::options_description hidden_opts("Hidden optioins");
    hidden_opts.add_options()
        // The default value is necessary for users may not set the command.
        (CMD_KEY_COMMAND.c_str(), value()->default_value(""))
        ("posargs", value_list());

    po::positional_options_description pos;
    pos.add(CMD_KEY_COMMAND.c_str(), 1);
    pos.add("posargs", -1);

    visible_opts.add(shell_opts).add(generic_opts);

    po::options_description cmdline_opts("All options");
    cmdline_opts.add(visible_opts).add(hidden_opts);

    // Begin to parse the options
    std::vector<po::option> opts;
    std::vector<std::string> remains;
    try {
        auto parsed = po::command_line_parser(argc, argv).options(cmdline_opts)
            .positional(pos).allow_unregistered().style(GLOBAL_CMDLINE_STYLE).run();
        po::store(parsed, _vm);
        // Exit at once when the user just asks for help.
        if (_vm.count("help")) {
            print_help_msg(std::cout, visible_opts);
            return true;
        }
        po::notify(_vm);
        opts = std::move(parsed.options);

        if (value_error) {
            print_help_msg(std::cerr, visible_opts);
            return false;
        }

        if (!_check_opt_group_order(opts, shell_opts, generic_opts)) {
            print_help_msg(std::cerr, visible_opts);
            return false;
        }

        remains = po::collect_unrecognized(opts, po::include_positional);
    } catch (const po::error &ex) {
        print_help_msg(std::cerr, visible_opts);
        return false;
    }

    // Further parse the content of cmdargs based on the command value.
    // The beginning element is the command value, that is streaming or fs
    if (!remains.empty()) {
        remains.erase(remains.begin());
    }
    auto command = _vm[CMD_KEY_COMMAND].as<std::string>();
    if (command == HADOOP_CMD_FUNC_FS) {
        if (!_parse_fs_cmd(remains)) {
            return false;
        }
    } else if (command == HADOOP_CMD_FUNC_STREAMING) {
        if (!_parse_streaming_cmd(remains)) {
            return false;
        }
    } else if (command == HADOOP_CMD_FUNC_JOB) {
        if (!_parse_job_cmd(remains)) {
            return false;
        }
    } else if (command == RM_CMD_FUNC_INFO) {
        if (!_parse_info_cmd(remains)) {
            return false;
        }
    } else {
        std::cerr << "Unrecognized command, expect [fs|streaming].\n";
        print_help_msg(std::cerr, visible_opts);
        return false;
    }
    if (!_extract_opt_keys(opts, shell_opts, _shell_opts) ||
            !_extract_opt_keys(opts, generic_opts, _generic_opts)) {
        print_help_msg(std::cerr, visible_opts);
        return false;
    }
    return true;
}

bool ParsedCommand::_parse_info_cmd(const std::vector<std::string> &args) {
    po::options_description info_opts("Info options");

    auto single_flag = []() {
        return po::value<bool>()->zero_tokens()->implicit_value(true);
    };

    info_opts.add_options()
        (RM_CMD_PATH.c_str(), value_list()->value_name("<path>")->multitoken())
        (RM_CMD_KEYS.c_str(), single_flag())
        (RM_CMD_MR_HOST.c_str(), single_flag())
        (RM_CMD_FS_HOST.c_str(), single_flag());

    if (args.empty()) {
        print_help_msg(std::cerr, info_opts);
        return false;
    }

    std::vector<po::option> opts;
    try {
        auto parsed = po::command_line_parser(args).options(info_opts)
            .style(GLOBAL_CMDLINE_STYLE).run();
        po::store(parsed, _vm);
        _vm.notify();

        opts = std::move(parsed.options);
    } catch (const po::error &ex) {
        std::cerr << "Parse error: " << ex.what() << "\n";
        print_help_msg(std::cerr, info_opts);
        return false;
    }
    if (!_extract_opt_keys(opts, info_opts, _command_opts)) {
        print_help_msg(std::cerr, info_opts);
        return false;
    }
    return true;
}

bool ParsedCommand::_parse_streaming_cmd(const std::vector<std::string> &args) {
    auto value_error = false;

    po::options_description streaming_opts("Streaming options");
    streaming_opts.add_options()
        (HADOOP_CMD_MAPPER.c_str(),
            value()->value_name("<runnable map command> [required]")->required())
        (HADOOP_CMD_REDUCER.c_str(),
            value()->value_name("<runnable reduce command>"))
        (HADOOP_CMD_OUTPUTFORMAT.c_str(), value()->value_name("<outputformat classname>"))
        (HADOOP_CMD_PARTITIONER.c_str(), value()->value_name("<partitioner classname>"))
        (HADOOP_CMD_INPUTFORMAT.c_str(), value()->value_name("<inputformat classname>"))
        (HADOOP_CMD_OUTPUT.c_str(), value()->value_name("<path> [required]")->required())
        (HADOOP_CMD_CACHEARCHIVE.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_CACHEFILE.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_FILE.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_INPUT.c_str(), value_list()->value_name(
            "<path> [required]")->required()->multitoken())

        (HADOOP_CMD_CMDENV.c_str(), value_list()->value_name("<n>=<v>")->notifier(
            [this, &value_error](const std::vector<std::string> &v) {
                value_error |= !this->_check_opt_eq_values(v);
            }));

    std::vector<po::option> opts;
    try {
        auto parsed = po::command_line_parser(args).options(streaming_opts)
            .style(GLOBAL_CMDLINE_STYLE).run();
        po::store(parsed, _vm);
        _vm.notify();

        opts = std::move(parsed.options);
    } catch (const po::error &ex) {
        std::cerr << "Parse error: " << ex.what() << "\n";
        print_help_msg(std::cerr, streaming_opts);
        return false;
    }

    if (args.empty() || value_error) {
        print_help_msg(std::cerr, streaming_opts);
        return false;
    }

    if (!_extract_opt_keys(opts, streaming_opts, _command_opts)) {
        print_help_msg(std::cerr, streaming_opts);
        return false;
    }
    return true;
}

bool ParsedCommand::_parse_fs_cmd(const std::vector<std::string> &args) {
    auto value_error = false;

    po::options_description visible_opts("FS options");
    visible_opts.add_options()
        // set multitoken to enable such cmdline: -cat path1 path2 path3
        (HADOOP_CMD_CAT.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_TEXT.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_LS.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_DUS.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_RMR.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_TOUCHZ.c_str(), value_list()->value_name("<path>")->multitoken())
        (HADOOP_CMD_MKDIR.c_str(), value_list()->value_name("<path>")->multitoken())

        (HADOOP_CMD_MV.c_str(), value_list()->value_name(
        "<local srcpath> <local dstpath>")->multitoken()
            ->notifier([this, &value_error](const std::vector<std::string> &v) {
                value_error |= !this->_check_opt_mv_values(v); }))

        (HADOOP_CMD_GETMERGE.c_str(), value_list()->value_name(
        "<remote srcpath> <local dstpath>")->multitoken()
            ->notifier([this, &value_error](const std::vector<std::string> &v) {
                value_error |= !this->_check_opt_getmerge_values(v); }))

        (HADOOP_CMD_PUT.c_str(), value_list()->value_name(
        "<local srcpath> <remote dstpath>")->multitoken()
            ->notifier([this, &value_error](const std::vector<std::string> &v) {
                value_error |= !this->_check_opt_put_values(v); }))

        (HADOOP_CMD_TEST.c_str(), value()->value_name("-[ezd] <path>")->zero_tokens()
            ->implicit_value(args.size() < 2 ? "" : args[1]));

    po::options_description hidden_opts("Hidden FS options");
    hidden_opts.add_options()
        (HADOOP_CMD_TEST_E.c_str(), value_list()->multitoken())
        (HADOOP_CMD_TEST_D.c_str(), value_list()->multitoken())
        (HADOOP_CMD_TEST_Z.c_str(), value_list()->multitoken());

    po::options_description fs_opts("All FS options");
    fs_opts.add(visible_opts).add(hidden_opts);

    if (args.empty() || value_error) {
        print_help_msg(std::cerr, visible_opts);
        return false;
    }

    std::vector<po::option> opts;
    try {
        auto parsed = po::command_line_parser(args).options(fs_opts)
            .style(GLOBAL_CMDLINE_STYLE).run();
        po::store(parsed, _vm);
        po::notify(_vm);

        opts = std::move(parsed.options);
    } catch (const po::error &ex) {
        std::cerr << "Parse error: " << ex.what() << "\n";
        print_help_msg(std::cerr, visible_opts);
        return false;
    }

    if (_vm.count(HADOOP_CMD_TEST)) {
        auto flag = _vm[HADOOP_CMD_TEST].as<std::string>();
        auto hit = false;
        for (auto &elem : {HADOOP_CMD_TEST_E, HADOOP_CMD_TEST_D, HADOOP_CMD_TEST_Z}) {
            hit |= (flag == HADOOP_CMD_FLAG_PREFIX + elem);
        }
        if (!hit) {
            std::cerr << "Unrecognized flag with -test, expect -[ezd].\n";
            print_help_msg(std::cerr, visible_opts);
            return false;
        }
    }
    if (!_extract_opt_keys(opts, fs_opts, _command_opts)) {
        print_help_msg(std::cerr, visible_opts);
        return false;
    }
    return true;
}

bool ParsedCommand::_parse_job_cmd(const std::vector<std::string> &args) {
    po::options_description job_opts("Job options");

    job_opts.add_options()
        (HADOOP_CMD_STATUS.c_str(), value()->value_name("<job-id>"));

    if (args.empty()) {
        print_help_msg(std::cerr, job_opts);
        return false;
    }

    std::vector<po::option> opts;
    try {
        auto parsed = po::command_line_parser(args).options(job_opts)
                .style(GLOBAL_CMDLINE_STYLE).run();
        po::store(parsed, _vm);
        _vm.notify();

        opts = std::move(parsed.options);
    } catch (const po::error &ex) {
        std::cerr << "Parse error: " << ex.what() << "\n";
        print_help_msg(std::cerr, job_opts);
        return false;
    }
    if (!_extract_opt_keys(opts, job_opts, _command_opts)) {
        print_help_msg(std::cerr, job_opts);
        return false;
    }
    return true;
}

bool ParsedCommand::_check_opt_eq_values(
        const std::vector<std::string> &values) const {
    auto ok = true;

    enum {KEY = 0, VALUE, LEN};
    for (auto &elem : values) {
        std::vector<std::string> fields;
        fields.reserve(LEN);
        str::split(elem, HADOOP_CMD_D_SEP, std::back_inserter(fields));
        if (fields.size() != LEN) {
            std::cerr << "Wrong option value: " << elem
                      << ", should match -D <property=value>\n";
            ok = false;
        } else if (fields[KEY] == HADOOP_CMD_D_KEY_QUEUE_NAME
                || fields[VALUE] == HADOOP_CMD_D_KEY_UGI) {
            std::cerr << "Unsupported property in -D: " << fields[KEY] << "\n";
            ok = false;
        }
    }
    return ok;
}

bool ParsedCommand::_check_opt_mv_values(
        const std::vector<std::string> &values) const {
    if (values.size() != 2) {
        std::cerr << "Wrong option value: -mv <remote srcpath> <remote dstpath>\n";
        return false;
    }
    return true;
}

bool ParsedCommand::_check_opt_getmerge_values(
        const std::vector<std::string> &values) const {
    if (values.size() != 2) {
        std::cerr << "Wrong option value: -getmerge <remote srcpath> <local dstpath>\n";
        return false;
    }
    return true;
}

bool ParsedCommand::_check_opt_put_values(
        const std::vector<std::string> &values) const {
    if (values.size() != 2) {
        std::cerr << "Wrong option value: -getmerge <local srcpath> <remote dstpath>\n";
        return false;
    }
    return true;
}

bool ParsedCommand::_check_opt_group_order(
        const std::vector<po::option> &parsed_opts,
        const po::options_description &shell_opts,
        const po::options_description &generic_opts) const {
    auto opt_ord = [&shell_opts, &generic_opts](const po::option &opt) {
        if (shell_opts.find_nothrow(opt.string_key, false) != nullptr) {
            return 0;
        } else if (opt.position_key == 0) {
            return 1;
        } else if (generic_opts.find_nothrow(opt.string_key, false) != nullptr) {
            return 2;
        } else {
            return 3;
        }
    };
    auto last = -1;
    for (const auto &elem : parsed_opts) {
        auto curr = opt_ord(elem);
        if (curr < last) {
            std::cerr << "Wrong option group order: " << elem.string_key << "\n";
            return false;
        }
        last = curr;
    }
    return true;
}

bool ParsedCommand::_extract_opt_keys(
        const std::vector<po::option> &parsed_opts,
        const po::options_description &desc,
        std::unordered_set<std::string> &res) const {
    for (auto &opt : parsed_opts) {
        try {
            if (desc.find_nothrow(opt.string_key, false) != nullptr) {
                res.insert(opt.string_key);
            }
        } catch (po::error &ex) {
            std::cerr << "Wrong option '" << str::join(" ",
                opt.original_tokens.begin(), opt.original_tokens.end())
                << "', error: " << ex.what() << "\n";
            return false;
        }
    }
    return true;
}

std::vector<std::string> ParsedCommand::opt_values(const std::string &key) const {
    namespace tx = boost::typeindex;
    auto &v = _vm[key];
    if (v.empty()) {
        return {};
    }
    auto &vt = v.value().type();
    if (vt == tx::type_id<std::string>().type_info()) {
        return { v.as<std::string>() };
    } else {
        return v.as<std::vector<std::string>>();
    }
}

std::string ParsedCommand::flags(const std::string &prefix,
        const std::string &key, std::vector<std::string> values,
        const std::string &multitoken_sep) {
    auto flag = prefix + key;
    if (values.empty()) {
        return flag;
    }
    // Convert the escaped flag values as it is passed by RMClient.
    for (auto &item : values) {
        item = inverse_escape_string(item);
    }

    if (values.size() == 1) {
        return flag + " " + values[0];
    }
    if (multitoken_sep.empty()) {
        // -k v -k v -k v
        return flag + " " + str::join(" " + flag + " ", values.begin(), values.end());
    }
    // -k v,v,v or -k v v v
    return flag + " " + str::join(multitoken_sep, values.begin(), values.end());
}

void ParsedCommand::debug(std::ostream &os) const {
    os << "\n\n";
    for (auto &opt : _vm) {
        os << opt.first << ":";
        try {
            os << " " << opt.second.as<std::string>();
        } catch (...) {
            auto values = opt.second.as<std::vector<std::string>>();
            for (auto &val : values) {
                os << " " << val;
            }
        }
        os << "\n";
    }
    os << "\nshell options:";
    for (auto &k : _shell_opts) {
        os << " " << k;
    }
    os << "\ngeneric options: ";
    for (auto &k : _generic_opts) {
        os << " " << k;
    }
    os << "\ncommand options: ";
    for (auto &k : _command_opts) {
        os << " " << k;
    }
    os << "\n\n";
}

} // rm
} // sre
} // offline
