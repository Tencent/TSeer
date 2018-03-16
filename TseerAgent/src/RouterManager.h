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

#ifndef __ROUTER_MANAGER_H_
#define __ROUTER_MANAGER_H_

/***********************************************
* 文件定义了路由信息管理类，该类的功能是缓存并提供路由信息
************************************************/

#include <string>
#include <map>
#include <vector>
#include <time.h>
#include "util/tc_lock.h"
#include "util/tc_singleton.h"
#include "util/tc_timeprovider.h"
#include "TseerAgentComm.h"

using namespace tars;

//每个SET的服务ID
struct SetServerId
{
    std::string _serverName;
    std::string _setInfo;

    bool operator<(const SetServerId &id) const
    {
        if (_serverName < id._serverName)
        {
            return true;
        }
        else if (_serverName == id._serverName)
        {
            return _setInfo < id._setInfo;
        }
        else
        {
            return false;
        }
    }

    bool operator==(const SetServerId &id) const
    {
        return _serverName == id._serverName && _setInfo == id._setInfo;
    }

    std::string toString() const
    {
        return _serverName + "|" + _setInfo;
    }
};

//每个节点的数据，上次的访问时间
struct RouterData
{
    RouterData() :_lastVisit(TNOW){}

    std::vector<Tseer::RouterNodeInfo> _routerNodeInfo;
    time_t _lastVisit;
};
typedef std::map<std::string, RouterData>::iterator RouterDataIter;


class RouterManager : public TC_Singleton<RouterManager>
{
public:
    RouterManager() {}

    /**    获取指定服务的ALL节点
    *    obj:            入参，服务名称
    *    routerNodeList:    出参，服务节点
    */
    int getAllRouters(const std::string &obj, std::vector<Tseer::RouterNodeInfo> &routerNodeList);

    /**    获取没有过期的ALL节点（上次访问时间至今不超过5小时）服务，清理已经过期的节点，成功返回0，失败返回-1
     *    allObj:        出参，服务名称
     */
    int getUnexpiredAllObjs(std::vector<std::string> &allObj);

    /**    替换指定服务的ALL节点 ，如果节点原先不存在则插入
     *    obj:            入参，服务名称
     *    routerNodeList:    入参，服务节点
     */
    int replaceAllRouters(const std::string &obj, const std::vector<Tseer::RouterNodeInfo> &routerNodeList);

    /**    获取指定服务的IDC节点
     *    obj:            入参，服务名称
     *    routerNodeList:    出参，服务节点
     */
    int getIdcRouters(const std::string &obj, std::vector<Tseer::RouterNodeInfo> &routerNodeList);

    /**    获取没有过期的IDC节点（上次访问时间至今不超过5小时）服务，清理已经过期的节点，成功返回0，失败返回-1
     *    idcObj:        出参，服务名称
     */
    int getUnexpiredIdcObjs(std::vector<std::string> &idcObj);

    /**    替换指定服务的IDC节点 ，如果节点原先不存在则插入
     *    obj:            入参，服务名称
     *    routerNodeList:    入参，服务节点
     */
    int replaceIdcRouters(const std::string &obj, const std::vector<Tseer::RouterNodeInfo> &routerNodeList);

    /**    获取指定服务的SET节点
     *    setServerId:    入参，SET服务Id
     *    routerNodeList:    出参，服务节点
     */
    int getSetRouters(const SetServerId &setServerId, std::vector<Tseer::RouterNodeInfo> &routerNodeList);

    /**    获取没有过期的SET节点（上次访问时间至今不超过5小时）服务，清理已经过期的节点，成功返回0，失败返回-1
     *    idcObj:        出参，服务名称
     */
    int getUnexpiredSetObjs(std::vector<SetServerId> &setObj);

    /**    替换指定服务的SET节点 ，如果节点原先不存在则插入
     *    setServerId:    入参，SET服务Id
     *    routerNodeList:    入参，服务节点
     */
    int replaceSetRouters(const SetServerId &setServerId, const std::vector<Tseer::RouterNodeInfo> &routerNodeList);

private:
    
    //<SERVER_NAME, data>
    std::map<std::string, RouterData>_allRouters;
    TC_ThreadLock _allRouterLock;

    //<SERVER_NAME, data>
    std::map<std::string, RouterData>    _idcRouters;
    TC_ThreadLock _idcRouterLock;

    //<SET_SERVER_ID, data>
    std::map<SetServerId, RouterData> _setRouters;
    TC_ThreadLock _setRouterLock;
};


#endif
