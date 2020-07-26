#include <iostream>
#include "errors.h"

bool GenericErrorCategory::equivalent(const std::error_code &code, int condition)
        const noexcept {
    if (code.category() == *this) {
        return code == std::error_code(static_cast<GenericErrCode>(condition));
    }

    auto cond = static_cast<GenericErrCode>(condition);
    if (code.category() == _hadoop_ec) {
        switch (cond) {
        case GenericErrCode::OP_FAIL: return code == HadoopErrCode::FAIL;
        case GenericErrCode::SYSTEM_ERROR: return code == HadoopErrCode::ERROR;
        default: return false;
        }
    }
    if (code.category() == _bos_ec) {
        switch (cond) {
        case GenericErrCode::OP_FAIL: return code == BOSErrCode::API_RET_FAIL;
        case GenericErrCode::SYSTEM_ERROR: return code == BOSErrCode::API_RET_ERROR;
        case GenericErrCode::USER_INPUT_ERROR:
            return code == BOSErrCode::FILE_NOT_EXIST || code == BOSErrCode::FILE_EXIST;
        default: return false;
        }
    }
    if (code.category() == _bmr_ec) {
        switch (cond) {
        case GenericErrCode::OP_FAIL: return code == BMRErrCode::JOB_STATE_ERROR;
        case GenericErrCode::SYSTEM_ERROR: return code == BMRErrCode::ADD_JOB_ERROR
                                               || code == BMRErrCode::JOB_TIMEOUT;
        default: return false;
        }
    }
    if (code.category() == _zlib_ec) {
        switch (cond) {
        case GenericErrCode::OP_FAIL: return code == ZlibErrCode::BUF_ERROR
                                          || code == ZlibErrCode::STREAM_END;
        case GenericErrCode::USER_INPUT_ERROR: return code == ZlibErrCode::NEED_DICT
                                                   || code == ZlibErrCode::DATA_ERROR;
        case GenericErrCode::SYSTEM_ERROR: return code == ZlibErrCode::MEM_ERROR
                                               || code == ZlibErrCode::VERSION_ERROR
                                               || code == ZlibErrCode::ERRNO;
        case GenericErrCode::INTERNAL_ERROR: return code == ZlibErrCode::STREAM_ERROR;
        default: return false;
        }
    }
    if (code.category() == _ua_ec) {
        return cond == GenericErrCode::USER_INPUT_ERROR;
    }
    // Unknown category
    return cond == GenericErrCode::INTERNAL_ERROR;
}
