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

#ifndef _REQUESTETCDCALLBACK_H
#define _REQUESTETCDCALLBACK_H

#include "util/tc_http_async.h"
#include "servant/Application.h"
#include <string>
#include <vector>
#include "util.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "RouterData.h"
#include "EtcdCommon.h"
#include "BaseHandle.h"

using namespace std;
using namespace rapidjson;

#define ETCDFILE_FUN  FILE_FUN <<EtcdReqStr(_etcdReqInfo)<<"|"


enum  EtcdAction
{
    ETCD_UPDATE,
    //批量更新数据
    ETCD_UPDATE_BATACH,
    //node上报心跳超时，则设置所有容器状态为inactive （非外部请求，主动发起）
    ETCD_UPDATE_NODE_TIMEOUT,
    //增加一条路由数据
    ETCD_API_ADD,
    //增加多条路由数据
    ETCD_API_ADD_PORTLIST,
    //更新多条路由数据
    ETCD_API_UPDATE_BATACH,
    //更新一条路由数据
    ETCD_API_SET,
    //获取路由数据
    ETCD_API_GET,
    //删除路由数据
    ETCD_API_DELETE,

    //更新自身心跳到ETCD（非外部请求，主动发起）
    ETCD_UPDATE_SELF_STATE,
    //增加自身的obj信息到基础服务信息表（非外部请求，主动发起）
    ETCD_ADD_SELF,

    //增加Node信息到etcd
    ETCD_ADD_NODE_INFO,
    //删除Node信息
    ETCD_DEL_NODE_INFO,
    //获取全部Seer Agent信息
    ETCD_GET_ALL_SEER_AGENTS,
    //获取某一Seer Agent的实时信息
    ETCD_GET_ONE_SEER_AGENT,
    //更新某一Node节点的心跳
    ETCD_KEEP_NODE_ALIVE,

    //添加IDC分组规则
    ETCD_ADD_IDC_RULE,
    //修改IDC分组规则
    ETCD_MODIFY_IDC_RULE,
    //批量删除IDC分组规则
    ETCD_DEL_IDC_RULE,
    //获取一个IDC分组规则
    ETCD_GET_ONE_IDC_RULE,
    //获取全部IDC分组规则
    ETCD_GET_ALL_IDC_RULE,

    //添加IDC优先级
    ETCD_ADD_IDC_PRIORITY,
    //修改IDC优先级
    ETCD_MODIFY_IDC_PRIORITY,
    //批量删除IDC优先级
    ETCD_DEL_IDC_PRIORITY,
    //获取全部IDC优先级
    ETCD_GET_ALL_IDC_PRIORITY,

    //添加业务集
    ETCD_ADD_SERVICE_GROUP,
    ETCD_UPDATE_SERVICE_GROUP,
    //获取所有业务集
    ETCD_GET_ALL_SERVICE_GROUP,
    //获取某个业务集的key
    ETCD_GET_SERVICE_GROUP_KEY,

    //获取所有agent发布包信息
    ETCD_GET_ALL_AGENT_PACKAGE_INFO,
    //获取所有灰度发布包信息
    ETCD_GET_ALL_GRAYAGENT_INFO,
    //更新发布包信息
    ETCD_UPDATE_AGENTPACKAGEINFO,
    //新增一条灰度记录
    ETCD_ADD_AGENT_GRAY_STATE,
    //删除一条发布包记录
    ETCD_DEL_AGENTPACKAGEINFO,
    //删除一条灰度信息
    ETCD_DEL_AGENT_GRAY_STATE
};


enum API_ERROR_CODE
{
    API_SUCC = 0,
    API_NO_PRIVILIGE = 1,
    API_NO_MODULETYPE=2,
    API_INTERNAL_ERROR=3,
    API_INVALID_PARAM =4,
    API_NO_MODULE = 5,
    API_VERSION_ERROR=6,
    API_NO_PORTNAME=7,
    API_SERVICEGRP_EXIST =8,
    API_PARTLY_SUCC =9,
    API_ALL_OR_PARTIAL_DATA_NOT_FOUND=10
};

