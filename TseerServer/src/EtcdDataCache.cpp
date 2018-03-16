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

#include "EtcdDataCache.h"
#include <sstream>
#include "util/tc_common.h"
#include "servant/TarsLogger.h"
#include "util/tc_timeprovider.h"
#include "util.h"
#include "EtcdHost.h"

using rapidjson::Value;
using rapidjson::SizeType;

string BaseServiceInfo::toString() const
{
    ostringstream os;
    os << "locator id:" << locatorId << "|servant:" << servant << "|endpoint:" << endpoint;
    return os.str();
}

int EtcdDataCache::getRouterData(const RouterData& req,RouterData& routerDataInfo)
{
    RouterDataCacheT::iterator it;
    RouterDataCacheT& rReadCache = _routerDataCache.getReaderData();
    string key = makeRouterKey(req);
    if((it = rReadCache.find(key)) != rReadCache.end())
    {
        routerDataInfo = it->second;
        return 0;
    }
    return -1;
}

int EtcdDataCache::getRouterDataByIp(const string& ip,vector<RouterData>& routerDataList)
{
    RouterDataCacheT::iterator it;
    RouterDataCacheT& rReadCache = _routerDataCache.getReaderData();
    for(it = rReadCache.begin();it != rReadCache.end();++it)
    {
        if(it->second.node_name == ip)
        {
            routerDataList.push_back(it->second);
        }
    }
    
    return 0;
}

void EtcdDataCache::getRouterData(const RouterDataCacheT& rReadCache,vector<map<string,string> >& routerDataList)
{
    map<string,int>  ipContainerNum;
    
       RouterDataCacheT::const_iterator itr;
    for(itr = rReadCache.begin();itr != rReadCache.end();++itr)
    {
        map<string,string> routerDataMap;
        routerDataMap[MODULETYPE] = itr->second.moduletype;
        routerDataMap[APPLICATION] = itr->second.application;
        routerDataMap[SERVICE_NAME] = itr->second.service_name;
        routerDataMap[NODE_NAME] = itr->second.node_name;
        routerDataMap[CONTAINER_NAME] = itr->second.container_name;
        
        routerDataMap[ENABLE_SET] = itr->second.enable_set;
        routerDataMap[SET_NAME] = itr->second.set_name;
        routerDataMap[SET_AREA] = itr->second.set_area;
        routerDataMap[SET_GROUP] = itr->second.set_group;
        
        routerDataMap[PROCESS_ID] = itr->second.process_id;
        
        routerDataMap[IP_GROUP_NAME] = itr->second.ip_group_name;
        routerDataMap[ENABLE_GROUP] = itr->second.enable_group;
        
        routerDataMap[SETTING_STATE] = itr->second.setting_state;
        routerDataMap[PRESENT_STATE] = itr->second.present_state;
        
        routerDataMap[NODE_PORT] = itr->second.node_port;
        routerDataMap[CONTAINER_PORT] = itr->second.container_port;
        
        routerDataMap[WEIGHT] = itr->second.weight;
        routerDataMap[BAK_FLAG] = itr->second.bak_flag;
        routerDataMap[GRID_FLAG] = itr->second.grid_flag;
        routerDataMap[WEIGHT_TYPE]=itr->second.weighttype;

        map<string,string>::const_iterator itrServant = itr->second.servant.begin();
        //具体obj和andpoint
        for(;itrServant != itr->second.servant.end();++itrServant)
        {
            routerDataMap[SERVANT] = itrServant->first;
            routerDataMap[ENDPOINT] = itrServant->second;
            routerDataList.push_back(routerDataMap);
        }
        
        //记录机器下有多少个容器数据
        if(ipContainerNum.find(itr->second.node_name) != ipContainerNum.end())
        {
            ipContainerNum[itr->second.node_name]++;
        }
        else
        {
            ipContainerNum[itr->second.node_name] = 1;
        }
    }
}

