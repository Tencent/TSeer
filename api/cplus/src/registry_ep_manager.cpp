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

#include "registry_ep_manager.h"

#include <arpa/inet.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include <set>
#include "dns_packer.h"
#include "tc_common.h"
#include "conn.h"
#include "tup.h"
#include "global.h"
using std::ostringstream;

namespace Tseerapi
{

RegistryEpManager::RegistryEpManager() :_index(0), _lastUpdateTime(0), _isIpsConstant(false)
{
}

int RegistryEpManager::getAvaliableIp(std::string &ip, std::string &errMsg)
{
    isNeedUpdate();

    if (_registryIps.empty())
    {
        ostringstream os;
        os << FILE_FUN << "no avaliable registry ip";
        errMsg = os.str();
        return -1;
    }

    size_t tryTime = 0;
    size_t ipNums = _registryIps.size();

    do 
    {
        _index = (_index + 1) % ipNums;
        ip = _registryIps[_index];
        if (checkActive(ip))
        {
            break;
        }
        ++tryTime;
    } while (tryTime < ipNums);

    //全部节点都不可用
    if (tryTime == ipNums)
    {
        ostringstream os;
        os << FILE_FUN << "has no valid registry nodes";
        errMsg = os.str();

        _index = random() % ipNums;
        ip = _registryIps[_index];
    }

    return 0;
}

void RegistryEpManager::reportResult(const std::string &ip, int result)
{
    std::map<std::string, RouterNodeStat>::iterator it = _ipsStat.find(ip);
    if (it == _ipsStat.end())
    {
        return;
    }

    size_t now = time(NULL);

    bool isFail = ((result == 0) ? false : true);
    RouterNodeStat &stat = it->second;

    //如果之前节点是不可用的
    if (!stat.isAvailable)
    {
        if (!isFail)
        {
            stat.isAvailable = true;
            stat.succNum = 1;
            stat.errNum = 0;
            stat.continueErrNum = 0;
            stat.continueErrTime = now + _checkTimeoutInfo.minFrequenceFailTime;
            stat.lastCheckTime = now + _checkTimeoutInfo.checkTimeoutInterval;
        }
        else
        {
            stat.errNum++;
        }

        return;
    }

    if (!isFail)
    {
        stat.succNum++;
    }
    else
    {
        stat.errNum++;
    }

    if (isFail)
    {
        if (stat.continueErrNum == 0)
        {
            stat.continueErrTime = now + _checkTimeoutInfo.minFrequenceFailTime;
        }

        stat.continueErrNum++;

        //在minFrequenceFailTime时间内，错误次数超过frequenceFailInvoke次
        if (stat.continueErrNum >= _checkTimeoutInfo.frequenceFailInvoke && now >= stat.continueErrTime)
        {
            stat.isAvailable = false;
            stat.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;

            return;
        }
    }
    else
    {
        stat.continueErrNum = 0;
    }

    //进行一轮统计
    if (now >= stat.lastCheckTime)
    {
        stat.lastCheckTime = now + _checkTimeoutInfo.checkTimeoutInterval;

        if (isFail && (stat.succNum + stat.errNum) >= _checkTimeoutInfo.minTimeoutInvoke &&
            stat.errNum >= _checkTimeoutInfo.timeoutRatio * (stat.succNum + stat.errNum))
        {
            stat.isAvailable = false;
            stat.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;
        }
        else
        {
            stat.succNum = 0;
            stat.errNum = 0;
        }
    }
}

void RegistryEpManager::setConsantIp(const std::string &ips)
{
    _isIpsConstant = true;

    //去空格添加
    vector<string> ipsTmp = TC_Common::sepstr<string>(ips, "|");
    for (size_t i = 0; i < ipsTmp.size(); ++i)
    {
        _registryIps.push_back(TC_Common::trim(ipsTmp[i]));
    }

    //初始化每个节点的状态
    for (size_t i = 0; i < _registryIps.size(); ++i)
    {
        if (_ipsStat.find(_registryIps[i]) == _ipsStat.end())
        {
            _ipsStat[_registryIps[i]].succNum = 0;
            _ipsStat[_registryIps[i]].errNum = 0;
            _ipsStat[_registryIps[i]].continueErrNum = 0;
            _ipsStat[_registryIps[i]].timeCost = 0;
            _ipsStat[_registryIps[i]].isAvailable = true;
            _ipsStat[_registryIps[i]].nextRetryTime = 0;
            _ipsStat[_registryIps[i]].lastCheckTime = 0;
            _ipsStat[_registryIps[i]].continueErrNum = 0;
        }
    }
}

void RegistryEpManager::isNeedUpdate()
{
    if (_isIpsConstant)
    {
        return;
    }

    time_t nowTime = time(NULL);

    //60s刷新一次
    if (nowTime - _lastUpdateTime > 60)
    {
        std::string errMsg;
        int ret = getRegistryFromDNS(errMsg);

        if (ret)
        {
            //std::cout << __FUNCTION__ << "|" << errMsg << std::endl;
        }

        _lastUpdateTime = nowTime;
    }
}

int RegistryEpManager::getRegistryFromDNS(std::string &errMsg)
{
    //从机器配置上获得DNS服务配置
    std::ifstream inFile(g_registry_dns_conf_file.c_str(), ios::in);
    if (!inFile)
    {
        return -1;
    }

    string buffStr;
    vector<string> vDnip;
    while (getline(inFile, buffStr))
    {
        if (buffStr.find_first_of('#') != string::npos)
        {
            continue;
        }

        if (buffStr.find("nameserver") != string::npos)
        {
            string sTemp;
            sTemp = Tseerapi::TC_Common::replace(buffStr, "nameserver", " ");
            sTemp = Tseerapi::TC_Common::trim(sTemp);
            if (!sTemp.empty())
            {
                vDnip.push_back(sTemp);
            }
            //std::cout << "DNS ip:" << sTemp << std::endl;    //输出获得的DNS IP
        }
    }
    if (vDnip.size() == 0)
    {
        return -1;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        ostringstream os;
        os << FILE_FUN << "create socket error";
        errMsg = os.str();
        return -1;
    }

    std::set<string> tmpIps;
    for (size_t i = 0; i < vDnip.size(); i++)
    {
        const char* domain = g_registry_dns.c_str();
        char buf[1024];
        uint32_t buflen = sizeof(buf);
        uint32_t domainlen = strlen(domain);
        uint16_t id = random();

        int ret = Tseerapi::IPv4ReqPack(buf, buflen, domain, domainlen, id);
        if (ret != 0)
        {
            //printf("IPv4ReqPack ret: %d\n", ret);
            continue;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(vDnip[i].c_str());
        addr.sin_port = htons(53);            //dns服务端口号

        ret = sendto(fd, buf, buflen, 0, (struct sockaddr*)&addr, sizeof(addr));
        if (ret != (int)buflen)
        {
            //printf("sendto ret: %d, buflen: %u, err: %m\n", ret, buflen);
            continue;
        }

        struct timeval tv = { 1, 0 };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
        ret = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        if (ret < (int)sizeof(Tseerapi::DnsHeader))
        {
            //printf("recvfrom ret: %d, err: %m\n", ret);
            continue;
        }

        std::vector<uint> ips;
        ret = Tseerapi::IPv4RspUnpack(ips, buf, ret, id);
        if (ret != 0)
        {
            //printf("IPv4RspUnpack ret: %d\n", ret);
            continue;
        }


        for (size_t i = 0; i < ips.size(); ++i)
        {
            //std::cout << string(inet_ntoa(*(in_addr*)&ips[i])) << std::endl;        //输出IP
            tmpIps.insert(string(inet_ntoa(*(in_addr*)&ips[i])));
        }

        if (tmpIps.size() == 0)
        {
            continue;
        }
        break;
    }

    //更新
    _registryIps.clear();
    for (std::set<std::string>::iterator it = tmpIps.begin(); it != tmpIps.end(); ++it)
    {
        _registryIps.push_back(*it);
    }
    
    //新节点加入状态
    for (size_t i = 0; i < _registryIps.size(); ++i)
    {
        if (_ipsStat.find(_registryIps[i]) == _ipsStat.end())
        {
            _ipsStat[_registryIps[i]].succNum = 0;
            _ipsStat[_registryIps[i]].errNum = 0;
            _ipsStat[_registryIps[i]].continueErrNum = 0;
            _ipsStat[_registryIps[i]].timeCost = 0;
            _ipsStat[_registryIps[i]].isAvailable = true;
            _ipsStat[_registryIps[i]].nextRetryTime = 0;
            _ipsStat[_registryIps[i]].lastCheckTime = 0;
            _ipsStat[_registryIps[i]].continueErrNum = 0;
        }
    }

    //删除不存在节点的状态
    std::map<std::string, RouterNodeStat>::iterator it = _ipsStat.begin();
    while (it != _ipsStat.end())
    {
        if (tmpIps.find(it->first) == tmpIps.end())
        {
            _ipsStat.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    close(fd);
    return 0;
}

bool RegistryEpManager::checkActive(const std::string &ip)
{
    std::map<std::string, RouterNodeStat>::iterator it = _ipsStat.find(ip);
    if (it == _ipsStat.end())
    {
        return false;
    }

    size_t now = time(NULL);

    //不可用且在相再尝试时间前
    if (!(it->second.isAvailable) && now < it->second.nextRetryTime)
    {
        return false;
    }

    if (!(it->second.isAvailable))
    {
        it->second.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;
    }

    return true;
}

}
