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

#ifndef __ETCD_HANDLE_H__
#define __ETCD_HANDLE_H__

#include "util/tc_http_async.h"
#include "util/tc_config.h"
#include <string>
#include <set>
#include <list>

#include "EtcdCommon.h"
#include "StoreCache.h"
#include "RequestEtcdCallback.h"
#include "Registry.h"
#include "BaseHandle.h"

struct ServiceGroupInfo;

class BaseServiceInfo;

class EtcdHandle : public BaseHandle
{
public:
    EtcdHandle();
    ~EtcdHandle();

    int init(TC_Config * pconf);

    /***********************************Server服务************************************************/

    int addServers(const vector<RouterData> & routerDataList,tars::TarsCurrentPtr current);
    int updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    int deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current);
    int getServers(const RouterDataRequest & reqInfo,tars::TarsCurrentPtr current);
    int getAllServers(vector<map<string, string> > &routerDataMap);
    
    int updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr& current);
    int updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    int updateNodeServerStates(const string& ip,const string& presentState);

    int keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

    /***********************************基础服务************************************************/

    int addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);
    int updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);

    /*********************************Node类服务************************************************/

    int addNodeInfo(const NodeServiceInfo &nodeInfo);
    int delNodeInfo(const string &nodeName, const string &networkId);
    int keepAlive(const string& name, const LoadInfo& li);
    
    int keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId);

    int updateNodeStateBatch(vector<NodeServiceInfo> &nodeList);

    int getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId);
    int getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval);
    int getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo);
    int getAllAliveSeerAgents(std::set<string> &aliveNodes);

    /*********************************IDC相关操作*************************************************/

    int addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current);
    int modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current);
    int delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current);
    int getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList);

    int addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    int modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    int delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current);
    int getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo);

    /*********************************ServiceGroup相关操作*************************************************/

    int addServiceGroup(const string& srvGrp, const string& key, const string& userListStr);
    int getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList);
    int updateServiceGroup(const string& srvGrp, const string& key,const string& userListStr);
    int getServiceGroupKey(const string& srvGrp,string& key);

    /*******************************SeerAgent管理********************************/
    int get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& AgentBaseInfos);
    int update_agent_graystate(const UpdateAgentInfoReq& req);
    int get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& AgentPackageInfos);
    int update_agent_packageinfo(const UpdatePackageInfoReq& req);

    int delete_agent_package(const AgentPackageReq& req);

    int add_agent_packageinfo(const AgentPackageInfo & reqInfo);
    
    static int maxRetryTime();
private:
    int addServiceGroupDir(const RouterData & routerDataInfo);
    int getOneIdcGroupRule(const string &groupId, IDCGroupInfo &idcGroupInfoList);

    //检查路由信息是否合法
    bool isRouterDataValid(const RouterData &routerDataInfo);

    /**
      * 获取指定节点下所有容器的路由信息
      * @param ip
      * @param[out] routerDataList
      *
      * @return 详情请参考RetCode
      */
    int getContainerRouterByIp(const string& ip,vector<RouterData>& routerDataList);
    
    /**
      * 组装更新etcd路由信息的请求url
      * @param[out]  sEtdReqUrl 
      * @return 详情请参考RetCode
      */
    int makeUpdateRouteUrl(const RouterData &routerDataInfo,string &sEtdReqUrl,EtcdAction etcdAction);
    /**
      * 组装获取etcd路由信息的请求url
      * @param[out]  sEtdReqUrl 
      * @return 详情请参考RetCode
      */
    int makeGetRouteRequestUrl(const RouterDataRequest &reqInfo,string &sEtdReqUrl);
    /**
      * 组装删除etcd路由信息的请求url
      * @param[out]  sEtdReqUrl 
      * @return 详情请参考RetCode
      */
    int makeDelRouteRequestUrl(const RouterDataRequest &reqInfo,string &sEtdReqUrl);
    
    /**
      * 把routerdata转换成value=obj@endpoint:obj@endpoint||||||N||inactive|inactive|0|0|100|||的格式
      */
    int makeRouteValue(const RouterData &routerDataInfo,string &sValue);

    bool isValidState(const string &state);

    template<typename T>
    void setEtcdReqInfo(const T& t, const EtcdAction& etcdAction, const tars::TarsCurrentPtr& current, EtcdReqestInfo& reqInfo);

    /***********************************基础服务************************************************/

    //组装更新etcd基础服务信息的请求URL
    int makeUpdateBaseServiceUrl(const BaseServiceInfo &baseServiceInfo, string &sEtcdReqUrl, EtcdAction etcdAction);

    //把基础服务信息转换成value=obj@endpoint|active|TARS_VERSION|N 的格式
    int makeBaseServiceValue(const BaseServiceInfo &baseServiceInfo, string &sValue);

    //检查基础服务信息是否合法
    bool isBaseServiceInfoValid(const BaseServiceInfo &baseServiceInfo);

    /*********************************Node类服务************************************************/
    
    //检查NodeServiceInfo信息是否合法
    bool isNodeServiceInfoValid(const NodeServiceInfo &info);

    //组装更新Node类服务信息的请求URL
    int makeUpdateNodeUrl(const string &networkId, const string &nodeName, string &sEtcdReqUrl, EtcdAction etcdAction);

    //把Node信息转换成value=obj@endpoint|node_name|node_networkid|endpoint_ip|endpoint_port|register_time|datadir|load_avg1|load_avg5|load_avg15
    //              |cpuuse_avg1|cpu_num|cpu_hz|memory_size|corefile_size|openfile_size|setting_state|present_state|tars_version|ostype
    //              |docker_node_version|template_name|groupId|locator
    int makeNodeValue(const NodeServiceInfo &nodeInfo, string &sValue);

    /*********************************IDC相关操作*************************************************/
    
    //把IDCGroupInfo转换成 value=group_name|allow_ip|denney_ip|rule_order
    int makeGroupInfoValue(const IDCGroupInfo &groupInfo, string &sValue);

    //把IDCPriority转换成 value=priority_name|order|group_list|station
    int makePriorityInfoValue(const IDCPriority &priorityInfo, string &sValue);

    //判断组名是否已经存在，不存在时生成新IDC分组的编号，获取所有分组信息使用下一个未使用的编号
    int isNameValidGenNewIdcGroupId(const string &groupName, string &newId);

    int updateOnePackageInfo(const AgentPackageInfo& agentPackageInfo);
    int getAllGrayAgentInfo(map<string,GrayAgentInfo>& grayAgentInfoList);
private:
    //异步http通信组件
    TC_HttpAsync _ast;

    //异步http通信组件
    TC_HttpAsync _apiAst;

    //进程心跳超时时间,默认125秒
    int _iTTLTimeout;

    static int _iReqEtcdRetryTime;
};


#endif


