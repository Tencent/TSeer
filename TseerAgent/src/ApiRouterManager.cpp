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

#include "ApiRouterManager.h"
#include "RollLogger.h"
#include "RouterManager.h"
#include "TseerAgentServer.h"
#include "CacheManager.h"

std::string getKeyFromReq(const Tseer::AgentRouterRequest &req)
{
    switch (req.getType)
    {
    case Tseer::GET_ALL:
        return req.obj + "|ALL";
    case Tseer::GET_IDC:
        return req.obj + "|IDC";
    case Tseer::GET_SET:
        return req.obj + "|SET|" + req.setname;
    default:
        return req.obj;
    }
}

std::string mergeErrMsg(const std::string &currErr, const std::string &subErr)
{
    return currErr + "\n\t" + subErr;
}

unsigned int g_node_normal_expire_interval = 5;
unsigned int g_node_short_expire_interval = 3;
unsigned int g_apimgr_expire_interval = 60;


ApiRouterManager::ApiRouterManager(const string& identifier)
{
    _identifier = identifier;
    _lastVist = TNOW;
}

const string& ApiRouterManager::getIdentifier() const
{
    return _identifier;
}

bool ApiRouterManager::timeout() const
{
    return (TNOW-_lastVist) > g_apimgr_expire_interval;
}

ApiRouterManager::~ApiRouterManager()
{
    RouteTableIter begin, end;
    for (begin = _loopRouterTable.begin(), end = _loopRouterTable.end(); begin != end; begin++)
    {
        delete (begin->second);
    }

    for (begin = _randomRouterTable.begin(), end = _randomRouterTable.end(); begin != end; begin++)
    {
        delete (begin->second);
    }

    for (begin = _chashRouterTable.begin(), end = _chashRouterTable.end(); begin != end; begin++)
    {
        delete (begin->second);
    }

    //静态权重的负载表清理
    for (begin = _staticWeightRouterTable.begin(), end = _staticWeightRouterTable.end(); begin != end; ++begin)
    {
        delete (begin->second);
    }

    for (begin = _allRouterTable.begin(), end = _allRouterTable.end(); begin != end; ++begin)
    {
        delete (begin->second);
    }
}

int ApiRouterManager::getRouter(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg)
{
    _lastVist = TNOW;
    //根据请求的负载类型，使用不同的策略表去找
    switch (req.lbType)
    {
        case Tseer::LB_TYPE_LOOP:
        {
            TC_ThreadLock::Lock lock(_loopRouterLock);
            return getRouter(req,rsp, errMsg, _loopRouterTable);
        }
        case Tseer::LB_TYPE_RANDOM:
        {
            TC_ThreadLock::Lock lock(_randomRouterLock);
            return getRouter(req,rsp, errMsg, _randomRouterTable);
        }
        case Tseer::LB_TYPE_CST_HASH:
        {
            TC_ThreadLock::Lock lock(_chashRouterLock);
            return getRouter(req,rsp, errMsg, _chashRouterTable, true);   //bHash设置为真
        }
        case Tseer::LB_TYPE_STATIC_WEIGHT:
        {
            TC_ThreadLock::Lock lock(_staticWRouterLock);
            return getRouter(req, rsp,errMsg, _staticWeightRouterTable);
        }
        case Tseer::LB_TYPE_ALL:
        {
            TC_ThreadLock::Lock lock(_allRouterLock);
            return getRouter(req,rsp,errMsg, _allRouterTable);
        }
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance lbType";
            errMsg = os.str();
            return -1;
        }
    }
}

int ApiRouterManager::updateStat(const Tseer::AgentRouterRequest &req, const Tseer::RouterNodeInfo &nodeInfoRsp, int iRet, int timeCost, string &errMsg)
{
    switch (req.lbType)
    {
        case Tseer::LB_TYPE_LOOP:
        {
            TC_ThreadLock::Lock lock(_loopRouterLock);
            return updateStat(req, nodeInfoRsp, iRet, timeCost, errMsg, _loopRouterTable);
        }
        case Tseer::LB_TYPE_RANDOM:
        {
            TC_ThreadLock::Lock lock(_randomRouterLock);
            return updateStat(req, nodeInfoRsp, iRet, timeCost, errMsg, _randomRouterTable);
        }
        case Tseer::LB_TYPE_CST_HASH:
        {
            TC_ThreadLock::Lock lock(_chashRouterLock);
            return updateStat(req, nodeInfoRsp, iRet, timeCost, errMsg, _chashRouterTable);
        }
        case Tseer::LB_TYPE_STATIC_WEIGHT:
        {
            TC_ThreadLock::Lock lock(_staticWRouterLock);
            return updateStat(req, nodeInfoRsp, iRet, timeCost, errMsg, _staticWeightRouterTable);
        }
        case Tseer::LB_TYPE_ALL:
        {
            TC_ThreadLock::Lock lock(_allRouterLock);
            return updateStat(req, nodeInfoRsp, iRet, timeCost, errMsg, _allRouterTable);
        }
        default:
        {
            ostringstream os;
            os << FILE_FUN << "unknown router load balance lbType";
            errMsg = os.str();
            return -1;
        }
    }   
}

