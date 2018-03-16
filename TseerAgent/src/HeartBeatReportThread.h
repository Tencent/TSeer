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

#ifndef _HEART_BEAT_REPORT_H_
#define _HEART_BEAT_REPORT_H_

#include "util/tc_thread.h"
#include "Registry.h"


class HeartBeatReportThread : public tars::TC_Thread, public tars::TC_ThreadLock
{
public:
    HeartBeatReportThread();

    //初始化，设置上报间隔（s），获取Registry的路由等；成功返回0，失败返回-1
    int init(int reportPeriod);

    virtual void run();

    void terminate();

    //添加api key信息
    void addKeyInfo(const string &key, const string &version);
private:

    bool _terminate;

    int _reportPeriod;

    Tseer::RegistryPrx _registryPrx;

    time_t _lastReportKeyTime;
    TC_ThreadLock _apiKeyLock;            
    set<string> _apiKeys;
       bool _init;
};

extern HeartBeatReportThread *g_ReapHeartBeatReportThread;

#endif
