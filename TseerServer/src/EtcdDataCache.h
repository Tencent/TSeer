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

#ifndef _ETCD_DATA_CACHE_H
#define _ETCD_DATA_CACHE_H

#include <string>
#include <map>
#include <vector>
#include "util/tc_thread_rwlock.h"
#include "util/tc_thread.h"
#include "util/tc_http_async.h"
#include "util/tc_singleton.h"
#include "util/tc_readers_writer_data.h"
#include "util/tc_config.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "RouterData.h"
#include "EtcdCommon.h"
#include "BaseHandle.h"

//路由信息
#define MODULETYPE "moduletype"
#define APPLICATION "application"
#define SERVICE_NAME "service_name"
#define NODE_NAME "node_name"
#define CONTAINER_NAME "container_name"
#define IP_GROUP_NAME "ip_group_name"
#define ENABLE_GROUP "enable_group"
#define ENABLE_SET "enable_set"
#define SET_NAME "set_name"
#define SET_AREA "set_area"
#define SET_GROUP "set_group" 
#define WEIGHT "weight"
#define SETTING_STATE "setting_state"
#define PROCESS_ID "process_id"
#define PRESENT_STATE "present_state"
#define NODE_PORT "node_port"
#define CONTAINER_PORT "container_port"
#define SERVANT "servant"
#define ENDPOINT "endpoint"
#define BAK_FLAG "bak_flag"
#define GRID_FLAG "grid_flag"
#define WEIGHT_TYPE "weight_type"

//基础服务信息
#define LOCATOR_ID "locator_id"

//Node信息
#define NODE_NAME "node_name"
#define NODE_NETWORKID "node_networkId"

//IDC
#define GROUP_ID "group_id"
#define PRIORITY_ID "priority_id"


//agent package info
#define OSVERSION     "osversion"
#define PACKAGENAME   "packagename"


struct NodeServiceInfo;
struct AgentPackageInfo;
struct GrayAgentInfo;

inline string rapidJsonStr(const rapidjson::Value & d)
{
    using namespace rapidjson;

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}

/**
 * 
 */
struct BaseServiceInfo
{
    string locatorId;
    string servant;
    string endpoint;
    string presentState;
    string tarsVersion;
    string enableGroup;

    string toString() const;
};

class EtcdDataCache : public TC_Singleton<EtcdDataCache>, public TC_ThreadLock
{
public:
    EtcdDataCache();
    ~EtcdDataCache();
    int init(TC_Config * pconf);

    /**
      * 全量加载etcd数据到内存态
      */
    int loadAllEtcdData(vector<map<string,string> >& routerDataList);
    
    //cache中缓存的路由数据格式
    typedef std::map<std::string, RouterData> RouterDataCacheT;

public:
    /**
      * 解析从etcd获取的路由数据到RouterDataCacheT数据结构中
      * @param jEtcdRootData
      * @param[out] mRouterData
      * @return 0成功，其它失败
      */
     int parseEtcdRootData(const rapidjson::Document& jEtcdRootData, RouterDataCacheT& routerDataCache, 
         map<string, BaseServiceInfo> &baseServiceInfoMap);
     
     /**
      * 从cache中获取指定格式的路由数据
      * key=RouterData,value=RouterData's value
      * eg:<application,Tseer>
      */
     void getRouterData(const RouterDataCacheT& rReadCache,vector<map<string,string> >& routerDataList);
    
     /**
      * 获取指定ip下的所有容器的路由记录
      */
     int getRouterDataByIp(const string& ip,vector<RouterData>& routerDataList);

     /**
      * 根据给定容器信息获取容器的路由信息
      */
     int getRouterData(const RouterData& req,RouterData& routerDataInfo);

     /**
     * 获取基础服务信息（转换成路由数据）
     */
     void getBaseServiceInfoAsRouteData(const map<string, BaseServiceInfo>&baseServiceInfoCache,vector<map<string, string> >& baseServiceDataList);

     /*
      * 解析从etcd get回来的路由数据到RouterDataCacheT数据结构中 
      */
     void parseEtcdRouteData(const rapidjson::Value &jRouterData, RouterDataCacheT& routerDataCache);

     //解析从etcd get回来的Node数据到
     void parseEtcdNodeData(const rapidjson::Value &jRouterData, vector<NodeServiceInfo> &nodes);

     void parseEtcdAgentPackageInfo(const rapidjson::Value &jPackageInfo, vector<AgentPackageInfo> &agentPackageInfoList);
    
     void parseEtcdGrayAgentPackageInfo(const rapidjson::Value &jPackageInfo, map<string,GrayAgentInfo> &gradyAgentPackageInfoList);

public:
     bool IsContainerNumEqual(const string& ip,int iRealNum);
private:
    /**
     * 解析从etcd get回来的基础服务信息数据
     */
    void parseEtcdBaseServiceData(const rapidjson::Value &jRouterData, map<string, BaseServiceInfo> &baseServiceInfoCache);

    /**
      * 创建容器的路由数据的key
      */
    string makeRouterKey(const RouterData &routerData);

    /**
      * 解析etcd中存储的路由数据中的value内容
      */
    int parseObjValue(const std::string &str,const string &sep,RouterData &routerDataInfo);
    
    /**
      * 解析etcd中存储的路由数据的key内容
      */
    int parseContainerKey(const string & str,const string &sep,RouterData& routerDataInfo);


    /***********************************基础服务**********************************/
    //创建基础服务信息的key
    string makeBaseServiceKey(const BaseServiceInfo &baseServiceInfo);

    //解析etcd中存储的基础服务信息的key内容
    int parseBaseServiceKey(const string &str, const string &sep, BaseServiceInfo &baseServiceInfo);

    //解析etcd中存储的基础服务信息的value内容
    int parseBaseServiceValue(const string &str, const string &sep, BaseServiceInfo &baseServiceInfo);

    
    /***********************************Node类**********************************/

    //接卸etcd中存储的Node数据中的value内容
    int parseNodeValue(const std::string &str, const string &sep, NodeServiceInfo &nodeInfo);
private:
    tars::TC_HttpRequest _httReq;

    //全量访问etcd的超时时间,默认是1分钟
    int _etcdReqTimeout;

    //路由数据
    tars::TC_ReadersWriterData<RouterDataCacheT> _routerDataCache;
};

#define ETCDROUTEDATA (EtcdDataCache::getInstance())

#endif

