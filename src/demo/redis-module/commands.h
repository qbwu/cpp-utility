/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file commands.h
 * @author qb.wu@outlook.com
 * @date 2019/02/14
 * @brief Geo commands.
 **/

#ifndef COMMANDS_H
#define COMMANDS_H

#include "redismodule.h"

namespace redis {

namespace geo {

int add_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int del_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int get_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int within_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int intersect_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

int bbox_command(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

}

}

#endif // COMMANDS_H
