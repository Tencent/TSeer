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

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "Util.h"
#include "IDCTest.h"

using namespace tars;
using namespace std;

extern string g_key;
extern string g_url;
extern string g_srvgrp;
extern string g_idcname;
void addIdcGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    rapidjson::Value idcname(g_idcname.c_str(), allocator);
    document.AddMember("group_name", idcname, allocator);

    rapidjson::Value allowIps(rapidjson::kArrayType);
    allowIps.PushBack("119.163.15.*", allocator);
    allowIps.PushBack("10.163.155.*", allocator);
    allowIps.PushBack("1.2.3.4", allocator);
    allowIps.PushBack("127.0.0.1", allocator);
    document.AddMember("allow_ip", allowIps, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("add_idc_group_rule", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void delIdcGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value delIds(rapidjson::kArrayType);
    delIds.PushBack("1", allocator);
    delIds.PushBack("3", allocator);
    delIds.PushBack("5", allocator);    
    document.AddMember("group_ids", delIds, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("del_idc_group_rule", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void modifyIdcGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember("group_id", "7", allocator);
    rapidjson::Value allowIps(rapidjson::kArrayType);
    allowIps.PushBack("119.119.119.*", allocator);
    allowIps.PushBack("220.222.1.*", allocator);
    allowIps.PushBack("10.11.12.*", allocator);
    document.AddMember("allow_ip", allowIps, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("modify_idc_group_rule", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void getAllIdcGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    string url = getRequestUrl("get_all_idc_group_rule", "");
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

void getIdcGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        string tmpUrl = g_url + "?key=" + g_key + "&interface_name=" + "getidcgroup";
        stHttpReq.setGetRequest(tmpUrl);

        cout << stHttpReq.encode() << endl;

        tars::TC_HttpResponse stHttpRep;
        int iRet = stHttpReq.doRequest(stHttpRep, 3000);
        if (iRet != 0)
        {
            cout << "iRet=" << iRet << endl;
            return;
        }
        
        rapidjson::Document result;
        rapidjson::ParseResult isParse = result.Parse(stHttpRep.getContent().c_str());
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
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}

//测试新增IDC优先级
void addPriority()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    
    rapidjson::Value name(g_idcname.c_str(), allocator);
    document.AddMember("priority_name", name, allocator);
    document.AddMember("order", "1", allocator);
    
    rapidjson::Value groupList(rapidjson::kArrayType);
    groupList.PushBack("4", allocator);
    groupList.PushBack("2", allocator);
    document.AddMember("group_id_list", groupList, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("add_idc_priority", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

//测试删除IDC优先级
void delPriority(const string &priorityId)
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value delIds(rapidjson::kArrayType);
    rapidjson::Value id(priorityId.c_str(), allocator);
    delIds.PushBack(id, allocator);
    document.AddMember("priority_ids", delIds, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("del_idc_priority", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

//测试修改IDC优先级
void modifyPriority(const string &priorityId)
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value id(priorityId.c_str(), allocator);
    document.AddMember("priority_id", id, allocator);
    document.AddMember("order", "99", allocator);
    document.AddMember("priority_name", "sparrow_modify_priority", allocator);
    rapidjson::Value groupList(rapidjson::kArrayType);
    groupList.PushBack("11", allocator);
    groupList.PushBack("22", allocator);
    groupList.PushBack("33", allocator);
    document.AddMember("group_id_list", groupList, allocator);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("modify_idc_priority", requestJson);
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}

//测试获取所有IDC优先级
void getPriority()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    string url = getRequestUrl("get_all_idc_priority", "");
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

        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw(sb);
        result.Accept(pw);
        string resultJson = sb.GetString();
        cout << resultJson << endl;
    }
}