/***************************************************************************
 *
 * Copyright (c) 2017 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.cpp
 * @author qb.wu@outlook.com
 * @date 2017/04/22
 * @brief
 **/

#include "utils.h"
#include <algorithm>
#include <functional>
#include <set>
#include <gflags/gflags.h>
#include <google/protobuf/descriptor.h>

namespace offline {

namespace srv {

ErrInfo ErrInfo::get_internal_err() {
    ErrInfo err_info;
    err_info.err_code = baidu::rpc::EINTERNAL;
    err_info.status_code = baidu::rpc::HTTP_STATUS_INTERNAL_SERVER_ERROR;
    err_info.reason = "The server encountered an unexpected internal error";

    return err_info;
}

ErrInfo ErrInfo::get_not_found_err() {
    ErrInfo err_info;
    err_info.err_code = baidu::rpc::ENOMETHOD;
    err_info.status_code = baidu::rpc::HTTP_STATUS_NOT_FOUND;
    err_info.reason = "The server has not found any matched App";

    return err_info;
}

double CoverageRecorder::get_coverage(void *ptr) {
    if (ptr == nullptr) {
        return 0;
    }

    CoverageRecorder *recorder = static_cast<CoverageRecorder*>(ptr);
    int64_t no_empty_req_num = recorder->_no_empty_req_window.get_value();
    int64_t total_req_num = recorder->_total_req_window.get_value();

    if (total_req_num == 0) {
        return 0;
    } else {
        return static_cast<double>(no_empty_req_num) / total_req_num;
    }
}

bool LogIdGenerator::init(int32_t port) {
    std::array<char, MAX_HOST_LEN> host;
    if (gethostname(host.data(), host.size()) != 0) {
        ULOG(FATAL) << "Failed to get hostname when initializing LogIdGenerator.";
        return false;
    }

    _local_side.assign(host.data());
    _local_side.append(":");
    _local_side.append(std::to_string(port));

    return true;
}

std::string LogIdGenerator::get_log_id() {
    // Get current timestamp.
    auto tp = std::chrono::system_clock::now();
    std::string timestamp = std::to_string(std::chrono::system_clock::to_time_t(tp));

    std::string log_id_str = timestamp + _local_side + std::to_string(++_counter);
    return std::to_string(sign::create_sign_mm64(log_id_str.data(), log_id_str.size()));
}

bool execute(const std::string &cmd) {
    ULOG(DEBUG) << "Run shell cmd: " << cmd;

    // TODO get the stderr to a local variable,
    //  we can not redirect 2>&1 simply, that will make the 'hadoop fs -cat'
    //  fail when the hadoop client outputs the log of INFO level.
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        ULOG(WARNING) << "Failed to run cmd, err: " << strerror(errno);
        return false;
    }

    std::string result;
    char buf[1024];
    while (std::fgets(buf, sizeof(buf), fp)) {
        result += buf;
    }
    str::trim(result);

    // Here we use a shortcut validation to check if the cmd succeed.
    // Extract the real return status with macros WIFEXITED(ret) and WEXITSTATUS(ret) if needed.
    int ret = pclose(fp);
    if (ret == 0) {
        return true;
    } else if (ret == -1) {
        ULOG(WARNING) << "Failed to get return status, err: " << strerror(errno);
    } else {
        ULOG(WARNING) << "Failed to run cmd, stderr: " << result;
    }

    return false;
}

bool open_conf(const std::string &conf_path, comcfg::Configure &ret) {
    boost::filesystem::path tmp_conf_path(conf_path);
    auto conf_dir = tmp_conf_path.parent_path().string();
    auto conf_file = tmp_conf_path.filename().string();

    if (ret.load(conf_dir.data(), conf_file.data()) != 0) {
        LOG(WARNING) << "Failed to open the config: " << conf_path;
        return false;
    }
    return true;
}

bool init_channel(const comcfg::ConfigUnit &chan_conf, baidu::rpc::Channel &chan) {
    const auto DEFAULT_LOAD_BALANCER = "rr";

    baidu::rpc::ChannelOptions chan_opts;
    const auto &chan_opts_conf = chan_conf["Options"];
    // C style interface, the chan_conf will only be set if there are not any errors.
    chan_opts_conf["timeout_ms"].get_int32(&chan_opts.timeout_ms);
    chan_opts_conf["connect_timeout_ms"].get_int32(&chan_opts.connect_timeout_ms);
    chan_opts_conf["backup_request_ms"].get_int32(&chan_opts.backup_request_ms);
    chan_opts_conf["max_retry"].get_int32(&chan_opts.max_retry);

    comcfg::ErrCode err_code;
    auto *endpoint = chan_conf["EndPoint"].to_cstr(&err_code);

    if (err_code) {
        LOG(ERROR) << "Failed to get the config of endpoint, error: "
            << chan_conf.seeError(err_code);
        return false;
    }

    auto load_balancer = chan_conf["load_balancer"].to_cstr(&err_code);
    if (err_code) {
        load_balancer = DEFAULT_LOAD_BALANCER;
    }

    if (chan.Init(endpoint, load_balancer, &chan_opts) != 0) {
        LOG(ERROR) << "Failed to initialize RPC channel to the "
                      "endpoint: " << endpoint;
        return false;
    }
    return true;
}

}  // srv

}  // offline
