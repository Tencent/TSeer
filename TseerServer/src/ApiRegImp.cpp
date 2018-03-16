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

#include "ApiRegImp.h"
#include "util.h"
#include "util/tc_md5.h"
#include "util/tc_timeprovider.h"
#include "servant/Application.h"
#include "util/tc_encoder.h"
#include "StoreCache.h"
#include "EtcdDataCache.h"
#include "QueryF.h"
#include "StoreProxy.h"
#include "TSeerServer.h"
#include "UpdateThread.h"

#define ADD  "addserver"
#define DEL  "delserver"
#define UPDATE  "updateserver"
#define GET  "getserver"
#define KEEPALIVE  "keepalive"

//管理平台专用
#define ADMIN_ADD "addservicegroup"
#define ADMIN_UPDATE "updateservicegroup"
#define ADMIN_GET "getservicegroup"
#define ADMIN_GETIDCGRP "getidcgroup"
#define ADMIN_DEL_AGENT "deleteagent"
#define ADMIN_UPDATE_AGENT_LOCATOR "updateagentlocator"

//IDC分组管理
#define IDC_ADD_RULE "add_idc_group_rule"
#define IDC_MODIFY_RULE "modify_idc_group_rule"
#define IDC_DEL_GROUP_RULE "del_idc_group_rule"
#define IDC_GET_ALL_RULE "get_all_idc_group_rule"
//IDC优先级管理
#define IDC_ADD_PRIORITY "add_idc_priority"
#define IDC_MODIFY_PRIORITY "modify_idc_priority"
#define IDC_DEL_PRIORITY "del_idc_priority"
#define IDC_GET_ALL_PRIORITY "get_all_idc_priority"

//SeerAgent管理
#define AGENT_GET_BASE_INFO "getagentbaseinfo"
#define AGENT_UPDATE_GRAYSTATE "updateagentgraystate"
#define AGENT_ADD_PACKAGE_INFO "addagentpackageinfo"
#define AGENT_GET_PACKAGE_INFO "getagentpackageinfo"
#define AGENT_UPDATE_PACKAGE_INFO "updateagentpackageinfo"
#define AGENT_DELETE_PACKAGE_INFO "deleteagentpackage"
#define AGENT_GET_OSTYPE_INFO "getagentostypeinfo"

#define WEB_ADMIN_KEY std::string("webadmin")
#define WEB_ADMIN_USER  std::string("adminuser")

#ifndef ROUTER_AGENT
#define ROUTER_AGENT std::string("router_agent")
#endif

// web传来的json-key
#define SERVICE_GROUP "service_group"
#define APP_NAME "app_name"
#define SERVER_NAME "servername"

using namespace tars;
void ApiRegImp::initialize()
{
    _supportMethods[ADD] = &ApiRegImp::addServer;
    _supportMethods[DEL] = &ApiRegImp::delServer;
    _supportMethods[UPDATE] = &ApiRegImp::updateServer;
    _supportMethods[GET] = &ApiRegImp::getServer;
    _supportMethods[KEEPALIVE] = &ApiRegImp::keepalive;

    _supportMethods[ADMIN_ADD] = &ApiRegImp::addServiceGroup;
    _supportMethods[ADMIN_UPDATE] = &ApiRegImp::updateServiceGroup;
    _supportMethods[ADMIN_GETIDCGRP] = &ApiRegImp::getIdcGroup;
    _supportMethods[ADMIN_GET] = &ApiRegImp::getServiceGroup;
    _supportMethods[ADMIN_DEL_AGENT] = &ApiRegImp::deleteAgent;
    _supportMethods[ADMIN_UPDATE_AGENT_LOCATOR] = &ApiRegImp::updateAgentLocator;

    _supportMethods[IDC_ADD_RULE] = &ApiRegImp::addIdcGroupRule;
    _supportMethods[IDC_MODIFY_RULE] = &ApiRegImp::modifyIdcGroupRule;
    _supportMethods[IDC_DEL_GROUP_RULE] = &ApiRegImp::delIdcGroupRule;
    _supportMethods[IDC_GET_ALL_RULE] = &ApiRegImp::getAllIdcGroupRule;
    _supportMethods[IDC_ADD_PRIORITY] = &ApiRegImp::addIdcPriority;
    _supportMethods[IDC_MODIFY_PRIORITY] = &ApiRegImp::modifyIdcPriority;
    _supportMethods[IDC_DEL_PRIORITY] = &ApiRegImp::delIdcPriority;
    _supportMethods[IDC_GET_ALL_PRIORITY] = &ApiRegImp::getAllIdcPriority;

    _supportMethods[AGENT_GET_BASE_INFO] = &ApiRegImp::getAgentBaseInfo;
    _supportMethods[AGENT_UPDATE_GRAYSTATE] = &ApiRegImp::updateAgentGrayState;
    _supportMethods[AGENT_GET_PACKAGE_INFO] = &ApiRegImp::getAgentPackageInfo;
    _supportMethods[AGENT_ADD_PACKAGE_INFO] = &ApiRegImp::addAgentPackageInfo;
    _supportMethods[AGENT_UPDATE_PACKAGE_INFO] = &ApiRegImp::updateAgentPackageInfo;
    _supportMethods[AGENT_DELETE_PACKAGE_INFO] = &ApiRegImp::deleteAgentPackageInfo;
    _supportMethods[AGENT_GET_OSTYPE_INFO] = &ApiRegImp::getAgentOstypeInfo;
}

void ApiRegImp::destroy()
{
}


unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
            strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else
            strTemp += str[i];
    }

    return strTemp;
}

bool getRandomKey(const string& srvGrp,string& key)
{
    unsigned long long int random_value = 0;
    size_t size = sizeof(random_value);
    
    ifstream urandom("/dev/urandom", ios::in|ios::binary); 
    if(urandom) 
    {
        bool bRet = false;
        urandom.read(reinterpret_cast<char*>(&random_value), size); 
        if(urandom)
        {
            key = TC_MD5::md5str(string(reinterpret_cast<char*>(&random_value)) + srvGrp);
            bRet = true;
        }
        urandom.close();
        return bRet;
    }
    else 
    {
       return false;
    }
}

void parseNormal(multimap<string, string> &multiParams, const string& reqBuffer)
{
    int flag = 0;
    string key;
    string value;
    string tmp;
    string::size_type len = reqBuffer.length();
    string::size_type pos = 0;

    while (pos < len)
    {
        tmp = "";

        if(flag == 0)
        {
            while ( (reqBuffer[pos] != '=') && (pos < len) )
            {
                tmp += (reqBuffer[pos] == '+') ? ' ' : reqBuffer[pos];

                ++pos;
            }
        }
        else
        {
            while ( (reqBuffer[pos] != '&') && (pos < len) )
            {
                tmp += (reqBuffer[pos] == '+') ? ' ' : reqBuffer[pos];

                ++pos;
            }
        }

        if (flag == 0)                         //param name
        {
            key = tmp;

            if ( (reqBuffer[pos] != '=') || (pos == len - 1) )
            {
                value = "";

                multiParams.insert(multimap<string, string>::value_type(key, value));
            }
            else
            {
                flag = 1;
            }
        }
        else
        {
            value = tmp;

            multiParams.insert(multimap<string, string>::value_type(key, value));

            flag = 0;
        }

        ++pos;
    }
}

int ApiRegImp::getReqParam(const tars::TC_HttpRequest& req,
                           string& method,
                           string& key,
                           string& methodParam,
                           string& errMsg) const
{
    __TRY__
    do {
        multimap<string, string> multiParams;
        if(req.isPOST())
        {
            parseNormal(multiParams,req.getContent());
        }
        else if(req.isGET())
        {    
            /*key=afasfsafd&interface_name=nameo&interface_params={}*/
            parseNormal(multiParams,req.getRequestParam());
        }
        else
        {
            errMsg = "invalid method";
            break;
        }

        multimap<string, string>::const_iterator it(multiParams.find("interface_name"));
        if(it != multiParams.end())
        {
            method = it->second;
        }
        else
        {
            errMsg = "invalid req url";
            break;  
        }

        it = multiParams.find("interface_params");
        if (it != multiParams.end())
        {
            methodParam = UrlDecode(it->second);
            APIIMP_LOG<<FILE_FUN<<methodParam<<endl;
        }

        return API_SUCC;
    } while(0);

    APIIMP_LOGERROR<<FILE_FUN<<errMsg<<endl;
    return API_INVALID_PARAM;
    __CATCH__
    return API_INTERNAL_ERROR;
}

bool ApiRegImp::IsApiVersionValid(const tars::TC_HttpRequest& req) const
{
    __TRY__
    vector<string> vStr = TC_Common::sepstr<string>(req.getRequestUrl(), "/");
    if (vStr.size() >= 1)
    {
        const string& version = vStr[0];
        return version == "v1";
    }
    __CATCH__

    return false;
}

