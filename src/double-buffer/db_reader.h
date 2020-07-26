/***************************************************************************
 *
 * Copyright (c) 2016 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file db_reader.h
 * @author qb.wu@outlook.com
 * @date 2016/09/12
 * @brief DBReader implemented by double buffer, one-reader-one-writer parallely.
 **/

#ifndef DB_READER_H
#define DB_READER_H

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <base/logging.h>
#include <simple_kv.h>
#include "utility.h"

namespace offline {
namespace ckv {

/*
 * SimpleKV reader. There's only one (read) thread calling the next(),
 * and one background (write) thread calling the _load().
 * */

class DBReader {
public:
    // [TODO] change to struct for clearer semantic
    using StrPair = std::pair<std::string, std::string>;
    using TaggedStrPair = std::pair<std::size_t, StrPair>;
    using TaggedStrPairVec = std::vector<TaggedStrPair>;

    DBReader(uint32_t tag,
        const offline::ds::SimpleIter<offline::ds::RocksDB>& iter, std::size_t size);

    ~DBReader() {
        if (_loader.joinable()) {
            _loader.join();
        }
    }

    DBReader(DBReader&& rhs) = delete;
    DBReader& operator=(DBReader&& rhs) = delete;
    DBReader(const DBReader&) = delete;
    DBReader& operator=(const DBReader&) = delete;

    TaggedStrPair next();

    template <typename Iter>
    std::size_t batch_fetch(Iter iter);

    bool has_next() const {
        return !(_rbuff_empty() && _eof);
    }

private:
    TaggedStrPair _get(const offline::ds::SimpleIter<offline::ds::RocksDB>& iter) const {
       return TaggedStrPair(_tag, StrPair(iter->key().ToString(), iter->value().ToString()));
    }

    void _load();
    bool _rbuff_empty() const {
        return _ridx >= _rest;
    }

    std::size_t _tag;
    std::size_t _buff_size;
    offline::ds::SimpleIter<offline::ds::RocksDB> _iter;

    // double buffer
    std::vector<TaggedStrPairVec> _dbuff;

    std::thread _loader;

    // components for synchronization

    // 0 or 1, current read buff#
    std::atomic<std::size_t> _buff_idx;

    // next read idx
    std::atomic<std::size_t> _ridx;

    // number of valid items in current read buffer
    std::atomic<std::size_t> _rest;

    // loading finish
    std::atomic<bool> _eof;

    std::mutex _rmtx;
    std::condition_variable _rcond;
    std::mutex _wmtx;
    std::condition_variable _wcond;
}; // DBReader

template <typename Iter>
std::size_t DBReader::batch_fetch(Iter iter) {
    if (!has_next()) {
        throw std::runtime_error("DBReader read EOF");
    }

    // region(_ridx < _rest) {
    {
        std::unique_lock<std::mutex> rlock(_rmtx);
        _rcond.wait(rlock, [this]() { return !_rbuff_empty(); });
    }

    std::size_t ret = _rest - _ridx;
    std::move(std::next(_dbuff[_buff_idx].begin(), _ridx),
              std::next(_dbuff[_buff_idx].begin(), _rest), iter);

    _ridx = _rest.load();

    // }

    {
        std::unique_lock<std::mutex> wlock(_wmtx);
        _wcond.notify_one();
    }
    return ret;
} // batch_fetch

} // ckv
} // offline

#endif // _DB_READER_H

