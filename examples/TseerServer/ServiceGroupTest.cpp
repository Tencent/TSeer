#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Util.h"
#include "ServiceGroupTest.h"


using namespace tars;
using namespace std;

extern string g_key;
extern string g_url;
extern string g_srvgrp;
extern string g_user;

void addServiceGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        tars::TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
        rapidjson::Value user(g_user.c_str(), allocator);
        document.AddMember("service_group",srvgrp , allocator);
        document.AddMember("user",user, allocator);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl = g_url;
        string tmpValue = "key=" + g_key + "&interface_name=" + "addservicegroup" + "&interface_params=" + sJson;
        stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpValue)));

        //cout << stHttpReq.encode() << endl;

        tars::TC_HttpResponse stHttpRep;
        int iRet = stHttpReq.doRequest(stHttpRep, 3000);
        if (iRet != 0)
        {
            cout << "iRet=" << iRet << endl;
        }
        string recvBuff = stHttpRep.getContent();
        cout << "rec:" << recvBuff << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}
void getServiceGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        tars::TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value user(g_user.c_str(), allocator);
        document.AddMember("user", user, allocator);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();

        cout << FILE_FUN << "|sJson=" << sJson << endl;
        string tmpUrl = g_url + "?key=" + g_key + "&interface_name=" + "getservicegroup" + "&interface_params=" + TC_Encoder::gbk2utf8(urlEncode(sJson));
        stHttpReq.setGetRequest(tmpUrl);

        //cout << stHttpReq.encode() << endl;

        tars::TC_HttpResponse stHttpRep;
        int iRet = stHttpReq.doRequest(stHttpRep, 3000);
        if (iRet != 0)
        {
            cout << "iRet=" << iRet << endl;
        }
        string recvBuff = stHttpRep.getContent();
        cout << "rec:" << recvBuff << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}

void updateServiceGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
        rapidjson::Value srvgrp(g_srvgrp.c_str(), allocator);
        document.AddMember("service_group",srvgrp , allocator);
        rapidjson::Value user(g_user.c_str(), allocator);
        document.AddMember("user",user, allocator);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl = g_url;
        string tmpValue = "key=" + g_key + "&interface_name=" + "updateservicegroup" + "&interface_params=" + sJson;
        stHttpReq.setPostRequest(tmpUrl, TC_Encoder::gbk2utf8(urlEncode(tmpValue)));

        //cout << stHttpReq.encode() << endl;

        TC_HttpResponse stHttpRep;
        int iRet = stHttpReq.doRequest(stHttpRep, 3000);
        if (iRet != 0)
        {
            cout << "iRet=" << iRet << endl;
        }
        string recvBuff = stHttpRep.getContent();
        cout << "rec:" << recvBuff << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}