void EtcdDataCache::getBaseServiceInfoAsRouteData(const map<string, BaseServiceInfo>& baseServiceInfoCache, vector<map<string, string> >& baseServiceDataList)
{
    map<string, BaseServiceInfo>::const_iterator itr = baseServiceInfoCache.begin();
    for (; itr != baseServiceInfoCache.end(); ++itr)
    {
        map<string, string> routerDataMap;
        routerDataMap[MODULETYPE] = itr->second.tarsVersion;
        routerDataMap[APPLICATION] = itr->second.tarsVersion;
        routerDataMap[SERVICE_NAME] = itr->second.tarsVersion;
        routerDataMap[NODE_NAME] = itr->second.tarsVersion;
        routerDataMap[CONTAINER_NAME] = "";

        routerDataMap[ENABLE_SET] = "N";
        routerDataMap[SET_NAME] = "";
        routerDataMap[SET_AREA] = "";
        routerDataMap[SET_GROUP] = "";

        routerDataMap[PROCESS_ID] = "";

        routerDataMap[IP_GROUP_NAME] = "";
        routerDataMap[ENABLE_GROUP] = "N";        //该字段没用，总是为N

        routerDataMap[SETTING_STATE] = itr->second.presentState;  //用present_state来作为setting_state
        routerDataMap[PRESENT_STATE] = itr->second.presentState;

        routerDataMap[NODE_PORT] = "";
        routerDataMap[CONTAINER_PORT] = "";

        routerDataMap[WEIGHT] = "";
        routerDataMap[BAK_FLAG] = "";
        routerDataMap[GRID_FLAG] = "";
        routerDataMap[WEIGHT_TYPE] = "";

        routerDataMap[SERVANT] = itr->second.servant;
        routerDataMap[ENDPOINT] = itr->second.endpoint;
        
        baseServiceDataList.push_back(routerDataMap);
    }
}


EtcdDataCache::EtcdDataCache():_etcdReqTimeout(60000)
{
    
}

EtcdDataCache::~EtcdDataCache()
{
}


int EtcdDataCache::init(TC_Config * pconf)
{
    __TRY__
    
    int httpTimeout  = TC_Common::strto<int>(pconf->get("/tars/etcd<etcdreqalltimeout>","60000"));
    _etcdReqTimeout = httpTimeout < 10000 ?10000:httpTimeout;
    
    return 0;
    __CATCH__
    return -1;  
}


int EtcdDataCache::parseContainerKey(const string & str,const string &sep,RouterData& routerDataInfo)
{
    //moduletype=Tseer/application=aa/service_name=bb/node_name=1.1.111.1/container_name=cc1
    vector<string> keyValue = TC_Common::sepstr<string>(str,sep);
    for(size_t i=0;i<keyValue.size();++i)
    {
        //moduletype=Tseer
        vector<string> sepKeyValue = TC_Common::sepstr<string>(keyValue[i],"=");
        if(sepKeyValue.size() >= 2)
        {
            if(sepKeyValue[0] == MODULETYPE)
            {
                routerDataInfo.moduletype = sepKeyValue[1];
            }
            else if(sepKeyValue[0] == APPLICATION)
            {
                routerDataInfo.application = sepKeyValue[1];
            }
            else if(sepKeyValue[0] == SERVICE_NAME)
            {
                routerDataInfo.service_name = sepKeyValue[1];
            }
            else if(sepKeyValue[0] == NODE_NAME)
            {
                routerDataInfo.node_name = sepKeyValue[1];
            }
            else if(sepKeyValue[0] == CONTAINER_NAME)
            {
                routerDataInfo.container_name = sepKeyValue[1];
            }
        }
    }

    if( routerDataInfo.moduletype.empty() || 
        routerDataInfo.application.empty() || 
        routerDataInfo.service_name.empty() ||
        routerDataInfo.node_name.empty() ||
        routerDataInfo.container_name.empty() )
    {
        return -1;
    }
    return 0;
}

