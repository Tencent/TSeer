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

#include "router_manager.h"

#include <sys/syscall.h>
#include <errno.h>
#include <sstream>
#include "global.h"
#include "TseerAgentComm.h"

using std::ostringstream;
using std::string;
using std::vector;
using std::map;
using std::set;
using Tseer::RouterNodeInfo;

namespace Tseerapi
{

RouterManager::RouterManager() :_remoteProvider(NULL) {}

RouterManager::~RouterManager()
{
    RouteTableIter begin, end;
    for (begin = _loopRouterTable.begin(), end = _loopRouterTable.end(); begin != end; begin++)
    {
        if (begin->second)
        {
            delete (begin->second);
            begin->second = NULL;
        }
    }
    _loopRouterTable.clear();

    for (begin = _randomRouterTable.begin(), end = _randomRouterTable.end(); begin != end; begin++)
    {
        if (begin->second)
        {
            delete (begin->second);
            begin->second = NULL;
        }
    }
    _randomRouterTable.clear();

    for (begin = _chashRouterTable.begin(), end = _chashRouterTable.end(); begin != end; begin++)
    {
        if (begin->second)
        {
            delete (begin->second);
            begin->second = NULL;
        }
    }
    _chashRouterTable.clear();

    //静态权重的负载表清理
    for (begin = _staticWeightRouterTable.begin(), end = _staticWeightRouterTable.end(); begin != end; ++begin)
    {
        if (begin->second)
        {
            delete (begin->second);
            begin->second = NULL;
        }
    }
    _staticWeightRouterTable.clear();

    for (begin = _allRouterTable.begin(), end = _allRouterTable.end(); begin != end; ++begin)
    {
        if (begin->second)
        {
            delete (begin->second);
            begin->second = NULL;
        }
    }
    _allRouterTable.clear();

    delete _remoteProvider;
}

void RouterManager::setProvider(RouteInfoProvider *remoteProvider, CacheProvider *cacheProvier)
{
    assert(remoteProvider != NULL);
    assert(cacheProvier != NULL);
    _remoteProvider = remoteProvider;
    _cacheProvider = cacheProvier;
}

int RouterManager::getRouter(InnerRouterRequest &req, string &errMsg)
{
    if (_remoteProvider == NULL || _cacheProvider == NULL)
    {
        ostringstream os;
        os << FILE_FUN << "not init provider for router";
        errMsg = os.str();
        return -1;
    }

    if (req.isAgent) {
        switch (req.type)
        {
        case LB_TYPE_ALL:
            return getRouterNormal(req, errMsg, _allRouterTable);
        case LB_TYPE_LOOP:
        case LB_TYPE_RANDOM:
        case LB_TYPE_STATIC_WEIGHT:
            return getRouterSingle(req, errMsg);
        case LB_TYPE_CST_HASH:
            return getRouterSingle(req, errMsg, true);
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance type";
            errMsg = os.str();
            return -1;
        }

        }
    }
    else
    {
        switch (req.type)
        {
        case LB_TYPE_LOOP:
            return getRouterNormal(req, errMsg, _loopRouterTable);
        case LB_TYPE_RANDOM:
            return getRouterNormal(req, errMsg, _randomRouterTable);
        case LB_TYPE_CST_HASH:
            return getRouterNormal(req, errMsg, _chashRouterTable, true);   //isHash设置为真
        case LB_TYPE_STATIC_WEIGHT:
            return getRouterNormal(req, errMsg, _staticWeightRouterTable);
        case LB_TYPE_ALL:
            return getRouterNormal(req, errMsg, _allRouterTable);
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance type";
            errMsg = os.str();
            return -1;
        }
        }
    }
}
int RouterManager::updateStat(const InnerRouterRequest &req, int ret, int timeCost, string &errMsg)
{
    if (req.isAgent) {
        switch(req.type)
        {
        case LB_TYPE_ALL:
            return updateStat(req, ret, timeCost, errMsg, _allRouterTable);
        case LB_TYPE_LOOP:
        case LB_TYPE_RANDOM:
        case LB_TYPE_STATIC_WEIGHT:
        case LB_TYPE_CST_HASH:
            return _statMgr.reportStat(req, ret, timeCost);
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance type";
            errMsg = os.str();
            return -1;
        }
        }
    }
    else
    {
        switch(req.type)
        {
        case LB_TYPE_LOOP:
            return updateStat(req, ret, timeCost, errMsg, _loopRouterTable);
        case LB_TYPE_RANDOM:
            return updateStat(req, ret, timeCost, errMsg, _randomRouterTable);
        case LB_TYPE_CST_HASH:
            return updateStat(req, ret, timeCost, errMsg, _chashRouterTable);
        case LB_TYPE_STATIC_WEIGHT:
            return updateStat(req, ret, timeCost, errMsg, _staticWeightRouterTable);
        case LB_TYPE_ALL:
            return updateStat(req, ret, timeCost, errMsg, _allRouterTable);
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance type";
            errMsg = os.str();
            return -1;
        }
        }
    }
}

