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

#ifndef __TSEER_AGENT_SERVER_H_
#define __TSEER_AGENT_SERVER_H_

#include <iostream>
#include "servant/Application.h"
#include "SyncRouterThread.h"
#include "TseerAgentUpdate.h"
#include "SelfUpdateThread.h"

//TSeer路由服务的应用名和服务名
#define TSEERSERVER_MODULE string("Tseer.TseerServer")

//TSeerAgent的应用名
#define TSEERAGENT_APPNAME string("Tseer")

//TSeerAgent的服务名
#define TSEERAGENT_SERVERNAME string("TseerAgent")


class TseerAgentServer : public tars::Application
{
public:
    virtual ~TseerAgentServer() {};

    virtual void initialize();

    virtual void destroyApp();

    SyncRouterThread * getSyncRouterThread() const { return _syncRouterThread; }

    uint32_t getMinTimeoutInvoke() const { return _checkTimeoutInfo.minTimeoutInvoke; }

    uint32_t getCheckTimeoutInterval() const { return _checkTimeoutInfo.checkTimeoutInterval; }

    uint32_t getFrequenceFailInvoke() const { return _checkTimeoutInfo.frequenceFailInvoke; }

    uint32_t getMinFrequenceFailTime() const { return _checkTimeoutInfo.minFrequenceFailTime; }

    float getRadio() const { return _checkTimeoutInfo.radio; }

    uint32_t getTryTimeInterval() const { return _checkTimeoutInfo.tryTimeInterval; }
public:
    TC_Endpoint getAdapterEndpoint(const string& name) const;
    
    string getVersion(string &serName, string &serVer, string &osVer) const;
    
    bool restartSelf(const Tseer::AgentConfig &cfg, string& result, TarsCurrentPtr current = NULL);
   /**
    *  更新启动配置
    */
    bool updateStartScript(const Tseer::AgentConfig& cfg, string& result);
    
    /**
     * 更新配置，重启自己
     */
    bool updateBinOver();

protected:
       //从路由中心同步路由的线程
    SyncRouterThread *    _syncRouterThread;
       //agent自更新线程
    SelfUpdateThread *       _pSelfUpdateThread;

    //超时屏蔽和重试策略
    CheckTimeoutInfo  _checkTimeoutInfo;

public:
    //agent的安装根路径,默认是/usr/local/app/
    string g_installPath;
       
    //agent服务的tars配置文件
    string g_configFile;
    /*
     *记录内网ip地址,如果配置了该地址
     *agent才会上报心跳以及进行自动升级管理
     */
    string g_innerIp;
};

extern TseerAgentServer g_app;

#endif

