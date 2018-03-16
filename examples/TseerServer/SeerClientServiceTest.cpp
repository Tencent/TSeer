#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "Util.h"
#include "SeerClientServiceTest.h"

using namespace tars;
using namespace std;

//测试添加Seer客户端服务
void addSeerClientService()
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember("manager_rtx", "tim;sparrow;jack", allocator);
    document.AddMember("department", "sparrow_department", allocator);
    document.AddMember("service_group", "sparrow_service_group", allocator);
    // string str = TC_Encoder::gbk2utf8("中文");
    // rapidjson::Value chinesestr (str.c_str(), allocator);
    // document.AddMember("service_group", chinesestr, allocator);
    document.AddMember("app_name", "sparrow_app_name", allocator);
    document.AddMember("servername", "sparrow_servername", allocator);
    document.AddMember("server_uid", "sparrow_test_uid1", allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();
    
    string url = getRequestUrl("clientaddservice", requestJson);
    tars::TC_HttpResponse response;
    if (doSyncRequest(url, response) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }

        rapidjson::StringBuffer rsb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> rpw(rsb);
        result.Accept(rpw);
        string resultJson = rsb.GetString();
        cout << resultJson << endl;
    }
}

//测试修改Seer客户端服务管理者
void modifySeerClientServiceManager()
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember("manager_rtx", "adminrtx", allocator);
    document.AddMember("api_key", "2dc9c5a9aa34e89e4a9ddd64deb2ee6b", allocator);
    document.AddMember("new_manager_rtx", "sparrow", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("clientmodifymanager", requestJson);
    tars::TC_HttpResponse response;

    if (doSyncRequest(url, response) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }

        rapidjson::StringBuffer rsb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> rpw(rsb);
        result.Accept(rpw);
        string resultJson = rsb.GetString();
        cout << resultJson << endl;
    }    
}

//测试删除Seer客户端服务
void delSeerClientService()
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember("manager_rtx", "sparrow", allocator);
    document.AddMember("api_key", "656033d74b6146d6daf4c88b31b7394e", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("clientdelservice", requestJson);
    tars::TC_HttpResponse response;

    if (doSyncRequest(url, response) == 0)
    {
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }

        rapidjson::StringBuffer rsb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> rpw(rsb);
        result.Accept(rpw);
        string resultJson = rsb.GetString();
        cout << resultJson << endl;
    }
}

//测试获取Seer客户端服务
void getSeerClientService()
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.AddMember("manager_rtx", "adminrtx", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("clientgetservice", requestJson);
    tars::TC_HttpResponse response;



    if (doSyncRequest(url, response) == 0)
    {

        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(response.getContent().c_str());
        if (!isParse)
        {
            cout << FILE_FUN << "parse return result failed" << endl;
            return;
        }

        rapidjson::StringBuffer rsb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> rpw(rsb);
        result.Accept(rpw);
        string resultJson = rsb.GetString();
        cout << resultJson << endl;
    }
}