/***************************************************************************
 *
 * Copyright (c) 2017 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.h
 * @author qb.wu@outlook.com
 * @date 2017/09/07 18:03:01
 * @brief
 *
 **/

#ifndef  UTILS_H
#define  UTILS_H

#include <memory>
#include <json/json.h>

namespace offline {
namespace srv {

template <typename Iter1, typename Iter2>
void str_to_json_list(Iter1 beg, Iter1 end, Iter2 out) {
    Json::Value settings;
    Json::CharReaderBuilder::strictMode(&settings);

    Json::CharReaderBuilder reader_builder;
    reader_builder.settings_ = std::move(settings);
    auto reader = std::unique_ptr<Json::CharReader>(reader_builder.newCharReader());

    std::string err;
    while (beg != end) {
        try {
            const auto &str = *beg;
            ++beg;
            Json::Value root;
            if (reader->parse(str.data(), str.data() + str.length(),
                              &root, &err)) {
                *out = std::move(root);
                ++out;
            } else {
                LOG(WARNING) << "Failed to parse the json: " << str
                    << "--> Err. " << err;
            }
        } catch (const std::exception &ex){
            LOG(WARNING) << "Caught exception when parsing the json: "
                         << ex.what();
        }
    }
}

template <typename JSON, typename Iter1, typename Iter2>
void find_members_by_path(JSON &data, Iter1 beg, Iter1 end, Iter2 &out) {
    if (!data.isObject() && !data.isArray()) {
        return;
    }

    if (data.isArray()) {
        for (auto &elem : data) {
            find_members_by_path(elem, beg, end, out);
        }
    } else { // Object
        if (beg == end) {
            if (data.isMember(*beg)) {
                *out = &data[*beg];
                ++out;
            }
        } else {
            find_members_by_path(data[*beg], beg + 1, end, out);
        }
    }
}

} // srv
} // offline

#endif // UTILS_H
