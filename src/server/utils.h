/***************************************************************************
 *
 * Copyright (c) 2017 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.h
 * @author qb.wu@outlook.com
 * @date 2017/04/22
 * @brief
 **/

#ifndef UTILS_H
#define UTILS_H

#include <atomic>
#include <fstream>
#include <vector>
#include <thread>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include <baidu/rpc/channel.h>
#include <bvar/bvar.h>
#include <pb_to_json.h>
#include <google/protobuf/repeated_field.h>
#include <str/str_utils.h>
#include <sign/sign.h>
#include <utils/date_utils.h>

namespace offline {

namespace srv {

struct ErrInfo {
    bool good() const {
        return err_code == 0;
    }

    static ErrInfo get_internal_err();
    static ErrInfo get_not_found_err();

    // See "baidu/rpc/errno.proto".
    int err_code = 0;

    // See "baidu/rpc/http_status_code.h".
    int status_code = baidu::rpc::HTTP_STATUS_OK;

    std::string reason;
};

class CoverageRecorder {
public:
    CoverageRecorder(const std::string &prefix,
            const std::string &name,
            time_t window_size) :
        _no_empty_req_window(&no_empty_req, window_size),
        _total_req_window(&total_req, window_size),
        _coverage(prefix, name, &CoverageRecorder::get_coverage, this) {}

    /* Compute coverage value. */
    static double get_coverage(void *);

    /* Record the num of requests with non-empty result and total requests. */
    bvar::Adder<int64_t> no_empty_req;
    bvar::Adder<int64_t> total_req;
private:
    /* Record the increment in a specified time window. */
    bvar::Window<bvar::Adder<int64_t>> _no_empty_req_window;
    bvar::Window<bvar::Adder<int64_t>> _total_req_window;

    /* Output coverage value when needed. */
    bvar::PassiveStatus<double> _coverage;
};

class LogIdGenerator {
public:
    LogIdGenerator() : _counter(0) {}

    bool init(int32_t port);

    /* Get a unique log id. */
    std::string get_log_id();

private:
    /* Local side: hostname : port. */
    std::string _local_side;

    /* Request counter, it would reset after reaching max value. */
    std::atomic<uint32_t> _counter;
};

bool execute(const std::string &cmd);

inline int run_cmd(const std::string &cmd) {
    // The child process will inherit the stdin/stdout/stderr of
    // the current process, which is just what we need.
    auto ret = system(cmd.c_str());
    LOG(NOTICE) << "Run shell cmd: { " << cmd << " } [ret_code:=" << ret
        << "][errno:=" << errno << "] err_text: { "
        << std::strerror(errno) << " }";
    return ret;
}

inline void set_response_failed(const ErrInfo &err_info,
        GetResponse &response) {
    response.set_status(static_cast<ResponseStatus>(err_info.status_code));
    response.set_reason(baidu::rpc::Errno_Name(
                static_cast<baidu::rpc::Errno>(err_info.err_code)) + ": " + err_info.reason);
}

inline std::string serialize_response(const GetResponse &res) {
    std::string res_data;
    std::string emsg;
    if (!ProtoMessageToJson(res, &res_data, &emsg)) {
        ULOG(WARNING) << "Failed to serialize response, error: " << emsg;
        return {};
    }

    return res_data;
}

bool open_conf(const std::string &conf_path, comcfg::Configure &ret);

bool init_channel(const comcfg::ConfigUnit &conf, baidu::rpc::Channel &chan);

}  // srv
}  // offline

#endif // UTILS_H
