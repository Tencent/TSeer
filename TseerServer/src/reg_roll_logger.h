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

#ifndef __TSEER_ROLL_LOGGER_H__
#define __TSEER_ROLL_LOGGER_H__

#include "util/tc_logger.h"
#include "servant/TarsLogger.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * 利用Tarslogger重新封装，主要是为了支持多个滚动日志
 * 本地日志帮助类, 单件
 * 循环日志单件是永生不死的, 保证任何地方都可以使用
 * 当该对象析够以后, 则直接cout出来
 */
class RollLoggerManager :
        public TC_ThreadLock,
        public TC_Singleton<RollLoggerManager, CreateUsingNew, DefaultLifetime>
{
public:
    typedef TC_Logger<RollWriteT, TC_RollBySize> RollLogger;

    RollLoggerManager();

    ~RollLoggerManager();
    /**
     * 设置本地信息
     * @param app, 业务名称
     * @param server, 服务名称
     * @param logpath, 日志路径
     * @param maxSize, 文件最大大小,字节
     * @param maxNum, 文件最大数
     */
    void setLogInfo(const string &app, const string &server, const string &logpath, int maxSize = 1024*1024*50, int maxNum = 10);

    /**
     * 设置同步写日志
     *
     * @param sync
     */
    void sync(RollLogger *pRollLogger, bool sync = true);

    /**
     * 获取循环日志
     *
     * @return RollLogger
     */
    RollLogger *logger(const string &file);

private:
    void initRollLogger(RollLogger *pRollLogger, const string &file, const string &format);

protected:

    /**
     * 应用
     */
    string                  _App;

    /**
     * 服务名称
     */
    string                  _Server;

    /**
     * 日志路径
     */
    string                  _Logpath;

    int                        _MaxSize;
    int                        _MaxNum;
    
    /**
     * 本地线程组
     */
    TC_LoggerThreadGroup    _local;

    /**
     * 记录日志文件
     */
    map<string, RollLogger*>    _loggers;

};

/**
 * 循环日志
 */
#define TSEER_LOG(x)             (RollLoggerManager::getInstance()->logger(x))
#endif

