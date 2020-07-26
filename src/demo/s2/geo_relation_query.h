/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file geo_relation_query.h
 * @author qb.wu@outlook.com
 * @date 2019/03/14
 * @brief Query the relationship among geometries.
 **/

#ifndef GEO_RELATION_QUERY_H
#define GEO_RELATION_QUERY_H

#include <algorithm>
#include "s2shape_wrapper.h"
#include "geometry.h"
#include "errors.h"

namespace redis {

namespace geo {

struct GeoRelationQueryOption {

    std::size_t circle_vertex_num = 36;

}; // GeoRelationQueryOption

class GeoRelationQuery {
public:
    enum class GeoRelation : char {
        WITHIN = 0,
        INTERSECT,
        OUTER // We don't distinguish CONTAIN and DISJOINT
    };

    GeoRelationQuery() = default;
    explicit GeoRelationQuery(const GeoRelationQueryOption &opts)
        : _opts(opts) {}

    enum class CopyMode { DEEP = 0, SHALLOW };
    void add_region(const ParsedGeo &geo) {
        add_region(geo, CopyMode::SHALLOW);
    }
    void add_region(const ParsedGeo &geo, CopyMode mode);

    bool match(const ParsedGeo &other, GeoRelation rel) const;

private:
    void _add_to_index(GeoType type,
        const S2Region &region, MutableS2ShapeIndex &index) const;

    template <typename T, typename S2ShapeImpl = typename T::Shape>
    void _add_to_index(const T &geo, MutableS2ShapeIndex &index) const {
        index.Add(std::unique_ptr<S2Shape>(new S2ShapeImpl(&geo)));
    }

    void _add_to_index(const S2Cap &circle, MutableS2ShapeIndex &index) const {
        index.Add(std::unique_ptr<S2Shape>(
            new CircleS2ShapeWrapper(circle, _opts.circle_vertex_num)));
    }

    void _add_to_index(const S2LatLngRect &rect, MutableS2ShapeIndex &index) const {
        index.Add(std::unique_ptr<S2Shape>(new RectangleS2ShapeWrapper(rect)));
    }

    bool _match(const S2LatLng &point, GeoRelation rel) const;
    bool _match(GeoType type, const S2Region &region, GeoRelation rel) const;

    using S2RegionUptr = std::unique_ptr<S2Region, std::function<void(S2Region*)>>;

    bool _rect_bounds_intersect(const S2Region &region) const {
        auto region_bound = region.GetRectBound();
        return std::any_of(_bounds.begin(), _bounds.end(),
           [&region_bound](const S2RegionUptr &elem) {
                return elem->GetRectBound().Intersects(region_bound);
           });
    }
    bool _rect_bounds_contain(const S2LatLng &point) const {
        return std::any_of(_bounds.begin(), _bounds.end(),
            [&point](const S2RegionUptr &elem) {
                return elem->GetRectBound().Contains(point);
           });
    }

    const GeoRelationQueryOption _opts;
    std::vector<S2RegionUptr> _bounds;
    // _index is lazily built until there's real query performing on it,
    // refer the API's document for details.
    MutableS2ShapeIndex _index;
}; // GeoRelationQuery

} // geo

} // redis
#endif // GEO_RELATION_QUERY_H


