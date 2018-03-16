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

#include "reg_roll_logger.h"
#include "servant/Communicator.h"


/////////////////////////////////////////////////////////////////////////////////////
RollLoggerManager::RollLoggerManager()
        : _MaxSize(1024 * 1024 * 50), _MaxNum(10)
{
}

RollLoggerManager::~RollLoggerManager()
{

    map<string, RollLogger*>::iterator it = _loggers.begin();
    while(it != _loggers.end())
    {
        delete it->second;
        ++it;
    }
    _loggers.clear();
}

void RollLoggerManager::setLogInfo(const string &app, const string &server, const string &logpath, int maxSize, int maxNum)
{
    _App       = app;
    _Server    = server;
    _Logpath   = logpath;

    _MaxSize = maxSize;
    _MaxNum = maxNum;
    //生成目录
    TC_File::makeDirRecursive(_Logpath + "/" + _App + "/" + _Server);
    _local.start(1);
}


void RollLoggerManager::sync(RollLogger *pRollLogger, bool sync)
{
    if(sync)
    {
        pRollLogger->unSetupThread();
    }
    else
    {
        pRollLogger->setupThread(&_local);
    }
}

RollLoggerManager::RollLogger* RollLoggerManager::logger(const string &file)
{
    Lock lock(*this);
    map<string, RollLogger*>::iterator it = _loggers.find(file);
    if( it == _loggers.end())
    {
        RollLogger *p = new RollLogger();
        //p->modFlag(RollLogger::HAS_MTIME);
        initRollLogger(p, file, "%Y%m%d");
        _loggers[file] = p;
        return p;
    }

    return it->second;
}

void RollLoggerManager::initRollLogger(RollLogger *pRollLogger, const string &file, const string &format)
{

    //初始化本地循环日志
    pRollLogger->init(_Logpath + "/" + _App + "/" + _Server + "/" + _App + "." + _Server + "_" + file, _MaxSize, _MaxNum);
    pRollLogger->modFlag(TC_DayLogger::HAS_TIME, false);
    pRollLogger->modFlag(TC_DayLogger::HAS_MTIME|TC_DayLogger::HAS_LEVEL|TC_DayLogger::HAS_PID, true);

    //设置为异步
    sync(pRollLogger, false);
}
