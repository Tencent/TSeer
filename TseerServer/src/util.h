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

#ifndef _TSEER_UTIL
#define _TSEER_UTIL

#include <map>
#include <vector>
#include <set>
#include <math.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include "util/tc_common.h"
#include "servant/TarsLogger.h"
#include "reg_roll_logger.h"
#include "RetCode.h"
#include "RouterData.h"

using namespace Tseer;


#define FILE_FUN __FUNCTION__ << ":" << __LINE__ << "|"

/*EtcdRouterDataProcessImp以及etcd操作相关滚动日志*/
#define ETCDPROC_LOG TSEER_LOG("etcdprocess")->debug()
#define ETCDPROC_LOGERROR TSEER_LOG("etcdprocess")->error()

/*RegistryImp以及etcd操作相关滚动日志*/
#define REGIMP_LOG TSEER_LOG("regimp")->debug()
#define REGIMP_LOGERROR TSEER_LOG("regimp")->error()


#define APIIMP_LOG TSEER_LOG("apiimp")->debug()
#define APIIMP_LOGERROR TSEER_LOG("apiimp")->error()

/*路由日志*/
#define ROUTE_LOG  TSEER_LOG("router")->debug()
#define ROUTE_LOGERROR  TSEER_LOG("router")->error()


/*加载etcd数据相关*/
#define ETCDLOAD_LOGINFO TSEER_LOG("etcdload")->info()
#define ETCDLOAD_LOG TSEER_LOG("etcdload")->debug()
#define ETCDLOAD_LOGERROR TSEER_LOG("etcdload")->error()

/*agent安装包相关日志*/
#define UPDATEPACKAGE_LOG  TSEER_LOG("updatethread")->debug()
#define UPDATEPACKAGE_LOGWARN  TSEER_LOG("updatethread")->warn()
#define UPDATEPACKAGE_LOGERROR  TSEER_LOG("updatethread")->error()

/*db操作日志*/
#define DB_LOG  TSEER_LOG("db")->debug()
#define DB_LOGERROR TSEER_LOG("db")->error()

#define STRLEN(x)  (string((x)).length())

#define MAKEVALUE(url,key,value) do{\
    url += "/";\
    url += key;\
    url += "=";\
    url += value;\
}while(0)

#define CHECK_SETVALUE(key) if (jData.HasMember(#key) && jData[#key].IsString())\
        {\
            key = jData[#key].GetString();\
        }\
        else\
        {\
            errMsg = #key " should not be empty";\
            ret = API_INVALID_PARAM;\
            break;\
        }

#define CHECK_SETKEYVALUE(key, value) if (jData.HasMember(key) && jData[key].IsString())\
        {\
            value = jData[key].GetString();\
        }\
        else\
        {\
            errMsg = key " should not be empty";\
            ret = API_INVALID_PARAM;\
            break;\
        }


//用于遍历数据Json
#define EXIST_GETVALUE_A(rvalue,key) do{\
    if (portList[i].HasMember(key) && portList[i][key].IsString())\
    {\
        rvalue= portList[i][key].GetString();\
    }\
}while(0)