void ApiRegImp::getLocatorList(tars::TarsCurrentPtr current)
{
    //没有传地址过来，自己取最新的活跃节点
    //获取最新的主控ip列表
    vector<EndpointInfo> activeEndPoint;
    vector<EndpointInfo> inactiveEndPoint;
    Tseer::QueryFPrx registryPrx = Application::getCommunicator()->stringToProxy<Tseer::QueryFPrx>(TSEERSERVER_QUERYOBJ);
    registryPrx->tars_endpoints(activeEndPoint, inactiveEndPoint);
    if (activeEndPoint.empty())
    {
        TC_HttpResponse httpResp;
        httpResp.setResponse(404, "OK", "fata errror");
        httpResp.setContentType("text/html;charset=utf-8");
        httpResp.setConnection("Keep-Alive");
        string httpBuffer = httpResp.encode();

        current->sendResponse(httpBuffer.c_str(), httpBuffer.length());
        FDLOG("Http") <<FILE_FUN<< "|clientIP:" << current->getIp()<<"|locator is empty"<<endl;
        return;
    }

    string tmpLocatorList;
    for (size_t i = 0; i < activeEndPoint.size(); i++)
    {
        string sEndPoint= "tcp -h " + activeEndPoint[i].host() + " -p " + TC_Common::tostr<int>(activeEndPoint[i].port());
        if (tmpLocatorList != "")
        {
            tmpLocatorList += ":" + sEndPoint;
        }
        else
        {
            tmpLocatorList = sEndPoint;
        }
    }

    string registryLocator = TSEERSERVER_QUERYOBJ + "@"+tmpLocatorList;
    TC_HttpResponse httpResp;
    httpResp.setResponse(200, "OK", registryLocator);
    httpResp.setContentType("text/html;charset=utf-8");
    httpResp.setConnection("Keep-Alive");
    string httpBuffer = httpResp.encode();

    current->sendResponse(httpBuffer.c_str(), httpBuffer.length());
    FDLOG("Http") <<FILE_FUN<< "|clientIP:" << current->getIp()<<"|get locator succ|"<<registryLocator<<endl;
}
void ApiRegImp::downloadInstallScript(tars::TarsCurrentPtr current)
{
    string context = g_app.getSeerInstallScript();
    int code = 200;
    if(context.empty())
    {
        context = "installscript is empty";
        code = 404;
    }

    TC_HttpResponse httpResp;
    httpResp.setResponse(code, "OK", context);
    httpResp.setContentType("text/html;charset=utf-8");
    httpResp.setConnection("Keep-Alive");

    string httpBuffer = httpResp.encode();
    current->sendResponse(httpBuffer.c_str(), httpBuffer.length());

    if (code == 200)
        FDLOG("Http") <<FILE_FUN<< "|clientIP:" << current->getIp()<<"|get installscript succ"<<endl;
    else;
        FDLOG("Http") <<FILE_FUN<< "|clientIP:" << current->getIp()<<"|installscript is empty"<<endl;
}

void ApiRegImp::downloadAgentPackage(tars::TarsCurrentPtr current,const string& reqParam)
{
    __TRY__
    string ostype = OS_VERSION;
    multimap<string, string> mmpParams;
    parseNormal(mmpParams,reqParam);
    if(mmpParams.find("os") != mmpParams.end())
    {
        ostype = mmpParams.find("os")->second;
    }

    string context;
    PackageData data;
    if (UPDATETHREAD->getFormalPackageData(ostype, data))
    {
        context.assign(data.dataBuff);
        
        TC_HttpResponse httpResp;
        httpResp.setResponse(200, "OK", context);
        httpResp.setContentType("text/html;charset=utf-8");
        httpResp.setConnection("Keep-Alive");
        string httpBuffer = httpResp.encode();

        current->sendResponse(httpBuffer.c_str(), httpBuffer.length());
        APIIMP_LOG << FILE_FUN<< "|clientIP:" << current->getIp()<< "|url:" << reqParam<<"|get data succ,context size:" << context.size() << endl;
        return;
    }
    __CATCH__
    
    APIIMP_LOG <<FILE_FUN << "|clientIP:" << current->getIp()<< "|url:" << reqParam << "|server has no packet" << endl;
    TC_HttpResponse httpResp;
    httpResp.setResponse(404, "OK", "error,has no packet");
    httpResp.setContentType("text/html;charset=utf-8");
    httpResp.setConnection("Keep-Alive");
    string httpBuffer = httpResp.encode();

    current->sendResponse(httpBuffer.c_str(), httpBuffer.length());
}

//以下调用如果使用了 STOREPROXY 来进行查询，如果异步请求发送失败，直接在此回包
//否则都在 STOREPROXY 的回调线程回包
int ApiRegImp::doRequest(TarsCurrentPtr current, vector<char> & response)
{
    API_ERROR_CODE ret = API_INTERNAL_ERROR; 
    string errMsg;
    do
    {
        __TRY__

        const vector<char>& request = current->getRequestBuffer();

        tars::TC_HttpRequest httpRequest;
        httpRequest.decode((const char*)&request[0], request.size());
        if (!httpRequest.isGET() && !httpRequest.isPOST())
        {
            errMsg = "only support get/post method";
            ret = API_INVALID_PARAM;
            break;
        }

        //检查api版本号是否合法
        if (!IsApiVersionValid(httpRequest))
        {
            if (httpRequest.getRequestUrl() == "/monitor/monitor.jsp")
            {
                current->setResponse(false);
                TC_HttpResponse response;
                response.setResponse("opps", 4);
                std::string data(response.encode());
                current->sendResponse(data.c_str(), data.size());
                APIIMP_LOG << FILE_FUN << "health checking opps" << endl;
                return 0;
            }

            //下载安装脚本
            if(httpRequest.getRequestUrl() == "/installscript")
            {
                current->setResponse(false);
                downloadInstallScript(current);
                APIIMP_LOG <<FILE_FUN<<"installscript|"<<httpRequest.getRequestUrl()<<endl;
                return 0;
            }
            
            //下载agent安装包
            if (httpRequest.getRequestUrl() == string("/"+ROUTER_AGENT))
            {
                current->setResponse(false);
                downloadAgentPackage(current,httpRequest.getRequestParam());
                return 0;
            }

            if(httpRequest.getRequestUrl() == "/getlocator")
            {
                current->setResponse(false);
                getLocatorList(current);
                return 0;
            }
            
            errMsg = "API version not support any more";
            ret = API_INVALID_PARAM;
            break;
        }

        string method;
        string key;
        string methodParam;
        if (getReqParam(httpRequest, method, key, methodParam, errMsg) != API_SUCC)
        {
            ret = API_INVALID_PARAM;
            break;
        }

        //检查method名字是否合法
        std::map<string, CommandHandler>::const_iterator it = _supportMethods.find(method);
        if (it == _supportMethods.end())
        {
            errMsg = "not support such API:" + method;
            ret = API_INVALID_PARAM;
            break;
        }

        rapidjson::Document document;
        rapidjson::ParseResult bParseOk = document.Parse(methodParam.c_str());
        
        //某些获取接口不需要传请求参数进来，
        //此时rapidjson解析时会认为失败，所以要做非空判断避免这种逻辑
        if (!bParseOk && !methodParam.empty())
        {
            errMsg = "invalid json params";
            ret = API_INVALID_PARAM;
            APIIMP_LOG << FILE_FUN << methodParam.c_str() << "|parse json error" << endl;
            break;
        }

        map<string, string> context;
        context.insert(make_pair("method",method));
        current->setResponseContext(context);

        (this->*(it->second))(current, document);
        return 0;

    __CATCHEXT__
    } while (0);

    APIIMP_LOGERROR << FILE_FUN << current->getIp() << ":" << current->getPort() << "|" << errMsg << "|" << ret << endl;
    if(current->isResponse())
    {
        doResponse(current, int(ret), errMsg);
    }
    return 0;
}


