/***************************************************************************
 *
 * Copyright (c) 2018 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file http_service.cpp
 * @author qb.wu@outlook.com
 * @date 2018/07/03 23:33:25
 * @brief
 *
 **/

#include <chrono>
#include <baidu/rpc/policy/gzip_compress.h>
#include <base/logging.h>
#include <base/iobuf.h>
#include <json/json.h>
#include "http_service.h"

namespace {

const std::string KEY_RESP_STATUS = "status";
const std::string KEY_RESP_MSG = "msg";

} // (anonymous)

namespace offline {
namespace sre {

void HTTPService::serve(const std::string &tag,
        google::protobuf::RpcController *cntl_base,
        const HTTPRequest * /*request*/,
        HTTPResponse * /*response*/,
        google::protobuf::Closure *done) {

    baidu::rpc::ClosureGuard done_guard(done);
    auto *cntl = static_cast<baidu::rpc::Controller*>(cntl_base);

    // Make the log_region destructor called before its arguments
    {
        auto req_id = create_job_id(tag);

        ServiceLogGuard log_region(_srvs_id(), req_id, *cntl);

        ErrInfo err_info;
        do {
            std::string emsg;
            auto &plain_req = _unpack_request(cntl, emsg);
            if (plain_req.empty()) {
                LOG(WARNING) << "Fail to unpack HTTPRequest: " << emsg;
                err_info = bad_request_error(std::move(emsg));
                break;
            }

            json resp;
            try {
                if (_process(tag, req_id, cntl, plain_req.to_string(),
                        resp, err_info)) {
                    resp[KEY_RESP_STATUS] = 0;
                    _pack_response(resp, cntl);
                    return;
                }
            } catch (const std::exception &ex) {
                LOG(WARNING) << "Uncaught exception in ServiceImpl: " << ex.what();
                err_info = internal_error();
            }

            break;

        } while (true);

        // error path
        _pack_response({
            { KEY_RESP_STATUS, err_info.http_status_code },
            { KEY_RESP_MSG, err_info.reason }
        }, cntl);
    }
}

base::IOBuf& HTTPService::_unpack_request(baidu::rpc::Controller *cntl,
        std::string &emsg) const {
    auto &header = cntl->http_request();
    auto &req = cntl->request_attachment();

    if (header.content_type() != "application/json") {
        emsg = "Invalid request content type.";
        req.clear();
    }

    const auto* encoding = header.GetHeader("Content-Encoding");
    if (encoding != nullptr && *encoding == "gzip") {
        base::IOBuf plain_req;
        if (!baidu::rpc::policy::GzipDecompress(req, &plain_req)) {
            emsg = "Fail to uncompress the request.";
            req.clear();
        }
        req.swap(plain_req);
    }

    return req;
}

void HTTPService::_pack_response(const json &plain_resp,
        baidu::rpc::Controller *cntl) const {

    cntl->http_response().set_content_type("application/json");
    cntl->set_response_compress_type(baidu::rpc::COMPRESS_TYPE_GZIP);

    try {
        base::IOBufBuilder os;
        // May throw
        os << plain_resp.dump();

        cntl->http_response().SetHeader(
                "Content-Length", std::to_string(os.buf().length()));

        os.move_to(cntl->response_attachment());
    } catch (const std::exception &ex) {
        LOG(WARNING) << "Fail to pack the response, error: " << ex.what();
        _set_failed(internal_error(), cntl);
    }
}

void HTTPService::_set_failed(const ErrInfo &err_info,
        baidu::rpc::Controller *cntl) const {
    cntl->SetFailed(err_info.err_code, err_info.reason.c_str());
    cntl->http_response().set_status_code(err_info.http_status_code);

}

} // sre
} // offline