Router* RouterManager::getRouterFrotableMap(InnerRouterRequest &req, string &errMsg, RouteTable &routerTable)
{
    //根据服务信息转化出路由表的KEY
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);

    if (iter == routerTable.end())
    {

        Router *r = new(std::nothrow) Router(req.type);
        //TODO：创建失败的处理
        if (!r)
        {
            ostringstream buffer;
            buffer << FILE_FUN << "create router error|errno:" << errno << "|info:" << strerror(errno);
            errMsg = buffer.str();
        }

        RouteTableIterRet ret = routerTable.insert(RouteTableIterElem(tableKey, r));
        if (!ret.second)    //插入失败
        {
            delete r;
            r = NULL;

            ostringstream buffer;
            buffer << FILE_FUN << "insert router error|errno:" << errno << "|info:" << strerror(errno);
            errMsg = buffer.str();
        }
        return r;
    }
    else
    {
        return iter->second;
    }
}

int RouterManager::getRouterFromRAM(Router *r, string &errMsg, InnerRouterRequest &req, Tseer::RouterNodeInfo &nodeInfo, bool isHash)
{
    int ret = 0;
    vector<RouterNodeInfo> nodeInfoVec;
    if (isHash)
        ret = r->getRouter(req.hashKey, nodeInfo, errMsg);
    else if (req.type != LB_TYPE_ALL)
        ret = r->getRouter(nodeInfo, errMsg);
    else
        ret = r->getRouters(nodeInfoVec, errMsg);
    if (ret == 0) //如果节点未过期
    {
        req.ip = nodeInfo.ip;
        req.port = nodeInfo.port;
        req.isTcp = nodeInfo.isTcp;
        req.slaveSet = nodeInfo.setname;
        if (req.type == LB_TYPE_ALL)
        {
            req.nodeInfoVec.clear();
            for (size_t i = 0; i < nodeInfoVec.size(); i++) {
                NodeInfo info;
                info.ip = nodeInfoVec[i].ip;
                info.port = nodeInfoVec[i].port;
                info.isTcp = nodeInfoVec[i].isTcp;
                info.slaveSet = nodeInfoVec[i].setname;
                req.nodeInfoVec.push_back(info);
            }
        }
    }
    return ret;
}