int ApiRegImp::addServer(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    APIIMP_LOG<<FILE_FUN<<"coming"<<endl;
    do{
        __TRY__
        
        Tseer::RouterData  data;

        CHECK_SETKEYVALUE(SERVICE_GROUP, data.moduletype);
        CHECK_SETKEYVALUE(APP_NAME, data.application);
        CHECK_SETKEYVALUE(SERVER_NAME, data.service_name);
        
        string trueKey;
        //检查业务集是否已经添加
        if(STOREPROXY->getServiceGroupKey(data.moduletype,trueKey) != 0)
        {
            errMsg = data.moduletype + " not  exist!";
            ret = API_NO_MODULETYPE;
            break;
        }

        if (jData.HasMember("port_list") && jData["port_list"].IsArray())
        {
            //有端口信息才处理
            vector<Tseer::RouterData> routerDataList;
            const rapidjson::Value &portList = jData["port_list"];
            for (rapidjson::SizeType i = 0; i < portList.Size(); ++i)
            {
                Tseer::RouterData tmpData = data;
                string port_name;
                EXIST_GETVALUE_A(port_name, "port_name");
                if (port_name.empty())
                {
                    APIIMP_LOGERROR << FILE_FUN << display(data) << ",port_name should not be empty" << endl;
                    continue;
                }
                else
                    APIIMP_LOG << FILE_FUN << "port_name = " << port_name <<endl;

                string ip;
                EXIST_GETVALUE_A(ip,"ip");
                if (ip.empty())
                {
                    APIIMP_LOGERROR << FILE_FUN << display(data) << ",ip should not be empty" << endl;
                    continue;
                }

                string port;
                EXIST_GETVALUE_A(port,"port");

                string protocol = "tcp";
                EXIST_GETVALUE_A(protocol,"protocol");

                tmpData.node_name = ip;
                tmpData.container_name = port_name + "_" + port;

                tmpData.servant[port_name] = protocol + " -h " + ip + " -p " + port;

                EXIST_GETVALUE_A(tmpData.enable_group, "enable_idc_group");
                EMPTYSETVALUE(tmpData.enable_group, "N");

                EXIST_GETVALUE_A(tmpData.ip_group_name, "ip_group_name");
                EMPTYSETVALUE(tmpData.ip_group_name, "");

                EXIST_GETVALUE_A(tmpData.enable_set, "enable_set");
                EMPTYSETVALUE(tmpData.enable_set, "N");

                EXIST_GETVALUE_A(tmpData.set_name, "set_name");
                EMPTYSETVALUE(tmpData.set_name, "");

                EXIST_GETVALUE_A(tmpData.set_area, "set_area");
                EMPTYSETVALUE(tmpData.set_area, "");

                EXIST_GETVALUE_A(tmpData.set_group, "set_group");
                EMPTYSETVALUE(tmpData.set_group, "");

                tmpData.setting_state = "active";
                EXIST_GETVALUE_A(tmpData.enable_heartbeat, "enable_heartbeat");
                EMPTYSETVALUE(tmpData.enable_heartbeat, "N");

                if ("n" == TC_Common::lower(tmpData.enable_heartbeat))
                {
                    tmpData.present_state = "active";
                }
                else
                {
                    //启用心跳,状态默认是inactive
                    tmpData.present_state = "inactive";
                }

                EXIST_GETVALUE_A(tmpData.grid_flag, "grid_flag");
                EMPTYSETVALUE(tmpData.grid_flag, "NORMAL");

                int tmpWeight = -1;     //权重值默认为-1，即不开启权重
                if (portList[i].HasMember("weight"))
                    tmpWeight = TC_Common::strto<int>(portList[i]["weight"].GetString());
                
                if(tmpWeight < 0)
                {
                    tmpData.weighttype = "";
                }
                else
                {
                    tmpData.weighttype = "1";//只支持静态权重
                }
                tmpData.weight = TC_Common::tostr(tmpWeight);
                
                routerDataList.push_back(tmpData);
            }

            if(routerDataList.size() > 0 && STOREPROXY->addServers(routerDataList,current) == 0)
            {
                APIIMP_LOG<<FILE_FUN<<display(data)<<"|add succ" << endl;
                ret = API_SUCC;
            }
            else
            {
                APIIMP_LOGERROR<<FILE_FUN<<display(data)<<"add failure,ret="<< API_INTERNAL_ERROR << endl;
                errMsg = "add server  failure";
                break;
            }
            
        }
        else
        {
            ret = API_INVALID_PARAM;
            errMsg = "should provide port_list param";
            break;
        }
        return ret;
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    return ret;
}

int ApiRegImp::updateServer(TarsCurrentPtr current,rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do {
        __TRY__

        Tseer::RouterData data;

        CHECK_SETKEYVALUE(SERVICE_GROUP, data.moduletype);
        CHECK_SETKEYVALUE(APP_NAME, data.application);
        CHECK_SETKEYVALUE(SERVER_NAME, data.service_name);

        if (jData.HasMember("port_list") && jData["port_list"].IsArray())
        {
            //有端口信息才处理
            vector<Tseer::RouterData> routerDataList;
            const rapidjson::Value &portList = jData["port_list"];
            for (rapidjson::SizeType i = 0; i < portList.Size(); ++i)
            {
                Tseer::RouterData  tmpData = data;
                string port_name;
                EXIST_GETVALUE_A(port_name, "port_name");

                string ip;
                EXIST_GETVALUE_A(ip, "ip");
                if(ip.empty() || port_name.empty())
                {
                    APIIMP_LOGERROR<<FILE_FUN << display(data)<< ",port_name/ip should not be empty" << endl;
                    continue;
                }

                string port;
                EXIST_GETVALUE_A(port, "port");

                string protocol;
                EXIST_GETVALUE_A(protocol, "protocol");
                EMPTYSETVALUE(protocol,"tcp");

                tmpData.node_name = ip;
                tmpData.container_name = port_name + "_" + port;
                
                tmpData.servant[port_name] = protocol + " -h " + ip + " -p " + port;

                EXIST_GETVALUE_A(tmpData.enable_group,"enable_idc_group");
                EXIST_GETVALUE_A(tmpData.ip_group_name,"ip_group_name");
                EXIST_GETVALUE_A(tmpData.enable_set,"enable_set");
                EXIST_GETVALUE_A(tmpData.set_name,"set_name");
                EXIST_GETVALUE_A(tmpData.set_area,"set_area");
                EXIST_GETVALUE_A(tmpData.set_group,"set_group");
                tmpData.setting_state = "active";
                EXIST_GETVALUE_A(tmpData.enable_heartbeat,"enable_heartbeat");            
                EXIST_GETVALUE_A(tmpData.present_state,"present_state");
                EXIST_GETVALUE_A(tmpData.grid_flag,"grid_flag");

                if (portList[i].HasMember("weight"))
                {
                    int tmpWeight = TC_Common::strto<int>(portList[i]["weight"].GetString());
                    if(tmpWeight < 0)
                    {
                        tmpData.weighttype = "";
                    }
                    else
                    {
                        tmpData.weighttype = "1";//只支持静态权重
                    }
                    tmpData.weight = TC_Common::tostr(tmpWeight);
                }
                
                routerDataList.push_back(tmpData);
            }

            if((ret = STOREPROXY->updateServers(routerDataList,current)) != 0)
            {
                APIIMP_LOGERROR<<FILE_FUN<<display(data)<<"|update failure,ret="<< API_INTERNAL_ERROR << endl;
                errMsg = "update server failure";
                break;
            }
            else
            {
                APIIMP_LOG<<FILE_FUN<<display(data)<<"|updateserver succ" << endl;
                ret = API_SUCC;
            }
        }
        else
        {
            errMsg = "should provide port_list param";
            APIIMP_LOGERROR<<FILE_FUN<<display(data)<<"|"<<errMsg<< endl;
            ret = API_INVALID_PARAM;
            
        }
        return ret;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);
    return ret;
}


int ApiRegImp::keepalive(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do {
        __TRY__

        Tseer::RouterData  data;

        CHECK_SETKEYVALUE(SERVICE_GROUP, data.moduletype);
        CHECK_SETKEYVALUE(APP_NAME, data.application);
        CHECK_SETKEYVALUE(SERVER_NAME, data.service_name);
        CHECK_SETKEYVALUE(PRESENT_STATE, data.present_state);

        if (jData.HasMember("port_list") && jData["port_list"].IsArray())
        {
            //有端口信息才处理
            vector<Tseer::RouterData> routerDataList;
            rapidjson::Value &portList = jData["port_list"];
            for (rapidjson::SizeType i = 0; i < portList.Size(); ++i)
            {
                Tseer::RouterData  tmpData = data;
                
                string port_name;
                EXIST_GETVALUE_A(port_name, "port_name");
                
                string ip;
                EXIST_GETVALUE_A(ip, "ip");
                if(ip.empty() || port_name.empty())
                {
                    APIIMP_LOGERROR<<FILE_FUN << display(data)<< ",port_name/ip should not be empty" << endl;
                    continue;
                }

                string port;
                EXIST_GETVALUE_A(port, "port");

                string protocol;
                EXIST_GETVALUE_A(protocol, "protocol");
                EMPTYSETVALUE(protocol,"tcp");

                tmpData.node_name = ip;
                tmpData.container_name = port_name + "_" + port;
                
                tmpData.servant[port_name] = protocol + " -h " + ip + " -p " + port;

                //获取心跳超时时间
                EXIST_GETVALUE_A(tmpData.heartbeattimeout,"heartbeattimeout");
                
                routerDataList.push_back(tmpData);
            }

            if((ret = STOREPROXY->keepServerAlive(routerDataList,current)) != 0)
            {
                APIIMP_LOGERROR<<FILE_FUN<<display(data)<<"keepalive failure,ret="<< API_INTERNAL_ERROR << endl;
                errMsg = "keepalive  failure";
                break;
            }
            else
            {
                APIIMP_LOG<<FILE_FUN<<display(data)<<"|keepalive succ" << endl;
                ret = API_SUCC;
            }
        }
        else
        {
            errMsg = "should provide port_list param";
            APIIMP_LOGERROR<<FILE_FUN<<display(data)<<"|"<<errMsg<< endl;
            ret = API_INVALID_PARAM;
            
        }
        __CATCHEXT__
    }while(0);
    

    doResponse(current,int(ret),errMsg);

    return ret;    
}


int ApiRegImp::getServer(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do {
        __TRY__

        Tseer::RouterDataRequest dataReq;
        CHECK_SETKEYVALUE(SERVICE_GROUP, dataReq.moduletype);
        EXIST_GETVALUE_EX(APP_NAME, dataReq.application);
        EXIST_GETVALUE_EX(SERVER_NAME, dataReq.service_name);

        if((ret = STOREPROXY->getServers(dataReq,current)) != 0)
        {
            APIIMP_LOGERROR<<FILE_FUN<<display(dataReq)<<"getserver failure,ret="<< API_INTERNAL_ERROR << endl;
            errMsg = "getserver  failure";
            break;
        }
        else
        {
            APIIMP_LOG<<FILE_FUN<<display(dataReq)<<"getserver succ" << endl;
            ret = API_SUCC;
        }
        return ret;
        
        __CATCHEXT__
    }while(0);
    
    doResponse(current, ret, errMsg);
    
    return ret;        
}

int ApiRegImp::delServer(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
    __TRY__

        Tseer::RouterDataRequest  dataReq;

        CHECK_SETKEYVALUE(SERVICE_GROUP, dataReq.moduletype);
        CHECK_SETKEYVALUE(APP_NAME, dataReq.application);
        CHECK_SETKEYVALUE(SERVER_NAME, dataReq.service_name);

        vector<Tseer::RouterDataRequest> routerDataList;
        if (jData.HasMember("port_list") && jData["port_list"].IsArray())
        {
            //有端口信息才处理
            rapidjson::Value &portList = jData["port_list"];
            for (rapidjson::SizeType i = 0; i < portList.Size(); ++i)
            {
                Tseer::RouterDataRequest  tmpData = dataReq;
                string port_name;
                EXIST_GETVALUE_A(port_name, "port_name");
                string ip;
                EXIST_GETVALUE_A(ip, "ip");
                if(ip.empty() || port_name.empty())
                {
                    APIIMP_LOGERROR<<FILE_FUN << display(tmpData)<< ",port_name/ip should not be empty" << endl;
                    continue;
                }

                string port;
                EXIST_GETVALUE_A(port, "port");
                
                tmpData.node_name = ip;
                tmpData.container_name = port_name + "_" + port;
                routerDataList.push_back(tmpData);
            }
        }
        else
        {
            routerDataList.push_back(dataReq);  
        }

        if((ret = STOREPROXY->deleteServers(routerDataList,current)) != 0)
        {
            APIIMP_LOGERROR<<FILE_FUN<<display(dataReq)<<"delserver failure,ret="<< API_INTERNAL_ERROR << endl;
            errMsg = "delserver from etcd failure";
            break;
        }
        else
        {
            APIIMP_LOG<<FILE_FUN<<display(dataReq)<<"delserver succ" << endl;
            ret = API_SUCC;
        }
        return ret;
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;        
}

int ApiRegImp::getServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string user;
        EXIST_GETVALUE(user);
        
        APIIMP_LOG<<FILE_FUN<< user<< endl;
        vector<ServiceGroupInfo> srvGrpInfoList;
        if (STOREPROXY->getAllServiceGroup(srvGrpInfoList) == 0)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value resultData(rapidjson::kArrayType);
            for (size_t i = 0; i < srvGrpInfoList.size(); i++)
            {
                if (user.empty() || 
                    user == WEB_ADMIN_USER || 
                    srvGrpInfoList[i].setUserList.count(user) == 1)
                {
                    rapidjson::Value jServiceGroup(rapidjson::kObjectType);
                    rapidjson::Value serviceGroupJson(srvGrpInfoList[i].serviceGrp.c_str(), allocator);
                    rapidjson::Value keyJson(srvGrpInfoList[i].key.c_str(), allocator);
                    
                    string userListStr = TC_Common::tostr(srvGrpInfoList[i].setUserList.begin(), srvGrpInfoList[i].setUserList.end(), ",");
                    rapidjson::Value userJson(userListStr.c_str(), allocator);

                    jServiceGroup.AddMember("service_group", serviceGroupJson, allocator);
                    jServiceGroup.AddMember("key", keyJson, allocator);
                    jServiceGroup.AddMember("user", userJson, allocator);

                    resultData.PushBack(jServiceGroup, allocator);
                }
            }

            ret = API_SUCC;
            if (resultData.Empty())
            {
                errMsg = user + " haven't registry a service group";
            }

            document.AddMember("data", resultData, allocator);
            doResponse(current, int(ret), document, errMsg);
        }
        else
        {
            errMsg = "load data failure";
            ret = API_INTERNAL_ERROR;
            APIIMP_LOGERROR << FILE_FUN << errMsg << endl;
            break;
        }

        return ret;
        __CATCHEXT__
    }while(0);
    

    doResponse(current,int(ret),errMsg);

    return ret;        
}

