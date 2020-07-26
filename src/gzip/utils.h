/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.h
 * @author qb.wu@outlook.com
 * @date 2019/10/14 14:16:50
 * @brief
 *
 **/

#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <iostream>
#include <zlib.h>

namespace offline {
namespace sre {
namespace rm {

class GzipInflater {
public:
    GzipInflater();

    void set_data_blk(unsigned char *blk, std::size_t blk_sz) {
        _strm.next_in = blk;
        _strm.avail_in = blk_sz;
    }

    std::size_t inflate(unsigned char *buf, std::size_t buf_len);

    ~GzipInflater() {
        inflateEnd(&_strm);
    }

private:
    z_stream _strm;
};

} // sre
} // offline

#endif // CLIENT_UTILS_H