int RouterManager::getRouterFromUniRam(string & errMsg, InnerRouterRequest & req, Tseer::RouterNodeInfo & nodeInfo, bool isHash)
{    
    string tableKey = getUnkeyFromReq(req);
    if (isHash)
    {
        HashRamCacheIter iter = _hashRamCache.find(tableKey);
        if (iter != _hashRamCache.end() && iter->second.size() != 0)
        {
            map<long, RouterNodeInfo>::iterator it_hash = iter->second.lower_bound(req.hashKey);
            if (it_hash != iter->second.end())
            {
                req.ip = it_hash->second.ip;
                req.port = it_hash->second.port;
                req.isTcp = it_hash->second.isTcp;
                req.slaveSet = it_hash->second.setname;
                
                return 0;
            }
            else
            {
                req.ip = iter->second.begin()->second.ip;
                req.port = iter->second.begin()->second.port;
                req.isTcp = iter->second.begin()->second.isTcp;
                req.slaveSet = iter->second.begin()->second.setname;

                return 0;
            }
        }
        else
        {
            //被调服务内存map不存在
            return -1;
        }
    }
    else
    {
        UniRamCacheIter iter = _uniRamCache.find(tableKey);
        if (iter != _uniRamCache.end() && iter->second.size() != 0)
        {
            UnnodeIndexIter indexIter = _uniCacheIndex.find(tableKey);
            if (indexIter == _uniCacheIndex.end())
            {
                //上次更新位置不存在
                _uniCacheIndex.insert(std::make_pair(tableKey, iter->second.begin()));
            }
            std::set<RouterNodeInfo>::iterator setIter = _uniCacheIndex.find(tableKey)->second;
            if (setIter == iter->second.end())
            {
                _uniCacheIndex.find(tableKey)->second = iter->second.begin();
                setIter = iter->second.begin();
            }
            
            req.ip = setIter->ip;
            req.port = setIter->port;
            req.isTcp = setIter->isTcp;
            req.slaveSet = setIter->setname;
            
            _uniCacheIndex.find(tableKey)->second++;
            return 0;
        }
        else
        {
            //被调服务内存set不存在
            return -1;
        }
    }
}

int RouterManager::getRouterSingle(InnerRouterRequest &req, string &errMsg, bool isHash)
{
    int ret = 0;
    string tableKey = getUnkeyFromReq(req);
    Tseer::RouterNodeInfo nodeInfo;
    Tseer::AgentRouterResponse rsp;
    string fileName = req.obj + req.slaveSet + TC_Common::tostr(req.lbGetType) + TC_Common::tostr(req.type);
    
    if (_remoteProvider->isAvailable())
    {
        ret = _remoteProvider->getRouteInfo(req, rsp, errMsg);
        if (ret == 0)
        {
            //填充返给客户的信息
            if (rsp.resultList.size() == 0 || rsp.resultList.size() > 1) {
                //远端返回内容不合法
                goto cache;
            }
            nodeInfo.ip = rsp.resultList[0].ip;
            nodeInfo.port = rsp.resultList[0].port;
            nodeInfo.isTcp = rsp.resultList[0].isTcp;
            nodeInfo.setname = rsp.resultList[0].setname;

            req.ip = rsp.resultList[0].ip;
            req.port = rsp.resultList[0].port;
            req.isTcp = rsp.resultList[0].isTcp;
            req.slaveSet = rsp.resultList[0].setname;
            
            HashRamCacheIter hashIter = _hashRamCache.find(tableKey);
            UniRamCacheIter uniIter = _uniRamCache.find(tableKey);

            //更新内存信息
            if (isHash)
            {
                if (hashIter == _hashRamCache.end())
                {
                    //首次访问，内存缓存map并不存在
                    std::map<long, RouterNodeInfo> tmpMap;
                    tmpMap.insert(std::make_pair(req.hashKey, nodeInfo));
                    _hashRamCache.insert(std::make_pair(tableKey, tmpMap));
                }
                else
                {
                    hashIter->second[req.hashKey] = nodeInfo;
                }
                
                //更新文件缓存
                _cacheMgr.updateHashCache(tableKey, fileName, _hashRamCache[tableKey]);
            }
            else
            {
                if (uniIter == _uniRamCache.end())
                {
                    //首次访问，内存缓存set并不存在
                    std::set<RouterNodeInfo> tmpSet;
                    tmpSet.insert(nodeInfo);
                    _uniRamCache.insert(std::make_pair(tableKey, tmpSet));
                }
                else
                {
                    uniIter->second.insert(nodeInfo);
                }
                
                //更新文件缓存
                _cacheMgr.updateUniCache(tableKey, fileName, _uniRamCache[tableKey]);
                
                std::set<RouterNodeInfo>::iterator indexIter;
                std::map<string, set<RouterNodeInfo>::iterator>::iterator indexMapIter;
                indexMapIter = _uniCacheIndex.find(tableKey);
                if (indexMapIter == _uniCacheIndex.end()) {
                    indexIter = _uniRamCache[tableKey].begin();
                    _uniCacheIndex.insert(std::make_pair(tableKey, indexIter));
                }
                indexIter = _uniCacheIndex.find(tableKey)->second;
                if (indexIter == _uniRamCache[tableKey].end())
                {
                    _uniCacheIndex.find(tableKey)->second = _uniRamCache[tableKey].begin();
                }
                else
                {
                    _uniCacheIndex.find(tableKey)->second++;
                }
            }

            return 0;

        }//if (ret == 0)
        else
        {
            goto cache;
        }
    }//if (_remoteProvider->isAvailable())
    else
    {
    cache:
        ret = getRouterFromUniRam(errMsg, req, nodeInfo, isHash);
        if (ret == 0)
        {
            return 0;
        }
        
        std::string subErr;
        if (isHash)
        {
            std::map<long, RouterNodeInfo> newNodeMap;
            ret = _cacheProvider->getRouteInfo(req, rsp, newNodeMap, subErr);
            if (ret == 0)
            {
                _hashRamCache.insert(std::make_pair(tableKey, newNodeMap));
                //再次在内存中查找
                ret = getRouterFromUniRam(errMsg, req, nodeInfo, isHash);
                if (ret == -1)
                {
                    //远程错误，本地也找不到节点
                    return -2;
                }
                
                return 0;
            }
            else
            {
                //远程错误，本地也找不到节点
                return -2;
            }
        }
        else
        {
            std::set<RouterNodeInfo> newNodeSet;
            ret = _cacheProvider->getRouteInfo(req, rsp, newNodeSet, subErr);
            if (ret == 0)
            {
                _uniRamCache.insert(std::make_pair(tableKey, newNodeSet));
                //再次在内存中查找
                ret = getRouterFromUniRam(errMsg, req, nodeInfo, isHash);
                if (ret == -1)
                {
                    //远程错误，本地也找不到节点
                    return -2;
                }
                
                return 0;
            }
            else
            {
                //远程错误，本地也找不到节点
                return -2;
            }
        }
    }
    return 0;
}

