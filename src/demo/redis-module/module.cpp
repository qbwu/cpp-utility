/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file module.cpp
 * @author qb.wu@outlook.com
 * @date 2019/02/14
 * @brief Module entry.
 **/

#include "module.h"
#include "commands.h"
#include "config.h"
#include "redis_module_utils.h"

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    const auto VERSION = 1;
    if (RedisModule_Init(ctx, "GEO", VERSION, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (!redis::geo::load_config(ctx, argv, argc)) {
        redis::geo::log_warning(ctx,
                "failed to load config, possible config options: "
                "max-level[0 - 24, default: 24], "
                "min-level[0 - 24, default: 0], "
                "max-cells[1 - INF, default: 8], "
                "circle-edges[3 - INF, default: 36]");
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.ADD",
                redis::geo::add_command,
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.DEL",
                redis::geo::del_command,
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.GET",
                redis::geo::get_command,
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.WITHIN",
                redis::geo::within_command,
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.INTERSECT",
                redis::geo::intersect_command,
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx,
                "GEO.BBOX",
                redis::geo::bbox_command,
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
