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

#ifndef _TSEER_SERVER_H
#define _TSEER_SERVER_H

#include "servant/Application.h"
#include "RegistryImp.h"
#include "QueryImp.h"
#include "ReapThread.h"

#define TSEER_APPNAME string("Tseer")
#define TSEER_SERVERNAME string("TseerServer")
#define TSEERSERVER_NAME string(TSEER_APPNAME + "." + TSEER_SERVERNAME)
#define TSEERSERVER_QUERYOBJ string(TSEERSERVER_NAME + ".QueryObj")
#define TSEERSERVER_REGISTRYOBJ string(TSEERSERVER_NAME + ".RegistryObj")
#define TSEERSERVER_APIOBJ string(TSEERSERVER_NAME + ".ApiRegObj")

class TSeerServer : public tars::Application
{
protected:
    /**
     * 初始化, 只会进程调用一次
     */
    virtual void initialize();


    /**
     * 析构, 每个进程都会调用一次
     */
    virtual void destroyApp();


public:


    /**
     * 获取registry对象的端口信息
     */
    map<string, string> getServantEndpoint() const
    {
        return(_mapServantEndpoint);
    }


    /**
     * 加载registry对象的端口信息
     */
    int loadServantEndpoint();

    /**
     * 获取指定adapter的监听端口信息
     */
    TC_Endpoint getAdapterEndpoint(const string& name) const;

    /**
     * 获取agent安装脚本
     */
    const std::string& getSeerInstallScript() const;
private:

    /**
     *
     * 注册路由信息到路由数据表中
     *
     */
    int registerSelf();
    /**
     * 从磁盘加载安装脚本到内存态
     */
    void loadInstallScript();
protected:
    /* 
     * 用于执行定时操作的线程对象
     */
    ReapThread        _reapThread;
    
    /**
     * 服务本身的监听端口列表
     */
    map<string, string> _mapServantEndpoint;
public:
    /**
     * 产品安装跟路径
     */
    string g_installPath;
    
    /**
     * 服务配置文件全路径名称
     */
    string g_configFile;

private:
    /**
     * agent安装脚本二进制包
     */
    string _installScript;
};

extern TSeerServer g_app;
#endif

