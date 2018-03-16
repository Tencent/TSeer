#include "util/tc_option.h"

#include "ServerTest.h"
#include "ServiceGroupTest.h"
#include "IDCTest.h"
#include "NodeTest.h"
#include "QueryTest.h"
#include "Global.h"
#include "Util.h"

using namespace std;

static int g_func = 0;
static string g_id = "";

void parseConfig(int argc, char *argv[])
{
    TC_Option option;
    option.decode(argc, argv);

    if (option.hasParam("help") || !option.hasParam("func"))
    {
        cout << "Usage : ./Test --func=<id>" << endl;
        //cout << "--key=key" << endl;
        cout << "1 -- addServers | 2 -- updateServers | 3 -- getServers | 4 -- deleteServers | 5 -- keepServerAlive" << endl;
        cout << "6 -- addServiceGroup | 7 -- getServiceGroup | 8 -- updateServiceGroup | 9 -- getServiceGroupKey" << endl;
        cout << "10 -- addIdcGroup | 11 -- delIdcGroup | 12 -- modifyIdcGroup | 13 -- getAllIdcGroup" << endl;
        cout << "14 -- addPriority | 15 -- delPriority | 16 -- modifyPriority | 17 -- getPriority" << endl;
        cout << "18 -- addOneAgentNode | 19 -- delOneAgentNode | 20 -- keepAliveAgent | 21 -- registerNode | 22 -- destroyNode | 23 -- keepAlive" << endl;
        cout << "24 -- getAllServers | 25 -- updateServerState | 26 -- updateServerStates | 27 -- getIdcGroup" << endl;
        cout << "28 -- findObjectById4Any | 29 -- findObjectByIdInSameSet | 30 -- findObjectByIdInSameGroup" << endl;
        exit(0);
    }

    if (option.hasParam("func"))
    {
        g_func = TC_Common::strto<int>(option.getValue("func"));
    }

    if (option.hasParam("key"))
    {
        g_key = option.getValue("key");
        cout << "g_key=" << g_key << endl;
    }

    if (option.hasParam("user"))
    {
        g_user = option.getValue("user");
        cout << "g_user=" << g_user << endl;
    }

    if (option.hasParam("srvg"))
    {
        g_srvgrp = option.getValue("srvg");
        cout << "g_srvgrp=" << g_srvgrp << endl;
    }

    if (option.hasParam("cnt"))
    {
        g_ttime = TC_Common::strto<int>(option.getValue("cnt"));
    }

    if (option.hasParam("interval"))
    {
        g_iloadInterval = TC_Common::strto<int>(option.getValue("interval"));
    }

    if(option.hasParam("idcname"))
    {
        g_idcname = option.getValue("idcname");
    }

    if(option.hasParam("id"))
    {
        g_id = option.getValue("id");
    }
}

void testServers()
{
    switch(g_func)
    {
        case 1:
            addServers();
            break;
        case 2:
            updateServers();
            break;
        case 3:
            getServers();
            break;
        case 4:
            deleteServers();
            break;
        case 5:
            keepServerAlive();
            break;
        case 33:
            updateServerState();
            break;
        case 34:
            updateServerStates();
            break;
    }
}

void testServiceGroup()
{
    switch(g_func)
    {
        case 6:
            addServiceGroup();
            break;
        case 7:
            getServiceGroup();
            break;
        case 8:
            updateServiceGroup();
            break;
    }
}

void testIDC()
{
    switch(g_func)
    {
        case 10:
            addIdcGroup();
            break;
        case 11:
            delIdcGroup();
            break;
        case 12:
            modifyIdcGroup();
            break;
        case 13:
            getAllIdcGroup();
            break;
        case 14:
            addPriority();
            break;
        case 15:
            delPriority(g_id);
            break;
        case 16:
            modifyPriority(g_id);
            break;
        case 17:
            getPriority();
            break;
        case 27:
            getIdcGroup();
            break;
    }
}

void testNode()
{
    switch(g_func)
    {
        case 18:
            addOneAgentNode();
            break;
        case 19:
            delOneAgentNode();
            break;
        case 20:
            keepAliveAgent();
            break;
        case 21:
            registerNode();
            break;
        case 22:
            destroyNode();
            break;
        case 23:
            keepAlive();
            break;
        case 24:
            getagentbaseinfo();
            break;
        case 25:
            getagentpackageinfo();
            break;
        case 26:
            updateagentgraystate();
            break;
        case 31:
            updateagentpackageinfo();
            break;
        case 32:
            deleteagentpackage();
            break;
        case 33:
            updateagentlocator();
            break;
    }
}

void testQuery()
{
    switch(g_func)
    {
        case 28:
            findObjectById4Any();
            break;
        case 29:
            findObjectByIdInSameSet();
            break;
        case 30:
            findObjectByIdInSameGroup();
            break;
    }
}
int main(int argc, char *argv[])
{
    parseConfig(argc, argv);
    testServers();
    testServiceGroup();
    testIDC();
    testNode();
    testQuery();
}