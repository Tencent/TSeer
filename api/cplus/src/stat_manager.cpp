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

#include "stat_manager.h"

#include <arpa/inet.h>
#include <time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sstream>
#include "global.h"
#include "tc_common.h"
#include "conn.h"
#include "tup.h"
using std::ostringstream;

static unsigned int seqNum = 0;

namespace Tseerapi
{
/********************************StatManager********************************************/

StatManager::StatManager() 
{
    _tid = syscall(SYS_gettid);
    
    //获取本机IP地址
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int cmd = SIOCGIFCONF;
    struct ifconf ifc;
    int numaddrs = 10;
    int old_ifc_len = 0;

    while (true)
    {
        int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
        ifc.ifc_len = bufsize;
        ifc.ifc_buf = (char*)malloc(bufsize);
        int rs = ioctl(fd, cmd, &ifc);

        if (rs == -1)
        {
            free(ifc.ifc_buf);
            _localIp = "UNKNOWN_IP";
        }
        else if (ifc.ifc_len == old_ifc_len)
        {
            break;
        }
        else
        {
            old_ifc_len = ifc.ifc_len;
        }

        numaddrs += 10;
        free(ifc.ifc_buf);
    }

    numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
    struct ifreq* ifr = ifc.ifc_req;
    for (int i = 0; i < numaddrs; ++i)
    {
        if (ifr[i].ifr_addr.sa_family == AF_INET)
        {
            struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&ifr[i].ifr_addr);
            if (addr->sin_addr.s_addr != 0)
            {
                char sAddr[INET_ADDRSTRLEN] = "\0";
                inet_ntop(AF_INET, &(*addr).sin_addr, sAddr, sizeof(sAddr));
                _localIp = sAddr;
                if (_localIp.find("127.0.0") == std::string::npos && _localIp != "0.0.0.0")
                {
                    break;
                }
            }
        }
    }
    free(ifc.ifc_buf);

    if (_localIp.find("127.0.0") != std::string::npos || _localIp == "0.0.0.0")
    {
        _localIp = "UNKNOWN_IP";
    }
}

StatManager::~StatManager()
{
}

int StatManager::reportStat(const InnerRouterRequest &req, int ret, int timeCost)
{
    int innerRet;
    string subErr;

    Tseer::NodeStat nodeStat;
    nodeStat.localIp = _localIp;
    nodeStat.timeCost = timeCost;
    nodeStat.ret = ret;
    nodeStat.moduleName = req.moduleName;
    nodeStat.setname = req.setInfo;

    nodeStat.ip = req.ip;
    nodeStat.port = req.port;
    nodeStat.obj = req.obj;
    nodeStat.isTcp = req.isTcp;
    
    switch (req.type) {
    case LB_TYPE_LOOP:
                nodeStat.lbType = Tseer::LB_TYPE_LOOP;
                break;
    case LB_TYPE_RANDOM:
                nodeStat.lbType = Tseer::LB_TYPE_RANDOM;
                break;
    case LB_TYPE_STATIC_WEIGHT:
                nodeStat.lbType = Tseer::LB_TYPE_STATIC_WEIGHT;
                break;
    case LB_TYPE_CST_HASH:
                nodeStat.lbType = Tseer::LB_TYPE_CST_HASH;
                break;
    case LB_TYPE_ALL:
                nodeStat.lbType = Tseer::LB_TYPE_ALL;
                break;
    }
       
    switch (req.lbGetType) {
    case LB_GET_IDC:
            nodeStat.getType = Tseer::GET_IDC;
            break;
    case LB_GET_SET:
            nodeStat.getType = Tseer::GET_SET;
            break;
    case LB_GET_ALL:
            nodeStat.getType = Tseer::GET_ALL;
            break;
    }
    nodeStat.identifier = TC_Common::tostr(getpid());

    innerRet = reportToAgent(nodeStat, subErr);

    return innerRet;
}

int StatManager::reportToAgent(const Tseer::NodeStat &nodeStat, string &errMsg)
{
    TarsUniPacket<> packet;
    packet.setRequestId(__sync_fetch_and_add(&seqNum, 1));
    packet.setVersion(3);
    packet.setServantName(g_agent_router_obj);
    packet.setTarsPacketType(1);

    packet.setFuncName(g_agent_call_stat_report_func);
    packet.put<Tseer::NodeStat>("stat", nodeStat);

    string buffStr("");

    try
    {
        packet.encode(buffStr);
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "getRouteFromAgent encode exception:" << ex.what();
        errMsg = os.str();
        return -1;
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "getRouteFromAgent encode unknown exception.";
        errMsg = os.str();
        return -1;
    }

    std::string subErr;
    int ret = Conn::NodeStatSendAgent(buffStr.c_str(), buffStr.size(), 100, subErr);
    if (ret < 0)
    {
        ostringstream os;
        os << FILE_FUN << "send data to agent failed";
        errMsg = mergeErrMsg(os.str(), subErr);
        return ret;
    }

    return 0;
}

}
