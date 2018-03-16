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

#ifndef _UPDTAE_THREAD_H_
#define _UPDTAE_THREAD_H_

#include "util/tc_thread.h"
#include "util/tc_monitor.h"
#include "util/tc_singleton.h"
#include "Registry.h"


struct PackageData
{
    string ostype;
    string version;
    string dataBuff;
    string packageName;
    string md5;
};

using namespace Tseer;

class UpdateThread : public TC_Thread, public TC_ThreadLock,public TC_Singleton<UpdateThread>
{
public:
    UpdateThread();
    ~UpdateThread();

    void run();

    void checkChange();

    bool getFormalPackageData(const string &ostype, PackageData &data);

    bool getGrayPackageData(const string &ostype, PackageData &data);

    bool getFormalInfo(const PackageInfo &reqInfo, PackageInfo &rspInfo);

    bool getGrayInfo(const PackageInfo &reqInfo, PackageInfo &rspInfo);
private:
    bool update(const PackageInfo &info);
private:
    bool _terminate;
    
    /**
     * 定期检查是否有新版本的时间间隔
     */
    time_t _interval;

    /*
    * agent通过自身的操作系统版本号，匹配对应的包信息
    * 每个操作系统只有一个正式版本和一个灰度版本 ,
    * wget拉数据使用了操作系统版本 故用ostype做key xxInfo和xxPacket是对应的
    */
    map<string, PackageInfo> _formalAgentInfoList;                        //key:ostype; value:packetInfo

    map<string, PackageInfo> _grayAgentInfoList;                    //agent需要灰度的节点  key:node_ip; value:packetInfo

    /**
     * 操作系统对应的正式发布包数据
     */
    map<string, PackageData> _formalAgentPackage;                        //key:ostype; value:packet
    /**
      *操作系统对应的灰度发布包数据
      */
    map<string, PackageData> _grayAgentPackage;                    //key:osttpe; value:packet
};

#define UPDATETHREAD (UpdateThread::getInstance())

#endif

