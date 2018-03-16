#include "util/tc_http.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "Registry.h"

#include "Util.h"
#include "ServerTest.h"

using namespace Tseer;
using namespace std;
using namespace tars;

extern string g_key;
extern string g_url;
extern string g_srvgrp;

extern Communicator g_com;

void addServers()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

    string tmpUrl = g_url;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

                
    rapidjson::Value port1(rapidjson::kObjectType);
    port1.AddMember("port_name", "obj1", allocator);
    port1.AddMember("ip", "3.2.3.4", allocator);
    port1.AddMember("port", "1234", allocator);
    port1.AddMember("protocol", "tcp", allocator);
    port1.AddMember("enable_heartbeat", "N", allocator);
    port1.AddMember("enable_set", "Y", allocator);
    port1.AddMember("set_name", "aa", allocator);
    port1.AddMember("set_area", "bb", allocator);
    port1.AddMember("set_group", "cc", allocator);

    rapidjson::Value port2(rapidjson::kObjectType);
    port2.AddMember("port_name", "obj1", allocator);
    port2.AddMember("ip", "3.2.3.5", allocator);
    port2.AddMember("port", "1235", allocator);
    port2.AddMember("protocol", "tcp", allocator);
    port2.AddMember("enable_heartbeat", "N", allocator);
    port2.AddMember("enable_set", "Y", allocator);
    port2.AddMember("set_name", "aa", allocator);
    port2.AddMember("set_area", "bb", allocator);
    port2.AddMember("set_group", "cc", allocator);
    
    rapidjson::Value port3(rapidjson::kObjectType);
    port3.AddMember("port_name", "obj2", allocator);
    port3.AddMember("ip", "3.2.3.5", allocator);
    port3.AddMember("port", "1235", allocator);
    port3.AddMember("protocol", "tcp", allocator);
    port3.AddMember("enable_heartbeat", "N", allocator);
    port3.AddMember("enable_idc_group", "Y", allocator);

    rapidjson::Value port4(rapidjson::kObjectType);
    port4.AddMember("port_name", "obj2", allocator);
    port4.AddMember("ip", "3.2.3.6", allocator);
    port4.AddMember("port", "1235", allocator);
    port4.AddMember("protocol", "tcp", allocator);
    port4.AddMember("enable_heartbeat", "N", allocator);
    port4.AddMember("enable_idc_group", "Y", allocator);
    
    rapidjson::Value portlist(rapidjson::kArrayType);
    portlist.PushBack(port1, allocator);
    portlist.PushBack(port2, allocator);
    portlist.PushBack(port3, allocator);
    portlist.PushBack(port4, allocator);
    
    rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
    document.AddMember("service_group",srvgrp , allocator);
    document.AddMember("app_name", "test112", allocator);
    document.AddMember("servername", "HelloServer112", allocator);
    document.AddMember("present_state", "active", allocator);
    document.AddMember("port_list",portlist, allocator);
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string sJson = sb.GetString();
    cout << FILE_FUN << "|sJson=" << sJson << endl;

    string tmpvalue = "key=" + g_key + "&interface_name=addserver" + "&interface_params=" + sJson;

    stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpvalue)));
    //cout << stHttpReq.encode() << endl;

    TC_HttpResponse response;
    if (stHttpReq.doRequest(response, 3000) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void deleteServers()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
    document.AddMember("service_group",srvgrp , allocator);
    //document.AddMember("app_name", "test112", allocator);
   // document.AddMember("servername", "HelloServer112", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string sJson = sb.GetString();
    cout << FILE_FUN << "|sJson=" << sJson << endl;

    string tmpUrl = g_url;
    string tmpValue = "key=" + g_key + "&interface_name=" + "delserver" + "&interface_params=" + sJson;
    stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpValue)));
    //cout << stHttpReq.encode() << endl;

    TC_HttpResponse response;
    if (stHttpReq.doRequest(response, 3000) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}
