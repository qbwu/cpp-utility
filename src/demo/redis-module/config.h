/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file config.h
 * @author qb.wu@outlook.com
 * @date 2019/03/19
 * @brief Config
 **/

#ifndef CONFIG_H
#define CONFIG_H

#include "redismodule.h"
#include "geometry.h"
#include "geo_relation_query.h"

namespace redis {

namespace geo {

// It's bad to use global variable,
// however, RedisModule API doens't give us a better way to pass in user defined data.
extern S2RegionCoverer::Options coverer_opts;

extern GeoRelationQueryOption relation_opts;

bool load_config(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

}

}

#endif // end REDIS_GEO_CONFIG_H
