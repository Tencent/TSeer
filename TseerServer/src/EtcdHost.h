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

#ifndef _ETCDHOST_H
#define _ETCDHOST_H

#include <vector>
#include <string>
#include <tr1/unordered_map>
#include "util/tc_thread.h"
#include "util/tc_http.h"
#include "util/tc_thread_rwlock.h"
#include "util/tc_singleton.h"
#include "util/tc_config.h"

using namespace tars;
using namespace std;

class EtcdHost : public TC_Singleton<EtcdHost>, public TC_Thread, public TC_ThreadLock
{
public:
    struct HostInfo
    {
        string    _url;
        string    _ip;
        string    _port;
        string    _stateUrl;
        /*
         * 0: normal
         * 1: timeout state appear
         * 2: stop accept request
         */
        HostInfo()
        {
        }


        HostInfo(const string&url, const string&ip, const string&port, const string&stateUrl)
            : _url(url), _ip(ip), _port(port), _stateUrl(stateUrl)
        {
        }
    };


    /**
     * 定义常量
     */
    enum
    {
        REAP_INTERVAL = 10000, /**轮训间隔时间**/
    };
public:
    EtcdHost();

    ~EtcdHost();


    int init(TC_Config * pconf);

    //获取etcd的链接
    string getHost();

    //获取ectd路由表的链接
    string getRouterHost();

    //获取etcd基础服务信息的链接
    string getBaseServiceHost();

    //获取etcd Node信息的链接接
    string getNodeInfoHost();

    //获取etcd idc_rule的链接
    string getIdcRuleHost();

    //获取etcd idc_priority的链接
    string getIdcPriorityHost();

        //获取etcd 业务集信息的链接
        string getServiceGroupHost();

        //获取Agent发布包信息的链接--t_server_patchs
        string getAgentPackageInfoHost();

        //获取GrayAgent信息的链接--t_server_publish
        string getGrayAgentInfoHost();
private:
    virtual void run();


    void mywait();

    int loadHostInfo(const vector<string>&vec);

    /**
      * 巡检etcd集群健康情况
      */
    void doCheck();
    bool doReadCheck(const string& url,const string& key,const string& keyvalue,bool checkResult=true);
    bool doWriteCheck(const string& url,const string& key,const string& keyvalue);

private:
    bool        _terminate;
    vector<string>    _etcdHostVec;

    typedef std::tr1::unordered_map<string, HostInfo> HostMapT;
    HostMapT    _activeHostMap;
    HostMapT    _inactiveHostMap;

    uint32_t _count;

    static tars::TC_ThreadRWLocker _mapRWLocker;
public:
    //etcd几张表的名字
    static string KEY_ROUTER_TABLE;        //路由表
    static string KEY_BASE_SERVICE;        //基础服务信息表
    static string KEY_NODE_INFO;        //Node服务信息表
    static string KEY_IDC_RULE;            //IDC 分组规则
    static string KEY_IDC_PRIORITY;        //IDC 优先级规则
       static string KEY_SERVICE_GROUP;    //业务集
    static string KEY_AGENT_PACKAGE_INFO;//agent发布包相关信息--t_server_patchs
    static string KEY_AGENT_GRAY_INFO;   //agent灰度设置相关信息--t_server_publish
};

#define ETCDHOST (EtcdHost::getInstance())
#endif
