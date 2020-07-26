/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file geo_relation_query.h
 * @author qb.wu@outlook.com
 * @date 2019/03/14
 * @brief
 **/

#include "geo_relation_query.h"
#include <s2/s2contains_point_query.h>
#include <s2/s2boolean_operation.h>

namespace redis {

namespace geo {

void GeoRelationQuery::add_region(const ParsedGeo &geo, CopyMode mode) {
    if (!geo) {
        throw Error("Invalid geo");
    }
    if (geo.type == GeoType::POINT) {
        throw Error("Can't use point to query geo relation");
    }
    try {
        _add_to_index(geo.type, *geo.region, _index);
        if (mode == CopyMode::SHALLOW) {
            _bounds.emplace_back(geo.region.get(), [](S2Region*) {});
        } else {
            _bounds.emplace_back(geo.region->Clone(),
                                 [](S2Region *p) { delete p; });
        }
    } catch (const Error&) {
        throw;
    } catch (const std::exception &ex) {
        throw Error(std::string("Geo relation query error: ") + ex.what());
    }
}

void GeoRelationQuery::_add_to_index(GeoType type, const S2Region &region,
        MutableS2ShapeIndex &index) const {
    switch (type) {
    case GeoType::LINE_STRING:
        _add_to_index(dynamic_cast<const S2Polyline&>(region), index);
        break;
    case GeoType::POLYGON:
        _add_to_index(dynamic_cast<const S2Polygon&>(region), index);
        break;
    case GeoType::CIRCLE:
        _add_to_index(dynamic_cast<const S2Cap&>(region), index);
        break;
    case GeoType::RECTANGLE:
        _add_to_index(dynamic_cast<const S2LatLngRect&>(region), index);
        break;
    default:
        throw Error("Unsupported geo type");
    }
}

bool GeoRelationQuery::match(const ParsedGeo &other, GeoRelation rel) const {
    if (!other) {
        throw Error("Invalid geo");
    }

    try {
        if (other.type == GeoType::POINT) {
            return _match(*other.point, rel);
        }
        return _match(other.type, *other.region, rel);
    } catch (const Error&) {
        throw;
    } catch (const std::exception &ex) {
        throw Error(std::string("Geo relation calculating error: ") + ex.what());
    }
}

bool GeoRelationQuery::_match(const S2LatLng &point, GeoRelation rel) const {
    if (!_rect_bounds_contain(point)) {
        return rel == GeoRelation::OUTER;
    }
    auto p = point.ToPoint();

    auto contains = std::any_of(_bounds.begin(), _bounds.end(),
        [&p](const S2RegionUptr &elem) {
            return elem->Contains(p);
        });

    if (contains) {
        return rel == GeoRelation::INTERSECT || rel == GeoRelation::WITHIN;
    }

    return rel == GeoRelation::OUTER;
}

bool GeoRelationQuery::_match(GeoType type, const S2Region &region,
        GeoRelation rel) const {
    if (!_rect_bounds_intersect(region)) {
        return rel == GeoRelation::OUTER;
    }
    MutableS2ShapeIndex other;
    _add_to_index(type, region, other);

    // including the vertices and edges(both directions) of polygons and polylines
    S2BooleanOperation::Options opts;
    opts.set_polygon_model(S2BooleanOperation::PolygonModel::CLOSED);
    opts.set_polyline_model(S2BooleanOperation::PolylineModel::CLOSED);

    switch (rel) {
    case GeoRelation::WITHIN:
        return S2BooleanOperation::Contains(_index, other, opts);
    case GeoRelation::INTERSECT:
        return S2BooleanOperation::Intersects(_index, other, opts);
    case GeoRelation::OUTER:
        return !S2BooleanOperation::Intersects(_index, other, opts);
    default:
        throw Error("Unsupported geo relation");
    }
}

} // geo

} // redis
