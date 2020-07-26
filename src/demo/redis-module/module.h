/***************************************************************************
 *
 * Copyright (c) 2019 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file module.h
 * @author qb.wu@outlook.com
 * @date 2019/02/14
 * @brief Module entry.
 **/

#ifndef MODULE_H
#define MODULE_H

#include "redismodule.h"

#ifdef __cplusplus

extern "C" {

#endif

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#ifdef __cplusplus

}

#endif

#endif // MODULE_H
