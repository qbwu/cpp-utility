/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file errors.h
 * @author qb.wu@outlook.com
 * @date 2019/10/31 20:57:30
 * @brief
 *
 **/

#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>
#include <zlib.h>
#include <system_error>
#include <Configure.h>

enum class HadoopErrCode {
    FAIL = 1,
    ERROR
};

class HadoopErrorCategory : public std::error_category {
public:
    static const HadoopErrorCategory& instance() {
        static HadoopErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "HadoopError"; }
    virtual std::string message(int ev) const {
        switch (static_cast<HadoopErrCode>(ev)) {
        case HadoopErrCode::FAIL: return "Hadoop-cmd failed";
        case HadoopErrCode::ERROR: return "Hadoop-cmd exit with error";
        default : return "Unknown hadoop-cmd error";
        }
    }
protected:
    HadoopErrorCategory() = default;
};

enum class UserArgErrCode {
    INVALID_RESOURCE_ID = 1,
    MISSING_PATH_SCHEME,
    UNSUPPORTED_PATH_SCHEME,
    UNSUPPORTED_COMPRESSION_FORMAT,
    MALFORMED_PATH,
    MALFORMED_JOB_ID,
    MISSING_REQUIRED_FLAGS,
    INVALID_USER_LOCAL_PATH,
    INVALID_META_ID,
    CROSS_BUCKET,
    PERMISION_DENIED
};

class UserArgErrorCategory : public std::error_category {
public:
    static const UserArgErrorCategory& instance() {
        static UserArgErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "UserArgumentError"; }
    virtual std::string message(int ev) const {
        switch (static_cast<UserArgErrCode>(ev)) {
        case UserArgErrCode::MISSING_PATH_SCHEME: return "Missing path scheme";
        case UserArgErrCode::UNSUPPORTED_PATH_SCHEME: return "Unsupported path scheme";
        case UserArgErrCode::UNSUPPORTED_COMPRESSION_FORMAT:
            return "Unsupported compression format";
        case UserArgErrCode::MALFORMED_PATH: return "Bad path structure";
        case UserArgErrCode::MALFORMED_JOB_ID: return "Bad job_id structure";
        case UserArgErrCode::MISSING_REQUIRED_FLAGS:
            return "Missing required flags in the command line";
        case UserArgErrCode::INVALID_USER_LOCAL_PATH: return "Invalid user local path";
        case UserArgErrCode::INVALID_RESOURCE_ID: return "Invalid resource id";
        case UserArgErrCode::INVALID_META_ID: return "Invalid meta id";
        case UserArgErrCode::CROSS_BUCKET: return "Invalid cross-bucket operation";
        case UserArgErrCode::PERMISION_DENIED: return "Permision denied";
        default : return "Unknown argument error";
        }
    }
protected:
    UserArgErrorCategory() = default;
};

enum class BOSErrCode {
    API_RET_ERROR = 1,
    API_RET_FAIL,
    FILE_NOT_EXIST,
    FILE_EXIST
};

class BOSErrorCategory : public std::error_category {
public:
    static const BOSErrorCategory& instance() {
        static BOSErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "BOSError"; }
    virtual std::string message(int ev) const {
        switch (static_cast<BOSErrCode>(ev)) {
        case BOSErrCode::API_RET_ERROR: return "BOS SDK return value error";
        case BOSErrCode::API_RET_FAIL: return "BOS operation failed";
        case BOSErrCode::FILE_NOT_EXIST: return "Invalid non-existent path";
        case BOSErrCode::FILE_EXIST: return "Invalid already existing path";
        default : return "Unknown BOS error.";
        }
    }
protected:
    BOSErrorCategory() = default;
};

enum class BMRErrCode {
    ADD_JOB_ERROR = 1,
    JOB_STATE_ERROR,
    JOB_TIMEOUT,
    QUERY_JOB_ERROR
};

