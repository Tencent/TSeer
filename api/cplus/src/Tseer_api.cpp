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

#include "Tseer_api.h"

#include <sys/syscall.h>
#include <errno.h>
#include <new>
#include <iostream>
#include <sstream>
#include "global.h"
#include "router_manager.h"
#include "route_info_provider.h"
#include "stat_manager.h"
#include "registry_ep_manager.h"
#include "tc_file.h"
#include "gc.h"
#include "tup.h"
#include "conn.h"
#include "Tseer_inner_comm.h"

namespace Tseerapi
{

//全局垃圾回收
static GC gc;

//每个使用api的线程有自己的路由、状态管理等数据
static __thread unsigned int tid = 0;               //线程id

//使用Agent api时需要初始化下列对象：
static __thread RouterManager *agent_router = NULL; 

//使用纯api时需要初始化下列对象：
static __thread RouterManager *registry_router = NULL;
static __thread RegistryEpManager *registry_ep_mgr = NULL;

//TUP通信时使用的消息顺序号以及接收数据buffer大小
static const unsigned int DEFAULT_RECV_BUFF_SIZE = 65535;

//接口级错误信息前缀
#ifndef INTERFACE_ERR
#define INTERFACE_ERR  "[tseeragent_router_api ERROR]|"
#endif

int ApiSetAgentIpInfo(const InitAgentApiParams &initParams, std::string &errMsg)
{
    //如果已经有值，那么就避免重复赋值（线上PHP的会反复调用该接口）
    if (!g_tseer_client_key.empty() || !g_reserved_value.empty())
    {
        return 0;
    }

    //如果g_init_times是0，则把值变成1，并返回true，否则返回false；原子操作
    if (__sync_bool_compare_and_swap(&g_init_times, 0, 1))
    {
        g_tseer_client_key = initParams.serviceKey;
        g_reserved_value = initParams.reserved;
    }
    return 0;
}

int ApiSetRegistryInfo(const InitRawApiParams &initParams, std::string &errMsg)
{
    //如果已经有值，那么就避免重复赋值
    if (!g_tseer_client_key.empty() || !g_reserved_value.empty())
    {
        return 0;
    }

    ostringstream errBuff;
    if (__sync_bool_compare_and_swap(&g_init_times, 0, 1))
    {
        g_is_using_registry = true;
        g_registry_port = initParams.registryPort;
        g_tseer_client_key = initParams.serviceKey;
        g_reserved_value = initParams.reserved;
        g_registry_dns_conf_file = initParams.dnsFile;

        //判断是使用域名方式还是直接指定IP方式
        //取domainName字段进行判断，如果符合 xx.xx.xx.xx|yy.yy.yy.yy ..则认为是ip模式
        vector<string> ips = TC_Common::sepstr<string>(initParams.domainName, "|");
        if (ips.size() > 1)
        {
            g_is_using_domain_to_get_registry = false;
            for (size_t i = 0; i < ips.size(); ++i)
            {
                //先判断长度，不可能大于15或小于7 (255.255.255.255，长度15；1.2.3.4，长度7)
                if (ips[i].size() > 15 || ips[i].size() < 7)
                {
                    errBuff << INTERFACE_ERR << FILE_FUN << "invalid ip length while using ips to initialize raw api, try remove useless blank character";
                    errMsg = errBuff.str();
                    return -1;
                }

                vector<string> ipSlice = TC_Common::sepstr<string>(ips[i], ".");
                if (ipSlice.size() != 4)
                {
                    errBuff << INTERFACE_ERR << FILE_FUN << "invalid ip while using ips to initialize raw api";
                    errMsg = errBuff.str();
                    return -1;
                }
                for (size_t j = 0; j < ipSlice.size(); ++j)
                {
                    if (!TC_Common::isdigit(ipSlice[j]))
                    {
                        errBuff << INTERFACE_ERR << FILE_FUN << "invalid ip while using ips to initialize raw api";
                        errMsg = errBuff.str();
                        return -1;
                    }

                    int sliceNum = TC_Common::strto<int>(ipSlice[j]);
                    if (sliceNum > 255 || sliceNum < 0)
                    {
                        errBuff << INTERFACE_ERR << FILE_FUN << "invalid ip while using ips to initialize raw api";
                        errMsg = errBuff.str();
                        return -1;
                    }
                }
            }

            g_registry_ips = initParams.domainName;
            return 0;
        }
        else if (ips.size() == 1)
        {
            //有可能是一串ip或域名
            vector<string> slice = TC_Common::sepstr<string>(ips[0], ".");
            if (slice.size() == 4)
            {
                //4段都是合法的ip数字，则认为是ip；否则跳转到域名逻辑
                for (size_t i = 0; i < slice.size(); ++i)
                {
                    if (!TC_Common::isdigit(slice[i]))
                    {
                        goto USE_DOMAIN;
                    }
                    int ipNum = TC_Common::strto<int>(slice[i]);
                    if (ipNum > 255 || ipNum < 0)
                    {
                        goto USE_DOMAIN;
                    }
                }

                g_registry_ips = initParams.domainName;
                g_is_using_domain_to_get_registry = false;
                return 0;
            }
            else
            {
        USE_DOMAIN:
                g_registry_dns_conf_file = initParams.dnsFile;
                g_registry_dns = initParams.domainName;

                //判断文件是否存在
                if (!TC_File::isFileExist(g_registry_dns_conf_file))
                {
                    errBuff << INTERFACE_ERR << FILE_FUN << "can not find dns config file in: " << g_registry_dns_conf_file;
                    errMsg = errBuff.str();
                    return -1;
                }
                if (g_registry_dns.empty())
                {
                    errBuff << INTERFACE_ERR << FILE_FUN << "null registry dns server domain";
                    errMsg = errBuff.str();
                    return -1;
                }

                    return 0;
                }
            }
            else
            {
                //domainName为空串
                errBuff << INTERFACE_ERR << FILE_FUN << "invalid value for param 'domainName', empty string";
                errMsg = errBuff.str();
                return -1;
            }
    }
    
    return 0;
}

static int initAgentRouter(std::string &errMsg)
{
    agent_router = new(std::nothrow) RouterManager;
    if (!agent_router)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create agent_router error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    //初始化远程的路由信息提供者
    AgentProvider *remoteProvider = new(std::nothrow) AgentProvider;
    if (!remoteProvider)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create agent remote provider error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        delete agent_router;
        agent_router = NULL;
        return -1;
    }
    
