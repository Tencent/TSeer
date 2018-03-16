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

#ifndef __TSEER_API_ROUTE_INFO_SOURCE_H_
#define __TSEER_API_ROUTE_INFO_SOURCE_H_

/**********************************************************
 *    文件定义了路由信息源接口，以及若干实现类:
 *    1.从本地文件提供路由信息
 *    2.从Agent提供路由信息
 *    3.从Tseer服务端提供路由信息
 **********************************************************/

#include <vector>
#include <string>
#include "router.h"
#include "time.h"
#include "Tseer_inner_comm.h"
#include "cache_manager.h"
#include "registry_ep_manager.h"
#include "TseerAgentComm.h"

namespace Tseerapi {

/********************************************************
 接口
 ********************************************************/
class RouteInfoProvider
{
public:
    virtual ~RouteInfoProvider() {}

    //获取路由信息，成功获取返回0，失败（通信失败、IO错误等）返回-1；是否查到数据由调用层检查出参判断
    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg) = 0;

    //是否可用
    virtual    bool isAvailable() const = 0;

    //增加次数，并检查此时是否可用
    virtual bool addFailedNumAndCheckAvailable() = 0;

    //设置为可用
    virtual void setAvailable() = 0;

};

/********************************************************
 *    Tseer服务端
 ********************************************************/

class RegistryProvider : public RouteInfoProvider
{
public:
    RegistryProvider();

    void init(const std::string &cacheDir, RegistryEpManager *mgr);

    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg);

    //总是返回真
    virtual bool isAvailable() const;

    //总是返回真
    virtual bool addFailedNumAndCheckAvailable();

    //不执行任何操作
    void setAvailable();

private:
    //从DNS获取Tseer服务端信息
    int getRegistryFromDNS(std::string &errMsg);

private:
    //磁盘缓存
    CacheManager _cacheMgr;

    //Tseer服务端节点管理
    RegistryEpManager *_registryEpMgr;

    time_t _lastReportApiVersionTime;
};

/********************************************************
 *    Agent
 ********************************************************/

class AgentProvider : public RouteInfoProvider
{
public:
    AgentProvider();

    void init(const std::string &cacheDir);

    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg);

    //返回Agent是否可用
    virtual bool isAvailable() const;

    //返回Agent在这次失败以后，是否可以判断为不可用（当失败次数大于等于5次时认为不可用）
    virtual bool addFailedNumAndCheckAvailable();

    //设置Agent为可用
    void setAvailable();

private:
    //失败判定阈值，当失败次数多于该值时认为Agent故障
    static const int AGENT_FAIL_THRESHOLD;

    //磁盘缓存
    CacheManager _cacheMgr;

    //线程ID
    unsigned _tid;

    bool _available;

    int _failedNum;
};

/********************************************************
 *    磁盘缓存文件
 *******************************************************/

class CacheProvider : public RouteInfoProvider
{
public:
    CacheProvider();

    void setCacheDir(const std::string &cacheDir);

    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg);

    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::map<long, Tseer::RouterNodeInfo> &routerNodes, std::string &errMsg);
    
    virtual int getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::set<Tseer::RouterNodeInfo> &routerNodes, std::string &errMsg);

    //总是返回可用
    virtual bool isAvailable() const;

    //总是返回可用
    virtual bool addFailedNumAndCheckAvailable();

    //不执行任何操作
    virtual void setAvailable();

private:
    //缓存文件是否存在
    bool isFileExist(const std::string sFullFileName);

private:
    std::string _cacheDir;
};

}

#endif
