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

#ifndef __FILEWRITER_FACTORY_H
#define __FILEWRITER_FACTORY_H

#include "util/tc_thread.h"
#include "util/tc_thread_queue.h"
#include "util/tc_autoptr.h"
#include "servant/ServantProxy.h"
#include "util/tc_monitor.h"
#include "util/tc_hash_fun.h"
#include "util/tc_singleton.h"
#include "Registry.h"
#include "StoreProxy.h"

//下载任务
struct FileTask
{
    //原始请求包
    std::string path;
    tars::Int64 size;
    std::string md5;

    //已经写入的长度
    long pos;
    
    //包含本地根路径的文件名
    string filePath;

    //保存包的文件描述符
    FILE *fp;
    
    tars::TC_ThreadRecLock* lock;
    
    /* 进入队列的时间,单位秒 */
    int64_t tInTime;
    FileTask():path(""),pos(0),fp(NULL),lock(NULL){}
};

#define POOLSIZE 50
class FileWriterFactory : public TC_ThreadLock,public TC_Singleton<FileWriterFactory, CreateUsingNew, DefaultLifetime>
{
    /**定义hash处理器*/
    typedef TC_Functor<uint32_t, TL::TLMaker<const string &>::Result> hash_functor;
public:
    FileWriterFactory() ;
public:
    tars::TC_ThreadRecLock* getRecLock(const FileTask& task) {
        size_t hashcode = _hashf(task.filePath);
        size_t index = hashcode % POOLSIZE;
        return &_lockPool[index];
    }
    
    int write(const Tseer::PushPackageInfo & pushPkInfo,string& result);

    int updateAgentInfo(const Tseer::PushPackageInfo & pushPkInfo);

    /*定时巡检是否有超时的task*/
    void timeout();
private:
    tars::TC_ThreadRecLock _lockPool[POOLSIZE];
    hash_functor           _hashf;

    //key=sFilePath
    map<string,FileTask> _taskList;

};
#endif

