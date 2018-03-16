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

#ifndef __TSEER_API_ROUTER_STAT_H_
#define __TSEER_API_ROUTER_STAT_H_

#include <string>
#include <stdint.h>

namespace Tseerapi
{

//节点屏蔽元信息
struct CheckTimeoutInfo
{
    //计算的最小的超时次数, 默认2次(在checkTimeoutInterval时间内超过了minTimeoutInvoke, 才计算超时)
    uint16_t    minTimeoutInvoke;

    //统计时间间隔, (默认60s)
    uint32_t    checkTimeoutInterval;

    //连续失败次数，节点连续失败次数大于它则认为失效（需要结合失败的持续时间）
    uint32_t    frequenceFailInvoke;

    //失败的持续时间，节点连着两次失败，且下一次比上一次的间隔大于它，则认为节点失效（需要结合连续失败次数）
    uint32_t    minFrequenceFailTime;

    //超时比例，大于该值则认为超时了 (0.1<=radio<=1.0)
    float        timeoutRatio;

    //重试时间间隔
    uint32_t    tryTimeInterval;                    

    CheckTimeoutInfo(): minTimeoutInvoke(2), checkTimeoutInterval(60), frequenceFailInvoke(5)
        , minFrequenceFailTime(5), timeoutRatio(0.5), tryTimeInterval(30) {}
};

//节点状态信息
struct RouterNodeStat
{
    std::string            ip;
    unsigned int        port;
    unsigned int        succNum;                //成功次数
    unsigned int        errNum;                //错误次数
    unsigned int        continueErrNum;        //持续错误的次数
    unsigned long long    timeCost;                //耗时
    bool                isAvailable;                //是否可用
    unsigned long long    nextRetryTime;            //下次再尝试的时间（在状态不可用的时候设置尝试）
    unsigned long long    lastCheckTime;            //上次统计时间
    unsigned long long    continueErrTime;        //错误持续时间点（用做统计时间点前的错误信息）
};

}

#endif
