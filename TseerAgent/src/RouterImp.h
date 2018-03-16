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

#ifndef __AGENT_ROUTER_IMP_H_
#define __AGENT_ROUTER_IMP_H_

#include "TseerAgent.h"

class RouterImp : public Tseer::Router
{
public:
    
    virtual ~RouterImp() {}

    virtual void initialize();

    virtual void destroy();

    virtual tars::Int32 getRouterNode(const Tseer::AgentRouterRequest & req, Tseer::AgentRouterResponse &rsp,tars::TarsCurrentPtr current);

    //模调Stat数据上报
    virtual tars::Int32 reportStat(const Tseer::NodeStat & stat, tars::TarsCurrentPtr current);
private:
    enum GET_ROUTER_RET
    {
        GET_ROUTER_SUCCESS = 0,              //成功
        GET_ROUTER_NOT_EXIST = -1,             //服务不存在（常因Obj名字错误或服务已经下线引起）
        GET_ROUTER_FAILED = -2,             //失败
        GET_ROUTER_INNER_ERR = -3,             //内部错误
        GET_ROUTER_PARAM_ERR = -4             //入参错误

    };
};

#endif
