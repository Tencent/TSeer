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

#ifndef __TSEER_API_ROUTER_H_
#define __TSEER_API_ROUTER_H_

/*******************************************************
* 文件定义了路由表，该表具有负载均衡和屏蔽节点的能力
* 提供了获取节点、上报模调、更新路由的接口
* 该表记录了路由信息来源于ALL、IDC或是SET
********************************************************/

#include <time.h>
#include <string>
#include <vector>
#include "Tseer_inner_comm.h"
#include "TseerAgentComm.h"
#include "stat_manager.h"
#include "lb.h"
#include "lb_factory.h"

namespace Tseerapi
{

class Router
{
public:
    /** 构造函数
     *    type:    该路由表的负载均衡类型
     */
    Router(LB_TYPE type);

    virtual ~Router();

    /**    获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
     *    nodeInfo:        出参，节点信息
     *    errMsg:            出参，失败时返回的错误信息
     */
    int getRouter(Tseer::RouterNodeInfo& nodeInfo, std::string &errMsg);

    /** 通过KEY获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
     *    key:            入参，当根据hash key获取节点时，需要提供的key
     *    nodeInfo:        出参，节点信息
     *    errMsg:            出参，失败时返回的错误信息
     */
    int getRouter(unsigned long long key, Tseer::RouterNodeInfo& nodeInfo, std::string &errMsg);

    /** 获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
     *  nodeInfoVec:      入参，更新的节点信息
     *  errMsg:           出参，失败时返回的错误信息
    */
    int getRouters(std::vector<Tseer::RouterNodeInfo> &nodeInfoVec, std::string &errMsg);
    /** 更新节点列表，过期时间，成功返回0，失败返回-1
     *    RouterNodeInfoVec:    入参，更新的节点信息
     *    expireInterval:    入参，下次过期的时间间隔
     *    errMsg:                出参，失败时返回的错误信息
     */
    int updateRouterNodes(const std::vector<Tseer::RouterNodeInfo> &RouterNodeInfoVec, std::string &errMsg);

    /**    上报某节点的调用结果，成功返回0，失败返回 -1
     *    ret:        返回值
     *    timeCost:    调用耗时
     */
    int statReqResult(const InnerRouterRequest& req, int ret, int timeCost);

    /** 更新过期时间，设置为现在时刻过后的interval秒
     */
    void updateExpretime(time_t interval);

private:
    //禁用函数
    Router(const Router&);
    Router& operator=(const Router&);

private:

    size_t                              _expreTime;

    size_t                              _lastVisit;

    LoadBalance                            *_lb;

    StatManager                            _statMgr;
};

}

#endif
