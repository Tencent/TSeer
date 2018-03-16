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

#include "util/tc_timeprovider.h"

#include "SyncRouterThread.h"
#include "TseerAgentServer.h"
#include "RollLogger.h"
#include "CacheManager.h"
#include "EndPoint.h"
#include "ApiRouterFactory.h"

using namespace Tseer;
using namespace tars;

namespace
{
    void EndpointF2EndPoint(const vector<tars::EndpointF> &vEpf, vector<EndPoint> &vEp)
    {
        for (vector<tars::EndpointF>::const_iterator it = vEpf.begin(); it != vEpf.end(); ++it)
        {
            EndPoint ep;
            ep._host = it->host;
            ep._istcp = it->istcp == 1 ? true : false;
            ep._port = it->port;
            ep._weight = it->weight;
            ep._set = it->setId;
            ep._timeout = it->timeout;
            vEp.push_back(ep);
        }
    }
}

SyncRouterThread::SyncRouterThread() 
: _terminate(false),
_iSyncInterval(60),
_iLastSyncTime(0),
_lastCheckTimeout(TNOW),
_pQueryPrx(NULL)
{
}

SyncRouterThread::~SyncRouterThread()
{
    if (isAlive())
    {
        terminate();
        getThreadControl().join();
    }
}


int SyncRouterThread::init()
{
    TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "SyncRouterThread init begin..." << std::endl;

    TC_Config& tcConfig = g_app.getConfig();

    //获取主控路由的地址
    string sRegistryObj = tcConfig.get("/tars/application/client<locator>", "");
    if (sRegistryObj.empty())
    {
        TSEER_LOG(SYNC_LOG)->error() << FILE_FUN << "SyncRouterThread init failed: "
            << "cannot get locator from conf" << std::endl;
        return -1;
    }
    
    Application::getCommunicator()->stringToProxy(sRegistryObj, _pQueryPrx);

    //从配置中获取同步间隔，默认15S
    _iSyncInterval = TC_Common::strto<size_t>(tcConfig.get("/tars/syncrouter<iSyncInterval>", "10"));

    TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "SyncRouterThread init ok." << std::endl;
    return 0;
}

void SyncRouterThread::terminate()
{
    _terminate = true;
    TC_ThreadLock::Lock lock(*this);
    notifyAll();
}

void SyncRouterThread::run()
{
    time_t tNow = 0;
    while (!_terminate)
    {
        try
        {
            tNow = TNOW;

            if (tNow - _iLastSyncTime >= _iSyncInterval)
            {
                _iLastSyncTime = tNow;
                syncRouterFromRegistry();
            }

                     if(tNow - _lastCheckTimeout >= (60 * 10))
                     {
                           //扫描看看是否有过期不用的apimgr
                            _lastCheckTimeout = tNow;
                            ApiRouterFactory::getInstance()->timeout();
                     }
                     
            {
                TC_ThreadLock::Lock lock(*this);
                timedWait(100); //ms
            }
        }
        catch (exception & ex)
        {
            TSEER_LOG(SYNC_LOG)->error() << FILE_FUN << "SyncRouterThread exception:" << ex.what() << std::endl;
        }
        catch (...)
        {
            TSEER_LOG(SYNC_LOG)->error() << FILE_FUN << "SyncRouterThread unknown exception" << std::endl;
        }
    }
}

void SyncRouterThread::syncRouterFromRegistry()
{
    TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "...sync begin..." << std::endl;
    int64_t iBegin = TNOWMS;

    //更新IDC的节点
    std::vector<string> objs;
    RouterManager::getInstance()->getUnexpiredIdcObjs(objs);
    std::vector<string>::iterator idcIter = objs.begin();

    while (idcIter != objs.end())
    {
        std::vector<RouterNodeInfo> newRouterNodes;
        std::string sMsg;
        IDC_getAndRecordRoutersFromRegistry(*idcIter, newRouterNodes, sMsg);
        TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "Sync IDC routers, obj:" << *idcIter <<
            "|result:" << sMsg << std::endl;
        ++idcIter;
    }

    //更新SET的节点
    std::vector<SetServerId> setObjs;
    RouterManager::getInstance()->getUnexpiredSetObjs(setObjs);
    std::vector<SetServerId>::iterator setIter = setObjs.begin();
    while (setIter != setObjs.end())
    {
        std::vector<RouterNodeInfo> newRouterNodes;
        std::string sMsg;
        SET_getAndRecordRoutersFromRegistry(*setIter, newRouterNodes, sMsg);
        TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "Sync SET routers, obj:" << setIter->toString()
            << "|result:" << sMsg << std::endl;
        ++setIter;
    }

    //更新ALL的节点
    objs.clear();
    RouterManager::getInstance()->getUnexpiredAllObjs(objs);
    std::vector<string>::iterator allIter = objs.begin();
    while (allIter != objs.end())
    {
        std::vector<RouterNodeInfo> newRouterNodes;
        std::string sMsg;
        ALL_getAndRecordRoutersFromRegistry(*allIter, newRouterNodes, sMsg);
        TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "Sync ALL routers, obj:" << *allIter
            << "|result:" << sMsg << std::endl;
        ++allIter;
    }

    int64_t iEnd = TNOWMS;
    TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "....sync end.... time cost(ms):" << iEnd - iBegin << std::endl;
}

