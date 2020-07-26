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

#include <google/protobuf/repeated_field.h>

namespace offline {
namespace srv {

template <typename ReqType>
bool get_string_field(const ReqType &req, const std::string &field_name,
        std::string &field_value) {
    const auto *descriptor = req.GetDescriptor();
    const auto *reflection = req.GetReflection();
    if (descriptor == nullptr || reflection == nullptr) {
        ULOG(WARNING) << "Failed to get meta of " << req.GetTypeName();
        return false;
    }

    const auto *field = descriptor->FindFieldByName(field_name);
    if (field == nullptr) {
        ULOG(WARNING) << "Missing " << req.GetTypeName() << "." << field_name;
        return false;
    }

    if (field->type() != google::protobuf::FieldDescriptor::TYPE_STRING
            || field->label() == google::protobuf::FieldDescriptor::LABEL_REPEATED) {
        ULOG(WARNING) << "Type error with " << req.GetTypeName() << "." << field_name;
        return false;
    }

    field_value = reflection->GetString(req, field);
    if (field_value.empty()) {
        ULOG(INFO) << "Empty " << req.GetTypeName() << "." << field_name;
        return true;
    }

    return true;
}

} // srv
} // offline

#endif // UTILS_H
