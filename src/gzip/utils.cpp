/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.cpp
 * @author qb.wu@outlook.com
 * @date 2019/10/16 15:52:30
 * @brief
 *
 **/

#include "utils.h"
#include "errors.h"

namespace offline {
namespace sre {
namespace rm {

GzipInflater::GzipInflater() {
    _strm.zalloc = Z_NULL;
    _strm.zfree = Z_NULL;
    _strm.opaque = Z_NULL;
    _strm.next_in = Z_NULL;
    _strm.avail_in = 0;
    if (inflateInit2(&_strm, 16 + MAX_WBITS) != Z_OK) {
        throw std::system_error(GenericErrCode::INTERNAL_ERROR,
            "Failed to initialize zlib");
    }
}

std::size_t GzipInflater::inflate(unsigned char *buf,
        std::size_t buf_len) {
    _strm.avail_out = buf_len;
    _strm.next_out = buf;

    auto ret = ::inflate(&_strm, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_BUF_ERROR) {
        throw std::system_error(static_cast<ZlibErrCode>(ret),
            "Failed to inflate data block");
    }
    return buf_len - _strm.avail_out;
}

} // rm
} // sre
} // offline
