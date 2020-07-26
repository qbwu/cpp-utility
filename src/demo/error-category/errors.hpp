inline std::error_code make_error_code(HadoopErrCode code) {
    return { static_cast<int>(code), HadoopErrorCategory::instance() };
}
inline std::error_code make_error_code(UserArgErrCode code) {
    return { static_cast<int>(code), UserArgErrorCategory::instance() };
}
inline std::error_code make_error_code(BOSErrCode code) {
    return { static_cast<int>(code), BOSErrorCategory::instance() };
}
inline std::error_code make_error_code(BMRErrCode code) {
    return { static_cast<int>(code), BMRErrorCategory::instance() };
}
inline std::error_code make_error_code(ZlibErrCode code) {
    return { static_cast<int>(code), ZlibErrorCategory::instance() };
}
inline std::error_code make_error_code(GenericErrCode code) {
    return { static_cast<int>(code), GenericErrorCategory::instance() };
}
inline std::error_condition make_error_condition(GenericErrCode code) {
    return { static_cast<int>(code), GenericErrorCategory::instance() };
}

namespace std {
template<>
struct is_error_code_enum<HadoopErrCode>: true_type {};

template<>
struct is_error_code_enum<UserArgErrCode>: true_type {};

template<>
struct is_error_code_enum<BOSErrCode>: true_type {};

template<>
struct is_error_code_enum<BMRErrCode>: true_type {};

template<>
struct is_error_code_enum<ZlibErrCode>: true_type {};

template<>
struct is_error_code_enum<GenericErrCode>: true_type {};

template<>
struct is_error_condition_enum<GenericErrCode>: true_type {};
}