int ApiRegImp::updateServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do {
        __TRY__
        string user;
        CHECK_SETVALUE(user);

        string service_group;
        CHECK_SETVALUE(service_group);

        APIIMP_LOG<<FILE_FUN<< service_group <<"|"<<user<< endl;;
        vector<ServiceGroupInfo> srvGrpInfoList;
        if((ret = STOREPROXY->getAllServiceGroup(srvGrpInfoList)) == 0)
        {
            string key;
            bool hasGrp = false;
            for(size_t i = 0;i < srvGrpInfoList.size();i++)
            {
                if(service_group == srvGrpInfoList[i].serviceGrp)
                {
                    hasGrp = true;
                    key = srvGrpInfoList[i].key;
                }
            }
            
            if(!hasGrp)
            {
                errMsg = "no such " + service_group;
                ret = API_INTERNAL_ERROR;
                APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
                break; 
            }
            
            if(STOREPROXY->updateServiceGroup(service_group,key,user) == 0)
            {
                ret = API_SUCC;
            }
            else
            {
                errMsg = "update record failure";
                ret = API_INTERNAL_ERROR;
                APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
                break;
            }
        }
        else
        {
            errMsg = "check record failure";
            ret = API_INTERNAL_ERROR;
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
            break;  
        }
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret;                
}

int ApiRegImp::getIdcGroup(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
    
        APIIMP_LOG<<FILE_FUN<< "coming"<< endl;
        
        vector<IDCGroupInfo> idcGroupInfoList;
        int queryRet = STOREPROXY->getAllIdcGroupRule(idcGroupInfoList);
        if (queryRet)
        {
            errMsg = "load data failure";
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
            break;
        }
        
        map<string,string> tmpInfo;
        for (size_t i = 0; i < idcGroupInfoList.size(); ++i)
        {
            tmpInfo.insert(make_pair(idcGroupInfoList[i].groupName, idcGroupInfoList[i].groupId));
        }
        
        if(tmpInfo.size() >= 0)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value resultData(rapidjson::kArrayType);
            for(map<string,string>::iterator it = tmpInfo.begin();it != tmpInfo.end();it++)
            {
                rapidjson::Value jServiceGroup(rapidjson::kObjectType);
                rapidjson::Value groupNameJson(it->first.c_str(), allocator);
                rapidjson::Value groupNameCnJson(it->second.c_str(), allocator);
                jServiceGroup.AddMember("group_name", groupNameJson, allocator);
                jServiceGroup.AddMember("group_name_cn", groupNameCnJson, allocator);

                resultData.PushBack(jServiceGroup, allocator);
            }

            ret = API_SUCC;

            document.AddMember("data", resultData, allocator);
            doResponse(current, int(ret), document, errMsg);
        }
        else
        {
            errMsg = "load data failure";
            ret = API_INTERNAL_ERROR;
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
            break;
        }

        return ret;
        __CATCHEXT__
    }while(0);
    

    doResponse(current,int(ret),errMsg);

    return ret;            
}


int ApiRegImp::addServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
            
        string user;
        EXIST_GETVALUE(user);

        string service_group;
        CHECK_SETVALUE(service_group);
        
        vector<string> userList = TC_Common::sepstr<string>(user,";");
        string userListStr;
        if(!userList.empty())
        {
            userListStr = TC_Common::tostr(userList.begin(),userList.end(),",");   
        }
        APIIMP_LOG<<FILE_FUN<< service_group <<"|"<<userListStr<< endl;
        
        string trueKey;
        //判断key是否已经存在
        if(STOREPROXY->getServiceGroupKey(service_group,trueKey) == 0)
        {
            errMsg = service_group + " alread exist!";
            ret = API_SERVICEGRP_EXIST;
            break;
        }
        else
        {
            string randKey;
            int retry = 0;
            do{
                if(getRandomKey(service_group,randKey))
                {
                    break;
                }
            }while(retry < 3);

            if(randKey.empty())
            {
                APIIMP_LOGERROR<<FILE_FUN<< " getRandomKey failure" <<endl;
                errMsg = service_group + " getRandomKey failure";
                ret = API_INTERNAL_ERROR;
                break;
            }
            
            APIIMP_LOG<<FILE_FUN<< service_group <<"|"<<userListStr<<"|key="<<randKey<< endl;
            
            if(STOREPROXY->addServiceGroup(service_group,randKey,userListStr) != 0)
            {
                errMsg = service_group + " record failure";
                APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
                ret = API_INTERNAL_ERROR;
                break;
            }

            rapidjson::Document document;
            document.SetObject();
            rapidjson::Value::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value jServiceGroup(rapidjson::kObjectType);
            rapidjson::Value serviceGroupJson(service_group.c_str(), allocator);
            rapidjson::Value keyJson(randKey.c_str(), allocator);
            jServiceGroup.AddMember("service_group", serviceGroupJson, allocator);
            jServiceGroup.AddMember("key", keyJson, allocator);
            
            rapidjson::Value resultData(rapidjson::kArrayType);
            resultData.PushBack(jServiceGroup, allocator);
            document.AddMember("data", resultData, allocator);

            ret = API_SUCC;
            doResponse(current, int(ret), document, errMsg);
        }

        return ret;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret;    
}

int ApiRegImp::deleteAgent(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string iplist;
        CHECK_SETVALUE(iplist);

        vector<string> ipAndPortList = TC_Common::sepstr<string>(iplist,";");
        
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Value::AllocatorType &allocator = document.GetAllocator();
        
        rapidjson::Value resultData(rapidjson::kArrayType);

        for(size_t i=0;i < ipAndPortList.size();i++)
        {
            string ip = ipAndPortList[i];
            try
            {
                UpdatePrx  prx = getAgentPrx(ip);
                prx->uninstall(errMsg);
                APIIMP_LOG<<FILE_FUN<<ip <<" uninstall succ,del db="<<ret<<endl;
            }catch(exception& ex)
            {
                rapidjson::Value errValue(rapidjson::kObjectType);
                rapidjson::Value ipJson(ip.c_str(), allocator);
                rapidjson::Value errMsgJson(ex.what(), allocator);
                errValue.AddMember("ip", ipJson, allocator);
                errValue.AddMember("err_msg", errMsgJson, allocator);
                APIIMP_LOGERROR<<FILE_FUN<< ip <<"|"<<string(ex.what())<<endl;
                resultData.PushBack(errValue, allocator);
            }
            
            int ret = STOREPROXY->destroyAgent(ip);
            APIIMP_LOG<<FILE_FUN<<ip <<"dele succ,del db="<<ret<<endl;
        }
        ret = API_SUCC;

        document.AddMember("data", resultData, allocator);
        doResponse(current, int(ret), document, "");
        return ret;

        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret;    
}

