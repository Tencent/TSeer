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

#ifndef __TSEER_API_REGISTRY_EP_MANAGER_H_
#define __TSEER_API_REGISTRY_EP_MANAGER_H_

/************************************************************************
 *    文件定义了Tseer服务端节点的管理类，该类负责维护Tseer服务端的节点
 ************************************************************************/

#include <vector>
#include <string>
#include <map>
#include "router_stat.h"

namespace Tseerapi
{

class RegistryEpManager
{
public:
    RegistryEpManager();

    //获取一个可用的路由节点，成功返回0，失败返回-1
    int getAvaliableIp(std::string &ip, std::string &errMsg);

    //上报节点调用情况，调用结果分为0（成功），-1（失败）
    void reportResult(const std::string &ip, int result);

    //设置Tseer服务端固定节点，一旦调用该函数，应用将一直循环使用这几个节点（不会去更新）
    //参数ips如果有多个ip，用"|"分割
    void setConsantIp(const std::string &ips);

private:

    void isNeedUpdate();

    //判断节点是否可用
    bool checkActive(const std::string &ip);

    //从DNS获取Tseer服务端信息
    int getRegistryFromDNS(std::string &errMsg);

private:
    //Tseer服务端的IP地址
    std::vector<std::string> _registryIps;

    //IP及它们节点状态
    std::map<std::string, RouterNodeStat> _ipsStat;

    //节点屏蔽控制信息
    CheckTimeoutInfo _checkTimeoutInfo;

    //轮询Tseer服务端，使用的下标
    size_t _index;

    //上次更新时间
    time_t _lastUpdateTime;

    //是否使用固定的几个IP
    bool _isIpsConstant;
};

}

#endif
