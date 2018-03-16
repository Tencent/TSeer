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

#ifndef __API_ROUTER_H_
#define __API_ROUTER_H_

/*******************************************************
* 文件定义了路由表，该表具有负载均衡和屏蔽节点的能力
* 提供了获取节点、上报模调、更新路由的接口
* 该表记录了路由信息来源于ALL、IDC或是SET
********************************************************/

#include <time.h>
#include <string>
#include <vector>

#include "ApiLb.h"
#include "ApiLbFactory.h"

using namespace std;

enum GET_ROUTER_RES
{
    GET_ROUTER_OK = 0,
    GET_ROUTER_NOTFIND = -1,
    GET_ROUTER_TIMEOUT = -2,
};

class ApiRouter
{
public:
    /** 构造函数
     *    lbType:    该路由表的负载均衡类型
     */
    ApiRouter(Tseer::LB_TYPE lbType);

    virtual ~ApiRouter();

    /**    获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
     *    nodeInfo:        出参，节点信息
     *    errMsg:            出参，失败时返回的错误信息
     */
    int getRouter(Tseer::RouterNodeInfo& nodeInfo,string &errMsg);

    /** 通过KEY获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
     *    iKey:            入参，当根据hash key获取节点时，需要提供的key
     *    nodeInfo:        出参，节点信息
     *    errMsg:            出参，失败时返回的错误信息
     */
    int getRouter(unsigned long long key, Tseer::RouterNodeInfo& nodeInfo, string &errMsg);


    /** 获取路由节点，成功返回0，路由信息过期返回-2，其它失败返回-1
    *  nodeInfoList:      入参，更新的节点信息
    *  errMsg:           出参，失败时返回的错误信息
    */
    int getRouters(vector<Tseer::RouterNodeInfo> &nodeInfoList, string &errMsg);

    /** 更新节点列表，过期时间，成功返回0，失败返回-1
     *    vRouterNodeInfo:    入参，更新的节点信息
     *    iExpireInterval:    入参，下次过期的时间间隔
     *    errMsg:                出参，失败时返回的错误信息
     */
    int updateRouterNodes(const vector<Tseer::RouterNodeInfo> &nodeInfoList, string &errMsg);

    /**    上报某节点的调用结果，成功返回0，失败返回 -1
     *    nodeInfoRsp:  上报的节点信息
     *    iRet:        返回值
     *    timeCost:    调用耗时
     */
    int statReqResult(const Tseer::RouterNodeInfo &nodeInfoRsp, int ret, int timeCost);

    /** 更新过期时间，设置为现在时刻过后的interval秒
     */
    void updateExpireTime(time_t interval);

private:
    //禁用函数
    ApiRouter(const ApiRouter&);
    ApiRouter& operator=(const ApiRouter&);

private:

    size_t                              _expireTime;

    size_t                              _lastVisit;

    LoadBalance                            *_lb;
};

#endif