int ApiRegImp::updateAgentLocator(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
          __TRY__
          string iplist;
          CHECK_SETVALUE(iplist);
          
          string locator;
          EXIST_GETVALUE(locator);

          string tmpLocatorList;
          if(locator != "")
          {
              vector<string> locatorPortlist = TC_Common::sepstr<string>(locator,"|;");
              for (size_t i = 0; i < locatorPortlist.size(); i++)
              {
                  vector<string> ipAndPortList = TC_Common::sepstr<string>(locatorPortlist[i],":");
                  if(ipAndPortList.size() < 2)
                  {
                      APIIMP_LOGERROR<<FILE_FUN<< locatorPortlist[i]<<" is invalid"<<endl;
                      continue;
                  }
                    
                  string endPoint= "tcp -h " + ipAndPortList[0] + " -p " + ipAndPortList[1];
                  if (tmpLocatorList != "")
                  {
                      tmpLocatorList += ":" + endPoint;
                  }
                  else
                  {
                      tmpLocatorList = endPoint;
                  }
              }

              if(tmpLocatorList.empty())
              {
                  APIIMP_LOGERROR<<FILE_FUN<< "locator param is invalid" << endl;
                  errMsg = "locator param is invalid";
                  break;
              }
          }
          else
          {
              //没有传地址过来，自己取最新的活跃节点
              //获取最新的主控ip列表
              vector<EndpointInfo> activeEndPoint;
              vector<EndpointInfo> inactiveEndPoint;
              Tseer::QueryFPrx registryPrx = Application::getCommunicator()->stringToProxy<Tseer::QueryFPrx>(TSEERSERVER_QUERYOBJ);
              registryPrx->tars_endpoints(activeEndPoint, inactiveEndPoint);
              if (activeEndPoint.size() == 0)
              {
                  ostringstream os;
                  os << "|Get active registry obj fail, size | " << activeEndPoint.size();
                  APIIMP_LOGERROR<<FILE_FUN<< os.str() << endl;
                  errMsg = os.str();
                  break;
              } 
               
              for (size_t i = 0; i < activeEndPoint.size(); i++)
              {
                  string sEndPoint= "tcp -h " + activeEndPoint[i].host() + " -p " + TC_Common::tostr<int>(activeEndPoint[i].port());
                  if (tmpLocatorList != "")
                  {
                      tmpLocatorList += ":" + sEndPoint;
                  }
                  else
                  {
                      tmpLocatorList = sEndPoint;
                  }
              }
          }

          string registryLocator = TSEERSERVER_QUERYOBJ + "@"+tmpLocatorList;
        
          APIIMP_LOG<<FILE_FUN<<registryLocator<<endl;

          vector<string> ipAndPortList = TC_Common::sepstr<string>(iplist,";");

          APIIMP_LOG<<FILE_FUN<<iplist<<endl;
        
          set<string> setFilter;
          setFilter.insert(ipAndPortList.begin(),ipAndPortList.end());
          NotifyUpdateEventPtr notifyEventPtr = new NotifyUpdateEvent();
          notifyEventPtr->_run_times = (int)setFilter.size();
          int timeout = 10000;
          int allTimeout = notifyEventPtr->_run_times* timeout;
          AgentConfig  agentConf;
          agentConf.locator = registryLocator;
          for(set<string>::iterator it = setFilter.begin();it != setFilter.end();it++)
          {
              agentConf.nodeIp = *it;
              try
              {
                  UpdatePrx  prx = getAgentPrx(agentConf.nodeIp);  
                  if(prx)
                  {
                      UpdatePrxCallbackPtr callback = new UpdateAgentCallbackImp(notifyEventPtr, agentConf.nodeIp);
                      prx->async_updateConfig(callback,agentConf);
                  }
                  else
                  {
                      APIIMP_LOGERROR<<FILE_FUN<<agentConf.nodeIp<<"|get proxy failure" << endl;
                      notifyEventPtr->addFail(agentConf.nodeIp,string("get proxy failure"));
                      notifyEventPtr->_atomic.inc();
                  }
              }
              catch(exception&ex)
              {
                  APIIMP_LOGERROR<<FILE_FUN<<agentConf.nodeIp<<"|exception:"<<ex.what() << endl;
                  notifyEventPtr->addFail(agentConf.nodeIp,string(ex.what()));
                  notifyEventPtr->_atomic.inc();
              }
          }

          if (notifyEventPtr->_run_times > 0 &&
              notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
          {
              APIIMP_LOGERROR<<FILE_FUN<<"all exception" << endl;
          }
          else
          {
              /* 等待结果 */
              TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
              notifyEventPtr->_monitor.timedWait(allTimeout);
          }

          rapidjson::Document document;
          document.SetObject();
          rapidjson::Document::AllocatorType &allocator = document.GetAllocator(); 
          rapidjson::Value resultData(rapidjson::kArrayType);
          if(setFilter.size() <= (notifyEventPtr->_succNum.size() + notifyEventPtr->_failureNum.size()))
          {
              if(notifyEventPtr->_failureNum.size() > 0)
              {
                  for(map<string,string>::iterator it = notifyEventPtr->_failureNum.begin();it != notifyEventPtr->_failureNum.end();it++)
                  {
                      rapidjson::Value errValue(rapidjson::kObjectType);
                      rapidjson::Value ipJson(it->first.c_str(), allocator);
                      rapidjson::Value errMsgJson(it->second.c_str(), allocator);
                      errValue.AddMember("ip", ipJson, allocator);
                      errValue.AddMember("err_msg", errMsgJson, allocator);
                      resultData.PushBack(errValue, allocator);
                  }
              }

              if(notifyEventPtr->_succNum.size() > 0)
              {
                  for(map<string,string>::iterator it = notifyEventPtr->_succNum.begin();it != notifyEventPtr->_succNum.end();it++)
                  {
                      rapidjson::Value errValue(rapidjson::kObjectType);
                      rapidjson::Value ipJson(it->first.c_str(), allocator);
                      rapidjson::Value errMsgJson(it->second.c_str(), allocator);
                      errValue.AddMember("ip", ipJson, allocator);
                      errValue.AddMember("err_msg", errMsgJson, allocator);
                      resultData.PushBack(errValue, allocator);
                  }
              }
            
              ret = API_SUCC;
              if(notifyEventPtr->_failureNum.size() == setFilter.size())
              {
                  ret = API_INTERNAL_ERROR;
              }

              if(notifyEventPtr->_succNum.size() > 0 &&
                 notifyEventPtr->_failureNum.size() > 0)
              {
                  ret = API_PARTLY_SUCC;
              }

              APIIMP_LOG<<FILE_FUN<< current->getIp() << " end|"<< iplist<<"|ret="<<ret<< endl;
          }
          else
          {
              ret = API_INTERNAL_ERROR;
              errMsg = "req succ,return invalid response num";
              APIIMP_LOGERROR<<FILE_FUN<< current->getIp() << " end|"<< iplist<<"|ret="<<ret<<"|"<<errMsg<< endl;
          }

          document.AddMember("data", resultData, allocator);
          doResponse(current, int(ret), document, "");
          return ret;
          __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);
    return ret;    
}

int ApiRegImp::addIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
            
        string group_name;
        CHECK_SETVALUE(group_name);
        group_name = TC_Common::trim(group_name);
            
        rapidjson::Value::MemberIterator allowIpsJson = jData.FindMember("allow_ip");
        if (allowIpsJson == jData.MemberEnd() || !allowIpsJson->value.IsArray())
        {
            errMsg = "not found allow_ip or allow_ip is not array format, invalid param";
            ret = API_INVALID_PARAM;
            break;
        }

        vector<string> allowIps;
        for (rapidjson::SizeType i = 0; i < allowIpsJson->value.Size(); ++i)
        {
            allowIps.push_back(allowIpsJson->value[i].GetString());
        }

        int asyncRet = STOREPROXY->addIdcGroupRule(group_name, allowIps, current);
        if (asyncRet == -2)
        {
            errMsg = "invalid dulplicated group name";
            ret = API_INVALID_PARAM;
            break;
        }
        else if (asyncRet == -1)
        {
            errMsg = "async call etcd failed";
            ret =API_INTERNAL_ERROR;
        }
        
        APIIMP_LOG<<FILE_FUN<< group_name<<"|ret="<<ret<< endl;
        return API_PARTLY_SUCC;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret;    
}

int ApiRegImp::modifyIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string group_id;
        CHECK_SETVALUE(group_id);
        
        rapidjson::Value::MemberIterator allowIpsJson = jData.FindMember("allow_ip");
        if (allowIpsJson == jData.MemberEnd() || !allowIpsJson->value.IsArray())
        {
            errMsg = "not found allow_ip or allow_ip is not array format, invalid param";
            ret = API_INVALID_PARAM;
        }
        
        vector<string> allowIps;
        for (rapidjson::SizeType i = 0; i < allowIpsJson->value.Size(); ++i)
        {
            allowIps.push_back(allowIpsJson->value[i].GetString());
        }

        int asyncRet = STOREPROXY->modifyIdcGroupRule(group_id, allowIps, current);
        if (asyncRet == -2)
        {
            errMsg = "invalid params, check group id correctness";
            ret = API_INVALID_PARAM;
            break;
        }
        else if (asyncRet != 0)
        {
            errMsg = "API_INTERNAL_ERROR";
            ret = API_INTERNAL_ERROR;
            break;
        }
        return API_PARTLY_SUCC;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret;    
}

int ApiRegImp::delIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
    __TRY__
        //参数检查，获取参数
        rapidjson::Value::MemberIterator groupIdsJson = jData.FindMember("group_ids");
        if (groupIdsJson == jData.MemberEnd() || !groupIdsJson->value.IsArray())
        {
            errMsg = "not found group_ids or group_ids is not array format, invalid param";
            ret = API_INVALID_PARAM;
            break;
        }

        if(groupIdsJson->value.Size()==0)
        {
            errMsg = "should provide group_ids param";
            ret = API_INVALID_PARAM;
            break; 
        }
        vector<string> groupIds;
        for (rapidjson::SizeType i = 0; i < groupIdsJson->value.Size(); ++i)
        {
            groupIds.push_back(groupIdsJson->value[i].GetString());
        }

        int asyncRet = STOREPROXY->delIdcGroupRule(groupIds, current);
        if (asyncRet)
        {
            errMsg = "async call etcd failed";
            ret = API_INTERNAL_ERROR;
            break;
        }
        return API_PARTLY_SUCC;
    __CATCHEXT__
    }while(0);
    doResponse(current,int(ret),errMsg);
    return ret;
}

int ApiRegImp::getAllIdcGroupRule(TarsCurrentPtr current, rapidjson::Document& )
{
    vector<IDCGroupInfo> groupInfos;
    int asyncRet = STOREPROXY->getAllIdcGroupRule(groupInfos);
    if (asyncRet)
    {
        string errMsg = "get idc rule  failed";
        doResponse(current, asyncRet, errMsg);
    }
    else
    {
        doGetIdcGroupResponse(current, API_SUCC, groupInfos);
    }

    return 0;
}

int ApiRegImp::addIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string order = "-1";
        CHECK_SETVALUE(order);
        int iOrder = TC_Common::strto<int>(order);
        if(iOrder < 0)
        {
            errMsg = "should provide order >0";
            ret = API_INVALID_PARAM;
            break;
        }
        
        string priority_name;
        CHECK_SETVALUE(priority_name);

        rapidjson::Value::MemberIterator groupIdListJson = jData.FindMember("group_id_list");
        if (groupIdListJson == jData.MemberEnd() || !groupIdListJson->value.IsArray())
        {
            errMsg = "not found group_id_list or group_id_list is not array format, invalid param";
            APIIMP_LOGERROR << FILE_FUN << errMsg << "|ret=" << API_INVALID_PARAM << endl;
            doResponse(current, int(API_INVALID_PARAM), errMsg);
            return API_INVALID_PARAM;
        }

        vector<string> groupIdList;
        for (rapidjson::SizeType i = 0; i < groupIdListJson->value.Size(); ++i)
        {
            groupIdList.push_back(groupIdListJson->value[i].GetString());
        }

        int asyncRet = STOREPROXY->addIdcPriority(iOrder, priority_name, groupIdList, current);
        if (asyncRet)
        {
            errMsg = "addIdcPriority data  failed";
            ret = API_INTERNAL_ERROR;
        }
        APIIMP_LOG<<FILE_FUN<< "ret="<<ret<< endl;
        return API_PARTLY_SUCC;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret; 
}

