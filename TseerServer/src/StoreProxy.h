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

#ifndef __STORE_PROXY_H__
#define __STORE_PROXY_H__

#include <string>
#include <set>
#include <list>
#include "util/tc_singleton.h"
#include "util/tc_config.h"

#include "RequestEtcdCallback.h"
#include "EtcdDataCache.h"
#include "Registry.h"
#include "TseerAgentUpdate.h"
#include "BaseHandle.h"

class StoreProxy : public TC_Singleton<StoreProxy>
{
public:
    StoreProxy();
    ~StoreProxy();

    /**
      * 初始化相应配置
      */
    int init(TC_Config * pconf);

    /**
      * 向数据库添加路由记录
      */
    int addServers(const vector<RouterData> & routerDataList,tars::TarsCurrentPtr current);

    /**
      * 更新数据库里的路由数据
      */
    int updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

     /**
      * 删除数据库里的路由数据
      */
    int deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current);

    /**
      * 获取数据库里的路由数据
      */
    int getServers(const RouterDataRequest & reqInfo,tars::TarsCurrentPtr current);

    /**
      * 更新路由数据的心跳状态
      */
    int updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr& current);

    /**
      * 批量更新路由数据的心跳状态
      */
    int updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

    /**
      * 路由数据保持心跳
      */
    int keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

    /**
      * 更新机器下面的所有路由数据的心跳状态
      */
    int updateNodeServerStates(const string& ip,const string& presentState);

    /**
      * 获取所有路由数据，提供给查询缓存
      */
    int getAllServers(vector<map<string, string> > &routerDataMap);

    /***********************************基础服务************************************************/

    /** 
     *  向数据库写入自己的基础信息
     */
    int addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);

    /**
     *  向数据库上报自身心跳
     */
    int updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);

    /*********************************Node类服务************************************************/

    /** 
    *  向数据库添加节点信息
    */
    int addNodeInfo(const NodeServiceInfo &nodeInfo);

    /** 
    *  删除数据库中一条节点信息
    */
    int delNodeInfo(const string &nodeName, const string &networkId);

    /** 
    *  获取所有节点信息
    */
    int getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId);

    /** 
    *  批量更新节点信息
    */
    int updateNodeStateBatch(vector<NodeServiceInfo> &nodeList);

    /**
      * 注册agent信息到路由中
      */
    int registerNodeExt(const NodeInstanceInfo & nodeInfo);
    
    int destroyAgent(const string& name);
    
    /** 
    *  获取一个SeerAgent节点信息
    */
    int getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo);

    /** 
    *  获取全部存活的SeerAgent节点名称
    */
    int getAllAliveSeerAgents(std::set<string> &aliveNodes);

    /** 
    *  获取数据库中所有的超时节点，针对于Mysql
    */
    int getAllTimeOutNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId, const unsigned int interval);

    /**
     * 更新node心跳时间及机器负载
     */
    int keepAliveExt(const NodeInstanceInfo& stNodeInstanceInfo);
    
    //心跳上报
    int keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId);
    
    /**
     * 更新node心跳时间及机器负载
     */
    int keepAlive(const string& name, const LoadInfo& li);

    /*********************************IDC相关操作*************************************************/

    /**
     *  添加IDC分组规则，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current);

    /**
     *  修改IDC分组规则，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current);

    /**
     *  批量删除IDC分组信息，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current);

    /**
     *  获取全部IDC分组信息
     */
    int getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList);

    /**
     *  添加IDC优先级规则，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);

    /**
     *  修改IDC优先级规则，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);

    /**
     *  批量删除IDC优先级规则，如果异步请求成功则负责回包，否则直接返回错误给上层调用
     */
    int delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current);

    /**
     *  获取全部IDC优先级规则
     */
    int getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo);

    /*********************************ServiceGroup相关操作*************************************************/

    /**
     *  增加一个业务集
     */
    int addServiceGroup(const string& srvGrp, const string& key, const string& userListStr);

    /**
     *  更新业务集
     */
    int updateServiceGroup(const string& srvGrp, const string& key,const string& userListStr);

    /**
     *  获取所有的业务集
     */
    int getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList);

    /**
     *  获取一个业务集的key
     */
    int getServiceGroupKey(const string& srvGrp,string& key);

    
   /*******************************SeerAgent管理********************************/
    int get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& agentBaseInfoList);
   
    int update_agent_graystate(const UpdateAgentInfoReq& req);
    
    int get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& agentPackageInfoList);
    
    int update_agent_packageinfo(const UpdatePackageInfoReq& req);
    
    int delete_agent_package(const AgentPackageReq& req);

    int getAgentInfo(const string &packageType, vector <AgentPackageInfo> & agentPackageInfoList);

    int getGrayAgentInfo(vector <GrayAgentInfo> & grayAgentInfoList);

    int add_agent_packageinfo(const AgentPackageInfo & stAgentInfo);

    void convertStruct(const PushPackageInfo & pushPkInfo, AgentPackageInfo& stAgentInfo);
private:

    //存储处理类
    BaseHandle *_baseHandle;

    //容器心跳开关
    int _heartbeat_switch;
};
#define STOREPROXY (StoreProxy::getInstance())
#endif