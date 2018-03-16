/**
 * Tencent is pleased to support the open source community by making Tseer available.
 *
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 * 
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * 
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

#ifndef __REAP_THREAD_H__
#define __REAP_THREAD_H__

#include <iostream>

#include "util/tc_thread.h"

#include "StoreCache.h"

using namespace Tseer;

/**
 * 用于执行定时操作的线程类
 */
class ReapThread : public TC_Thread, public TC_ThreadLock
{
public:

    ReapThread();

    ~ReapThread();

    /*
     * 结束线程
     */
    void terminate();

    /**
     * 初始化
     */
    int init();

    /**
     * 轮询函数
     */
    virtual void run();
private:
    /**
     * 更新服务自身心跳
     */
    int updateRegistryInfo();
protected:
    //线程结束标志
    bool _terminate;

    //数据库操作
    StoreCache _storeCache;

    //全量加载时间,单位是秒,默认5秒
    int _loadAllObjectsInterval;

    //registry心跳频率,单位是秒，默认60秒
    int _registryHeartBeatInterval;

    //是否启用数据恢复保护功能，默认为打开
    bool _becoverProtect;

    //启用数据恢复保护功能状态下极限值
    int _recoverProtectRate;

    //主控心跳时间更新开关
    bool _regHeartBeatOff;
};

#endif
