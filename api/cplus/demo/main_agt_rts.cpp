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

#include <iostream>
#include <vector>
#include "seer_api.h"
#include "seer_comm.h"

using namespace std;
using namespace routerapi;

int main()
{
    int iRet = 0;
    string sErr;

    InitAgentApiParams initParams;
    initParams.sServiceKey = "seerDemoKey";
    iRet = ApiSetAgentIpInfo(initParams, sErr);
    if (iRet != 0) {
        cout << "init Agent error" << endl;
        return 0;
    }

    //初始化ApiGetRoutes的参数
    RoutersRequest req;
    req.sObj = "tencent.tencentServer.HelloService";
    req.lbGetType = LB_GET_SET;
    req.sSetInfo = "sz.a.b";
    req.type = LB_TYPE_ALL;

    iRet = ApiGetRoutes(req, sErr);
    if (iRet == 0) {
        vector<NodeInfo>::vIter;
        string sNodeInfo;
        for (vIter = req.vNodeInfo.begin(); vIter != req.vNodeInfo.end(); vIter++) {
            sNodeInfo += vIter->sIp + ":";
            sNodeInfo += vIter->iPort + " | ";
            if (vIter->bTcp) {
                sNodeInfo += "tcp | ";
            } else {
                sNodeInfo += "udp | ";
            }
            sNodeInfo += vIter->sSlaveSet;
            cout << "[NODEINFO]: " << sNodeInfo << endl;

            //调用被调
            sleep(5);

            //调用结束上报调用结果
            RouterRequest reportReq;
            reportReq.sObj = req.sObj;
            reportReq.lbGetType = req.lbGetType;
            reportReq.sSetInfo = req.sSetInfo;
            reportReq.sIp = vIter->sIp;
            reportReq.iPort = vIter->iPort;
            reportReq.bTcp = vIter->bTcp;

            ApiRouteResultUpdate(reportReq, 0, 1, sErr);
            cout << "[report]sErr: " << sErr << endl;
        }
    }
}