int SyncRouterThread::IDC_getAndRecordRoutersFromRegistry(const string &obj,
    vector<RouterNodeInfo> &routerNodeList, std::string &sMsg)
{
    int iRet = 0;
    try
    {
        vector<tars::EndpointF> activeEpf;
        vector<tars::EndpointF> inactiveEpf;

        int64_t iBegin = TNOWMS;
        //向主控查询同一个IDC下的节点
        int iRet = _pQueryPrx->findObjectByIdInSameGroup(obj, activeEpf, inactiveEpf);
        int64_t iEnd = TNOWMS;

        if (iRet == 0)
        {
            vector<RouterNodeInfo> vRouterNodesTmp;
            for (size_t i = 0; i < activeEpf.size(); i++)
            {
                RouterNodeInfo nodeInfo;
                nodeInfo.ip = activeEpf[i].host;
                nodeInfo.port = activeEpf[i].port;
                nodeInfo.isTcp = (activeEpf[i].istcp > 0);
                nodeInfo.setname = (activeEpf[i].setId);
                if (activeEpf[i].weight < 0)
                {
                    nodeInfo.weight = 100;
                }
                else
                {
                    nodeInfo.weight = activeEpf[i].weight;
                }

                vRouterNodesTmp.push_back(nodeInfo);
            }

            //刷新内存的路由信息
            RouterManager::getInstance()->replaceIdcRouters(obj, vRouterNodesTmp);

            //磁盘缓存
            vector<EndPoint> activeEp;
            vector<EndPoint> inactiveEp;
            ::EndpointF2EndPoint(activeEpf, activeEp);
            ::EndpointF2EndPoint(inactiveEpf, inactiveEp);
            g_cacheManager->updateIDCCache(obj, activeEp, inactiveEp);

            //写入出参
            routerNodeList.clear();
            routerNodeList = vRouterNodesTmp;

            ostringstream os;
            os << "get router from registry success |obj:" << obj
                << "|nodes size:" << routerNodeList.size()
                << "|timecost(ms):" << (iEnd - iBegin);
            sMsg = os.str();
        }
        else
        {
            ostringstream os;
            os << "get router from registry failed |obj:" << obj
                << "|return:" << iRet << "|timecost(ms):" << (iEnd - iBegin);
            sMsg = os.str();
            iRet = -1;
        }
    }
    catch (exception &ex)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + obj + "|exception:" + ex.what();
    }
    catch (...)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + obj + "|unknown exception";
    }
    return iRet;
}