    remoteProvider->init("./routersCache/");
    //初始化本地的路由信息提供者
    CacheProvider *cacheProvider = new(std::nothrow) CacheProvider;
    if (!cacheProvider)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create cache provider error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        delete agent_router;
        agent_router = NULL;
        delete remoteProvider;
        remoteProvider = NULL;
        return -1;
    }
    if (!cacheProvider)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create uni cache provider error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        delete agent_router;
        agent_router = NULL;
        delete remoteProvider;
        remoteProvider = NULL;
        return -1;
    }
    cacheProvider->setCacheDir("./routersCache/");

    agent_router->setProvider(remoteProvider, cacheProvider);
    return 0;
}

//初始化registry_ep_mgr
static int initRegistryEpManager(std::string &errMsg)
{
    if (registry_ep_mgr)
    {
        return 0;
    }

    registry_ep_mgr = new(std::nothrow) RegistryEpManager;
    if (!registry_ep_mgr)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create registry endpoint manager error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }
    else
    {
        //如果是使用指定ip方式来初始化的，则设置上固定节点
        if (!g_is_using_domain_to_get_registry)
        {
            registry_ep_mgr->setConsantIp(g_registry_ips);
        }

        tid = syscall(SYS_gettid);
        gc.addObj(OBJ_REGISTRY_EP_MGR, registry_ep_mgr);
        return 0;
    }
}

//初始化registry_router
static int initRegistryRouter(std::string &errMsg)
{
    registry_router = new(std::nothrow) RouterManager;
    if (!registry_router)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create registry manager error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    //初始化远程路由信息提供者
    RegistryProvider *remoteProvider = new(std::nothrow) RegistryProvider;
    if (!remoteProvider)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create registry provider error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        delete registry_router;
        registry_router = NULL;
        return -1;
    }

    std::string subErr;
    int ret = initRegistryEpManager(subErr);
    if (ret)
    {
        delete registry_router;
        registry_router = NULL;
        delete remoteProvider;
        remoteProvider = NULL;
        
        ostringstream buffer;
        buffer << FILE_FUN << "init registry endpoint manager for registry router failed";
        errMsg = mergeErrMsg(buffer.str(), subErr);

        return -1;
    }

    remoteProvider->init("./routersCache/", registry_ep_mgr);

    //初始化本地路由信息提供者
    CacheProvider *cacheProvider = new(std::nothrow) CacheProvider;
    if (!cacheProvider)
    {
        ostringstream buffer;
        buffer << FILE_FUN << "create cache provider error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        delete remoteProvider;
        remoteProvider = NULL;
        delete registry_router;
        registry_router = NULL;
        //不需要清理registry_ep_mgr
        return -1;
    }
    cacheProvider->setCacheDir("./routersCache/");

    registry_router->setProvider(remoteProvider, cacheProvider);
    return 0;
}

