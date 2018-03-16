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

#include "RouterManager.h"

#include "RollLogger.h"
using Tseer::RouterNodeInfo;

namespace
{
    unsigned int FIVE_HOURS = 5 * 60 * 60;
}

int RouterManager::getAllRouters(const std::string &obj, std::vector<Tseer::RouterNodeInfo> &routerNodeList)
{
    int iRet = -1;
    {
        TC_ThreadLock::Lock lock(_allRouterLock);

        RouterDataIter it = _allRouters.find(obj);
        if (it != _allRouters.end())
        {
            routerNodeList = it->second._routerNodeInfo;
            
            //更新最近访问时间
            it->second._lastVisit = TNOW;
            iRet = 0;
        }
    }

    return iRet;
}

int RouterManager::getUnexpiredAllObjs(std::vector<std::string> &allObj)
{
    time_t nowTime = TNOW;

    allObj.clear();
    {
        TC_ThreadLock::Lock lock(_allRouterLock);

        RouterDataIter it = _allRouters.begin();
        while (it != _allRouters.end())
        {
            //超过5小时没访问则清理
            if (nowTime - it->second._lastVisit > ::FIVE_HOURS)
            {
                TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "erase useless server router infom obj:" 
                    << it->first << std::endl;
                _allRouters.erase(it++);
            }
            else
            {
                allObj.push_back(it->first);
                ++it;
            }
        }
    }
    
    return 0;
}

int RouterManager::replaceAllRouters(const std::string &obj, const std::vector<Tseer::RouterNodeInfo> &routerNodeList)
{
    {
        TC_ThreadLock::Lock lock(_allRouterLock);
        _allRouters[obj]._routerNodeInfo = routerNodeList;
    }

    return 0;
}

int RouterManager::getIdcRouters(const string &obj, vector<RouterNodeInfo> &routerNodeList)
{
    int iRet = -1;
    {
        TC_ThreadLock::Lock lock(_idcRouterLock);

        RouterDataIter it = _idcRouters.find(obj);
        if(it != _idcRouters.end())
        {
            routerNodeList = it->second._routerNodeInfo;

            //更新最近访问时间
            it->second._lastVisit = TNOW;            
            iRet = 0;
        }
    }

    return iRet;
}

int RouterManager::getUnexpiredIdcObjs(std::vector<string> &idcObj)
{
    time_t nowTime = TNOW;

    idcObj.clear();
    {
        TC_ThreadLock::Lock lock(_idcRouterLock);

        RouterDataIter it = _idcRouters.begin();
        while(it != _idcRouters.end())
        {
            //超过5小时没访问则清理
            if (nowTime - it->second._lastVisit > ::FIVE_HOURS)
            {
                TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "erase useless server router infom obj:" 
                    << it->first << std::endl;
                _idcRouters.erase(it++);
            }
            else
            {
                idcObj.push_back(it->first);
                ++it;
            }
        }
    }

    return 0;
}

int RouterManager::replaceIdcRouters(const string &obj, const vector<RouterNodeInfo> &routerNodeList)
{
    {
        TC_ThreadLock::Lock lock(_idcRouterLock);
        _idcRouters[obj]._routerNodeInfo = routerNodeList;
    }

    return 0;
}

int RouterManager::getSetRouters(const SetServerId &setServerId, std::vector<Tseer::RouterNodeInfo> &routerNodeList)
{
    int iRet = -1;
    {
        TC_ThreadLock::Lock lock(_setRouterLock);

        std::map<SetServerId, RouterData>::iterator it = _setRouters.find(setServerId);
        if (it != _setRouters.end())
        {
            routerNodeList = it->second._routerNodeInfo;
            it->second._lastVisit = TNOW;
            iRet = 0;
        }
    }

    return iRet;
}

int RouterManager::getUnexpiredSetObjs(std::vector<SetServerId> &setObj)
{
    time_t nowTime = TNOW;

    setObj.clear();
    {
        TC_ThreadLock::Lock lock(_setRouterLock);

        std::map<SetServerId, RouterData>::iterator it = _setRouters.begin();
        while (it != _setRouters.end())
        {
            //超过5小时没访问则清理
            if (nowTime - it->second._lastVisit > ::FIVE_HOURS)
            {
                TSEER_LOG(SYNC_LOG)->debug() << FILE_FUN << "|erase useless server router infom obj:" 
                    << it->first._serverName << std::endl;
                _setRouters.erase(it++);
            }
            else
            {
                setObj.push_back(it->first);
                ++it;
            }
        }
    }

    return 0;
}

int RouterManager::replaceSetRouters(const SetServerId &setServerId, const std::vector<Tseer::RouterNodeInfo> &routerNodeList)
{
    {
        TC_ThreadLock::Lock lock(_setRouterLock);
        _setRouters[setServerId]._routerNodeInfo = routerNodeList;
    }

    return 0;
}
