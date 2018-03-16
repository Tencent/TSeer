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