static int getRoute(InnerRouterRequest &req, std::string& errMsg)
{
    int ret = 0;
    
    //根据使用Registry还是Agent，选择不同的路由查询方式
    if (g_is_using_registry)
    {
        if (!registry_router)
        {
            if (initRegistryRouter(errMsg))
            {
                return -1;
            }
            else
            {
                tid = syscall(SYS_gettid);
                gc.addObj(OBJ_ROUTE, registry_router, tid);
            }
        }
        req.isAgent = false;
        ret = registry_router->getRouter(req, errMsg);
    }
    else
    {
        if (!agent_router)
        {
            if (initAgentRouter(errMsg))
            {
                return -1;
            }
            else
            {
                tid = syscall(SYS_gettid);
                gc.addObj(OBJ_ROUTE, agent_router, tid);
            }
        }
        if (req.type != LB_TYPE_ALL)
        {
            req.isAgent = true;
        }
        else
        {
            req.isAgent = false;
        }
        ret = agent_router->getRouter(req, errMsg);
    }

    //用户输入错误的服务名称，或服务已经下线
    if (ret == -1)
    {
        errMsg = "Cannot find endpoints for obj:" + req.obj + "|set:" + req.setInfo + 
            ", check the correctness of slave obj name, or the obj has been deleted";
        return -1;
    }
    else if (ret == -2)
    {
        const std::string agentErr = "Connect to Agent failed and cannot find obj from cache.";
        const std::string registryErr = "Connect to Registry failed and cannot find obj from cache.";
        if (g_is_using_registry)
        {
            std::string errTmp = registryErr + "|obj:" + req.obj + "|set:" + req.setInfo;
            errMsg = mergeErrMsg(errTmp, errMsg);
        }
        else
        {
            std::string errTmp = agentErr + "|obj:" + req.obj + "|set:" + req.setInfo;
            errMsg = mergeErrMsg(errTmp, errMsg);
        }
    }

    return ret;
}

int ApiGetRoutes(RoutersRequest &req, std::string &errMsg)
{
    ostringstream errBuff;

    if (req.obj.empty())
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "empty server obj name";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    if (req.lbGetType != LB_GET_IDC && req.lbGetType != LB_GET_SET && req.lbGetType != LB_GET_ALL)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "invalid LB_GET_TYPE";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    if (req.lbGetType == LB_GET_SET && req.setInfo.empty())
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "empty set info while getting from set";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    InnerRouterRequest r(req);
    int ret = getRoute(r, errMsg);
    req.nodeInfoVec = r.nodeInfoVec;
    if (ret != 0)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << errMsg;
        errMsg = errBuff.str();
        switch (ret)
        {
        case -1:
            return GET_ROUTER_NOT_EXIST;
        case -2:
            return GET_ROUTER_FAILED;
        case -3:
            return GET_ROUTER_INNER_ERR;
        default:
            return GET_ROUTER_FAILED;   //不可能执行到这一步
        }
    }
    return GET_ROUTER_SUCCESS;
}

int ApiGetRoute(RouterRequest &req, std::string &errMsg)
{
    ostringstream errBuff;

    if (req.obj.empty())
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "empty server obj name";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    if (req.lbGetType != LB_GET_IDC && req.lbGetType != LB_GET_SET && req.lbGetType != LB_GET_ALL)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "invalid LB_GET_TYPE";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    if (req.lbGetType == LB_GET_SET && req.setInfo.empty())
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "empty set info while getting from set";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }
    
    if (req.type != LB_TYPE_LOOP && req.type != LB_TYPE_RANDOM && req.type != LB_TYPE_STATIC_WEIGHT 
        && req.type != LB_TYPE_CST_HASH)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << "invalid LB_TYPE";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;
    }

    if(req.type == LB_TYPE_ALL/*轮训类型不能为all*/)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << " LB_TYPE_ALL is limited";
        errMsg = errBuff.str();
        return GET_ROUTER_PARAM_ERR;  
    }
    
    InnerRouterRequest r(req);
    int ret = getRoute(r, errMsg);
    req.ip = r.ip;
    req.port = r.port;
    req.isTcp = r.isTcp;
    req.slaveSet = r.slaveSet;
    if (ret != 0)
    {
        errBuff << INTERFACE_ERR << FILE_FUN << errMsg;
        errMsg = errBuff.str();
        switch (ret)
        {
        case -1:
            return GET_ROUTER_NOT_EXIST;
        case -2:
            return GET_ROUTER_FAILED;
        case -3:
            return GET_ROUTER_INNER_ERR;
        default:
            return GET_ROUTER_FAILED;   //不可能执行到这一步
        }
    }
    return GET_ROUTER_SUCCESS;
}

int ApiRouteResultUpdate(const RouterRequest &req, int ret, int timeCost, std::string &errMsg)
{
    //入参检查
    if (req.obj.empty())
    {
        ostringstream buffer;
        buffer << INTERFACE_ERR << FILE_FUN << "empty obj";
        errMsg = buffer.str();
        return -1;
    }

    //更新节点屏蔽信息
    InnerRouterRequest r(req);
    if (!g_is_using_registry && req.type != LB_TYPE_ALL)
    {
        r.isAgent = true;
    }

    if (agent_router)
    {
        agent_router->updateStat(r, ret, timeCost, errMsg);
    }
    
    if (registry_router)
    {
        registry_router->updateStat(r, ret, timeCost, errMsg);
    }
    return 0;
}

}