int EtcdDataCache::parseObjValue(const std::string &str,const string &sep,RouterData &routerDataInfo)
{
    //格式:value=obj@endpoint:obj@endpoint||||||N||inactive|inactive|0|0|100|||
    vector<string> keyValue = TC_Common::sepstr<string>(str, sep, true);
    if(keyValue.size()< 15)
    {
        return -1;
    }

    {
        // objs= obj@endpoint:obj@endpoint
        vector<string> objs = TC_Common::sepstr<string>(keyValue[0],":");
        for(uint32_t i=0;i<objs.size();++i)
        {
            //obj@endpoint
            vector<string> endpointKv = TC_Common::sepstr<string>(objs[i],"@");
            if(endpointKv.size() >= 2)
            {
                routerDataInfo.servant[endpointKv[0]] = endpointKv[1];
            }
        }
    }
    
    routerDataInfo.set_name = keyValue[1];
    routerDataInfo.set_area = keyValue[2];
    routerDataInfo.set_group = keyValue[3];
    routerDataInfo.process_id = keyValue[4];
    routerDataInfo.ip_group_name = keyValue[5];
    routerDataInfo.enable_set = keyValue[6];
    routerDataInfo.enable_group = keyValue[7];
    routerDataInfo.setting_state = keyValue[8];
    routerDataInfo.present_state = keyValue[9];
    routerDataInfo.node_port = keyValue[10];
    routerDataInfo.container_port = keyValue[11];
    routerDataInfo.weight = keyValue[12];
    routerDataInfo.bak_flag = keyValue[13];
    routerDataInfo.grid_flag = keyValue[14];
    if(keyValue.size()>15)
    {
        routerDataInfo.weighttype = keyValue[15];
        if(keyValue.size()>16)
        {
            routerDataInfo.enable_heartbeat = keyValue[16];
        }
    }
    return 0;
}

string EtcdDataCache::makeRouterKey(const RouterData &routerData)
{
    string key = routerData.moduletype;
    key += "|";
    key += routerData.application;
    key += "|";
    key += routerData.service_name;
    key += "|";
    key += routerData.node_name;
    key += "|";
    key += routerData.container_name;

    return key;
}

void EtcdDataCache::parseEtcdRouteData(const rapidjson::Value &jRouterData,RouterDataCacheT& routerDataCache)
{
    /**
    jRouterData=
     "nodes": [
            {
                "key": "/routertable/moduletype=ns", 
                "dir": true, 
                "nodes": [
                    {
                        "key": "/routertable/moduletype=ns/application=itil", 
                        "dir": true, 
                        "nodes": [], 
                    }
                ], 
            }, 
            {
                "key": "/routertable/moduletype=Tseer", 
                "dir": true, 
                "nodes": [], 
            }, 
            {}, 
            {}
        ]
     */
    for (SizeType i = 0; i < jRouterData.Size(); ++i)
    {
        if(jRouterData[i].HasMember("dir"))
        {
            Value::ConstMemberIterator itr = jRouterData[i].FindMember("nodes");
            if (itr != jRouterData[i].MemberEnd() && itr->value.IsArray())
            {
                parseEtcdRouteData(itr->value,routerDataCache);
            }
        }
        else
        {
            //遍历所有的key
            __TRY__
            RouterData tmpRouterData;
            
            bool hasObj = false;
            bool hasPresentState = false;
            string presentState = "inactive";
            if(ISFAILURE(jRouterData.IsArray()))
            {
                continue;
            }
            
            for(SizeType j=0;j<jRouterData.Size();++j)
            {
                const rapidjson::Value& rjRouterData = jRouterData[j];
                Value::ConstMemberIterator itr = rjRouterData.FindMember("key");
                if (itr == rjRouterData.MemberEnd() ||! itr->value.IsString())
                {
                    continue;
                }
                
                string key = itr->value.GetString();
                //取最后一个子串
                //key1=/routertable/moduletype=/application=/service_name=r/node_name=/container_name=/key
                //key2=/routertable/moduletype=/application=/service_name=r/node_name=/container_name=/present_state
                //key3=XXXXX
                string lastStr = key.substr(key.find_last_of("/")+1);    
                if(OBJNAMEEKEY == lastStr)
                {
                    //obj路由数据
                    //子串moduletype=/application=/service_name=r/node_name=/container_name=/key
                    string subStr = key.substr(STRLEN("/routertable/"));
                    
                    //子串moduletype=/application=/service_name=r/node_name=/container_name=
                    string objKey = subStr.substr(0,(subStr.length()-STRLEN("/key")));
                    if(ISSUCC(parseContainerKey(objKey,"/",tmpRouterData)))
                    {
                        //目录解析成功再解析value
                        Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                        if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                        {
                            if(ISSUCC(parseObjValue(itv->value.GetString(),"|",tmpRouterData)))
                            {
                                hasObj = true;
                            }
                        }
                    }
                }
                else if(PRESENTSTATEKEY == lastStr)
                {
                    //obj实时状态数据
                    Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                    if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                    {
                        presentState = itv->value.GetString();
                        hasPresentState = true;
                    }
                }
            }
            
            if(hasObj)
            {

                if (!hasPresentState &&
                    tmpRouterData.enable_heartbeat != "" &&
                    TC_Common::lower(tmpRouterData.enable_heartbeat) == "n")
                {
                    //不开启心跳超时检查的状态始终是active
                    presentState = "active";
                }

                tmpRouterData.present_state = presentState;
                routerDataCache[makeRouterKey(tmpRouterData)] = tmpRouterData;
            }
            else
            {
                ETCDLOAD_LOGERROR<<FILE_FUN<<"parse error:"<<rapidJsonStr(jRouterData)<< endl;
            }
            __CATCH__

            //然后这个容器级别的嵌套可以退出了
            break;
        }
    }   
}