class BMRErrorCategory : public std::error_category {
public:
    static const BMRErrorCategory& instance() {
        static BMRErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "BMRError"; }
    virtual std::string message(int ev) const {
        switch (static_cast<BMRErrCode>(ev)) {
        case BMRErrCode::ADD_JOB_ERROR: return "Failed to submit job to BMR";
        case BMRErrCode::JOB_STATE_ERROR: return "Job did not finish successfully";
        case BMRErrCode::JOB_TIMEOUT: return "Timeout and stop tracking the job state";
        case BMRErrCode::QUERY_JOB_ERROR: return "Failed to query the job info";
        default : return "Unknown BMR error";
        }
    }
protected:
    BMRErrorCategory() = default;
};

enum class ZlibErrCode {
    STREAM_END = Z_STREAM_END,
    NEED_DICT = Z_NEED_DICT,
    DATA_ERROR = Z_DATA_ERROR,
    ERRNO = Z_ERRNO,
    STREAM_ERROR = Z_STREAM_ERROR,
    MEM_ERROR = Z_MEM_ERROR,
    BUF_ERROR = Z_BUF_ERROR,
    VERSION_ERROR = Z_VERSION_ERROR
};

class ZlibErrorCategory : public std::error_category {
public:
    static const ZlibErrorCategory& instance() {
        static ZlibErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "ZlibError"; }
    virtual std::string message(int ev) const {
        switch (static_cast<ZlibErrCode>(ev)) {
        case ZlibErrCode::STREAM_END: return "Reach end of the zlib stream";
        case ZlibErrCode::NEED_DICT: return "Corrupted data in the dictionary section";
        case ZlibErrCode::DATA_ERROR: return "Corrupted data block";
        case ZlibErrCode::ERRNO: return "R/W file error";
        case ZlibErrCode::STREAM_ERROR: return "Zlib state error";
        case ZlibErrCode::MEM_ERROR: return "Cannot allocate memory";
        case ZlibErrCode::BUF_ERROR: return "Empty data blocks";
        case ZlibErrCode::VERSION_ERROR:
            return "Compiling-time and run-time versions mismatch";
        default : return "Unknown Zlib error";
        }
    }
protected:
    ZlibErrorCategory() = default;
};

enum class GenericErrCode {
    CONFIG_ERROR = 1,
    OP_FAIL = 2,
    SYSTEM_ERROR = 100,
    USER_INPUT_ERROR = 200,
    INTERNAL_ERROR = 1000
};

class GenericErrorCategory : public std::error_category {
public:
    static const GenericErrorCategory& instance() {
        static GenericErrorCategory instance;
        return instance;
    }

    virtual const char* name() const noexcept { return "Error"; }
    virtual std::string message(int ev) const {
        switch (static_cast<GenericErrCode>(ev)) {
        case GenericErrCode::CONFIG_ERROR: return "Configure error";
        case GenericErrCode::OP_FAIL: return "Operation fail";
        case GenericErrCode::SYSTEM_ERROR: return "Encounted system error";
        case GenericErrCode::USER_INPUT_ERROR: return "User input error";
        case GenericErrCode::INTERNAL_ERROR: return "Internal error";
        default : return "Unknown error";
        }
    }

    bool equivalent(const std::error_code &code, int condition)
            const noexcept override;

protected:
    GenericErrorCategory() : _hadoop_ec(HadoopErrorCategory::instance()),
            _bos_ec(BOSErrorCategory::instance()),
            _bmr_ec(BMRErrorCategory::instance()),
            _zlib_ec(ZlibErrorCategory::instance()),
            _ua_ec(UserArgErrorCategory::instance()) {}

    const std::error_category &_hadoop_ec;
    const std::error_category &_bos_ec;
    const std::error_category &_bmr_ec;
    const std::error_category &_zlib_ec;
    const std::error_category &_ua_ec;
};

// Here are some template spetializations and global functions required by STL
// to implement the std::error_code.
#include "errors.hpp"

class ConfigureError : public std::system_error {
public:
    ConfigureError(const std::string &conf_name, const std::string &what)
            : std::system_error(GenericErrCode::CONFIG_ERROR,
            _error_message(conf_name, what)) {}

    ConfigureError(const std::string &conf_name, const std::exception &ex)
            : std::system_error(GenericErrCode::CONFIG_ERROR,
            _error_message(conf_name, ex.what())) {}

private:
    const std::string _error_message(const std::string &conf_name,
            const std::string &detail) const {
        return "Configure [" + conf_name + "] error: " + detail;
    }
};

#endif // ERRORS_H
