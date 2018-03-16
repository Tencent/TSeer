#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "Registry.h"

#include "Util.h"
#include "NodeTest.h"

using namespace tars;
using namespace std;
using namespace Tseer;


extern Communicator g_com;

void addOneAgentNode()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    NodeInstanceInfo nodeInfo;
    nodeInfo.nodeName = "10.134.11.20";
    nodeInfo.nodeObj = "Sparrow.Test.NodeObj@tcp -h 10.134.11.20 -p 9999 -t 60000";
    nodeInfo.endpointIp = "10.134.11.20";
    nodeInfo.endpointPort = 9999;
    nodeInfo.version = "v0.16";
    nodeInfo.osversion = "CentOs-6.2-64";
    nodeInfo.locator = "Docker.TSeerServer.QueryObj@tcp -h 127.0.0.1 -p 9903";
    
    int ret = g_regprx->registerNodeExt(nodeInfo);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

void delOneAgentNode()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember("iplist", "10.134.11.21", allocator);
    document.AddMember("user", "webadmin", allocator);
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
    document.Accept(pw);
    string requestJson = sb.GetString();

    string url = getRequestUrl("deleteagent", requestJson);

    TC_HttpResponse response;
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

void keepAliveAgent()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    NodeInstanceInfo nodeInfo;
    nodeInfo.nodeName = "10.134.11.21";

    int ret = g_regprx->keepAliveExt(nodeInfo);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

void registerNode()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    string nodeName = "10.130.64.36";
    NodeInfo ni;
    ni.nodeObj = "Sparrow.Test.NodeObj@tcp -h 10.134.11.21 -p 9999 -t 60000";
    LoadInfo li;
    int ret = g_regprx->registerNode(nodeName, ni, li);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

void destroyNode()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    int ret = g_regprx->destroy("10.130.64.36");
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

void keepAlive()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    RegistryPrx g_regprx = g_com.stringToProxy<RegistryPrx>(REGISTRY_OBJ);
    string nodeName = "10.130.64.36";
    LoadInfo li;
    li.avg1 = 0.45;
    li.avg5 = 0.192;
    int ret = g_regprx->keepAlive(nodeName, li);
    if (ret != 0)
    {
        cout << FILE_FUN << "call failed" << endl;
    }
    else
    {
        cout << FILE_FUN << " success!" << endl;
    }
}

extern string g_key;
extern string g_url;

void getagentbaseinfo()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        string tmpUrl = g_url + "?key=" + g_key + "&interface_name=" + "getagentbaseinfo";
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

void updateagentgraystate()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("iplist","10.134.11.21" , allocator);
        document.AddMember("ostype", "SPA_OS_VER", allocator);
        document.AddMember("grayscale_state", "2", allocator);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl =  "key=" + g_key + "&interface_name=" + "updateagentgraystate"+ "&interface_params=" + sJson;;

        stHttpReq.setPostRequest(g_url, TC_Encoder::gbk2utf8(urlEncode(tmpUrl)));
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

        rapidjson::StringBuffer sb1;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw1(sb1);
        result.Accept(pw1);
        string resultJson = sb1.GetString();
        cout << resultJson << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}


void getagentpackageinfo()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");

        string tmpUrl = g_url + "?key=" + g_key + "&interface_name=" + "getagentpackageinfo";
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

void updateagentpackageinfo()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("package_name","router_agent_874.tgz" , allocator);
        document.AddMember("ostype", "SPA_OS_VER", allocator);
        document.AddMember("package_type", "1", allocator);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl =  "key=" + g_key + "&interface_name=" + "updateagentpackageinfo"+ "&interface_params=" + sJson;;

        stHttpReq.setPostRequest(g_url, TC_Encoder::gbk2utf8(urlEncode(tmpUrl)));
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

        rapidjson::StringBuffer sb1;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw1(sb1);
        result.Accept(pw1);
        string resultJson = sb1.GetString();
        cout << resultJson << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }   
}

void deleteagentpackage()
{
     cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("package_name","router_agent_874.tgz" , allocator);
        document.AddMember("ostype", "SPA_OS_VER", allocator);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl =  "key=" + g_key + "&interface_name=" + "deleteagentpackage"+ "&interface_params=" + sJson;;

        stHttpReq.setPostRequest(g_url, TC_Encoder::gbk2utf8(urlEncode(tmpUrl)));
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

        rapidjson::StringBuffer sb1;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw1(sb1);
        result.Accept(pw1);
        string resultJson = sb1.GetString();
        cout << resultJson << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }      
}

void updateagentlocator()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    try
    {
        TC_HttpRequest stHttpReq;
        stHttpReq.setCacheControl("no-cache");
        stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("iplist","127.0.0.1" , allocator);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> pw(sb);
        document.Accept(pw);
        string sJson = sb.GetString();
        cout << FILE_FUN << "|sJson=" << sJson << endl;

        string tmpUrl =  "key=" + g_key + "&interface_name=" + "updateagentlocator"+ "&interface_params=" + sJson;;

        stHttpReq.setPostRequest(g_url, TC_Encoder::gbk2utf8(urlEncode(tmpUrl)));
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

        rapidjson::StringBuffer sb1;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> pw1(sb1);
        result.Accept(pw1);
        string resultJson = sb1.GetString();
        cout << resultJson << endl;
    }
    catch (exception &e)
    {
        cout << "testwupImp exception:" << e.what() << endl;
    }
}