void updateServers()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

    string tmpUrl = g_url;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

                
    rapidjson::Value port1(rapidjson::kObjectType);
    port1.AddMember("port_name", "obj1", allocator);
    port1.AddMember("ip", "1.2.3.4", allocator);
    port1.AddMember("port", "1234", allocator);
    port1.AddMember("protocol", "tcp", allocator);
    port1.AddMember("enable_heartbeat", "N", allocator);
    port1.AddMember("weight", "50", allocator);
    port1.AddMember("enable_idc_group", "Y", allocator);
    port1.AddMember("ip_group_name", "shenzhen_1", allocator);
    port1.AddMember("enable_set", "Y", allocator);
    port1.AddMember("set_name", "aa", allocator);
    port1.AddMember("set_area", "bb", allocator);
    port1.AddMember("set_group", "cc", allocator);
    rapidjson::Value portlist(rapidjson::kArrayType);
    portlist.PushBack(port1, allocator);
    
    rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
    document.AddMember("service_group",srvgrp , allocator);
    document.AddMember("app_name", "test112", allocator);
    document.AddMember("servername", "HelloServer112", allocator);
    document.AddMember("port_list",portlist, allocator);
    document.AddMember("present_state", "active", allocator);
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string sJson = sb.GetString();
    
    cout << FILE_FUN << "|sJson=" << sJson << endl;

    string tmpvalue = "key=" + g_key + "&interface_name=" + "updateserver" + "&interface_params=" + sJson;
    stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpvalue)));
    //cout << stHttpReq.encode() << endl;

    TC_HttpResponse response;
    if (stHttpReq.doRequest(response, 3000) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void getServers()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;

    tars::TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
    document.AddMember("service_group",srvgrp , allocator);
    document.AddMember("app_name", "test112", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string sJson = sb.GetString();
    cout << FILE_FUN << "|sJson=" << sJson << endl;
    string tmpUrl = g_url + "?key=" + g_key + "&interface_name=" + "getserver" + "&interface_params=" + TC_Encoder::gbk2utf8(urlEncode(sJson));
    stHttpReq.setGetRequest(tmpUrl);

    // cout << stHttpReq.encode() << endl;

    TC_HttpResponse response;
    if (stHttpReq.doRequest(response, 3000) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void keepServerAlive()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    tars::TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

    string tmpUrl = g_url;

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

                
    rapidjson::Value port1(rapidjson::kObjectType);
    port1.AddMember("port_name", "obj1", allocator);
    port1.AddMember("ip", "1.2.3.4", allocator);
    port1.AddMember("port", "1234", allocator);
    port1.AddMember("protocol", "udp", allocator);

    rapidjson::Value port2(rapidjson::kObjectType);
    port2.AddMember("port_name", "obj2", allocator);
    port2.AddMember("ip", "1.2.3.4", allocator);
    port2.AddMember("port", "1235", allocator);
    port2.AddMember("protocol", "tcp", allocator);
    
    rapidjson::Value portlist(rapidjson::kArrayType);
    portlist.PushBack(port1, allocator);
    portlist.PushBack(port2, allocator);
    
    rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
    document.AddMember("service_group",srvgrp , allocator);
    document.AddMember("app_name", "test112", allocator);
    document.AddMember("servername", "HelloServer112", allocator);
    document.AddMember("present_state", "active", allocator);
    document.AddMember("port_list",portlist, allocator);
    
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string sJson = sb.GetString();
    cout << FILE_FUN << "|sJson=" << sJson << endl;

    string tmpvalue = "key=" + g_key + "&interface_name=" + "keepalive" + "&interface_params=" + sJson;
    stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpvalue)));
    //cout << stHttpReq.encode() << endl;

    TC_HttpResponse response;
    if (stHttpReq.doRequest(response, 3000) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}



void updateServerState()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    Tseer::ServerStateInfo stState;

    stState.moduleType = g_srvgrp;
    stState.application = "test112";
    stState.serverName = "HelloServer112";
    stState.nodeName = "1.2.3.4";
    stState.containerName = "obj1_1234";
    stState.serverState = Inactive;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    int ret = g_regprx->updateServer(stState);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

void updateServerStates()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    Tseer::ServerStateInfo stState;
    stState.moduleType = g_srvgrp;
    stState.application = "test112";
    stState.serverName = "HelloServer112";
    stState.nodeName = "1.2.3.4";
    stState.containerName = "obj1_1234";
    stState.serverState = Active;

    vector<Tseer::ServerStateInfo> vState;
    vState.push_back(stState);

    stState.containerName = "obj2_1235";
    vState.push_back(stState);
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    int ret = g_regprx->updateServerBatch(vState);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}