#define EXIST_GETVALUE(key) if (jData.HasMember(#key) && jData[#key].IsString())\
        {\
            key = jData[#key].GetString();\
        }

#define EXIST_GETVALUE_EX(keyname, key) if (jData.HasMember(keyname) && jData[keyname].IsString())\
        {\
            key = jData[keyname].GetString();\
        }


#define EMPTYSETVALUE(key,value) key=((key).empty() ?(value):(key))

#define __TRY__        try{
#define __CATCH__    }catch(exception& ex) \
    { \
        TLOGERROR(FILE_FUN << "exception: " << ex.what() << endl); \
    } \
    catch(...) \
    { \
        TLOGERROR(FILE_FUN << "unknown exception" << endl); \
    }

#define __CATCHEXT__    }catch(exception& ex) \
    { \
        errMsg += string(ex.what());\
        TLOGERROR(FILE_FUN << "exception: " << ex.what() << endl); \
    } \
    catch(...) \
    { \
        errMsg += "unknown exception";\
        TLOGERROR(FILE_FUN << "unknown exception" << endl); \
    }

#define STARTTIME int64_t startMs = TNOWMS;

#define USETIME "|time use:" << TC_Common::tostr(TNOWMS - startMs) << "(ms)"

#define ISFAILURE(x) (!(x))
#define ISSUCC(x)  ((x)==0)
#define DAY_IN_SECONDS (60 * 60 * 24)
#define ROUND(x)    ((long)round((x)))
#define MAX(x, y)    ((x) > (y) ? (x) : (y))
#define MIN(x, y)    ((x) < (y) ? (x) : (y))

#define BUILDDOT3KEY(a, b, c)        string((a) + "." + (b) + "." + (c))
#define BUILDDOT4KEY(a, b, c, d)    string((a) + "." + (b) + "." + (c) + "." + (d))
#define MSTIMEINSTR(x) (TC_Common::tm2str((x)/1000, "%Y-%m-%d %H:%M:%S"))

template<typename T>
inline string display(const T& t)
{
    stringstream stream;
    stream.str("");
    t.displaySimple(stream);
    return(stream.str());
}

namespace tars
{
    template <>
    inline string TC_Common::tostr(const set<string>&t)
    {
        return(TC_Common::tostr(t.begin(), t.end(), ","));
    }


    template <>
    inline string TC_Common::tostr(const set<int>&t)
    {
        return(TC_Common::tostr(t.begin(), t.end(), ","));
    }

    template <>
    inline string TC_Common::tostr(const set<long>&t)
    {
        return(TC_Common::tostr(t.begin(), t.end(), ","));
    }

    template <>
    inline string TC_Common::tostr(const set<double>&t)
    {
        return(TC_Common::tostr(t.begin(), t.end(), ","));
    }
}

inline string toStr(const Tseer::RouterDataRequest &stReq)
{
    return  stReq.node_name+ "|"+ stReq.container_name;
}

inline string toStr(const Tseer::RouterData & stReq)
{

    return  stReq.node_name+ "|"+ stReq.container_name;
}

inline uint32_t stringIpToInt(const std::string& sip)
{
    string      ip1, ip2, ip3, ip4;
    uint32_t    dip, p1, p2, p3;
    dip             = 0;
    p1              = sip.find('.');
    p2              = sip.find('.', p1 + 1);
    p3              = sip.find('.', p2 + 1);
    ip1             = sip.substr(0, p1);
    ip2             = sip.substr(p1 + 1, p2 - p1 - 1);
    ip3             = sip.substr(p2 + 1, p3 - p2 - 1);
    ip4             = sip.substr(p3 + 1, sip.size() - p3 - 1);
    (((unsigned char*)&dip)[0]) = TC_Common::strto<unsigned int>(ip1);
    (((unsigned char*)&dip)[1]) = TC_Common::strto<unsigned int>(ip2);
    (((unsigned char*)&dip)[2]) = TC_Common::strto<unsigned int>(ip3);
    (((unsigned char*)&dip)[3]) = TC_Common::strto<unsigned int>(ip4);
    return(htonl(dip));
}
inline string Ip2Str(uint32_t ip)
{
    char        str[50];
    unsigned char   *p = (unsigned char*)&ip;
    sprintf(str, "%u.%u.%u.%u", p[3], p[2], p[1], p[0]);
    return(string(str));
}


inline string Ip2StarStr(uint32_t ip)
{
    char        str[50];
    unsigned char   *p = (unsigned char*)&ip;
    sprintf(str, "%u.%u.%u.*", p[3], p[2], p[1]);
    return(string(str));
}

template<class Compare>
inline bool needUpdate(const string& oldVer, const string& newVer,Compare f)
{
    /**
      * 版本号转换成数值大小比较,格式v2.02
      */
     vector<string> oldVerList = TC_Common::sepstr<string>(oldVer, ".");
     vector<string> newVerList = TC_Common::sepstr<string>(newVer, ".");

     if(oldVerList.size() != 2 || newVerList.size() != 2)
     {
         return false;
     }

     string masterOldVer = oldVerList[0].substr(1, oldVerList[0].length() - 1);
     string masterNewVer = newVerList[0].substr(1, newVerList[0].length() - 1);

     if(!TC_Common::isdigit(masterOldVer) || !TC_Common::isdigit(oldVerList[1])
             || !TC_Common::isdigit(masterNewVer) || !TC_Common::isdigit(newVerList[1]))
     {
         return false;
     }

     int masterOldV = TC_Common::strto<int>(masterOldVer);
     int masterNewV = TC_Common::strto<int>(masterNewVer);
     if(masterOldV == masterNewV)
     {
         //subvesion
         int subOldV = TC_Common::strto<int>(oldVerList[1]);
         int subNewV = TC_Common::strto<int>(newVerList[1]);
         if(f(subOldV,subNewV))
         {
              return true;
         }
     }
     else
    {
         //master version
         if(f(masterOldV,masterNewV))
         {
             return true;
         }
    }
     return false;

}

// get default network interface
inline bool get_default_if(char *iface, int size) {
    if (size < 8) return false;
    memset(iface, 0, size);

    FILE *f = fopen("/proc/net/route", "r");
    if (!f) return false;

    char dest[64]  = {0, };
    while (!feof(f)) {
        if (fscanf(f, "%s %s %*[^\r\n]%*c", iface, dest) != 2) continue;
        if (strcmp(dest, "00000000") == 0) {
            break;
        }
    }

    return strlen(iface) ? true : false;
}

// get eth0's ipv4 address
inline bool get_ip(const char *iface, char* ip, int size) {
    if (size < INET_ADDRSTRLEN) return false;
    memset(ip, 0, size);

    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if ((ifa->ifa_addr->sa_family == AF_INET) && (strcmp(ifa->ifa_name,iface) == 0)) {
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
        }
    }

    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);

    return strlen(ip) ? true : false;
}

#endif