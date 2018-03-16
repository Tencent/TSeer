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

#ifndef _NODE_SELF_UPDATE_THREAD_H__
#define _NODE_SELF_UPDATE_THREAD_H__

#include <map>
#include <string>
#include "util/tc_thread.h"
#include "util/tc_monitor.h"
#include "Registry.h"


typedef bool (*upd_done_cb)(const string&);
struct PacketData
{   
    Tseer::PackageInfo verMd5;
    upd_done_cb done_cb;
    bool        trigger;
};

class SelfUpdateThread : public TC_Thread, public TC_ThreadLock
{
public:
    SelfUpdateThread(bool autoUpdate, string path);
    ~SelfUpdateThread(){};
    int init();
    void run();
    void terminate();
private:
    /**
      * 下载新的发布包
      */
    bool download(const string &dstPath, const Tseer::PackageInfo &info);
    /**
      * 检查agent是否有新的发布包
      */
    bool needUpdate(const Tseer::PackageInfo &reqInfo, const Tseer::PackageInfo &rspInfo);
private:
    map<string, PacketData> _mPacketInfo; /* fullpath, packetdata */

    Tseer::RegistryPrx    _updtePrx;
    bool    _terminate;

    time_t        _tInterval;

    //是否允许自动更新，默认为true
    bool        _autoUpdate;
    string      _serName;
    string      _servVersion;
    string      _osVersion;
    string      _path;
    string      _packageName;
};


#endif

