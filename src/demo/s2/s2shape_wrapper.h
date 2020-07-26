/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file s2shape_wrapper.h
 * @author qb.wu@outlook.com
 * @date 2019/03/15
 * @brief
 **/

#ifndef GEO_S2SHAPE_WRAPPER_H
#define GEO_S2SHAPE_WRAPPER_H

#include <cmath>
#include <cassert>
#include "geometry.h"
#include "errors.h"

namespace redis {

namespace geo {

class CircleS2ShapeWrapper final : public S2Shape {
public:
    CircleS2ShapeWrapper(const S2Cap &cap, std::size_t sampling_num);

    virtual int num_edges() const { return _n; }

    virtual Edge edge(int edge_id) const {
        assert(0 <= edge_id && edge_id < num_edges());
        return { _vertex(edge_id), _vertex((edge_id + 1) % _n) };
    }

    virtual int dimension() const { return 2; }

    virtual ReferencePoint GetReferencePoint() const {
        return { _center, true };
    }

    virtual int num_chains() const { return 1; }

    virtual Chain chain(int chain_id) const {
        assert(chain_id == 0);
        return { 0, num_edges() };
    };

    virtual Edge chain_edge(int chain_id, int offset) const {
        assert(chain_id == 0 && offset < num_edges());
        return edge(offset);
    }

    virtual ChainPosition chain_position(int edge_id) const {
        assert(0 <= edge_id && edge_id < num_edges());
        return { 0, edge_id };
    }

    virtual TypeTag type_tag() const {
        return kMinUserTypeTag + 1;
    }

private:
    S2Point _vertex(int vtx_id) const;

    const S2Point _center;
    const std::size_t _n = 0;

    double _sin_lat = 0.0;
    double _cos_lat = 0.0;
    double _sin_radius = 0.0;
    double _cos_radius = 0.0;

    double _rad_step = 0.0;
    double _center_lng_rad = 0.0;

}; // CircleS2ShapeWrapper

class RectangleS2ShapeWrapper final : public S2Shape {
public:
    explicit RectangleS2ShapeWrapper(const S2LatLngRect &rect);

    virtual int num_edges() const { return 4; }

    virtual Edge edge(int edge_id) const {
        assert(0 <= edge_id && edge_id < num_edges());
        switch (edge_id) {
        case 0: return { _bottom_left, _bottom_right };
        case 1: return { _bottom_right, _top_right };
        case 2: return { _top_right, _top_left };
        default: return { _top_left, _bottom_left };
        }
    }

    virtual int dimension() const { return 2; }

    virtual ReferencePoint GetReferencePoint() const {
        return { _center, true };
    }

    virtual int num_chains() const { return 1; }

    virtual Chain chain(int chain_id) const {
        assert(chain_id == 0);
        return { 0, num_edges() };
    };

    virtual Edge chain_edge(int chain_id, int offset) const {
        assert(chain_id == 0 && offset < num_edges());
        return edge(offset);
    }

    virtual ChainPosition chain_position(int edge_id) const {
        assert(0 <= edge_id && edge_id < num_edges());
        return { 0, edge_id };
    }

    virtual TypeTag type_tag() const {
        return kMinUserTypeTag + 2;
    }

private:
    const S2Point _center;
    S2Point _bottom_left;
    S2Point _bottom_right;
    S2Point _top_right;
    S2Point _top_left;

}; // RectangleS2ShapeWrapper

} // geo

} // redis

#endif // GEO_S2SHAPE_WRAPPER_H