int SyncRouterThread::SET_getAndRecordRoutersFromRegistry(const SetServerId &setServerId,
    vector<Tseer::RouterNodeInfo> &routerNodeList, std::string &sMsg)
{
    int iRet = 0;
    try
    {
        vector<tars::EndpointF> activeEpf;
        vector<tars::EndpointF> inactiveEpf;

        int64_t iBegin = TNOWMS;
        //向主控查询同一个SET下的节点
        int iRet = _pQueryPrx->findObjectByIdInSameSet(setServerId._serverName, setServerId._setInfo, activeEpf, inactiveEpf);
        int64_t iEnd = TNOWMS;

        if (iRet == 0)
        {
            vector<RouterNodeInfo> vRouterNodesTmp;
            for (size_t i = 0; i < activeEpf.size(); i++)
            {
                RouterNodeInfo nodeInfo;
                nodeInfo.ip = activeEpf[i].host;
                nodeInfo.port = activeEpf[i].port;
                nodeInfo.isTcp = (activeEpf[i].istcp > 0 ? true : false);
                nodeInfo.setname = (activeEpf[i].setId);
                if (activeEpf[i].weight < 0)
                {
                    nodeInfo.weight = 100;
                }
                else
                {
                    nodeInfo.weight = activeEpf[i].weight;
                }

                vRouterNodesTmp.push_back(nodeInfo);
            }

            //刷新内存的路由信息
            RouterManager::getInstance()->replaceSetRouters(setServerId, vRouterNodesTmp);

            //写入出参
            routerNodeList.clear();
            routerNodeList = vRouterNodesTmp;

            //磁盘缓存
            vector<EndPoint> activeEp;
            vector<EndPoint> inactiveEp;
            ::EndpointF2EndPoint(activeEpf, activeEp);
            ::EndpointF2EndPoint(inactiveEpf, inactiveEp);
            g_cacheManager->updateSetCache(setServerId._serverName, setServerId._setInfo, activeEp, inactiveEp);

            ostringstream os;
            os << "get router from registry success |obj:" << setServerId._serverName
                << "|nodes size:" << routerNodeList.size()
                << "|timecost(ms):" << (iEnd - iBegin);
            sMsg = os.str();
        }
        else
        {
            ostringstream os;
            os << "get router from registry failed |obj:" << setServerId._serverName
                << "|return:" << iRet << "|timecost(ms):" << (iEnd - iBegin);
            sMsg = os.str();
            iRet = -1;
        }
    }
    catch (exception &ex)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + setServerId._serverName + "|exception:" + ex.what();
    }
    catch (...)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + setServerId._serverName + "|unknown exception";
    }
    return iRet;
}

int SyncRouterThread::ALL_getAndRecordRoutersFromRegistry(const std::string &obj,
    vector<Tseer::RouterNodeInfo> &routerNodeList, std::string &sMsg)
{
    int iRet = 0;
    try
    {
        vector<tars::EndpointF> activeEpf;
        vector<tars::EndpointF> inactiveEpf;

        int64_t iBegin = TNOWMS;
        //向主控查询同一个IDC下的节点
        int iRet = _pQueryPrx->findObjectById4Any(obj, activeEpf, inactiveEpf);
        int64_t iEnd = TNOWMS;

        if (iRet == 0)
        {
            vector<RouterNodeInfo> vRouterNodesTmp;
            for (size_t i = 0; i < activeEpf.size(); i++)
            {
                RouterNodeInfo nodeInfo;
                nodeInfo.ip = activeEpf[i].host;
                nodeInfo.port = activeEpf[i].port;
                nodeInfo.isTcp = (activeEpf[i].istcp > 0 ? true : false);
                nodeInfo.setname = (activeEpf[i].setId);
                if (activeEpf[i].weight < 0)
                {
                    nodeInfo.weight = 100;
                }
                else
                {
                    nodeInfo.weight = activeEpf[i].weight;
                }

                vRouterNodesTmp.push_back(nodeInfo);
            }

            //刷新内存的路由信息
            RouterManager::getInstance()->replaceAllRouters(obj, vRouterNodesTmp);

            //写入出参
            routerNodeList.clear();
            routerNodeList = vRouterNodesTmp;

            //磁盘缓存
            vector<EndPoint> activeEp;
            vector<EndPoint> inactiveEp;
            ::EndpointF2EndPoint(activeEpf, activeEp);
            ::EndpointF2EndPoint(inactiveEpf, inactiveEp);
            g_cacheManager->updateAllCache(obj, activeEp, inactiveEp);

            ostringstream os;
            os << "get router from registry success |obj:" << obj
                << "|nodes size:" << routerNodeList.size()
                << "|timecost(ms):" << (iEnd - iBegin);
            sMsg = os.str();
        }
        else
        {
            ostringstream os;
            os << "get router from registry failed |obj:" << obj
                << "|return:" << iRet << "|timecost(ms):" << (iEnd - iBegin);
            iRet = -1;
        }
    }
    catch (exception &ex)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + obj + "|exception:" + ex.what();
    }
    catch (...)
    {
        iRet = -1;
        sMsg = "get router from registry failed |obj:" + obj + "|unknown exception";
    }
    return iRet;
}
