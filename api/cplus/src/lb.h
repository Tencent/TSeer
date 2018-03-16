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

#ifndef __TSEER_API_LB_H_
#define __TSEER_API_LB_H_

#include <string>
#include "Tseer_inner_comm.h"
#include "TseerAgentComm.h"

namespace Tseerapi
{

class LoadBalance
{
public:
    virtual ~LoadBalance(){};

    //新增节点
    virtual int addRouter(const Tseer::RouterNodeInfo& nodeInfo) = 0;

    //根据负载均衡获取一个节点
    virtual int getRouter(Tseer::RouterNodeInfo& nodeInfo, std::string &errMsg);

    //根据负载均衡和key值获取一个节点，只当负载均衡算法与key值相关时此函数才有意义
    virtual int getRouter(unsigned long long key, Tseer::RouterNodeInfo& nodeInfo, std::string &errMsg);

    //获取所有节点
    virtual int getRouters(std::vector<Tseer::RouterNodeInfo> &nodeInfoVec, std::string &errMsg);

    //清理节点
    virtual void clear() = 0;

    //重建所有状态
    virtual int rebuild() = 0;

    //删除指定的节点
    virtual int del(const std::vector<Tseer::RouterNodeInfo> &nodeInfoVec) = 0;

    //记录节点调用状态
    virtual int statResult(const Tseer::RouterNodeInfo& req, int ret, int timeCost) = 0;
};

}

#endif
