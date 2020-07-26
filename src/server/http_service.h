/***************************************************************************
 *
 * Copyright (c) 2018 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file http_service.h
 * @author qb.wu@outlook.com
 * @date 2018/07/03 23:33:25
 * @brief
 *
 **/

#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

#include <chrono>
#include <base/iobuf.h>
#include <base/logging.h>
#include <baidu/rpc/controller.h>
#include <baidu/rpc/server.h>
#include <json/json.h>

DECLARE_string(instance_name);

namespace offline {
namespace sre {

class HTTPService {
public:
    virtual ~HTTPService() = default;

    void serve(google::protobuf::RpcController *cntl_base,
               const HTTPRequest *request,
               HTTPResponse *response,
               google::protobuf::Closure *done) {
        serve(_srvs_id(), cntl_base, request, response, done);
    }

    void serve(const std::string &tag,
               google::protobuf::RpcController *cntl_base,
               const HTTPRequest *request,
               HTTPResponse *response,
               google::protobuf::Closure *done);

private:
    virtual const std::string& _srvs_id() const {
        static const std::string id = "Refinery";
        return id;
    }

    virtual bool _process(const std::string &tag,
        const std::string &req_id,
        const baidu::rpc::Controller *cntl,
        const std::string &req, json &resp, ErrInfo &err_info) = 0;

    base::IOBuf&
    _unpack_request(baidu::rpc::Controller *cntl, std::string &emsg) const;

    void _pack_response(const json &plain_resp,
            baidu::rpc::Controller *cntl) const;

    void _set_failed(const ErrInfo &err_info, baidu::rpc::Controller *cntl) const;

};

class ServiceLogGuard {
public:
    ServiceLogGuard(const std::string &ent_id,
        const std::string &req_id,
        const baidu::rpc::Controller &cntl)
            : _ent_id(ent_id), _cntl(cntl), _req_id(req_id),
            _start(std::chrono::steady_clock::now()) {
        LOG(NOTICE) << "[ job_id:=" << req_id
            << " ][ instance:" << FLAGS_instance_name
            << " ][ service:" << _ent_id
            << " ][ reqip:" << _cntl.remote_side()
            << " ][ cntl_log_id:" << _cntl.log_id()
            << " ] req:={ " << _cntl.request_attachment()
            << " } req_hdr:={ User-Agent: "
            << _cntl.http_request().GetHeader("User-Agent") << " }";
    }

    ~ServiceLogGuard() {
        auto stop = std::chrono::steady_clock::now();
        LOG(NOTICE) << "[ job_id:=" << _req_id
            << " ][ instance:" << FLAGS_instance_name
            << " ][ service: " << _ent_id
            << " ][ reqip: " << _cntl.remote_side()
            << " ][ cntl_log_id:" << _cntl.log_id()
            << " ][ proctime(total):" << std::chrono::duration_cast<
                std::chrono::milliseconds>(stop - _start).count()
            << "(ms) ] [ err_code: " << _cntl.ErrorCode()
            << " ] rep:={ " << _cntl.response_attachment() << " }";
    }

private:
    const std::string &_ent_id;
    const baidu::rpc::Controller &_cntl;
    const std::string &_req_id;
    const std::chrono::time_point<std::chrono::steady_clock> _start;
};

} // sre
} // offline

#endif // HTTP_SERVICE_H
