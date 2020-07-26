/***************************************************************************
 *
 * Copyright (c) 2016 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file db_reader.cpp
 * @author qb.wu@outlook.com
 * @date 2016/09/12
 * @brief implementation of db_reader.h
 **/

#include "db_reader.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <condition_variable>
#include <thread>
#include <base/logging.h>
#include <simple_kv.h>
#include "utility.h"

namespace offline {
namespace ckv {

DBReader::DBReader(uint32_t tag,
    const offline::ds::SimpleIter<offline::ds::RocksDB>& iter, std::size_t size)
    : _tag(tag), _buff_size(size), _iter(iter), _buff_idx(0), _ridx(size),
    _rest(size), _eof(!_iter->valid()) {

    if (size == 0) {
        throw std::invalid_argument("buffer size of DBReader should not be zero.");
    }

    _dbuff.reserve(2);
    _dbuff.push_back(TaggedStrPairVec(size));
    _dbuff.push_back(TaggedStrPairVec(size));

    _loader = std::thread(&DBReader::_load, this);

    if (_eof) {
        LOG(NOTICE) << "RocksDB for DBReader is empty initially";
    }
}

DBReader::TaggedStrPair DBReader::next() {
    if (!has_next()) {
        throw std::runtime_error("DBReader read EOF");
    }

    // region(_ridx < _rest) {
    {
        std::unique_lock<std::mutex> rlock(_rmtx);
        _rcond.wait(rlock, [this]() { return !_rbuff_empty(); });
    }
    assert(_ridx < _rest);
    auto ret = std::move(_dbuff[_buff_idx][_ridx++]);
    // }

    if (_rbuff_empty()) {
        std::unique_lock<std::mutex> wlock(_wmtx);
        _wcond.notify_one();
    }
    return ret;
} // next

void DBReader::_load() {
    DLOG(INFO) << "Back thread for db_" << _tag << " loading";
    do {
        std::size_t rec_idx = 0;
        // 2 buffers are operated seperately
        std::size_t idx = 1 - _buff_idx;

        while (_iter->valid()) {
            _dbuff[idx][rec_idx++] = _get(_iter);
            _iter->next();
            if (rec_idx == _buff_size)
                break;
        }

        // region(_ridx >= _rest ( == _buff_size)) {
        {
            std::unique_lock<std::mutex> wlock(_wmtx);
            _wcond.wait(wlock, [this]() { return _rbuff_empty(); });
        }

        // Switch the current read buffer, instruction sequence is critical here,
        // especially the *_ridx = 0* (controlling the mutual exclusion between
        // *_load* and *next*) and the *_eof = (!_iter->valid())* (controlling the
        // mutual exclusion between *_load* and *_has_next*)

        _buff_idx = idx;
        assert(_rest > 0);
        _rest = rec_idx;
        assert(_rest <= _ridx);
        _ridx = 0;
        // }
        _eof = (!_iter->valid());

        {
            std::unique_lock<std::mutex> rlock(_rmtx);
            _rcond.notify_one();
        }

    } while (_iter->valid());

    DLOG(INFO) << "Background thread over";
    return;
} // _load

} // ckv
} // offline
