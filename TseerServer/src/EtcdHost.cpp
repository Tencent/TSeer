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

#include "EtcdHost.h"
#include "util/tc_ex.h"
#include "util/tc_config.h"
#include "servant/TarsLogger.h"
#include "servant/Application.h"
#include "util.h"
#include "util/tc_http.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define TEST_KEY string("healthcheck/key_" + ServerConfig::LocalIp)
#define TEST_VALUE string("healthcheckvalue")
using namespace Tseer;
using namespace rapidjson;

tars::TC_ThreadRWLocker EtcdHost::_mapRWLocker;


string EtcdHost::KEY_ROUTER_TABLE = "routertable";
string EtcdHost::KEY_BASE_SERVICE = "base_service";
string EtcdHost::KEY_NODE_INFO = "node_info";
string EtcdHost::KEY_IDC_RULE = "idc_rule";
string EtcdHost::KEY_IDC_PRIORITY = "idc_priority";
string EtcdHost::KEY_SERVICE_GROUP = "service_group";
string EtcdHost::KEY_AGENT_PACKAGE_INFO   = "agent_package_info";
string EtcdHost::KEY_AGENT_GRAY_INFO      = "agent_gray_info";

EtcdHost::EtcdHost(): _terminate(false),_count(0) {}

EtcdHost::~EtcdHost()
{
    _terminate = true;

    if(isAlive())
    {
        getThreadControl().join();
    }

    TC_ThreadLock::Lock lock(*this);
    notifyAll();
}

int EtcdHost::init(TC_Config * pconf)
{
    try
    {   
        string hostStr = pconf->get("/tars/application/etcd<host>", "127.0.0.1:2379");
        vector<string> hostVec = TC_Common::sepstr<string>(hostStr, ";");
        for (size_t i = 0; i < hostVec.size(); i++)
        {
            hostVec[i] = "http://" + hostVec[i] + "/v2/keys/";
        }
        int ret = loadHostInfo(hostVec);
        if(ret != 0)
        {
            TLOGERROR(FILE_FUN<< "load etcd Host Info error!" << endl);
            return -1;
        }

        if(_activeHostMap.empty())
        {
            TLOGERROR(FILE_FUN<< "load etcd Host Info error,has no etcd host!" << endl);
            return -1;
        }

        //检查etcd健康情况
        doCheck();
        if(_etcdHostVec.empty())
        {
            TLOGERROR(FILE_FUN<< "All etcd host have been dead" << endl);
            return -1;
        }
        
        TLOGDEBUG(FILE_FUN<<"EtcdHost initiliaze succ"<<endl);
        return 0;
    }
    catch(TC_Exception & ex)
    {
        TSEER_LOG("etcdcheck")->debug() << FILE_FUN << "get etcd host error: " << ex.what()<< endl;
        return -1;
    }
}

int EtcdHost::loadHostInfo(const vector<string> &vec)
{
    for(uint32_t i=0;i<vec.size();++i)      
    {
        TC_URL stUrl;
        bool ret = stUrl.parseURL(vec[i]);
        if(ISFAILURE(ret))
        {
            TSEER_LOG("etcdcheck")->debug() << FILE_FUN << "get etcd host error: " << vec[i]<< endl;
            return -1;
        }

        string key = stUrl.getDomain();
        key += ":";
        key += stUrl.getPort();

        string sStateUrl = "http://";
        sStateUrl += stUrl.getDomain(); 
        sStateUrl += ":";
        sStateUrl += stUrl.getPort();
        sStateUrl += "/v2/keys";
        
        HostInfo stHostInfo(vec[i],stUrl.getDomain(),stUrl.getPort(),sStateUrl);
        _activeHostMap[key] = stHostInfo;
    }

    return 0;
}

string EtcdHost::getHost()
{
    ++_count;
    tars::TC_ThreadRLock readLock(_mapRWLocker);
    string url = _etcdHostVec[_count % _etcdHostVec.size()];
    return url;
}

string EtcdHost::getRouterHost()
{
    ++_count;
    string url;

    {
        tars::TC_ThreadRLock readLock(_mapRWLocker);
        url = _etcdHostVec[_count % _etcdHostVec.size()];
    }

    return url + KEY_ROUTER_TABLE; 
}

string EtcdHost::getBaseServiceHost()
{
    ++_count;
    string url;

    {
        tars::TC_ThreadRLock readLock(_mapRWLocker);
        url = _etcdHostVec[_count % _etcdHostVec.size()];
    }

    return url + KEY_BASE_SERVICE;
}

string EtcdHost::getNodeInfoHost()
{
    ++_count;
    string url;

    {
        tars::TC_ThreadRLock readLock(_mapRWLocker);
        url = _etcdHostVec[_count % _etcdHostVec.size()];
    }

    return url + KEY_NODE_INFO;
}

string EtcdHost::getIdcRuleHost()
{
    ++_count;
    string url;

    {
        tars::TC_ThreadRLock readLock(_mapRWLocker);
        url = _etcdHostVec[_count % _etcdHostVec.size()];
    }

    return url + KEY_IDC_RULE;
}

