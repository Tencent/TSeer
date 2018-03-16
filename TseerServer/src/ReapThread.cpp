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

#include "ReapThread.h"
#include "FileWriterFactory.h"
#include "StoreProxy.h"
#include "TSeerServer.h"

ReapThread::ReapThread() :
_terminate(false),
_loadAllObjectsInterval(5),
_registryHeartBeatInterval(60),
_becoverProtect(true),
_recoverProtectRate(30),
_regHeartBeatOff(false)
{
}


ReapThread::~ReapThread()
{
    if(isAlive())
    {
        terminate();
        notifyAll();
        getThreadControl().join();
    }
}


int ReapThread::init()
{
    TLOGDEBUG(FILE_FUN << "begin ReapThread init" << endl);

    extern TC_Config * g_pconf;
    _storeCache.init(g_pconf);
    
    /*主控自身心跳更新间隔*/
    _registryHeartBeatInterval = TC_Common::strto<int>((*g_pconf).get("/tars/reap<registryheartbeatinterval>", "60"));

    /* 加载对象列表的时间间隔 */
    _loadAllObjectsInterval = TC_Common::strto<int>((*g_pconf).get("/tars/reap<loadAllObjectsInterval>", "10"));

    /* 是否启用DB恢复保护功能 */
    _becoverProtect = (*g_pconf).get("/tars/reap<recoverProtect>", "Y") == "N" ? false : true;

    /* 启用DB恢复保护功能状态下极限值 */
    _recoverProtectRate = TC_Common::strto<int>((*g_pconf).get("/tars/reap<recoverProtectRate>", "30"));

    /* 是否关闭更新主控心跳时间,一般需要迁移主控服务是，设置此项为Y */
    _regHeartBeatOff = (*g_pconf).get("/tars/reap<heartbeatoff>", "N") == "Y" ? true : false;

    /* 最小值保护 */
    _loadAllObjectsInterval = _loadAllObjectsInterval < 5 ? 5 : _loadAllObjectsInterval;

    _registryHeartBeatInterval = _registryHeartBeatInterval < 60 ? 60 : _registryHeartBeatInterval;

    _recoverProtectRate = _recoverProtectRate < 1 ? 30 : _recoverProtectRate;

    /*先触发一次心跳上报*/
    updateRegistryInfo();
       
    /* 加载对象列表 */
    _storeCache.loadObjectIdCache(_becoverProtect, _recoverProtectRate);

    TLOGDEBUG(FILE_FUN << "ReapThread init ok." << endl);

    return(0);
}


void ReapThread::terminate()
{
    TLOGDEBUG(FILE_FUN << "ReapThread terminate" << endl);
    _terminate = true;
}

int ReapThread::updateRegistryInfo()
{
    if(_regHeartBeatOff)
    {
        TLOGDEBUG(FILE_FUN<<"updateRegistryInfo not need to update reigstry status !" << endl);
        return(0);
    }

    extern TSeerServer       g_app;
    map<string, string>::iterator   iter;
    map<string, string>     mapServantEndpoint = g_app.getServantEndpoint();
    if(mapServantEndpoint.size() == 0)
    {
        TLOGERROR(FILE_FUN<<"fatal error, get registry servant failed!" << endl);
        return(-1);
    }
    
    TC_Endpoint locator;
    locator.parse(mapServantEndpoint[TSEERSERVER_QUERYOBJ]);
    
    //所有obj都用该ip:port作为locator id
    string locatorId = locator.getHost() + ":" + TC_Common::tostr<int>(locator.getPort());

    //将自身信息加入到ETCD中
    int ret = STOREPROXY->updateBaseServiceInfo(locatorId, mapServantEndpoint);
    if (ret)
    {
        TLOGERROR(FILE_FUN << "add self endpoint to etcd failed, ret: " << ret << endl);
    }
    return ret;
}

void ReapThread::run()
{
    /* 全量加载时间 */
    time_t lastLoadAllObjectsStep = TNOW;
    time_t lastHeartbeat = 0;
    while(!_terminate)
    {
        try
        {
            time_t now = TNOW;
            if(now - lastHeartbeat>= _registryHeartBeatInterval)
            {
                lastHeartbeat = now;
                updateRegistryInfo();
            }
            
            //加载对象列表
            if(now - lastLoadAllObjectsStep >= _loadAllObjectsInterval)
            {
                lastLoadAllObjectsStep = now;
                _storeCache.loadObjectIdCache(_becoverProtect, _recoverProtectRate);
            }

            {
                FileWriterFactory::getInstance()->timeout();
            }

            //轮询心跳超时的主控
            TC_ThreadLock::Lock lock(*this);
            timedWait(100); /* ms */
        }
        catch(exception& ex)
        {
            TLOGERROR(FILE_FUN << "ReapThread exception:" << ex.what() << endl);
        }
        catch(...)
        {
            TLOGERROR(FILE_FUN << "ReapThread unknown exception:" << endl);
        }
    }
}