int ApiRegImp::modifyIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string order = "-1";
        CHECK_SETVALUE(order);
        int iOrder = TC_Common::strto<int>(order);
        if(iOrder < 0)
        {
            errMsg = "should provide order >0";
            ret = API_INVALID_PARAM;
            break;
        }
        
        string priority_name;
        CHECK_SETVALUE(priority_name);
        
        string priority_id;
        CHECK_SETVALUE(priority_id);

        rapidjson::Value::MemberIterator groupIdListJson = jData.FindMember("group_id_list");
        if (groupIdListJson == jData.MemberEnd() || !groupIdListJson->value.IsArray())
        {
            errMsg = "not found group_id_list or group_id_list is not array format, invalid param";
            APIIMP_LOGERROR << FILE_FUN << errMsg << "|ret=" << API_INVALID_PARAM << endl;
            doResponse(current, int(API_INVALID_PARAM), errMsg);
            return API_INVALID_PARAM;
        }

        vector<string> groupIdList;
        for (rapidjson::SizeType i = 0; i < groupIdListJson->value.Size(); ++i)
        {
            groupIdList.push_back(groupIdListJson->value[i].GetString());
        }

        int asyncRet = STOREPROXY->modifyIdcPriority(priority_id, iOrder, priority_name, groupIdList, current);
        if (asyncRet)
        {
            errMsg = "moidify data  failed";
            ret = API_INTERNAL_ERROR;
            break;
        }
        APIIMP_LOG<<FILE_FUN<< "ret="<<ret<< endl;
        return API_PARTLY_SUCC;
        __CATCHEXT__
    }while(0);
    
    doResponse(current,int(ret),errMsg);

    return ret; 
}

int ApiRegImp::delIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;

    //参数检查、获取参数
    rapidjson::Value::MemberIterator priorityIdsJson = jData.FindMember("priority_ids");
    if (priorityIdsJson == jData.MemberEnd() || !priorityIdsJson->value.IsArray())
    { 
        errMsg = "not found priority_ids or priority_ids is not array format";
        APIIMP_LOGERROR << FILE_FUN << errMsg << "|ret=" << API_INVALID_PARAM << endl;
        doResponse(current, int(API_INVALID_PARAM), errMsg);
        return API_INVALID_PARAM;
    }
    
    vector<string> priorityIds;
    for (rapidjson::SizeType i = 0; i < priorityIdsJson->value.Size(); ++i)
    {
        string id(priorityIdsJson->value[i].GetString());
        if(id!="")
        {
            priorityIds.push_back(priorityIdsJson->value[i].GetString());
        }
    }

    if(priorityIds.empty())
    {
        errMsg = "should provide priority_ids param";
        doResponse(current, int(API_INVALID_PARAM), errMsg);
        return API_INVALID_PARAM;
    }
    
    int asyncRet = STOREPROXY->delIdcPriority(priorityIds, current);
    if (asyncRet)
    {
        errMsg = "del data  failed";
        doResponse(current, API_INTERNAL_ERROR, errMsg);
    }

    return 0;
}

int ApiRegImp::getAllIdcPriority(TarsCurrentPtr current, rapidjson::Document& )
{
    vector<IDCPriority> idcPriority;
    int asyncRet = STOREPROXY->getAllIdcPriority(idcPriority);
    if (asyncRet)
    {
        string errMsg = "get data  failed";
        doResponse(current, asyncRet, errMsg);
    }
    else
    {
        doGetIdcPriorityResponse(current, API_SUCC, idcPriority);
    }

    return 0;
}

int ApiRegImp::getAgentBaseInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__

        string ip;
        string ostype;
        string present_state;
        if(!jData.IsNull())
        {
            EXIST_GETVALUE(ip);
            EXIST_GETVALUE(ostype);
            EXIST_GETVALUE(present_state);
        }
        AgentBaseInfoReq req;
        req.ip = ip;
        req.ostype = ostype;
        req.present_state = present_state;

        vector<AgentBaseInfo> agentBaseInfos;
        int ret = STOREPROXY->get_agent_baseinfo(req,agentBaseInfos);
        if (ret == 0)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value resultData(rapidjson::kArrayType);

            for (size_t i = 0; i < agentBaseInfos.size(); ++i)
            {
                rapidjson::Value ip(agentBaseInfos[i].ip.c_str(), allocator);
                rapidjson::Value ostype(agentBaseInfos[i].ostype.c_str(), allocator);
                rapidjson::Value present_state(agentBaseInfos[i].present_state.c_str(), allocator);
                rapidjson::Value version(agentBaseInfos[i].version.c_str(), allocator);
                rapidjson::Value locator(agentBaseInfos[i].locator.c_str(), allocator);
                rapidjson::Value gray_version(agentBaseInfos[i].gray_version.c_str(), allocator);
                rapidjson::Value formal_version(agentBaseInfos[i].formal_version.c_str(), allocator);
                rapidjson::Value grayscale_state(agentBaseInfos[i].grayscale_state.c_str(), allocator);
                rapidjson::Value last_reg_time(agentBaseInfos[i].last_reg_time.c_str(), allocator);
                rapidjson::Value last_heartbeat_time(agentBaseInfos[i].last_heartbeat_time.c_str(), allocator);
                
                rapidjson::Value serviceJson(rapidjson::kObjectType);
                serviceJson.AddMember("ip", ip, allocator);
                serviceJson.AddMember("present_state", present_state, allocator);
                serviceJson.AddMember("ostype", ostype, allocator);
                serviceJson.AddMember("version", version, allocator);
                serviceJson.AddMember("locator", locator, allocator);
                serviceJson.AddMember("gray_version", gray_version, allocator);
                serviceJson.AddMember("formal_version", formal_version, allocator);
                serviceJson.AddMember("grayscale_state", grayscale_state, allocator);
                serviceJson.AddMember("last_reg_time", last_reg_time, allocator);
                serviceJson.AddMember("last_heartbeat_time", last_heartbeat_time, allocator);
                resultData.PushBack(serviceJson, allocator);
            }

            document.AddMember("data", resultData, allocator);
            doResponse(current, API_SUCC, document, "");

            ret = API_SUCC;
        }
        else
        {
            errMsg = "query get_agent_baseinfo failed, internal error";
            APIIMP_LOGERROR << FILE_FUN << errMsg << endl;
            ret =  API_INTERNAL_ERROR;
            break;
        }
        return ret;
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;      
}

int ApiRegImp::updateAgentGrayState(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg;
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__

        string iplist;
        string ostype;
        string grayscale_state;

        CHECK_SETVALUE(iplist);
        CHECK_SETVALUE(ostype);
        CHECK_SETVALUE(grayscale_state);

        vector<string> ipAndPortList = TC_Common::sepstr<string>(iplist,";|");
        if(ipAndPortList.empty())
        {
            errMsg = "iplist is invalid format";
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
            break;
        }

        map<string,string> succIplist;
        map<string,string> failIplist;
        for(size_t i = 0;i < ipAndPortList.size();i++)
        {
            UpdateAgentInfoReq req;
            req.ip = ipAndPortList[i];
            req.ostype = ostype;
            req.grayscale_state = grayscale_state;

            __TRY__
                int ret = STOREPROXY->update_agent_graystate(req);
                if (ret == 0)
                {
                    succIplist.insert(make_pair(req.ip,""));
                    ret = API_SUCC;
                }
                else
                {
                    errMsg = "update_agent_graystate  failure";
                    APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
                    failIplist.insert(make_pair(req.ip,errMsg));
                }
            __CATCH__
        }
        
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
        rapidjson::Value resultData(rapidjson::kArrayType);
        if(failIplist.size() > 0)
        {
            for(map<string,string>::iterator it = failIplist.begin();it != failIplist.end();it++)
            {
                rapidjson::Value errValue(rapidjson::kObjectType);
                rapidjson::Value ipJson(it->first.c_str(), allocator);
                rapidjson::Value errMsgJson(it->second.c_str(), allocator);
                errValue.AddMember("ip", ipJson, allocator);
                errValue.AddMember("err_msg", errMsgJson, allocator);
                resultData.PushBack(errValue, allocator);
            }
        }

        ret = API_SUCC;
        if(failIplist.size() == ipAndPortList.size())
        {
            ret = API_INTERNAL_ERROR;
        }

        if(succIplist.size() > 0 &&
            failIplist.size() > 0)
        {
            ret = API_PARTLY_SUCC;
        }
        APIIMP_LOG<<FILE_FUN<< current->getIp() << " end|"<< iplist<<"|ret="<<ret<< endl;
        document.AddMember("data", resultData, allocator);
        doResponse(current, int(ret), document, "");
        return ret;
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;       
}

int ApiRegImp::addAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        
        string ostype;
        CHECK_SETVALUE(ostype);

        string version;
        CHECK_SETVALUE(version);

        string package_type;
        CHECK_SETVALUE(package_type);

        string package_name;
        CHECK_SETVALUE(package_name);
        
        string md5;
        CHECK_SETVALUE(md5);

        string upload_user;
        EXIST_GETVALUE(upload_user);
       
        string upload_time = TC_Common::tm2str(TNOW, "%Y-%m-%d %H:%M:%S");
        
        AgentPackageInfo info;
        info.ostype = ostype;
        info.md5 = md5;
        info.package_name = package_name;
        info.package_type = package_type;
        info.version = version;
        info.uploadUser = upload_user;
        info.uploadTime = upload_time;
        
        ret = STOREPROXY->add_agent_packageinfo(info);
        if(ret == 0)
        {
            ret = API_SUCC;
        }
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;          
}