int ApiRouterManager::getRouter(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp,string &errMsg, RouteTable &routerTable, bool bHash)
{
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);
    rsp.getType = req.getType;
    
    if (iter != routerTable.end())
    {
        int iRet = 0;
        Tseer::RouterNodeInfo nodeInfo;
        vector<Tseer::RouterNodeInfo> nodeInfoList;
        if (bHash)
        {
            iRet = iter->second->getRouter(req.hashKey, nodeInfo, errMsg);
        }
        else
        {
            if(req.lbType != Tseer::LB_TYPE_ALL)
                iRet = iter->second->getRouter(nodeInfo, errMsg);
            else
                iRet = iter->second->getRouters(nodeInfoList, errMsg);
        }
        
        if (iRet == GET_ROUTER_OK) //如果节点未过期
        {
            if(req.lbType != Tseer::LB_TYPE_ALL)
                rsp.resultList.push_back(nodeInfo);
            else
                rsp.resultList = nodeInfoList;

            return 0;
        }
        else if (iRet == GET_ROUTER_TIMEOUT)        //节点过期
        {
            if(req.lbType != Tseer::LB_TYPE_ALL)
                rsp.resultList.push_back(nodeInfo);
            else
                rsp.resultList = nodeInfoList;

            //从主控拉取数据
            Tseer::AgentRouterResponse tmpRsp;
            int iRet = getRouterFromServer(req, tmpRsp, errMsg);
            if (iRet == 0)
            {
                //更新内存信息
                ApiRouter *r = getRouterFromTable(req, errMsg, routerTable);
                if (r->updateRouterNodes(tmpRsp.resultList, errMsg))
                {
                    ostringstream os;
                    os << FILE_FUN << "update router info in memory failed";
                    errMsg = mergeErrMsg(os.str(), errMsg);
                    return -3;
                }
                
                //主控可用，设置正常的过期时间
                r->updateExpireTime(g_node_normal_expire_interval);     

                //获取节点
                nodeInfoList.clear();
                int iRet = getRouterFromRAM(r, errMsg, req,nodeInfoList, bHash);
                if(iRet == 0)
                {
                    rsp.resultList = nodeInfoList;
                    return 0;
                }
                
                //节点不存在（用户输入服务名错误，或服务已经下线），返回-1
                return -1;      
            }
            else  //远程Provider通信异常
            {
                ApiRouter *r = getRouterFromTable(req, errMsg, routerTable);
                if (r == NULL)
                {
                    ostringstream os;
                    os << FILE_FUN << "create map failed:" + tableKey;
                    errMsg = os.str();
                    return -3;      //其它错误
                }
                r->updateExpireTime(g_node_short_expire_interval);      //探测远程Provider，设置更短的过期时间
                
                //使用过期的节点
                return 0;
            }
        } // else if (iRet == -2)，除此外的情况都算在内存中找不到节点信息
    }
    else
    {
        //是新请求
        //从主控拉取数据
        Tseer::AgentRouterResponse tmpRsp;
        int iRet = getRouterFromServer(req, tmpRsp, errMsg);
        if (iRet == 0)
        {
            //更新内存信息
            ApiRouter *r = getRouterFromTable(req, errMsg, routerTable);
            if (r->updateRouterNodes(tmpRsp.resultList, errMsg))
            {
                ostringstream os;
                os << FILE_FUN << "update router info in memory failed";
                errMsg = mergeErrMsg(os.str(), errMsg);
                return -3;
            }
            
            //主控可用，设置正常的过期时间
            r->updateExpireTime(g_node_normal_expire_interval);     
        
            //获取节点
            vector<Tseer::RouterNodeInfo> nodeInfoList;
            int iRet = getRouterFromRAM(r, errMsg, req,nodeInfoList, bHash);
            if(iRet == 0)
            {
                rsp.resultList = nodeInfoList;
                return 0;
            }
            
            //节点不存在（用户输入服务名错误，或服务已经下线），返回-1
            return -1;      
        }
        else  //远程获取路由信息失败，从本地查找缓存，并根据远程Provider是否可用，选择过期时间
        {
            Tseer::AgentRouterResponse tmpRsp;
            int iRet = g_cacheManager->getRouterFromDiskCache(req, tmpRsp, errMsg);
            if (iRet == 0)  
            {               
                //更新内存信息
                ApiRouter *r = getRouterFromTable(req, errMsg, routerTable);
                if (r == NULL)
                {
                    ostringstream os;
                    os << FILE_FUN << "create map failed:" + tableKey;
                    errMsg = mergeErrMsg(os.str(), errMsg);
                    return -3;      //其它错误
                }
                
                if (r->updateRouterNodes(tmpRsp.resultList, errMsg))
                {
                    ostringstream os;
                    os << FILE_FUN << "update router info in memory failed";
                    errMsg = mergeErrMsg(os.str(), errMsg);
                    return -3;
                }

                r->updateExpireTime(g_node_short_expire_interval);      //探测远程Provider，设置更短的过期时间

                //获取节点
                vector<Tseer::RouterNodeInfo> nodeInfoList;
                int iRet = getRouterFromRAM(r, errMsg, req,nodeInfoList, bHash);
                if(iRet == 0)
                {
                    rsp.resultList = nodeInfoList;
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
    return -1;
}

ApiRouter* ApiRouterManager::getRouterFromTable(const Tseer::AgentRouterRequest &req, string &errMsg, RouteTable &routerTable)
{
    //根据服务信息转化出路由表的KEY
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);

    if (iter == routerTable.end())
    {
        ApiRouter *r = new(std::nothrow) ApiRouter(req.lbType);
        if (!r)
        {
            ostringstream msgBuffer;
            msgBuffer << FILE_FUN << "create router error|errno:" << errno << "|info:" << strerror(errno);
            errMsg = msgBuffer.str();
        }

        RouteTableIterRet ret = routerTable.insert(RouteTableIterElem(tableKey, r));
        if (!ret.second)    //插入失败
        {
            delete r;
            r = NULL;

            ostringstream msgBuffer;
            msgBuffer << FILE_FUN << "insert router error|errno:" << errno << "|info:" << strerror(errno);
            errMsg = msgBuffer.str();
        }
        return r;
    }
    else
    {
        return iter->second;
    }    
}

int ApiRouterManager::getRouterFromRAM(ApiRouter *r, string &errMsg, const Tseer::AgentRouterRequest &req, vector<Tseer::RouterNodeInfo>& nodeInfoList,bool bHash)
{
    int iRet = 0;
    vector<RouterNodeInfo> tmpNodeInfoList;
    Tseer::RouterNodeInfo nodeInfo;
    if (bHash)
    {
        iRet = r->getRouter(req.hashKey, nodeInfo, errMsg);
    }
    else if(req.lbType != Tseer::LB_TYPE_ALL)
    {
        iRet = r->getRouter(nodeInfo, errMsg);
    }
    else
    {
        iRet = r->getRouters(tmpNodeInfoList, errMsg);
    }
    
    if (iRet == 0) //如果节点未过期
    {
        if(req.lbType != Tseer::LB_TYPE_ALL)
        {
            nodeInfoList.push_back(nodeInfo);
        }
        else
        {
            nodeInfoList = tmpNodeInfoList;
        }
    }
    return iRet;    
}

int ApiRouterManager::updateStat(const Tseer::AgentRouterRequest &req, const Tseer::RouterNodeInfo &nodeInfoRsp, 
                                                   int iRet, int timeCost, string &errMsg, RouteTable &routerTable)
{
    string tableKey = getKeyFromReq(req);
    RouteTableIter iter = routerTable.find(tableKey);
    if (iter != routerTable.end())
    {
        return iter->second->statReqResult(nodeInfoRsp, iRet, timeCost);
    }
    else
    {
        ostringstream os;
        os << FILE_FUN << "cannot found router for: " + tableKey;
        errMsg = os.str();
        return -1;
    }    
}

int ApiRouterManager::getRouterFromServer(const Tseer::AgentRouterRequest & req,Tseer::AgentRouterResponse &rsp,string &errMsg)
{
    int iRet = 0;
    do
    {
        if(req.getType == Tseer::GET_IDC)
        {
            iRet = getIdcRouterNodes(req, rsp);
        }
        else if(req.getType == Tseer::GET_SET)
        {
            if (req.setname.empty())
            {
                ostringstream os;
                os<< FILE_FUN << "empty set info";
                errMsg = os.str();
                iRet = -1;
                break;
            }
            iRet = getSetRouterNodes(req, rsp);
        }
        else if (req.getType == Tseer::GET_ALL)
        {
            iRet = getAllRouterNodes(req, rsp);
        }
        else
        {
            ostringstream os;
            os<< FILE_FUN << "UNKNOWN get type";
            errMsg = os.str();
            iRet = -1;  
        }
    }while(0);  
    return iRet;
}

int ApiRouterManager::getIdcRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp)
{
    vector<Tseer::RouterNodeInfo> nodeInfoList;    
    int iRet = RouterManager::getInstance()->getIdcRouters(req.obj, nodeInfoList);
    if(iRet == 0)
    {
        rsp.obj = req.obj;
        rsp.resultList = nodeInfoList;

        TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN<< "get obj: " << req.obj << " from [cache], size: " << nodeInfoList.size() << std::endl;
        return 0;
    }
    else
    {
        //在内存中抄不到服务路由，从server中心获取，并存到内存和磁盘中
        std::string errMsg;
        iRet = g_app.getSyncRouterThread()->IDC_getAndRecordRoutersFromRegistry(req.obj, nodeInfoList, errMsg);
        if(iRet == 0)
        {
            rsp.obj = req.obj;
            rsp.setname.clear();
            rsp.resultList = nodeInfoList;

            TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN << errMsg <<"|ret="<<0<< std::endl;
            return 0;
        }
        else
        {
            TSEER_LOG(COMMON_LOG)->error() << FILE_FUN << errMsg <<"|ret="<<-1<< std::endl;
            return -1;
        }
    }
}

int ApiRouterManager::getSetRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp)
{
    vector<Tseer::RouterNodeInfo> nodeInfoList;    //服务节点

    SetServerId setServerid;
    setServerid._serverName = req.obj;
    setServerid._setInfo = req.setname;

    int iRet = RouterManager::getInstance()->getSetRouters(setServerid, nodeInfoList);
    if (iRet == 0)
    {
            rsp.getType = Tseer::GET_SET;
            rsp.obj = req.obj;
            rsp.setname = req.setname;
            rsp.resultList = nodeInfoList;

            TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN<< "get obj: " << req.obj << " from [cache], size: " << nodeInfoList.size() << std::endl;
            return 0;
    }
    else
    {
        //在内存中抄不到服务路由，从server中心获取，并存到内存和磁盘中
        std::string errMsg;
        iRet = g_app.getSyncRouterThread()->SET_getAndRecordRoutersFromRegistry(setServerid, nodeInfoList, errMsg);
        if (iRet == 0)
        {
            rsp.obj = req.obj;
            rsp.setname = req.setname;
            rsp.resultList = nodeInfoList;

            TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN << errMsg <<"|ret="<<0<< std::endl;
            return 0;
        }
        else
        {
            TSEER_LOG(COMMON_LOG)->error() << FILE_FUN << errMsg <<"|ret="<<-1<< std::endl;
            return -1;
        }
    }
}