void EtcdDataCache::parseEtcdNodeData(const rapidjson::Value &jRouterData, vector<NodeServiceInfo> &nodes)
{
    /**
    jRouterData=
    "nodes": [
            {
                "key": "/node_info/node_networkId=", 
                "dir": true, 
                "nodes": [
                    {
                        "key": "/node_info/node_networkId=/node_name=", 
                        "dir": true, 
                        "nodes": [], 
                    }
                ], 
            }, 
            ...
           
        ]
    */

    for (SizeType i = 0; i < jRouterData.Size(); ++i)
    {
        if (jRouterData[i].HasMember("dir"))
        {
            Value::ConstMemberIterator itr = jRouterData[i].FindMember("nodes");
            if (itr != jRouterData[i].MemberEnd() && itr->value.IsArray())
            {
                parseEtcdNodeData(itr->value, nodes);
            }
        }
        else
        {
        __TRY__
            if (ISFAILURE(jRouterData.IsArray()))
            {
                continue;
            }

            NodeServiceInfo node;
            bool hasNode = false;
            string state = "inactive";

            for (SizeType j = 0; j < jRouterData.Size(); ++j)
            {
                const rapidjson::Value& rjRouterData = jRouterData[j];
                Value::ConstMemberIterator itr = rjRouterData.FindMember("key");
                if (itr == rjRouterData.MemberEnd() || !itr->value.IsString())
                {
                    continue;
                }

                string key = itr->value.GetString();

                string lastStr = key.substr(key.find_last_of("/") + 1);
                if (OBJNAMEEKEY == lastStr)
                {
                    Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                    if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                    {
                        if (ISSUCC(parseNodeValue(itv->value.GetString(), "|", node)))
                        {
                            hasNode = true;
                        }
                    }
                }
                else if (PRESENTSTATEKEY == lastStr)
                {
                    Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                    if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                    {
                        state = itv->value.GetString();
                    }
                }
            }

            if (hasNode)
            {
                node.presentState = state;
                nodes.push_back(node);
            }
            else
            {
                ETCDLOAD_LOGERROR<<FILE_FUN<<"parse error:"<<rapidJsonStr(jRouterData)<< endl;
            }

            return;

        __CATCH__
        }
    }
}

string EtcdDataCache::makeBaseServiceKey(const BaseServiceInfo &baseServiceInfo)
{
    string key = baseServiceInfo.locatorId;
    key += "|";
    key += baseServiceInfo.servant;

    return key;
}

int EtcdDataCache::parseBaseServiceKey(const string &str, const string &sep, BaseServiceInfo &baseServiceInfo)
{
    //locator_id=/servant=

    vector<string> values = TC_Common::sepstr<string>(str, sep, true);
    if (values.size() < 2)
    {
        return -1;
    }

    vector<string> locatorIdPair = TC_Common::sepstr<string>(values[0], "=");
    if (locatorIdPair.size() > 2)
    {
        return -1;
    }

    baseServiceInfo.locatorId = locatorIdPair[1];
    return 0;
}

