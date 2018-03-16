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

#ifndef __SYNC_ROUTER_THREAD_H_
#define __SYNC_ROUTER_THREAD_H_

#include <iostream>
#include "util/tc_thread.h"
#include "servant/QueryF.h"

#include "RouterManager.h"
#include "TseerAgentComm.h"


using namespace tars;

class SyncRouterThread : public TC_Thread, public TC_ThreadLock
{
public:

    SyncRouterThread();

    ~SyncRouterThread();

    void terminate();

    int init();

    //定时执行syncRouterFromRegistry函数
    virtual void run();

    //从TARS主控获取IDC下的节点，刷新内存中的路由，并把路由信息写到磁盘上
    int IDC_getAndRecordRoutersFromRegistry(const string &obj, vector<Tseer::RouterNodeInfo> &routerNodeList, std::string &sMsg);

    //从TARS主控获取SET下的节点，刷新内存中的路由，并把信息写到磁盘上
    int SET_getAndRecordRoutersFromRegistry(const SetServerId &setServerId, vector<Tseer::RouterNodeInfo> &routerNodeList, std::string &sMsg);

    //从TARS主控获取ALL下的节点，刷新内存中的路由，并把信息写到磁盘上
    int ALL_getAndRecordRoutersFromRegistry(const std::string &obj, vector<Tseer::RouterNodeInfo> &routerNodeList, std::string &sMsg);

private:

    //刷新所有服务的路由节点信息，并回写到磁盘上
    void syncRouterFromRegistry();

protected:

    bool        _terminate;

    time_t        _iSyncInterval;

    time_t        _iLastSyncTime;

    time_t        _lastCheckTimeout;
    
    tars::QueryFPrx    _pQueryPrx;

    string        _sRoutersCacheDir;

    TC_ThreadLock _lock;

};

#endif