inline string ActionStr(EtcdAction e)
{
    switch(e)
    {
       case ETCD_UPDATE:return "ETCD_UPDATE";
       case ETCD_UPDATE_BATACH:return "ETCD_UPDATE_BATACH";
       case ETCD_UPDATE_NODE_TIMEOUT:return "ETCD_UPDATE_NODE_TIMEOUT";
       case ETCD_API_ADD:return "ETCD_API_ADD";
       case ETCD_API_ADD_PORTLIST:return "ETCD_API_ADD_PORTLIST";
       case ETCD_API_UPDATE_BATACH:return "ETCD_API_UPDATE_BATACH";
       case ETCD_API_SET:return "ETCD_API_SET";
       case ETCD_API_GET:return "ETCD_API_GET";
       case ETCD_API_DELETE:return "ETCD_API_DELETE";

       case ETCD_UPDATE_SELF_STATE:return "ETCD_UPDATE_SELF_STATE";
       case ETCD_ADD_SELF:return "ETCD_ADD_SELF";
       case ETCD_ADD_NODE_INFO:return "ETCD_ADD_NODE_INFO";
       case ETCD_DEL_NODE_INFO:return "ETCD_DEL_NODE_INFO";
       case ETCD_GET_ALL_SEER_AGENTS:return "ETCD_GET_ALL_SEER_AGENTS";
       case ETCD_GET_ONE_SEER_AGENT:return "ETCD_GET_ONE_SEER_AGENT";
       case ETCD_KEEP_NODE_ALIVE:return "ETCD_KEEP_NODE_ALIVE";

       case ETCD_ADD_IDC_RULE:return "ETCD_ADD_IDC_RULE";
       case ETCD_MODIFY_IDC_RULE:return "ETCD_MODIFY_IDC_RULE";
       case ETCD_DEL_IDC_RULE:return "ETCD_DEL_IDC_RULE";
       case ETCD_GET_ONE_IDC_RULE:return "ETCD_GET_ONE_IDC_RULE";
       case ETCD_GET_ALL_IDC_RULE:return "ETCD_GET_ALL_IDC_RULE";

       case ETCD_ADD_IDC_PRIORITY:return "ETCD_ADD_IDC_PRIORITY";
       case ETCD_MODIFY_IDC_PRIORITY:return "ETCD_MODIFY_IDC_PRIORITY";
       case ETCD_DEL_IDC_PRIORITY:return "ETCD_DEL_IDC_PRIORITY";
       case ETCD_GET_ALL_IDC_PRIORITY:return "ETCD_GET_ALL_IDC_PRIORITY";

       case ETCD_ADD_SERVICE_GROUP:return "ETCD_ADD_SERVICE_GROUP";
       case ETCD_UPDATE_SERVICE_GROUP: return "ETCD_UPDATE_SERVICE_GROUP";
       case ETCD_GET_ALL_SERVICE_GROUP:return "ETCD_GET_ALL_SERVICE_GROUP";
       case ETCD_GET_SERVICE_GROUP_KEY:return "ETCD_GET_SERVICE_GROUP_KEY";
       
       case ETCD_GET_ALL_AGENT_PACKAGE_INFO:return "ETCD_GET_ALL_AGENT_PACKAGE_INFO";
       case ETCD_GET_ALL_GRAYAGENT_INFO:return "ETCD_GET_ALL_GRAYAGENT_INFO";
       case ETCD_UPDATE_AGENTPACKAGEINFO:return "ETCD_UPDATE_AGENTPACKAGEINFO";
       case ETCD_ADD_AGENT_GRAY_STATE:return "ETCD_ADD_AGENT_GRAY_STATE";
       case ETCD_DEL_AGENTPACKAGEINFO:return "ETCD_DEL_AGENTPACKAGEINFO";
       case ETCD_DEL_AGENT_GRAY_STATE:return "ETCD_DEL_AGENT_GRAY_STATE";
       default:return "unknown";
    }
}


class NotifyEvent : public TC_HandleBase
{
public:

    //事件总数
    int                 _run_times;

    //已经完成的事件计数
    tars::TC_Atomic             _atomic;
    
    tars::TC_Atomic             _atomicFault;
    tars::TC_ThreadLock         _monitor;

    //是否可以唤醒的标识位，防止假醒
    bool _canWake;

    NotifyEvent() : _run_times(0), _atomic(0), _atomicFault(0), _canWake(false){}
};
typedef tars::TC_AutoPtr<NotifyEvent> NotifyEventPtr;


struct EtcdReqestInfo
{
    EtcdReqestInfo() :current(NULL), notifyPtr(NULL){}

    //需要回调时需要设置，如果是内部发起的请求不需要设置
    tars::TarsCurrentPtr current;

    //操作指令，必填
    EtcdAction etcdAction;

    //开始时间,毫秒
    long startTime;

    //请求IP
    string etcdHost;

    //请求端口
    uint32_t etcdPort;