int EtcdDataCache::parseBaseServiceValue(const string &str, const string &sep, BaseServiceInfo &baseServiceInfo)
{
    //格式 sValue=servant@endpoint|active|TARS_VERSION|N
    vector<string> values = TC_Common::sepstr<string>(str, sep, true);
    if (values.size() < 4)
    {
        return -1;
    }

    vector<string> servantAndEndpoint = TC_Common::sepstr<string>(values[0], "@");
    if (servantAndEndpoint.size() < 2)
    {
        return -1;
    }

    //locator字段并不需要
    baseServiceInfo.servant = servantAndEndpoint[0];
    baseServiceInfo.endpoint = servantAndEndpoint[1];
    baseServiceInfo.presentState = values[1];
    baseServiceInfo.tarsVersion = values[2];
    baseServiceInfo.enableGroup = values[3];

    return 0;
}

int EtcdDataCache::parseNodeValue(const std::string &str, const string &sep, NodeServiceInfo &nodeInfo)
{
    //格式obj@endpoint|node_name|node_networkid|endpoint_ip|endpoint_port|register_time|datadir|load_avg1|load_avg5|load_avg15
    //  |cpuuse_avg1|cpu_num|cpu_hz|memory_size|corefile_size|openfile_size|setting_state|present_state|tars_version|ostype
    //  |docker_node_version|template_name|groupId|locator|last_heartbeat_time

    vector<string> values = TC_Common::sepstr<string>(str, sep, true);
    if (values.size() < 24)
    {
        TSEER_LOG("debug")->debug() << FILE_FUN << "size:" << values.size() << endl;
        return -1;
    }

    nodeInfo.nodeName = values[1];
    nodeInfo.nodeNetworkId = values[2];
    nodeInfo.nodeObj = values[0];
    nodeInfo.endpointIp = values[3];
    nodeInfo.endpointPort = values[4];
    nodeInfo.registerTime = values[5];
    nodeInfo.dataDir = values[6];
    nodeInfo.loadAvg1 = values[7];
    nodeInfo.loadAvg5 = values[8];
    nodeInfo.loadAvg15 = values[9];
    nodeInfo.cpuuseAvg1 = values[10];
    nodeInfo.cpuNum = values[11];
    nodeInfo.cpuHz = values[12];
    nodeInfo.memorySize = values[13];
    nodeInfo.coreFileSize = values[14];
    nodeInfo.openFileSize = values[15];
    nodeInfo.settingState = values[16];
    nodeInfo.presentState = values[17];
    nodeInfo.version = values[18];
    nodeInfo.ostype = values[19];
    nodeInfo.dockerNodeVersion = values[20];
    nodeInfo.templateName = values[21];
    nodeInfo.groupId = values[22];
    nodeInfo.locator = values[23];
    nodeInfo.last_hearttime = values[24];
    return 0;
}

