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

#include "HeartBeatReportThread.h"
#include "TseerAgentServer.h"
#include "util.h"
#include "RollLogger.h"

HeartBeatReportThread *g_ReapHeartBeatReportThread = NULL;

HeartBeatReportThread::HeartBeatReportThread() :_terminate(false),_init(false){}

int HeartBeatReportThread::init(int reportPeriod)
{
    TSEER_LOG(HEART_BEAST_LOG)->debug() << "HeartBeatReportThread init begin..." << std::endl;
    
    _reportPeriod = reportPeriod;
    _lastReportKeyTime = 0;
    try
    {
        g_app.getCommunicator()->stringToProxy(TSEERSERVER_MODULE + ".RegistryObj", _registryPrx);
        
        string adapterName = "UpdateObjAdapter";
        TC_Endpoint endPoint   = g_app.getAdapterEndpoint(adapterName);
        
        Tseer::NodeInstanceInfo initNodeInfo;
        initNodeInfo.nodeObj       = ServerConfig::Application + "." + ServerConfig::ServerName + ".UpdateObj@" + endPoint.toString();
        initNodeInfo.nodeName = initNodeInfo.endpointIp = endPoint.getHost();
        initNodeInfo.endpointPort  = endPoint.getPort();
        
        string serName;
        string serVer;
        string osVer;

        g_app.getVersion(serName, serVer, osVer);

        initNodeInfo.version = serVer;
        initNodeInfo.osversion = osVer;
        initNodeInfo.locator = Application::getCommunicator()->getProperty("locator");
            
        TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << display(initNodeInfo) << "|Agent regist to heart beat db..." << std::endl;
        int ret = _registryPrx->registerNodeExt(initNodeInfo);
        if (ret)
        {
            TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent regist failed, ret:" << ret << std::endl;
            return -1;
        }
        else
        {
            _init = true;
            return 0;
        }
    }
    catch (tars::TarsException &e)
    {
        TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent regist failed, exception:" << e.what() << std::endl;
        return -1;
    }
}

void HeartBeatReportThread::run()
{
    while (!_terminate)
    {
        if(!_init)
        {
            init(60);
        }
        else
        {
            Tseer::NodeInstanceInfo nodeInfo;
            
            string serName;
            string serVer;
            string osVer;
            g_app.getVersion(serName, serVer, osVer);
            nodeInfo.version = serVer;
            nodeInfo.osversion = osVer;
            nodeInfo.nodeName = g_app.g_innerIp;
            nodeInfo.locator = Application::getCommunicator()->getProperty("locator");
            try
            {    
                int ret;
                if (TNOW - _lastReportKeyTime > 60 * 30)        //30分钟上报一次
                {
                    map<string, string> apiKeyContext;
                    set<string> tmpSet;
                    {
                        TC_ThreadLock::Lock lock(_apiKeyLock);
                        tmpSet.swap(_apiKeys);
                    }

                    //每个值之间用;分割，主控收到数据后再切割
                    std::string keys;
                    for (set<string>::iterator it = tmpSet.begin(); it != tmpSet.end(); ++it)
                    {
                        keys += (*it) + ";";
                    }
                    apiKeyContext.insert(std::make_pair("KEY_INFO", keys));
                    ret = _registryPrx->keepAliveExt(nodeInfo, apiKeyContext);
                    TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent report api keys success:" << keys << endl;

                    _lastReportKeyTime = TNOW;
                }
                else
                {
                    ret = _registryPrx->keepAliveExt(nodeInfo);
                }

                if (ret)
                {
                    TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent keep alive failed, ret:" << ret << std::endl;
                }
                else
                {
                    TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent keep alive success" << std::endl;
                }
            }
            catch (tars::TarsException &e)
            {
                TSEER_LOG(HEART_BEAST_LOG)->debug() << FILE_FUN << "Agent keep alive failed, exception:" << e.what() << std::endl;
            }
        }

        //休息1分钟
        {
            TC_ThreadLock::Lock lock(*this);
            timedWait(_reportPeriod * 1000);
        }
    }
}

void HeartBeatReportThread::terminate()
{
    _terminate = true;

    {
        TC_ThreadLock::Lock sync(*this);
        notifyAll();
    }
}

void HeartBeatReportThread::addKeyInfo(const string &key, const string &version)
{
    //组合成key|version的格式
    TC_ThreadLock::Lock lock(_apiKeyLock);
    _apiKeys.insert(key + "|" + version);
}