int RouterManager::getRouterNormal(InnerRouterRequest &req, string &errMsg, RouteTable &routerTable, bool isHash /* = false */)
{
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);
    Tseer::RouterNodeInfo nodeInfo;
    if (iter != routerTable.end())
    {
        int ret = 0;
        vector<RouterNodeInfo> nodeInfoVec;
        if (isHash)
            ret = iter->second->getRouter(req.hashKey, nodeInfo, errMsg);
        else if(req.type != LB_TYPE_ALL)
            ret = iter->second->getRouter(nodeInfo, errMsg);
        else
            ret = iter->second->getRouters(nodeInfoVec, errMsg);
        if (ret == 0) //如果节点未过期
        {
            req.ip = nodeInfo.ip;
            req.port = nodeInfo.port;
            req.isTcp = nodeInfo.isTcp;
            req.slaveSet = nodeInfo.setname;
            if(req.type == LB_TYPE_ALL)
            {
                req.nodeInfoVec.clear();
                for(size_t i = 0; i < nodeInfoVec.size(); i++) {
                    NodeInfo info;
                    info.ip = nodeInfoVec[i].ip;
                    info.port = nodeInfoVec[i].port;
                    info.isTcp = nodeInfoVec[i].isTcp;
                    info.slaveSet = nodeInfoVec[i].setname;
                    req.nodeInfoVec.push_back(info);
                }
            }
            return 0;
        }
        else if (ret == -2)        //节点过期
        {
            //保留过期的节点信息
            req.ip = nodeInfo.ip;
            req.port = nodeInfo.port;
            req.isTcp = nodeInfo.isTcp;
            req.slaveSet = nodeInfo.setname;

            req.nodeInfoVec.clear();
            for(size_t i = 0; i < nodeInfoVec.size(); i++) {
                NodeInfo info;
                info.ip = nodeInfoVec[i].ip;
                info.port = nodeInfoVec[i].port;
                info.isTcp = nodeInfoVec[i].isTcp;
                info.slaveSet = nodeInfoVec[i].setname;
                req.nodeInfoVec.push_back(info);
            }

            //向远程provider查询路由
            Tseer::AgentRouterResponse rsp;
            int ret = _remoteProvider->getRouteInfo(req, rsp, errMsg);
            if (ret == 0)
            {
                _remoteProvider->setAvailable();

                //更新内存信息
                Router *r = getRouterFrotableMap(req, errMsg, routerTable);
                if (r->updateRouterNodes(rsp.resultList, errMsg))
                {
                    ostringstream os;
                    os << FILE_FUN << "update router info in memory failed";
                    mergeErrMsg(os.str(), errMsg);
                    return -3;
                }
                r->updateExpretime(g_node_normal_expire_interval);     //远程Provider可用，设置正常的过期时间

                //获取节点
                int ret = getRouterFromRAM(r, errMsg, req, nodeInfo, isHash);
                if(ret == 0)
                {
                    return 0;
                }

                return -1;      //节点不存在（用户输入服务名错误，或服务已经下线），返回-1
            }
            else  //远程Provider通信异常
            {
                if (!_remoteProvider->isAvailable())        //已经不可用的状况下保留原来状态
                {
                    Router *r = getRouterFrotableMap(req, errMsg, routerTable);
                    if (r == NULL)
                    {
                        ostringstream os;
                        os << FILE_FUN << "create map failed:" + tableKey;
                        errMsg = os.str();
                        return -3;      //其它错误
                    }
                    r->updateExpretime(g_node_long_expire_interval);       //远程Provider不可用，设置更长的过期时间

                    //使用过期的节点
                    return 0;
                }
                else  //探测远程Provider是否已经出了问题
                {
                    if (!_remoteProvider->addFailedNumAndCheckAvailable())
                    {
                        Router *r = getRouterFrotableMap(req, errMsg, routerTable);
                        if (r == NULL)
                        {
                            ostringstream os;
                            os << FILE_FUN << "create map failed:" + tableKey;
                            errMsg = os.str();
                            return -3;      //其它错误
                        }
                        r->updateExpretime(g_node_long_expire_interval);       //远程Provider不可用，设置更长的过期时间

                        //使用过期的节点
                        return 0;
                    }
                    else
                    {
                        Router *r = getRouterFrotableMap(req, errMsg, routerTable);
                        if (r == NULL)
                        {
                            ostringstream os;
                            os << FILE_FUN << "create map failed:" + tableKey;
                            errMsg = os.str();
                            return -3;      //其它错误
                        }
                        r->updateExpretime(g_node_short_expire_interval);      //探测远程Provider，设置更短的过期时间

                        //使用过期的节点
                        return 0;
                    }
                }
            }
        } // else if (ret == -2)，除此外的情况都算在内存中找不到节点信息
    }

    //在原本的内存中找不到节点信息
    if (_remoteProvider->isAvailable())
    {
        Tseer::AgentRouterResponse rsp;
        int ret = _remoteProvider->getRouteInfo(req, rsp, errMsg);
        if (ret == 0)
        {
            _remoteProvider->setAvailable();

            //更新内存信息
            Router *r = getRouterFrotableMap(req, errMsg, routerTable);
            if (r == NULL)
            {
                ostringstream os;
                os << FILE_FUN << "create map failed:" + tableKey;
                errMsg = os.str();
                return -3;      //其它错误
            }
            if (r->updateRouterNodes(rsp.resultList, errMsg))
            {
                ostringstream os;
                os << FILE_FUN << "update router info in memory failed";
                mergeErrMsg(os.str(), errMsg);
                return -3;
            }
            r->updateExpretime(g_node_normal_expire_interval);     //远程Provider可用，设置正常的过期时间

            //获取节点
            int ret = getRouterFromRAM(r, errMsg, req, nodeInfo, isHash);
            if(ret == 0)
            {
                return 0;
            }

            return -1;      //节点不存在（用户输入服务名有误，或服务已经下线），返回-1
        }
        else  //远程获取路由信息失败，从本地查找缓存，并根据远程Provider是否可用，选择过期时间
        {
            Tseer::AgentRouterResponse rsp;
            std::string subErr;
            int ret = _cacheProvider->getRouteInfo(req, rsp, subErr);
            if (ret == 0)  
            {               
                //更新内存信息
                Router *r = getRouterFrotableMap(req, subErr, routerTable);
                if (r == NULL)
                {
                    ostringstream os;
                    os << FILE_FUN << "create map failed:" + tableKey;
                    subErr = os.str();
                    return -3;      //其它错误
                }
                if (r->updateRouterNodes(rsp.resultList, subErr))
                {
                    ostringstream os;
                    os << FILE_FUN << "update router info in memory failed";
                    mergeErrMsg(os.str(), subErr);
                    return -3;
                }
                if (!_remoteProvider->addFailedNumAndCheckAvailable())
                    r->updateExpretime(g_node_normal_expire_interval);     //远程Provider可用，设置正常的过期时间
                else
                    r->updateExpretime(g_node_short_expire_interval);      //探测远程Provider，设置更短的过期时间

                //获取节点
                int ret = getRouterFromRAM(r, errMsg, req, nodeInfo, isHash);
                if(ret == 0)
                {
                    return 0;
                }
                else
                {
                    ostringstream os;
                    os << FILE_FUN << "get route from cache failed";
                    subErr = mergeErrMsg(os.str(), subErr);
                    errMsg = mergeErrMsg(errMsg, subErr);
                    return -2;  //远程路由信息提供者错误，本地缓存也找不到节点
                }
            }
            else
            {
                ostringstream os;
                os << FILE_FUN << "get route from cache failed, IO error";
                subErr = mergeErrMsg(os.str(), subErr);
                errMsg = mergeErrMsg(errMsg, subErr);
                return -2;  //远程路由信息提供者错误，本地缓存也找不到节点
            }
        }
    }
    else    //远程provider不可用，直接从本地查找
    {
        Tseer::AgentRouterResponse rsp;
        int ret = _cacheProvider->getRouteInfo(req, rsp, errMsg);
        if (ret == 0)
        {
            //更新内存信息
            Router *r = getRouterFrotableMap(req, errMsg, routerTable);
            if (r == NULL)
            {
                ostringstream os;
                os << FILE_FUN << "create map failed:" + tableKey;
                errMsg = os.str();
                return -3;      //其它错误
            }
            if (r->updateRouterNodes(rsp.resultList, errMsg))
            {
                ostringstream os;
                os << FILE_FUN << "update router info in memory failed";
                mergeErrMsg(os.str(), errMsg);
                return -3;
            }
            r->updateExpretime(g_node_long_expire_interval);           //Agent故障，设置较长的超时时间

            //获取节点
            int ret = getRouterFromRAM(r, errMsg, req, nodeInfo, isHash);
            if(ret == 0)
            {
                return 0;
            }
            else
            {
                ostringstream os;
                os << FILE_FUN << "get route from cache failed";
                errMsg = mergeErrMsg(os.str(), errMsg);
                return -2;  //远程路由信息提供者错误，本地缓存也找不到节点
            }
        }
        else
        {
            ostringstream os;
            os << FILE_FUN << "get route from cache failed, IO error";
            errMsg = mergeErrMsg(os.str(), errMsg);
            return -2;  //远程路由信息提供者错误，本地缓存也找不到节点
        }
    }
}

int RouterManager::updateStat(const InnerRouterRequest &req, int ret, int timeCost, string &errMsg, RouteTable &routerTable)
{
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);
    if (iter != routerTable.end())
    {
        return iter->second->statReqResult(req, ret, timeCost);
    }
    else
    {
        ostringstream os;
        os << FILE_FUN << "cannot found router for: " + tableKey;
        errMsg = os.str();
        return -1;
    }
}


}
