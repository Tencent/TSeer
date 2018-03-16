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

#ifndef __TSEER_API_ROUTER_MGR_H_
#define __TSEER_API_ROUTER_MGR_H_

/******************************************************
* 该文件定义了路由管理类，该类的作用是
* 1.维护服务与路由表间的关系
* 2.封装路由表（Router）的接口对外提供路由获取、模调上报的接口
* 3.自动从外部更新路由信息
******************************************************/

#include <map>
#include <string>
#include "router.h"
#include "Tseer_inner_comm.h"
#include "route_info_provider.h"

namespace Tseerapi
{

class RouterManager
{
protected:

    // 格式为 <server name + | + GET_TYPE, Router*>，
    // 每个服务的名称加上获取类型作为KEY（一个服务可能有ALL、IDC、SET三种方式）
    typedef std::map<std::string, Router* >                                    RouteTable;
    typedef RouteTable::iterator                                            RouteTableIter;
    typedef RouteTable::value_type                                            RouteTableIterElem;
    typedef std::pair<RouteTableIter, bool>                                    RouteTableIterRet;
    typedef std::map<std::string, std::map<long, Tseer::RouterNodeInfo> >    HashRamCache;
    typedef std::map<std::string, std::set<Tseer::RouterNodeInfo> >            UniRamCache;
    typedef UniRamCache::iterator                                            UniRamCacheIter;
    typedef HashRamCache::iterator                                            HashRamCacheIter;
    typedef std::map<std::string, std::set<Tseer::RouterNodeInfo>::iterator> UniCacheIndex;
    typedef UniCacheIndex::iterator                                            UnnodeIndexIter;

public:
    RouterManager();

    virtual ~RouterManager();
    
    //设置路由信息获取的源头
    void setProvider(RouteInfoProvider *remoteProvider, CacheProvider *cacheProvider);    
    /** 获取某节点的路由
     *    当查询不到或者内存的路由信息过期时，会从外部更新路由
     *    req:    入参、出参，服务的请求信息以及返回的节点、真实获取方式
     *    errMsg:    出参，发生错误时返回的错误信息
     *
     *    返回值：
     *    0，成功
     *    -1，由于服务不存在找不到节点（用户输入名字有误，或服务已经下线）
     *    -2，远程路由信息提供者错误，本地缓存也找不到节点
     *    -3，内部错误
     */
    int getRouter(InnerRouterRequest &req, std::string &errMsg);

    /**    上报服务调用结果，成功返回0，失败返回-1
     *    req:        入参，服务信息
     *    ret:        入参，服务调用的返回值
     *    timeCost:    入参，服务调用耗时
     *    errMsg:        出参，发生错误时返回的错误信息
     */
    int updateStat(const InnerRouterRequest &req, int ret, int timeCost, std::string &errMsg);

protected:

    //从指定路由表中获取某个服务的路由，如果没有则插入
    Router* getRouterFrotableMap(InnerRouterRequest &req, std::string &errMsg, RouteTable &routerTable);

    //getRouter的辅助函数，处理Agent ApiGetRouters，纯Api ApiGetRouter/ApiGetRouters
    int getRouterNormal(InnerRouterRequest &req, std::string &errMsg, RouteTable &routerTable, bool isHash = false);

    //getRouter的辅助函数，主要进行Agent ApiGetRouter的处理
    int getRouterSingle(InnerRouterRequest &req, std::string &errMsg, bool isHash = false);

    //updateStat的辅助函数
    int updateStat(const InnerRouterRequest &req, int ret, int timeCost, std::string &errMsg, RouteTable &routerTable);

    //从内存中更新节点信息
    int getRouterFromRAM(Router *r, std::string &errMsg, InnerRouterRequest &req, Tseer::RouterNodeInfo &nodeInfo, bool isHash);

    //更新UniRamCache的节点信息
    int getRouterFromUniRam(std::string &errMsg, InnerRouterRequest &req, Tseer::RouterNodeInfo &nodeInfo, bool isHash);
protected:

    //远程路由信息提供者（Agent或Registry）
    RouteInfoProvider    *_remoteProvider;

    //磁盘缓存路由信息提供者
    CacheProvider    *_cacheProvider;

    //轮询路由表
    RouteTable            _loopRouterTable;

    //随机路由表
    RouteTable            _randomRouterTable;

    //哈希路由表
    RouteTable            _chashRouterTable;

    //静态权重路由表
    RouteTable            _staticWeightRouterTable;

    //无策略路由表
    RouteTable          _allRouterTable;

    CacheManager        _cacheMgr;

    StatManager            _statMgr;

    UniRamCache            _uniRamCache;

    HashRamCache        _hashRamCache;

    UniCacheIndex         _uniCacheIndex;
};

}

#endif
