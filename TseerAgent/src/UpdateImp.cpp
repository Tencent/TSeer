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

#include "UpdateImp.h"
#include "util.h"
#include "TseerAgentServer.h"
#include "RollLogger.h"

void UpdateImp::initialize()
{
}

int UpdateImp::updateConfig(const Tseer::AgentConfig & cfg,std::string &result,tars::TarsCurrentPtr current)
{
    /**
     * 客户端代理更新node的locator，实现方式: 通过更新execute.sh启动脚本里node的启动参数，
     *   然后使用脚本重启自身，达到node自更新的效果。
     *
     * 这个接口的本质功能是修改locator代理的值然后重启下。
     */
    int ret = -1;
    TSEER_LOG(UPDATE_LOG)->debug() << FILE_FUN << display(cfg) << "|coming " << current->getIp() << endl;
    do
    {
        try
        {
            /* 更新tarsnode自身的locator */
            if(cfg.locator != "")
            {
                if(!g_app.restartSelf(cfg, result, current))
                {
                    TSEER_LOG(UPDATE_LOG)->error() << FILE_FUN << result << endl;
                    break;
                }
            }
            else{
                result = "invalid locator param";
                TSEER_LOG(UPDATE_LOG)->error() << FILE_FUN << result << endl;
                break;
            }
            TLOGERROR(FILE_FUN << " update all servers config succ" << endl);
            ret = 0;
        }
        catch(exception& ex)
        {
            result = string(ex.what());
            TSEER_LOG(UPDATE_LOG)->error() << FILE_FUN << result << endl;
        }
    }
    while(0);

    return(ret);
}

bool cleanup_node_dir(long arg)
{
    string cmdStr = "";

    cmdStr += "sleep 5; "; /* 睡眠一下，等主进程正常退出 */
     /* 保险措施，万一主进程卡住了，这里强制干掉 */
    cmdStr += TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME+"/util/stop.sh") + " >> /tmp/cleanup_agent.txt; "; 
    cmdStr += "rm -rf " +TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME);

    popen_sendMsg(cmdStr);
    return true;
}
extern void doAnythingForSelf(bool (*cb)(long arg), long arg);
tars::Int32 UpdateImp::uninstall(std::string &result,tars::TarsCurrentPtr current)
{
    /*
    * 1 清除crontab
    * 2 停止自监控脚本
    * 3 停止服务
    * 4 删除目录结构
    */

    ostringstream os;
    string cmdScript;

    os.str("");
    os << "#!/bin/sh" << endl;
    os << "crontab -l > ./crontab.tmp && " << endl;
    os << "sed -i \'/mon_TseerAgent.sh/d\' ./crontab.tmp && " << endl;
    os << "crontab ./crontab.tmp && " << endl;
    os << "rm -f ./crontab.tmp" << endl;
    os << "chmod -x " + TC_File::simplifyDirectory(g_app.g_installPath + "/"+TSEERAGENT_APPNAME+ "/" + TSEERAGENT_SERVERNAME+"/bin/" +TSEERAGENT_SERVERNAME)<< endl;
    cmdScript = "/tmp/cleanup_agent.sh";
    TC_File::save2file(cmdScript, os.str());
    if(!TC_File::canExecutable(cmdScript))
    {
        TC_File::setExecutable(cmdScript, true);
    }
    popen_sendMsg(cmdScript);
    unlink(cmdScript.c_str());
    unlink("crontab.tmp");

    if(current)
        Tseer::Update::async_response_uninstall(current, 0, "succ");

    try
    {
        Application::terminate();
    }
    catch(...)
    {
        TSEER_LOG("uninstall")->debug() << FILE_FUN << "Application::terminate() exception" << endl;
    }

    TSEER_LOG("uninstall")->debug() << FILE_FUN << "uninstall over" << endl;

    doAnythingForSelf(cleanup_node_dir, 0);
    return 0;    
}