    //原子等待操作符
    NotifyEventPtr  notifyPtr;

    /*******************************路由数据*************************************/
    std::string moduletype;
    std::string application;
    std::string service_name;
    std::string node_name;
    std::string container_name;

    //add和set路由数据的时候有值
    RouterData  routerDataInfo;

    //deleteData和get路由数据的时候有值
    RouterDataRequest  routerDataReq;
    vector<RouterDataRequest> reqRouteList;

    //批量处理服务状态的时候有值
    vector<RouterData> needUpdateRouterList;

    /*******************************Node数据*************************************/
    vector<NodeServiceInfo> nodeServiceInfo;

    /*********************************IDC分组************************************/
    std::string newGroupId;
    std::string groupId;
    std::string groupName;
    vector<string> allowIps;

    vector<string> delGroupId;          //需要删除的group id列表
    vector<IDCGroupInfo> idcGroupInfoList;  //从etcd获得的IDC分组信息

    /********************************IDC优先级***********************************/
    std::string newPriorityId;
    std::string priorityId;
    int order;
    std::string priorityName;
    std::vector<std::string> groupIdList;

    vector<string> delPriorityIds;      //需要删除的priority id列表
    vector<IDCPriority> idcPriority;    //从etcd获得的IDC优先级信息

    /********************************ServiceGroup***********************************/
    vector<ServiceGroupInfo> serviceInfoArr;

    //更新、获取、删除Agent时的结构
    AgentPackageInfo agentPackageInfo;
    vector<AgentPackageInfo> agentPackageInfoList;
    map<string,GrayAgentInfo> grayAgentInfoList;

};

inline string EtcdReqStr(const EtcdReqestInfo& etcdReqInfo)
{
    string client;
    if (etcdReqInfo.current)
    {
        client = etcdReqInfo.current->getIp();
    }
    else
    {
        client = "NULL";
    }
    return "client=" + client + "|" + \
            ActionStr(etcdReqInfo.etcdAction) + "|" +\
            etcdReqInfo.moduletype + "." + \
            etcdReqInfo.application + "." + \
            etcdReqInfo.service_name + "|" + \
            etcdReqInfo.node_name + "_" + \
            etcdReqInfo.container_name + "|etcdhost=" + \
            etcdReqInfo.etcdHost + ":" + \
            TC_Common::tostr(etcdReqInfo.etcdPort) + "|" + \
            MSTIMEINSTR(etcdReqInfo.startTime);
}

class RequestEtcdCallback : public tars::TC_HttpAsync::RequestCallback
{
public:
    RequestEtcdCallback(const EtcdReqestInfo& etcdReqInfo) :_etcdReqInfo(etcdReqInfo){}
    ~RequestEtcdCallback(){}

    virtual void onResponse(bool close, tars::TC_HttpResponse &HttpResponse);
    virtual void onException(const string &ex);
    virtual void onTimeout();
    virtual void onClose();

    EtcdReqestInfo _etcdReqInfo;

private:

    //异步响应客户端请求
    //某些请求是是服务端自己发出的，不需要回包，但也统一通过该接口处理
    void responseClient(int ret, long timeUse, const vector<RouterData>& routerDataList = vector<RouterData>());

    //上报访问etcd服务的统计结果
    void reportEtcdStat(int ret, tars::StatReport::StatResult statResult, long timeUse);

    //处理获取的路由数据
    void doGetRouteResponse(const rapidjson::Document &jEtcdRootData);

    //处理批量获取的Seer Agent数据
    void doGetSeerAgentResponse(const rapidjson::Document &jEtcdRootData);

    //批量获取IDC分组数据
    void doGetIdcRule(const rapidjson::Document &jEtcdRootData);

    //获取一个IDC分组数据
    void doGetOneIdcRule(const rapidjson::Document &jEtcdRootData);

    //批量获取IDC优先级信息
    void doGetIdcPriority(const rapidjson::Document &jEtcdRootData);

    //获取业务集信息
    void doGetServiceGroup(const rapidjson::Document &jEtcdRootData);

    //获取业务集对应的key
    void doGetServiceGroupKey(const rapidjson::Document &jEtcdRootData);
    
    //获取agent发布包信息
    void doGetAgentPackageInfoResponse(const rapidjson::Document& jEtcdRootData);

    //获取灰度发布包信息
    void doGetGrayAgentInfoResponse(const rapidjson::Document& jEtcdRootData);
};

typedef tars::TC_AutoPtr<RequestEtcdCallback> RequestEtcdCallbackPtr;
#endif