string EtcdHost::getIdcPriorityHost()
{
    ++_count;
    string url;

    {
        tars::TC_ThreadRLock readLock(_mapRWLocker);
        url = _etcdHostVec[_count % _etcdHostVec.size()];
    }

    return url + KEY_IDC_PRIORITY;
}
string EtcdHost::getServiceGroupHost()
{
    return getHost() + KEY_SERVICE_GROUP;
}


string EtcdHost::getAgentPackageInfoHost()
{
    return getHost() + KEY_AGENT_PACKAGE_INFO;
}

string EtcdHost::getGrayAgentInfoHost()
{
    return getHost() + KEY_AGENT_GRAY_INFO;
}

bool EtcdHost::doReadCheck(const string& url,const string& key,const string& keyvalue,bool checkResult)
{
    __TRY__
        do
        {
            tars::TC_HttpRequest stHttpReq;
            string tmpUrl = url + "/" + key;
            stHttpReq.setGetRequest(tmpUrl);
            tars::TC_HttpResponse httpRsp;
            int ret = stHttpReq.doRequest(httpRsp,3000);
            if(ret != TC_ClientSocket::EM_SUCCESS)
            {
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN << tmpUrl<<"|"<<ret <<endl; 
                break;
            }
            
            string valueStr = httpRsp.getContent();
            TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|"<<valueStr<<endl;

            //采用新的json解析组件
            rapidjson::Document document;
            rapidjson::ParseResult  parseOk = document.Parse(valueStr.c_str());
            if(!parseOk)
            {
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|parse error!"<<endl;
                break;
            }
            
            Value::ConstMemberIterator it = document.FindMember("errorCode");
            if (it != document.MemberEnd() && it->value.IsNumber())
            {
                string  errorMsg    = document["message"].GetString();
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|parse error|"<<errorMsg<<endl;
                break;
            }
            
            it = document.FindMember("node");
            if (it != document.MemberEnd() && it->value.IsObject())
            {
                Value::ConstMemberIterator itr = it->value.FindMember("value");
                if (itr != it->value.MemberEnd() && itr->value.IsString())
                {
                    string value  = itr->value.GetString();
                    if(checkResult &&
                        value != keyvalue)
                    {
                        TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|parse error|"<<value<<endl;
                        break;
                    }   
                }
            }
            else
            {
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|parse error!"<<endl;
                break;  
            }
            return true;
        }while(0);  
        return false;
    __CATCH__
    return false;
}


bool EtcdHost::doWriteCheck(const string& url,const string& key,const string& keyvalue)
{
    __TRY__
        do
        {
            //write
            tars::TC_HttpRequest stHttpReq;
            stHttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");

            string tmpUrl = url + "/" + key;
            string value = "value=" + keyvalue;
            stHttpReq.setPutRequest(tmpUrl,value);
            
            tars::TC_HttpResponse httpRsp;
            int ret = stHttpReq.doRequest(httpRsp,3000);
            if(ret != TC_ClientSocket::EM_SUCCESS)
            {
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN << tmpUrl<<"|"<<ret <<endl; 
                break;
            }
            TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<tmpUrl<<"|"<<httpRsp.getContent()<<endl;
            return true;
        }while(0);  
        return false;
    __CATCH__
    return false;   
}

void EtcdHost::doCheck()
{
    srand(TNOWMS);
    HostMapT::iterator itr = _activeHostMap.begin();
    vector<string> etcdHostVec;
    for(;itr!=_activeHostMap.end();++itr)   
    {
        string url = itr->second._stateUrl;

        __TRY__
            bool writeHealth = false;
            bool readHealth = false;
            string keyvalue = TEST_VALUE + "_" + TC_Common::tostr(rand()%TNOWMS);

            if(doWriteCheck(url,TEST_KEY,keyvalue))
            {
                writeHealth = true;
            }
            
            //读写之间间隔1s
            sleep(1);

            /*如果无法写就不检查读取的结果值了*/
            if(doReadCheck(url,TEST_KEY,keyvalue,writeHealth))
            {
                readHealth = true;
            }
            
            if(writeHealth||readHealth)
            {
                etcdHostVec.push_back(itr->second._url);
            }

            if(!writeHealth || !readHealth)
            {
                string warning = readHealth ? "readonly" :"dead";
                string info = "etcd host " + itr->second._ip + ":" + itr->second._port + ",has been " + warning; 
                TSEER_LOG("etcdcheck")->debug() << FILE_FUN<<url<< "|" << info << endl;
            }
        __CATCH__
    }

    if(etcdHostVec.size()!=0)
    {
        tars::TC_ThreadWLock writeLock ( _mapRWLocker );
        _etcdHostVec.swap(etcdHostVec);
    }
    else
    {
        string info = "All etcd host have been dead"; 
        TSEER_LOG("etcdcheck")->debug() << FILE_FUN << info << endl;
    }
}


void EtcdHost::mywait()
{
    TC_ThreadLock::Lock lock(*this);
    timedWait(REAP_INTERVAL);
}

void EtcdHost::run()
{
    while(!_terminate)
    {
        __TRY__
        doCheck();
        __CATCH__
        mywait();
    }
}