int ApiRouterManager::getAllRouterNodes(const Tseer::AgentRouterRequest &req, Tseer::AgentRouterResponse &rsp)
{
    vector<Tseer::RouterNodeInfo> nodeInfoList;    //服务节点
    
    int iRet = RouterManager::getInstance()->getAllRouters(req.obj, nodeInfoList);
    if (iRet == 0)
    {
        rsp.getType = Tseer::GET_ALL;        //实际获取类型总是ALL
        rsp.obj = req.obj;
        rsp.setname = "";
        rsp.resultList = nodeInfoList;

        TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN
            << "get obj: " << req.obj << " from [cache], size: " << nodeInfoList.size() << std::endl;
        return 0;
    }
    else
    {
        //在内存中抄不到服务路由，从TAF主控获取，并存到内存和磁盘中
        std::string errMsg;
        iRet = g_app.getSyncRouterThread()->ALL_getAndRecordRoutersFromRegistry(req.obj, nodeInfoList, errMsg);
        if (iRet == 0)
        {
            rsp.getType = Tseer::GET_ALL;
            rsp.obj = req.obj;
            rsp.setname = "";
            rsp.resultList = nodeInfoList;
            
            TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN << errMsg <<"|ret="<<0<< std::endl;
            return 0;
        }
        else
        {
            TSEER_LOG(COMMON_LOG)->error() << FILE_FUN << errMsg <<"|ret="<<-1<< std::endl;
            return -1;
        }
    }
}


