/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file s2shape_wrapper.cpp
 * @author qb.wu@outlook.com
 * @date 2019/03/15
 * @brief
 **/

#include "s2shape_wrapper.h"

namespace redis {

namespace geo {

CircleS2ShapeWrapper::CircleS2ShapeWrapper(const S2Cap &cap,
    std::size_t sampling_num) : _center(cap.center()),
        _n(sampling_num) {
    assert(_n >= 3);
    _rad_step = 2 * M_PI / _n;

    auto center = S2LatLng(_center);
    _center_lng_rad = center.lng().radians();
    _sin_lat = sin(center.lat());
    _cos_lat = cos(center.lat());

    _sin_radius = sin(cap.radius());
    _cos_radius = cos(cap.radius());

    // Need to generate polygon in CW orientation.
    if (_cos_radius < 0) {
        _rad_step = -_rad_step;
        _center_lng_rad = M_PI + _center_lng_rad;
    }
}

S2Point CircleS2ShapeWrapper::_vertex(int vtx_id) const {
    auto theta = S1Angle::Radians(vtx_id * _rad_step);

    auto lat_rad = asinl(
        _sin_lat * _cos_radius + _cos_lat * _sin_radius * sin(theta));

    auto lng_rad = _center_lng_rad
        + asinl(cos(theta) * _sin_radius / cos(lat_rad));

    return S2LatLng::FromRadians(lat_rad, lng_rad).Normalized().ToPoint();
}

RectangleS2ShapeWrapper::RectangleS2ShapeWrapper(const S2LatLngRect &rect)
        : _center(rect.GetCenter().ToPoint()) {
    _bottom_left = rect.lo().ToPoint();
    _bottom_right = S2LatLng(rect.lat_lo(), rect.lng_hi()).ToPoint();
    _top_right = rect.hi().ToPoint();
    _top_left = S2LatLng(rect.lat_hi(), rect.lng_lo()).ToPoint();
}

} // geo

} // redis
