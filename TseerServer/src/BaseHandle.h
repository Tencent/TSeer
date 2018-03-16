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

#ifndef __BASE_HANDLE_H__
#define __BASE_HANDLE_H__

#include "util/tc_config.h"
#include <string>
#include <set>
#include <list>

#include "StoreCache.h"
#include "Registry.h"

struct NodeServiceInfo
{
    string nodeName;
    string nodeNetworkId;       //表示node信息类型，有dcnode和router_agent
    string nodeObj;                 //xx@ -h -p -t
    string endpointIp;
    string endpointPort;
    string registerTime;
    string dataDir;
    string loadAvg1;
    string loadAvg5;
    string loadAvg15;
    string cpuuseAvg1;
    string cpuNum;
    string cpuHz;
    string memorySize;
    string coreFileSize;
    string openFileSize;
    string settingState;
    string presentState;
    string version;
    string ostype;
    string dockerNodeVersion;
    string templateName;
    string groupId;
    string locator;
    string last_hearttime;

    //从NodeInstanceInfo生成一个NodeServiceInfo，只用于Node的节点注册
    //会根据调用的当前时间来为registerTime赋值
    static NodeServiceInfo fromNodeInstanceInfo(const NodeInstanceInfo &instanceInfo, bool isNode = false)
    {
        NodeServiceInfo result;
        if (isNode)
        {
            result.nodeNetworkId = DCNODE_NETWORK_ID;
        }
        else
        {
            result.nodeNetworkId = TSEER_AGENT_NET_WORK_ID;
        }

        result.nodeName = instanceInfo.nodeName;
        result.nodeObj = instanceInfo.nodeObj;
        result.endpointIp = instanceInfo.endpointIp;
        result.endpointPort = TC_Common::tostr<int>(instanceInfo.endpointPort);
        result.version = instanceInfo.version;
        result.ostype = instanceInfo.osversion;
        result.locator = instanceInfo.locator;

        const string timeFormat = "%Y-%m-%d %H:%M:%S";
        result.registerTime = TC_Common::tm2str(TNOW, timeFormat);      //注册时间为调用该函数的时间

        return result;
    }
    static NodeServiceInfo fromNodeInfo(const NodeInfo ni, const LoadInfo li)
    {
        NodeServiceInfo result;
        result.nodeNetworkId = DCNODE_NETWORK_ID;
        result.nodeName = ni.nodeName;
        result.nodeObj = ni.nodeObj;
        result.endpointIp = ni.endpointIp;
        result.endpointPort = TC_Common::tostr<int>(ni.endpointPort);
        result.version = ni.version;
        result.dataDir = ni.dataDir;
        result.cpuNum = TC_Common::tostr<int>(ni.cpu_num);
        result.cpuHz = TC_Common::tostr<int>(ni.cpu_hz);
        result.memorySize = TC_Common::tostr<int>(ni.memory_size);
        result.loadAvg1 = li.avg1;
        result.loadAvg5 = li.avg5;
        result.loadAvg15 = li.avg15;
        return result;
    }

    string toStringSeer()
    {
        std::ostringstream os;
        os << "node_name:" << nodeName
            << "|node_networkId:" << nodeNetworkId
            << "|node_obj:" << nodeObj
            << "|endpoint_ip:" << endpointIp
            << "|endpoint_port:" << endpointPort
            << "|register_time:" << registerTime
            << "|ostype:" << ostype
            << "|locator:" << locator
         << "|last_hearttime:"<<last_hearttime;
        return os.str();
    }

};

//IDC Group信息用于etcd操作
struct IDCGroupInfo
{
    string groupId;
    string groupName;       //分组名字
    string ruleOrder;         //只允许是 "allow_denny"或"denny_allow"；tseer做为对外产品时不需要这个字段
    vector<string> allowIp;     //允许IP列表
    vector<string> denneyIp;    //拒绝IP列表；tseer做为对外产品时不需要这个字段

    string toString()
    {
        string result = groupId + "|" + groupName + "|" + ruleOrder + "[";
        for (size_t i = 0; i < allowIp.size(); ++i)
        {
            result += allowIp[i] + "|";
        }
        result += "]|[";
        for (size_t i = 0; i < denneyIp.size(); ++i)
        {
            result += "|" + denneyIp[i];
        }
        result += "]";
        return result;
    }
};

//IDC 优先级信息，和t_group_priority类似，用于etcd操作
struct IDCPriority
{
    string priorityId;
    string priorityName;
    int order;
    vector<string> groupList;   //Group id组
    string station;             //地区；tseer作为对外产品时不需要这个字段

