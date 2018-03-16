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

#ifndef __ROUTER_STAT_H_
#define __ROUTER_STAT_H_

#include <string>
#include "TarsAgent.h"

namespace tarsagent
{

struct CheckTimeoutInfo
{
    uint32_t iMinTimeoutInvoke;
    uint32_t iCheckTimeoutInterval;
    uint32_t iFrequenceFailInvoke;
    uint32_t iMinFrequenceFailTime;
    float      fRadio;
    uint32_t  iTryTimeInterval;

    CheckTimeoutInfo()
    : iMinTimeoutInvoke(2)
    , iCheckTimeoutInterval(59)
    , iFrequenceFailInvoke(5)
    , iMinFrequenceFailTime(5)
    , fRadio(0.5)
    , iTryTimeInterval(30)
    {
        iMinTimeoutInvoke = g_app.getMinTimeoutInvoke();
        iCheckTimeoutInterval = g_app.getCheckTimeoutInterval();
        iFrequenceFailInvoke = g_app.getFrequenceFailInvoke();
        iMinFrequenceFailTime = g_app.getMinFrequenceFailTime();
        fRadio = g_app.getRadio();
        iTryTimeInterval = g_app.getTryTimeInterval();
    }
};

struct RouterNodeStat
{
    std::string            ip;
    unsigned short        port;
    unsigned int        succNum;
    unsigned int        errNum;
    unsigned int        continueErrNum;
    unsigned long long    timeCost;
    bool                available;
    unsigned long long    nextRetryTime;
    unsigned long long    lastCheckTime;
    unsigned long long    continueErrTime;
};

}

#endif
