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

#ifndef __API_ROUTER_MANAGER_H_
#define __API_ROUTER_MANAGER_H_

#include <map>
#include <string>
#include "ApiRouter.h"
#include "util/tc_lock.h"

/***********************************************
* 用来管理所有api过来的路由请求
************************************************/
class ApiRouterManager
{
protected:

    // 格式为 <server name + | + GET_TYPE, ApiRouter*>，
    // 每个服务的名称加上获取类型作为KEY（一个服务可能有ALL、IDC、SET三种方式）
    typedef std::map<std::string, ApiRouter* >                     RouteTable;
    typedef RouteTable::iterator                                RouteTableIter;
    typedef RouteTable::value_type                              RouteTableIterElem;
    typedef std::pair<RouteTableIter, bool>                     RouteTableIterRet;

public:
    ApiRouterManager(const string& identifier);

    virtual ~ApiRouterManager();
        
    /** 获取某节点的路由
     *  当查询不到或者内存的路由信息过期时，会从外部更新路由
     *  req:    入参、出参，服务的请求信息以及返回的节点、真实获取方式
     *  errMsg:   出参，发生错误时返回的错误信息
     *
     *  返回值：
     *  0，成功
     *  -1，由于服务不存在找不到节点（用户输入名字有误，或服务已经下线）
     *  -2，远程路由信息提供者错误，本地缓存也找不到节点
     *  -3，内部错误
     */
    int getRouter(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg);

    /** 上报服务调用结果，成功返回0，失败返回-1
     *  req:        入参，服务信息
     *  nodeInfoRsp:  入参,要更新的被调状态信息
     *  iRet:       入参，服务调用的返回值
     *  timeCost:  入参，服务调用耗时
     *  errMsg:       出参，发生错误时返回的错误信息
     */
    int updateStat(const Tseer::AgentRouterRequest &req, const Tseer::RouterNodeInfo &nodeInfoRsp, int iRet, int timeCost, string &errMsg);

    /**
      * 满足一段时间不在访问
      */
    bool timeout() const;
    const string& getIdentifier() const;
private:

    //从指定路由表中获取某个服务的路由，如果没有则插入
    ApiRouter* getRouterFromTable(const Tseer::AgentRouterRequest &req, string &errMsg, RouteTable &routerTable);

    //getRouter的辅助函数
    int getRouter(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp,string &errMsg, RouteTable &routerTable, bool bHash = false);

    //updateStat的辅助函数
    int updateStat(const Tseer::AgentRouterRequest &req, const Tseer::RouterNodeInfo &NodeInfoRsp, int iRet, int timeCost, string &errMsg, RouteTable &routerTable);

    //从内存中更新节点信息
    int getRouterFromRAM(ApiRouter *r, string &errMsg, const Tseer::AgentRouterRequest &req,vector<Tseer::RouterNodeInfo>& nodeInfoList, bool bHash);

    int getRouterFromServer(const Tseer::AgentRouterRequest & req,Tseer::AgentRouterResponse &rsp,string &errMsg);
    int getIdcRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp);
    int getSetRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp);
    int getAllRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp);

    int getRouterFromDiskCache(const Tseer::AgentRouterRequest & req,Tseer::AgentRouterResponse &rsp,string &errMsg);
protected:
    //轮询路由表
    RouteTable          _loopRouterTable;
    TC_ThreadLock _loopRouterLock;
    
    //随机路由表
    RouteTable          _randomRouterTable;
    TC_ThreadLock _randomRouterLock;

    //哈希路由表
    RouteTable          _chashRouterTable;
    TC_ThreadLock _chashRouterLock;

    //静态权重路由表
    RouteTable          _staticWeightRouterTable;
    TC_ThreadLock _staticWRouterLock;

    //无策略路由表
    RouteTable          _allRouterTable;
    TC_ThreadLock _allRouterLock;
private:
    //用来记录上次api访问的时间，如果超过一段时间不访问，需要清理
    time_t   _lastVist;
    string    _identifier;
};

#endif