    bool operator <(const IDCPriority &p) const
    {
        return order < p.order;
    }

    string toString()
    {
        string result = priorityId + "|" + priorityName + "|" + TC_Common::tostr<int>(order)
            +"|" + station + "[";
        for (size_t i = 0; i < groupList.size(); ++i)
        {
            result += groupList[i] + "|";
        }
        result += "]";
        return result;
    }
};

struct ServiceGroupInfo
{
    string serviceGrp;
    string key;
    set<string> setUserList;
};

//请求agent信息的参数
struct AgentBaseInfoReq
{
    string ip;
    string ostype;
    string present_state;
    AgentBaseInfoReq()
    {
        ip = "";
        ostype = "";
        present_state = "";
    }
};

struct AgentBaseInfo
{
    string ip;
    string ostype;
    string present_state;
    string version;
    string locator;
    string gray_version;
    string formal_version;
    string grayscale_state;
    string md5;
    string package_name;
    string last_reg_time;
    string last_heartbeat_time;
    string user;
};


struct UpdateAgentInfoReq
{
    string ip;
    string ostype;
    string grayscale_state;
};

const string TSEER_PACKAGE_NONE_TYPE = "0";
const string TSEER_PACKAGE_GRAY_TYPE = "1";
const string TSEER_PACKAGE_FORMAL_TYPE = "2";

struct AgentPackageReq
{
    string ostype;
    string package_name;
    AgentPackageReq()
    {
        ostype = "";
        package_name = "";
    }
};

struct AgentPackageInfo
{
    string ostype;
    string version;
    string package_type;
    string package_name;
    string md5;
    string uploadUser;
    string uploadTime;
};

struct UpdatePackageInfoReq
{
    string package_name;
    string ostype;
    string grayscale_state;
};

struct GrayAgentInfo
{
    string ip;
    string grayscale_version;
    string grayscale_state;
    string package_name;
    string ostype;
    string md5;
};

class BaseHandle
{
public:
    BaseHandle();
    
    virtual ~BaseHandle();

    virtual int init(TC_Config * pconf);

    /***********************************Server服务************************************************/

    virtual int addServers(const vector<RouterData> &routerDataList,tars::TarsCurrentPtr current);
    virtual int updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    virtual int deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current);
    virtual int getServers(const RouterDataRequest & reqInfo, tars::TarsCurrentPtr current);
    virtual int getAllServers(vector<map<string, string> > &routerDataMap);

    virtual int updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr& current);
    virtual int updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    virtual int updateNodeServerStates(const string& ip, const string& presentState);
    virtual int keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

    /***********************************基础服务************************************************/
    virtual int addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);
    virtual int updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);
    

    /*********************************Node类服务************************************************/
    virtual int addNodeInfo(const NodeServiceInfo &nodeInfo);
    virtual int delNodeInfo(const string &nodeName, const string &networkId);
    virtual int keepAlive(const string& name, const LoadInfo& li);
    virtual int keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId);
    
    virtual int updateNodeStateBatch(vector<NodeServiceInfo> &nodeList);
    virtual int getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId);
    virtual int getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval);
    virtual int getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo);
    virtual int getAllAliveSeerAgents(std::set<string> &aliveNodes);

    /*********************************IDC相关操作*************************************************/
    virtual int addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current);
    virtual int modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current);
    virtual int delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current);
    virtual int getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList);

    virtual int addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    virtual int modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    virtual int delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current);
    virtual int getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo);

    /*********************************ServiceGroup相关操作*************************************************/
    virtual int addServiceGroup(const string& srvGrp, const string& key, const string& userListStr);
    virtual int updateServiceGroup(const string& srvGrp,const string& key, const string& userListStr);
    virtual int getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList);
    virtual int getServiceGroupKey(const string& srvGrp,string& key);

    /*******************************SeerAgent管理********************************/
    virtual int get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& AgentBaseInfos);
    virtual int update_agent_graystate(const UpdateAgentInfoReq& req);
    virtual int get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& AgentPackageInfos);
    virtual int update_agent_packageinfo(const UpdatePackageInfoReq& req);
    virtual int delete_agent_package(const AgentPackageReq& req);
    virtual int add_agent_packageinfo(const AgentPackageInfo & reqInfo);
protected:
    //node心跳超时时间，单位秒，默认250秒
    string _nodeTimeout;

    //主控自身心跳超时时间，单位秒，默认250秒
    string _registryTimeout;
};

#endif