int ApiRegImp::getAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        string ostype = "";
        if(!jData.IsNull())
        {
            EXIST_GETVALUE(ostype);
        }
        AgentPackageReq req;
        req.ostype = ostype;
        
        vector<AgentPackageInfo> agentPackageInfos;
        int ret = STOREPROXY->get_agent_packageinfo(req,agentPackageInfos);
        if (ret == 0)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value resultData(rapidjson::kArrayType);

            for (size_t i = 0; i < agentPackageInfos.size(); ++i)
            {
                rapidjson::Value ostype(agentPackageInfos[i].ostype.c_str(), allocator);
                rapidjson::Value version(agentPackageInfos[i].version.c_str(), allocator);
                rapidjson::Value package_type(agentPackageInfos[i].package_type.c_str(), allocator);
                rapidjson::Value package_name(agentPackageInfos[i].package_name.c_str(), allocator);
                
                rapidjson::Value serviceJson(rapidjson::kObjectType);
                serviceJson.AddMember("ostype", ostype, allocator);
                serviceJson.AddMember("version", version, allocator);
                serviceJson.AddMember("package_type", package_type, allocator);
                serviceJson.AddMember("package_name", package_name, allocator);
                resultData.PushBack(serviceJson, allocator);
            }

            document.AddMember("data", resultData, allocator);
            doResponse(current, API_SUCC, document, "");

            ret = API_SUCC;
        }
        else
        {
            errMsg = "query agent_packageinfo failed, internal error";
            APIIMP_LOGERROR << FILE_FUN << errMsg << endl;
            ret =  API_INTERNAL_ERROR;
            break;
        }
        return ret;
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;         
}

int ApiRegImp::updateAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__

        string package_name;
        CHECK_SETVALUE(package_name);

        string ostype;
        CHECK_SETVALUE(ostype);

        string package_type;
        CHECK_SETVALUE(package_type);
        
        UpdatePackageInfoReq req;
        req.package_name = package_name;
        req.ostype = ostype;
        req.grayscale_state = package_type;
        
        ret = STOREPROXY->update_agent_packageinfo(req);
        if (ret == 0)
        {
            ret = API_SUCC;
        }
        else
        {
            errMsg = "update_agent_packageinfo  failure";
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
        }  
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;       
}

int ApiRegImp::deleteAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__

        string package_name;
        CHECK_SETVALUE(package_name);

        string ostype;
        CHECK_SETVALUE(ostype);

        AgentPackageReq req;
        req.package_name = package_name;
        req.ostype = ostype;
        
        ret = STOREPROXY->delete_agent_package(req);
        if (ret == 0)
        {
            ret = API_SUCC;
        }
        else
        {
            errMsg = "delete_agent_package  failure";
            APIIMP_LOGERROR<<FILE_FUN<< errMsg<<endl;
        }  
        __CATCHEXT__
    }while(0);

    doResponse(current,int(ret),errMsg);
    
    return ret;       
}

int ApiRegImp::getAgentOstypeInfo(TarsCurrentPtr current, rapidjson::Document &jData)
{
    string errMsg = "";
    int ret = API_INTERNAL_ERROR;
    do{
        __TRY__
        vector<string> ostypeList;
        ostypeList.push_back("CentOs-6.2-64");
        ostypeList.push_back("CentOs-6.5-64");
        ostypeList.push_back("CentOs-7.2-64");
        
        //TODO 加载所有支持的os版本
        if(1)
        {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

            rapidjson::Value resultData(rapidjson::kArrayType);

            for (size_t i = 0; i < ostypeList.size(); i++)
            {
                    rapidjson::Value jServiceGroup(rapidjson::kObjectType);
                    rapidjson::Value ostype(ostypeList[i].c_str(), allocator);
                    jServiceGroup.AddMember("ostype", ostype, allocator);
                    resultData.PushBack(jServiceGroup, allocator);
            }

            ret = API_SUCC;
            document.AddMember("data", resultData, allocator);
            doResponse(current, int(ret), document, errMsg);
        }
        else
        {
            errMsg = "load data failure";
            ret = API_INTERNAL_ERROR;
            APIIMP_LOGERROR << FILE_FUN << errMsg << endl;
            break;
        }

        return ret;
        __CATCHEXT__
    }while(0);
    

    doResponse(current,int(ret),errMsg);

    return ret;            
}

void ApiRegImp::doGetRouteResponse(TarsCurrentPtr & current, int ret, const vector<Tseer::RouterData>& routerDataList, const string& errMsg)
{
    __TRY__

    //key=BUILDDOT3KEY(service_group,app_name,servername),value=portlist
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

    map<string, vector<rapidjson::Value *> > tmpModulePortList;
    map<string, rapidjson::Value *> tmpModuleList;
    for(size_t i =0;i<routerDataList.size();i++)
    {
        rapidjson::Value *module = new rapidjson::Value(rapidjson::kObjectType);
        
        string key = BUILDDOT3KEY(routerDataList[i].moduletype,routerDataList[i].application,routerDataList[i].service_name);
        rapidjson::Value serviceGroupJson(routerDataList[i].moduletype.c_str(), allocator);
        rapidjson::Value appNameJson(routerDataList[i].application.c_str(), allocator);
        rapidjson::Value serverNameJson(routerDataList[i].service_name.c_str(), allocator);
        
        module->AddMember("service_group", serviceGroupJson, allocator);
        module->AddMember("app_name", appNameJson, allocator);
        module->AddMember("servername", serverNameJson, allocator);
        
        rapidjson::Value portList(rapidjson::kArrayType);
        module->AddMember("port_list", portList, allocator);
        tmpModuleList[key] = module;

        
        //具体obj和andpoint
        map<string,string>::const_iterator itrServant = routerDataList[i].servant.begin();
        for(;itrServant != routerDataList[i].servant.end();++itrServant)
        {
            rapidjson::Value *port = new rapidjson::Value(rapidjson::kObjectType);
            const string& servant = itrServant->first;
            
            rapidjson::Value portNameJson(servant.c_str(), allocator);
            port->AddMember("port_name", portNameJson, allocator);

            try
            {
                TC_Endpoint ep;
                ep.parse(itrServant->second);
                rapidjson::Value ipJson(ep.getHost().c_str(), allocator);
                string protocolStr = ep.isTcp() ? "tcp" : "udp";
                rapidjson::Value protocolJson(protocolStr.c_str(), allocator);
                rapidjson::Value portJson(TC_Common::tostr(ep.getPort()).c_str(), allocator);
                port->AddMember("ip", ipJson, allocator);
                port->AddMember("protocol", protocolJson, allocator);
                port->AddMember("port", portJson, allocator);

            } catch(...)
            {
                port->AddMember("protocol", "tcp", allocator);
            }

            rapidjson::Value enableIdcGroupJson(routerDataList[i].enable_group.c_str(), allocator);
            rapidjson::Value ipGroupNameJson(routerDataList[i].ip_group_name.c_str(), allocator);
            rapidjson::Value enableSetJson(routerDataList[i].enable_set.c_str(), allocator);
            rapidjson::Value setNameJson(routerDataList[i].set_name.c_str(), allocator);
            rapidjson::Value setAreaJson(routerDataList[i].set_area.c_str(), allocator);
            rapidjson::Value setGroupJson(routerDataList[i].set_group.c_str(), allocator);
            rapidjson::Value presentStateJson(routerDataList[i].present_state.c_str(), allocator);
            rapidjson::Value gridFlagJson(routerDataList[i].grid_flag.c_str(), allocator);
            rapidjson::Value weightJson(routerDataList[i].weight.c_str(), allocator);
            rapidjson::Value enableHeartbeatJson(routerDataList[i].enable_heartbeat.c_str(), allocator);

            port->AddMember("enable_idc_group", enableIdcGroupJson, allocator);
            port->AddMember("ip_group_name", ipGroupNameJson, allocator);
            port->AddMember("enable_set", enableSetJson, allocator);
            port->AddMember("set_name", setNameJson, allocator);
            port->AddMember("set_area", setAreaJson, allocator);
            port->AddMember("set_group", setGroupJson, allocator);
            port->AddMember("present_state", presentStateJson, allocator);
            port->AddMember("grid_flag", gridFlagJson, allocator);
            port->AddMember("weight", weightJson, allocator);
            port->AddMember("enable_heartbeat", enableHeartbeatJson, allocator);

            tmpModulePortList[key].push_back(port);
        }       
    }
    
    for (map<string, vector<rapidjson::Value *> >::iterator it = tmpModulePortList.begin();
        it != tmpModulePortList.end(); it++)
    {
        vector<rapidjson::Value *>& rPortList = it->second;
        for (size_t i = 0; i < rPortList.size(); i++)
        {
            rapidjson::Value *module = tmpModuleList[it->first];
            rapidjson::Value &portList = (*module)["port_list"];
            portList.PushBack(*(rPortList[i]), allocator);
        }
    }

    rapidjson::Value resultData(rapidjson::kArrayType);
    for (map<string, rapidjson::Value *>::iterator it = tmpModuleList.begin();
        it != tmpModuleList.end(); it++)
    {
        resultData.PushBack(*(it->second), allocator);
    }
    
    TC_HttpResponse  response;

    string callChainStr = current->getBindAdapter()->getEndpoint().getHost() + ":" + TC_Common::tostr(current->getBindAdapter()->getEndpoint().getPort());
    rapidjson::Value callChainJson(callChainStr.c_str(), allocator);
    rapidjson::Value errMsgJson(errMsg.c_str(), allocator);
    document.AddMember("call_chain", callChainJson, allocator);
    document.AddMember("err_msg", errMsgJson, allocator);
    document.AddMember("ret_code", ret, allocator);
    document.AddMember("data", resultData, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);
    
    string responseContent = sb.GetString();

    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;
    APIIMP_LOG << FILE_FUN << method<<"|responseContent=" << responseContent << endl;

    //清理内存
    for (map<string, rapidjson::Value *>::iterator it = tmpModuleList.begin(); it != tmpModuleList.end(); ++it)
    {
        rapidjson::Value *v = it->second;
        delete v;
    }
    for (map<string, vector<rapidjson::Value *> >::iterator it = tmpModulePortList.begin(); it != tmpModulePortList.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); ++i)
        {
            rapidjson::Value *v = it->second[i];
            delete v;
        }
    }
    
    response.setResponse(responseContent.c_str(),responseContent.size());
    current->sendResponse(response.encode().c_str(),response.encode().size());
    __CATCH__   
}