void EtcdDataCache::parseEtcdBaseServiceData(const rapidjson::Value &jRouterData, map<string, BaseServiceInfo> &baseServiceInfoCache)
{
    /**
    jRouterData=
     "nodes": [
            {
                "key": "/base_service/locator_id=..", 
                "dir": true, 
                "nodes": [
                    {
                        "key": "/routertable/locator_id=../servant=..", 
                        "dir": true, 
                        "nodes": [], 
                    }
                ], 
            }, 

            {}, 
            {}
        ]
     */
    for (SizeType i = 0; i < jRouterData.Size(); ++i)
    {
        if(jRouterData[i].HasMember("dir"))
        {
            Value::ConstMemberIterator itr = jRouterData[i].FindMember("nodes");
            if (itr != jRouterData[i].MemberEnd() && itr->value.IsArray())
            {
                parseEtcdBaseServiceData(itr->value, baseServiceInfoCache);
            }
        }
        else
        {
            //遍历所有的key
            __TRY__
            BaseServiceInfo stTmpServiceData;
            
            bool hasObj = false;
            bool hasPresentState = false;
            string presentState = "inactive";      //默认状态为inactive，除非在心跳数据中找到对应的数据
            if(ISFAILURE(jRouterData.IsArray()))
            {
                continue;
            }
            
            for (SizeType j = 0; j < jRouterData.Size(); ++j)
            {
                const rapidjson::Value& rjRouterData = jRouterData[j];
                Value::ConstMemberIterator itr = rjRouterData.FindMember("key");
                if (itr == rjRouterData.MemberEnd() ||! itr->value.IsString())
                {
                    continue;
                }
                
                string key = itr->value.GetString();
                //取最后一个子串
                //key1=/base_service/locator_id=/servant=/key
                //key2=/base_service/locator_id=/servant=/present_state
                string lastStr = key.substr(key.find_last_of("/")+1);    
                if(OBJNAMEEKEY == lastStr)
                {
                    //主控本身路由数据
                    //子串locator_id=/servant=/key
                    string subStr = key.substr(STRLEN("/base_service/"));
                    
                    //子串locator_id=/servant=/
                    string objKey = subStr.substr(0,(subStr.length()-STRLEN("/key")));
                    if (ISSUCC(parseBaseServiceKey(objKey, "/", stTmpServiceData)))
                    {
                        //目录解析成功再解析value
                        Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                        if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                        {
                            if (ISSUCC(parseBaseServiceValue(itv->value.GetString(), "|", stTmpServiceData)))
                            {
                                hasObj = true;
                            }
                        }
                    }
                }
                else if (PRESENTSTATEKEY == lastStr)
                {
                    //obj实时状态数据
                    Value::ConstMemberIterator itv = rjRouterData.FindMember("value");
                    if (itv != rjRouterData.MemberEnd() && itv->value.IsString())
                    {
                        presentState = itv->value.GetString();
                        hasPresentState = true;
                        (void)hasPresentState;
                    }
                }
            }
            
            if(hasObj)
            {
                stTmpServiceData.presentState = presentState;
                baseServiceInfoCache[makeBaseServiceKey(stTmpServiceData)] = stTmpServiceData;
            }

            return;

            __CATCH__
        }
    }   
}

void EtcdDataCache::parseEtcdAgentPackageInfo(const rapidjson::Value &jPackageInfo, vector<AgentPackageInfo> &agentPackageInfoList)
{
    /**
 jAgentInfo=
    "nodes":[
        {
            "key":"/agent_package_info/osversion=",
            "dir": true,
            "nodes": [
                {
                    "key": "/agent_package_info/osversion=/packageName=",
                    "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                },
                {
                    "key": "/agent_package_info/osversion=/packageName=",
                    "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                },
                ...
            ]
        },
        {
            "key":"/agent_package_info/osversion=",
            "dir": true,
            "nodes": [
                {
                    "key": "/agent_package_info/osversion=/packageName=",
                    "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                },
                {
                    "key": "/agent_package_info/osversion=/packageName=",
                    "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                },
                ...
            ]
        },
        ...
    ]
 */
    for (SizeType i = 0; i < jPackageInfo.Size(); ++i)
    {
        if (jPackageInfo[i].HasMember("dir"))
        {
            Value::ConstMemberIterator itr = jPackageInfo[i].FindMember("nodes");
            if (itr != jPackageInfo[i].MemberEnd() && itr->value.IsArray())
            {
                parseEtcdAgentPackageInfo(itr->value, agentPackageInfoList);
            }
        }
        else
        {
            __TRY__

            AgentPackageInfo info;

            const rapidjson::Value& jKeyValue = jPackageInfo[i];
            Value::ConstMemberIterator itk = jKeyValue.FindMember("key");
            if (itk == jKeyValue.MemberEnd() || !itk->value.IsString())
            {
                continue;
            }

            //"key": "/agent_package_info/osversion=/packageName="
            const string &key = itk->value.GetString();
            //osversion=/packageName=
            string keySubStr = key.substr(STRLEN("/" + EtcdHost::KEY_AGENT_PACKAGE_INFO +"/"));
            if (TC_Common::sepstr<string>(keySubStr, "/", true).size() < 2)
            {
                continue;
            }

            Value::ConstMemberIterator itv = jKeyValue.FindMember("value");
            if (itv != jKeyValue.MemberEnd() && itv->value.IsString())
            {
                //"value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                const string &value = itv->value.GetString();

                vector<string> values = TC_Common::sepstr<string>(value, "|", true);
                if (values.size() < 7)
                {
                    continue;
                }
                info.ostype = values[0];
                info.package_type = values[1];
                info.package_name = values[2];
                info.md5 = values[3];
                info.version = values[4];
                info.uploadUser = values[5];
                info.uploadTime = values[6];

                agentPackageInfoList.push_back(info);
            }
            __CATCH__
        }
    }

    return;
}

