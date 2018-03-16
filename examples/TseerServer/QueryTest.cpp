#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "QueryF.h"

#include "Util.h"
#include "QueryTest.h"

using namespace Tseer;
using namespace std;

extern Communicator g_com;

void display(Tseer::EndpointF point)
{
    if(point.istcp)
        cout << "tcp ";
    else
        cout << "udp ";
    cout << "-h " << point.host << " -p " << point.port << " | setId = " << point.setId << endl;
}

void findObjectById4Any()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    Tseer::QueryFPrx prx =   g_com.stringToProxy<Tseer::QueryFPrx>(QUERY_OBJ);
    string nodeObj = "apitest222.test112.HelloServer112.obj1";
    
    vector<Tseer::EndpointF> activeEp;
    vector<Tseer::EndpointF> inactiveEp;

    if(prx->findObjectById4Any(nodeObj, activeEp, inactiveEp) == 0)
    {
        cout << "<<< activeEp, size = "<< activeEp.size() << " >>>" << endl;
        for(size_t i = 0; i < activeEp.size(); i++)
        {
           display(activeEp[i]);
        }
        cout << "<<< inactiveEp, size = "<< inactiveEp.size() << " >>>" << endl;
        for(size_t i = 0; i < inactiveEp.size(); i++)
        {
           display(inactiveEp[i]);
        }
    }
    else
    {
        cout << FILE_FUN << "call failed" << endl;
    }
}

void findObjectByIdInSameGroup()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    Tseer::QueryFPrx prx =   g_com.stringToProxy<Tseer::QueryFPrx>(QUERY_OBJ);
    string nodeObj = "apitest222.test112.HelloServer112.obj1";
    
    vector<Tseer::EndpointF> activeEp;
    vector<Tseer::EndpointF> inactiveEp;

    if(prx->findObjectByIdInSameGroup(nodeObj, activeEp, inactiveEp) == 0)
    {
        cout << "<<< activeEp, size = "<< activeEp.size() << " >>>" << endl;
        for(size_t i = 0; i < activeEp.size(); i++)
        {
           display(activeEp[i]);
        }
        cout << "<<< inactiveEp, size = "<< inactiveEp.size() << " >>>" << endl;
        for(size_t i = 0; i < inactiveEp.size(); i++)
        {
           display(inactiveEp[i]);
        }
    }
    else
    {
        cout << FILE_FUN << "call failed" << endl;
    }
}

void findObjectByIdInSameSet()
{
    cout << "<------------------------ " << __FUNCTION__ << " ------------------------>" << endl;
    Tseer::QueryFPrx prx =   g_com.stringToProxy<Tseer::QueryFPrx>(QUERY_OBJ);
    string nodeObj = "apitest222.test112.HelloServer112.obj1";
    
    vector<Tseer::EndpointF> activeEp;
    vector<Tseer::EndpointF> inactiveEp;
    string setId = "aa.bb.cc";
    if(prx->findObjectByIdInSameSet(nodeObj, setId, activeEp, inactiveEp) == 0)
    {
        cout << "<<< activeEp, size = "<< activeEp.size() << " >>>" << endl;
        for(size_t i = 0; i < activeEp.size(); i++)
        {
           display(activeEp[i]);
        }
        cout << "<<< inactiveEp, size = "<< inactiveEp.size() << " >>>" << endl;
        for(size_t i = 0; i < inactiveEp.size(); i++)
        {
           display(inactiveEp[i]);
        }
    }
    else
    {
        cout << FILE_FUN << "call failed" << endl;
    }
}