void ApiRegImp::doGetIdcGroupResponse(TarsCurrentPtr current, int ret, const vector<IDCGroupInfo> &idcGroupInfoList)
{
    __TRY__

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    TC_Endpoint endpoint = current->getBindAdapter()->getEndpoint();
    string callChain = endpoint.getHost() + TC_Common::tostr(endpoint.getPort());

    rapidjson::Value callChainTmp(callChain.c_str(), allocator);
    document.AddMember("call_chain", callChainTmp, allocator);
    document.AddMember("err_msg", "", allocator);
    document.AddMember("ret_code", ret, allocator);

    rapidjson::Value data(rapidjson::kArrayType);
    for (size_t i = 0; i < idcGroupInfoList.size(); ++i)
    {
        rapidjson::Value groupInfo(rapidjson::kObjectType);
        rapidjson::Value groupId(idcGroupInfoList[i].groupId.c_str(), allocator);
        rapidjson::Value groupName(idcGroupInfoList[i].groupName.c_str(), allocator);
        groupInfo.AddMember("group_id", groupId, allocator);
        groupInfo.AddMember("group_name", groupName, allocator);

        rapidjson::Value allowIps(rapidjson::kArrayType);
        for (size_t j = 0; j < idcGroupInfoList[i].allowIp.size(); ++j)
        {
            rapidjson::Value tmp(idcGroupInfoList[i].allowIp[j].c_str(), allocator);
            allowIps.PushBack(tmp, allocator);
        }
        groupInfo.AddMember("allow_ip", allowIps, allocator);
        data.PushBack(groupInfo, allocator);
    }
    document.AddMember("data", data, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string responseContent = sb.GetString();
    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;
    APIIMP_LOG << FILE_FUN << method<<"|responseContent=" << responseContent << endl;

    TC_HttpResponse response;
    response.setResponse(responseContent.c_str(), responseContent.size());
    current->sendResponse(response.encode().c_str(), response.encode().size());

    __CATCH__
}

void ApiRegImp::doGetIdcPriorityResponse(TarsCurrentPtr current, int ret, const vector<IDCPriority> &idcPriority)
{
    __TRY__

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    TC_Endpoint endpoint = current->getBindAdapter()->getEndpoint();
    string callChain = endpoint.getHost() + TC_Common::tostr(endpoint.getPort());

    rapidjson::Value callChainTmp(callChain.c_str(), allocator);
    document.AddMember("call_chain", callChainTmp, allocator);
    document.AddMember("err_msg", "", allocator);
    document.AddMember("ret_code", ret, allocator);

    rapidjson::Value data(rapidjson::kArrayType);
    for (size_t i = 0; i < idcPriority.size(); ++i)
    {
        rapidjson::Value priorityInfo(rapidjson::kObjectType);
        rapidjson::Value priorityId(idcPriority[i].priorityId.c_str(), allocator);
        rapidjson::Value priorityName(idcPriority[i].priorityName.c_str(), allocator);
        priorityInfo.AddMember("priority_id", priorityId, allocator);
        priorityInfo.AddMember("priority_name", priorityName, allocator);
        priorityInfo.AddMember("order", idcPriority[i].order, allocator);

        rapidjson::Value groupList(rapidjson::kArrayType);
        for (size_t j = 0; j < idcPriority[i].groupList.size(); ++j)
        {
            rapidjson::Value tmp(idcPriority[i].groupList[j].c_str(), allocator);
            groupList.PushBack(tmp, allocator);
        }
        priorityInfo.AddMember("group_list", groupList, allocator);
        data.PushBack(priorityInfo, allocator);
    }
    document.AddMember("data", data, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string responseContent = sb.GetString();
    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;
    APIIMP_LOG << FILE_FUN << method<<"|responseContent=" << responseContent << endl;

    TC_HttpResponse response;
    response.setResponse(responseContent.c_str(), responseContent.size());
    current->sendResponse(response.encode().c_str(), response.encode().size());

    __CATCH__
}

void ApiRegImp::doResponse(TarsCurrentPtr current, int ret,const string& errMsg)
{
    __TRY__

    TC_HttpResponse  response;

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
    
#if 0
    string callChain = current->getBindAdapter()->getEndpoint().getHost() + ":" + TC_Common::tostr(current->getBindAdapter()->getEndpoint().getPort());
#else
    TC_Endpoint endpoint = current->getBindAdapter()->getEndpoint();
    string callChain = endpoint.getHost() + TC_Common::tostr(endpoint.getPort());
#endif
    rapidjson::Value callChainJson(callChain.c_str(), allocator);
    rapidjson::Value errMsgJson(errMsg.c_str(), allocator);
    document.AddMember("call_chain", callChainJson, allocator);
    document.AddMember("err_msg", errMsgJson, allocator);
    document.AddMember("ret_code", ret, allocator);

    rapidjson::Value resultData(rapidjson::kArrayType);
    document.AddMember("data", resultData, allocator);
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);
    
    string responseContent = sb.GetString();
    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;
    APIIMP_LOG<<FILE_FUN<<method<<"|responseContent=" << responseContent << endl;
    
    response.setResponse(responseContent.c_str(),responseContent.size());
    current->sendResponse(response.encode().c_str(),response.encode().size());
    __CATCH__
}

void ApiRegImp::doResponse(TarsCurrentPtr current, int ret, rapidjson::Document &document, const string& errMsg)
{
    __TRY__

    TC_HttpResponse  response;
    
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

    TC_Endpoint endpoint = current->getBindAdapter()->getEndpoint();
    string callChain = endpoint.getHost() + TC_Common::tostr(endpoint.getPort());
    rapidjson::Value callChainJson(callChain.c_str(), allocator);
    rapidjson::Value errMsgJson(errMsg.c_str(), allocator);
    
    document.AddMember("call_chain", callChainJson, allocator);
    document.AddMember("err_msg", errMsgJson, allocator);
    document.AddMember("ret_code", ret, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document.Accept(writer);
    string responseContent = sb.GetString();
    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;
    APIIMP_LOG << FILE_FUN << method<<"|responseContent=" << responseContent << endl;
    
    response.setResponse(responseContent.c_str(),responseContent.size());
    current->sendResponse(response.encode().c_str(),response.encode().size());
    __CATCH__
}

void ApiRegImp::doAddIdcResponse(TarsCurrentPtr current, int ret, const string &id)
{
    __TRY__

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    TC_Endpoint endpoint = current->getBindAdapter()->getEndpoint();
    string callChain = endpoint.getHost() + TC_Common::tostr(endpoint.getPort());

    rapidjson::Value callChainTmp(callChain.c_str(), allocator);
    document.AddMember("call_chain", callChainTmp, allocator);
    document.AddMember("err_msg", "", allocator);
    document.AddMember("ret_code", ret, allocator);

    rapidjson::Value data(rapidjson::kArrayType);
    rapidjson::Value result(rapidjson::kObjectType);
    rapidjson::Value idJson(id.c_str(), allocator);
    result.AddMember("id", idJson, allocator);
    data.PushBack(result, allocator);
    
    document.AddMember("data", data, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string responseContent = sb.GetString();

    string method;
    map<string, string>::const_iterator itMethod = current->getResponseContext().find("method");
    if (itMethod != current->getResponseContext().end())
        method = itMethod->second;

    APIIMP_LOG << FILE_FUN << method<<"|responseContent=" << responseContent << endl;
    TC_HttpResponse response;
    response.setResponse(responseContent.c_str(), responseContent.size());
    current->sendResponse(response.encode().c_str(), response.encode().size());

    __CATCH__
}

UpdatePrx ApiRegImp::getAgentPrx(const string & nodeName)
{
    NodeServiceInfo nodeServiceInfo;
    if (STOREPROXY->getOneSeerAgent(nodeName, nodeServiceInfo))
    {
        APIIMP_LOG << FILE_FUN<<nodeName << "|get alive seer agents failed" << endl;
        return NULL;
    }
    
    if (nodeServiceInfo.presentState == "inactive")
    {
        throw TarsException("agent '"+ nodeName + "' heartbeart timeout,please check for it");
    }

    Tseer::UpdatePrx agentPrx;
    g_app.getCommunicator()->stringToProxy(nodeServiceInfo.nodeObj, agentPrx);

    return agentPrx;
}

void NotifyUpdateEvent::addSuc(const string& ip)
{
    TC_ThreadLock::Lock lock(_metux);
    _succNum.insert(make_pair(ip,"succ"));
}

void NotifyUpdateEvent::addFail(const string& ip,const string& errMsg)
{
    TC_ThreadLock::Lock lock(_metux);
    _failureNum.insert(make_pair(ip,errMsg));
}

void UpdateAgentCallbackImp::callback_updateConfig(tars::Int32 ret,const std::string& result)
{
    APIIMP_LOG<<FILE_FUN << _ip << "|ret:"<<ret<<"|" <<result<< endl;
    if(ret==0)
    {
        _notify->addSuc(_ip);
    }
    else
    {
        _notify->addFail(_ip,result);
    }

    _notify->_atomic.inc();
    if(_notify->_atomic.get() == _notify->_run_times)
    {
        TC_ThreadLock::Lock lock(_notify->_monitor);
        _notify->_monitor.notify();
    }
}


void UpdateAgentCallbackImp::callback_updateConfig_exception(tars::Int32 ret)
{
    APIIMP_LOGERROR<<FILE_FUN << _ip << "|failure,ret=" << ret << endl;
    _notify->addFail(_ip,"exception");
    _notify->_atomic.inc();
    if(_notify->_atomic.get() == _notify->_run_times)
    {
        TC_ThreadLock::Lock lock(_notify->_monitor);
        _notify->_monitor.notify();
    }
}