void EtcdDataCache::parseEtcdGrayAgentPackageInfo(const rapidjson::Value &jGrayInfo, map<string,GrayAgentInfo> &gradyAgentPackageInfoList)
{
    /*
    {
        "action":"get",
        "node":{
            "key":"/agent_gray_info",
            "dir":true,
            "nodes":[
                {
                    "key":"/agent_gray_info/nodeip=",
                    "value":"node_ip|grayscale_version|grayscale_state|package_name|ostype"
                },
                {
                    "key":"/agent_gray_info/nodeip=",
                    "value":"node_ip|grayscale_version|grayscale_state|package_name|ostype"
                },
                ...
            ],
        }
    }
    */
    

    //解析每个grayinfo_nodeip的key和value

    for (rapidjson::SizeType i = 0; i < jGrayInfo.Size(); ++i)
    {
        rapidjson::Value::ConstMemberIterator itk = jGrayInfo[i].FindMember("key");
        if (itk == jGrayInfo[i].MemberEnd())
        {
            continue;
        }
        //"key": "/gray_agent_info/nodeip="
        const string &key = itk->value.GetString();

        //nodeip=
        string keySubStr = key.substr(STRLEN("/" + EtcdHost::KEY_AGENT_GRAY_INFO + "/"));
        vector<string> nodeIpKv = TC_Common::sepstr<string>(keySubStr, "=");
        if (nodeIpKv.size() < 2)
        {
            continue;
        }

        rapidjson::Value::ConstMemberIterator itv = jGrayInfo[i].FindMember("value");
        if (itv == jGrayInfo[i].MemberEnd())
        {
            continue;
        }

        const string &value = itv->value.GetString();

        //"value":"node_ip|grayscale_version|grayscale_state|package_name|ostype";
        vector<string> values = TC_Common::sepstr<string>(value, "|", true); 
        if (values.size() < 5)
        {
            continue;
        }

        GrayAgentInfo info;
        info.ip = values[0];
        info.grayscale_version = values[1];
        info.grayscale_state = values[2];
        info.package_name = values[3];
        info.ostype = values[4];

        gradyAgentPackageInfoList.insert(make_pair(info.ip,info));
    }    
}

int EtcdDataCache::parseEtcdRootData(const rapidjson::Document& jEtcdRootData,RouterDataCacheT& routerDataCache, 
    map<string, BaseServiceInfo> &baseServiceInfoMap)
{
    /*格式如下
    {
        "action":"get",
        "node":{
            "dir":true,
            "nodes":[
                {
                "key":"/routertable",
                "dir":true,
                ...
                },

                {
                "key":"/base_service",
                "value":"this is awesome",
                ...
                },
                ...
            ]
        }
    }
    */
    do
    {
        __TRY__
        rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
        if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
        {
            const rapidjson::Value& jRoutertable = itr->value;
            if(ISFAILURE(jRoutertable.HasMember("nodes")))
            {
                ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                break;
            }

            const rapidjson::Value &jRouterData = jRoutertable["nodes"];
            if(ISFAILURE(jRouterData.IsArray()))
            {
                ETCDLOAD_LOGERROR <<FILE_FUN<<"jRouterData nodes is not array"<<endl;
                break;
            }

            for (SizeType i = 0; i < jRouterData.Size(); ++i)
            {
                const rapidjson::Value &nodeData = jRouterData[i];
                Value::ConstMemberIterator itr = nodeData.FindMember("key");
                if (itr == nodeData.MemberEnd() || !itr->value.IsString())
                {
                    continue;
                }

                //传到parseEtcdxxx函数的都是array类型的json点
                Value::ConstMemberIterator nodesItr = nodeData.FindMember("nodes");
                if (nodesItr == nodeData.MemberEnd() || !nodesItr->value.IsArray())
                {
                    continue;
                }

                string keyStr = itr->value.GetString();
                keyStr = keyStr.substr(1);  //去掉"/"
                if (keyStr == EtcdHost::KEY_ROUTER_TABLE)
                {
                    parseEtcdRouteData(nodesItr->value, routerDataCache);
                }
                else if (keyStr == EtcdHost::KEY_BASE_SERVICE)
                {
                    parseEtcdBaseServiceData(nodesItr->value, baseServiceInfoMap);
                }
            }
            return 0;
        }
        __CATCH__
    }while(0);
    return -1;  
}

int EtcdDataCache::loadAllEtcdData(vector<map<string,string> >& routerDataList)
{
    __TRY__
    STARTTIME
    //获取全量etcd路由数据
    string activeEtcdUrl = ETCDHOST->getHost() + "?recursive=true";

    _httReq.setGetRequest(activeEtcdUrl);
    
    string host;
    uint32_t port;
    _httReq.getHostPort(host,port);

    do
    {
        tars::TC_HttpResponse  httRsp; 
        int ret = _httReq.doRequest(httRsp,_etcdReqTimeout);

        if(ret != TC_ClientSocket::EM_SUCCESS)
        {
            string errMsg = "Fata Error!!!reqeust all etcd data from " + host + ":" + TC_Common::tostr(port) + " failure,ret=" + TC_Common::tostr(ret);
            ETCDLOAD_LOGERROR << FILE_FUN << errMsg << USETIME << endl;
            break;
        }

        const string& valueStr = httRsp.getContent();
        ETCDLOAD_LOG << FILE_FUN << "request result size=" << valueStr.size() << USETIME << endl;

        //解析Json
        rapidjson::Document document;
        rapidjson::ParseResult  bParseOk = document.Parse(valueStr.c_str());
        if(!bParseOk)
        {
            ETCDLOAD_LOGERROR << FILE_FUN<<activeEtcdUrl<<"|parse json error"<<USETIME<<endl;
            break;  
        }

        RouterDataCacheT allRouterData;
        map<string, BaseServiceInfo> baseServiceInfo;
        if (ISSUCC(parseEtcdRootData(document, allRouterData, baseServiceInfo)))
        {
            _routerDataCache.getWriterData() = _routerDataCache.getReaderData();
            if(allRouterData.size() > 0||baseServiceInfo.size() > 0)
            {
                //转换成路由数据
                getRouterData(allRouterData,routerDataList);

                if (baseServiceInfo.size() > 0)
                {
                    getBaseServiceInfoAsRouteData(baseServiceInfo,routerDataList);
                    ETCDLOAD_LOG << FILE_FUN << "load baseServiceInfo succ, size:" << baseServiceInfo.size() << USETIME << endl;
                }
                else
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "no baseServiceInfo data" << endl;
                }
                
                _routerDataCache.getWriterData() = allRouterData;

                //更新数据之后，读写池子交换
                _routerDataCache.swap();
                ETCDLOAD_LOG << FILE_FUN << "load allRouterData succ, size:" << allRouterData.size() << USETIME << endl;
                return 0;
            }
            else
            {
                string errMsg =  "Fata Error!!!reqeust all etcd data from " + host + ":" + TC_Common::tostr(port) +" succ,but parse data empty";
                ETCDLOAD_LOGERROR << FILE_FUN << errMsg << USETIME << endl;
            }
        }
        else
        {
            string errMsg =  "Fata Error!!!reqeust all etcd data from " + host + ":" + TC_Common::tostr(port) +" succ,but parse data failure";
            ETCDLOAD_LOGERROR<<FILE_FUN<<errMsg<<USETIME<<endl;
        }
    }while(0);
    
    return -1;
    
    __CATCH__
    
    string errMsg = "load all etcd data from  exception";
    ETCDLOAD_LOGERROR << FILE_FUN<<errMsg<<endl;
    return -1;
}


