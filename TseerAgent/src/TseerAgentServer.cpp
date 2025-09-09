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

#include "util/tc_option.h"
#include "util/tc_file.h"
#include "util.h"

#include "TseerAgentServer.h"
#include "RouterImp.h"
#include "HeartBeatReportThread.h"
#include "UpdateImp.h"
#include "RollLogger.h"
#include "CacheManager.h"

using namespace std;

/**
 * 若要修改代码发布，请修改此版本号。
 * 升级时，主版本号与副版本号均转换成数值大小比较。
 */
#define AGENTVER "v0.01"                                                /* 格式为: `v主版本.副版本` */

#define TSEERAGENT_VERSION "TseerAgent_" OS_VERSION "_" AGENTVER     /* 不可修改 */



TseerAgentServer g_app;

void TseerAgentServer::initialize()
{
    //滚动日志也打印毫秒
    TarsRollLogger::getInstance()->logger()->modFlag(TC_DayLogger::HAS_MTIME);

    RollLoggerManager::getInstance()->setLogInfo(ServerConfig::Application,
            ServerConfig::ServerName, ServerConfig::LogPath,
            ServerConfig::LogSize, ServerConfig::LogNum);
    
    addServant<RouterImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".RouterObj");

    //agent升级管理obj
    if(!g_app.g_innerIp.empty())
    {
        addServant<UpdateImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".UpdateObj");
    }

    tars::TC_Config &conf = g_app.getConfig();
    _checkTimeoutInfo.minTimeoutInvoke = TC_Common::strto<uint32_t>(conf.get("/tars/tarsagent<minTimeoutInvoke>", "2"));
    _checkTimeoutInfo.checkTimeoutInterval = TC_Common::strto<uint32_t>(conf.get("/tars/tarsagent<checkTimeoutInterval>", "30"));
    _checkTimeoutInfo.frequenceFailInvoke = TC_Common::strto<uint32_t>(conf.get("/tars/tarsagent<frequenceFailInvoke>", "5"));
    _checkTimeoutInfo.minFrequenceFailTime = TC_Common::strto<uint32_t>(conf.get("/tars/tarsagent<minFrequenceFailTime>", "5"));
    _checkTimeoutInfo.radio = TC_Common::strto<float>(conf.get("/tars/tarsagent<radio>", "0.5"));
    _checkTimeoutInfo.tryTimeInterval = TC_Common::strto<uint32_t>(conf.get("/tars/tarsagent<tryTimeInterval>", "15"));

    //路由同步线程
    _syncRouterThread = new SyncRouterThread();
    int ret = _syncRouterThread->init();
    if (ret)
    {
        TLOGERROR(FILE_FUN<<"sync router thread error." << endl);
        //assert(ret == 0);
    }
    _syncRouterThread->start();

    //心跳上报线程
    g_ReapHeartBeatReportThread = new HeartBeatReportThread();
    ret = g_ReapHeartBeatReportThread->init(60);
    if (ret)
    {
        TLOGERROR(FILE_FUN<<"heart beart thread error." << endl);
        //assert(ret == 0);
    }
    g_ReapHeartBeatReportThread->start();

    //缓存管理
    g_cacheManager = new CacheManager();
    g_cacheManager->init(5, ServerConfig::DataPath + "routersCache/");

    /* 自更新线程 */
    bool autoUpdate = TC_Common::lower(conf.get("/tars/node<autoupdate>", "false")) == "true";
    string path = TC_File::simplifyDirectory(g_app.g_installPath  + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME);
    _pSelfUpdateThread = new SelfUpdateThread(autoUpdate,path);
    _pSelfUpdateThread->init();
    _pSelfUpdateThread->start();
    TLOGDEBUG(FILE_FUN<<"succ" << endl);
}

string TseerAgentServer::getVersion(string &serName, string &serVer, string &osVer) const
{
    vector<string> v = TC_Common::sepstr<string>(string(TSEERAGENT_VERSION), "_");
    assert(v.size() == 3);

    serName = v[0];
    osVer = v[1];
    serVer = TC_Common::lower(v[2]);
    return TSEERAGENT_VERSION;
}


TC_Endpoint TseerAgentServer::getAdapterEndpoint(const string& name) const
{
    LOG->info() <<      FILE_FUN << name << endl;
    TC_EpollServerPtr   pEpollServerPtr = Application::getEpollServer();
    assert(pEpollServerPtr);
    TC_EpollServer::BindAdapterPtr pBindAdapterPtr = pEpollServerPtr->getBindAdapter(name);
    if(pBindAdapterPtr)
    {
    return(pBindAdapterPtr->getEndpoint());    
    }
    return TC_Endpoint();
}

void TseerAgentServer::destroyApp()
{
    if(_syncRouterThread)
    {
        _syncRouterThread->terminate();
        _syncRouterThread->getThreadControl().join();

        delete _syncRouterThread;
        _syncRouterThread = NULL;

        TLOGDEBUG(FILE_FUN<<"SyncRouterThread destroy." << endl);
    }
    
    if(_pSelfUpdateThread)
    {
        _pSelfUpdateThread->terminate();
        _pSelfUpdateThread->getThreadControl().join();

        delete _pSelfUpdateThread;
        _pSelfUpdateThread = NULL;

        TLOGDEBUG(FILE_FUN<<"SelfUpdateThread destroy." << endl);
    }

}

bool TseerAgentServer::updateStartScript(const Tseer::AgentConfig& cfg, string& result)
{
    try
    {
        string configFile = TC_File::simplifyDirectory(g_app.g_installPath + "/"+TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME +"/conf/"+TSEERAGENT_SERVERNAME + ".conf");
        TC_Config paramConf;
        paramConf.parseFile(configFile);

        TC_Config           newConf;
        map<string, string> m;
        m["locator"] = cfg.locator;
        m["localip"] = cfg.nodeIp;
        newConf.insertDomainParam( "/server", m, true);
        
        paramConf.joinConfig(newConf,true);
        //新的配置生成配置文件
        string sNewConfigContent  = TC_Common::replace(paramConf.tostr(),"\\s"," ");
        TC_File::save2file(configFile,sNewConfigContent);
    
        ostringstream os;
        os << "#!/bin/sh" << endl;
        os << "SEER_BASEDIR="<<TC_File::simplifyDirectory(g_app.g_installPath)<< endl;
        os << "TARGET="<<TSEERAGENT_SERVERNAME<< endl;
        os << endl;
        os << "bin=\"${SEER_BASEDIR}/"<<TSEERAGENT_APPNAME<<"/${TARGET}/bin/${TARGET}\""<<endl;
        os << "conf=\"${SEER_BASEDIR}/"<<TSEERAGENT_APPNAME<<"/${TARGET}/conf/${TARGET}.conf\""<<endl;
        
        os << "PID=`ps -eopid,cmd | grep \"$bin\"| grep \"${TARGET}\" |  grep -v \"grep\"|grep -vw \"sh\" |awk '{print $1}'`" << endl;
        os << "echo $PID" << endl;
        os << endl;
        os << "if [ \"$PID\" != \"\" ]; then" << endl;
        os << "\tkill -9 $PID" << endl;
        os << "\techo \"kill -9 $PID\"" << endl;
        os << "fi" << endl;
        os << endl;
        os << "ulimit -c unlimited" << endl;
        os << "ulimit -a" << endl;
        os << "nohup $bin --config=$conf 2>&1 &"<< endl;
        os << endl;

        string startScript = TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME +"/util/start.sh");
        TC_File::save2file(startScript, os.str());
        if(!TC_File::canExecutable(startScript))
        {
            TC_File::setExecutable(startScript, true);
        }

      ostringstream osstop;
     osstop << "#!/bin/sh" << endl;
        osstop << "SEER_BASEDIR="<<TC_File::simplifyDirectory(g_app.g_installPath)<< endl;
        osstop << "TARGET="<<TSEERAGENT_SERVERNAME<< endl;
        osstop << endl;
        osstop << "bin=\"${SEER_BASEDIR}/"<<TSEERAGENT_APPNAME<<"/${TARGET}/bin/${TARGET}\""<<endl;
        osstop << "PID=`ps -eopid,cmd | grep \"$bin\"| grep \"${TARGET}\" |  grep -v \"grep\"|grep -vw \"sh\" |awk '{print $1}'`" << endl;
        osstop << "echo $PID" << endl;
        osstop << endl;
        osstop << "if [ \"$PID\" != \"\" ]; then" << endl;
        osstop << "\tkill -9 $PID" << endl;
        osstop << "\techo \"kill -9 $PID\"" << endl;
        osstop << "fi" << endl;
        osstop << endl;
      string stopScript = TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME +"/util/stop.sh");
        TC_File::save2file(stopScript, osstop.str());
        if(!TC_File::canExecutable(stopScript))
        {
            TC_File::setExecutable(stopScript, true);
        }
        TLOGDEBUG(FILE_FUN<< endl);
        return true;
    }
    catch(exception& ex)
    {
        result = string(ex.what());
        TLOGERROR(FILE_FUN<<result << endl);
    }
    return(false);
}


void closeAllFds()
{
    vector<string> procPathList;
    TC_File::listDirectory("/proc/self/fd", procPathList, false);
    for(vector<string>::iterator it = procPathList.begin(); it != procPathList.end(); it++)
    {
        string fd = TC_File::extractFileName(*it);
        if(TC_File::isFileExist(*it, S_IFLNK) && TC_Common::isdigit(fd))
        {
            close(atoi(fd.c_str()));
        }
    }
}

void doAnythingForSelf(bool (*cb)(long arg), long arg)
{
    TLOGDEBUG(FILE_FUN<< endl);
    signal(SIGCHLD, SIG_IGN);
    pid_t son = fork();
    if(son < 0)
    {
        TLOGDEBUG(FILE_FUN<<"fork son failed" << endl);
        exit(-1);
    }

    if(son == 0)
    {
        setsid();
        pid_t grandson = fork();
        if(grandson < 0)
        {
            TLOGDEBUG(FILE_FUN<<"fork grandson failed" << endl);
            _exit(-1);
        }
        else if(grandson > 0)
        {
            _exit(0);
        }
        else
        {
            closeAllFds();
            chdir("/tmp");
            umask(0);

            if(cb)
            {
                cb(arg);
            }

            _exit(0);
        }
    }
}

bool stop_then_start(long arg)
{
    string cmd = "sleep 5;";
    cmd += TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME+"/util/stop.sh") +"; sleep 1;";
    cmd += TC_File::simplifyDirectory(g_app.g_installPath + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME+"/util/start.sh");
    popen_sendMsg(cmd);
    return true;
}

bool TseerAgentServer::updateBinOver()
{
    Tseer::AgentConfig cfg;

    TLOGDEBUG(FILE_FUN << "to do Restart" << endl);

    cfg.locator = g_app.getConfig().get("/tars/application/client<locator>");
    cfg.nodeIp = g_app.getConfig().get("/tars/application/server<localip>");
    string result;

    return g_app.restartSelf(cfg, result);
}



bool TseerAgentServer::restartSelf(const Tseer::AgentConfig&cfg, string& result, TarsCurrentPtr current)
{
    updateStartScript(cfg, result);

    TLOGDEBUG(FILE_FUN << cfg.nodeIp <<"|"<<cfg.locator<< endl);

    if(current)
    {
        Update::async_response_updateConfig(current, 0, "succ");
    }
    doAnythingForSelf(stop_then_start, 0);
    return true;
}

void usage()
{
        cerr<<"Usage: "<<TSEERAGENT_SERVERNAME <<" [OPTIONS]"<<endl;
        cerr<<endl;
        cerr<<"Options:" <<endl;
        cerr<<"--help"<<endl;
        cerr<<"--version"<<endl;
        cerr<<"--config=<path>\t the config file"<<endl; 
        exit(-1);
}


int parseConfig(int argc, char *argv[])
{
    TC_Option Op;
    Op.decode(argc, argv);

    if (Op.hasParam("version"))
    {
        std::cout << "SeerAgent version: " << TSEERAGENT_VERSION << std::endl;
        return 1;
    }
    
    string configFile = "";
    if (Op.hasParam("config")) {
        configFile = Op.getValue("config");
    } else {
        usage();
    }

    //获取本机配置的 ip (IPV4)
    char iface[8];
    char ip[INET_ADDRSTRLEN];
    if (!get_default_if(iface, 8) || !get_ip(iface, ip, INET_ADDRSTRLEN)) {
        cerr << FILE_FUN << " get default ip via /proc/net/route failed, using default value instead" << endl;
        memset(ip, 0, INET_ADDRSTRLEN);
    }

    //读取配置文件
    if(!TC_File::isFileExistEx(configFile)) {
        cerr <<configFile << " file don't exist!" << endl;
        exit(-1);
    }

    TC_Config paramConf;
    paramConf.parseFile(configFile);

    string sInstallpatch = paramConf.get("/server<installpath>", "");
    if(sInstallpatch.empty()) {
        sInstallpatch = g_app.g_installPath;
    }
    
    //解析主控地址列表
    std::string locator = paramConf.get("/server<locator>", "");
    if (locator.empty()) {
        cerr<<"you should provide locator"<<endl;
        exit(-1);
    }
    if(locator.find("@") == string::npos) {
        string tmpList;
        vector<string> ipPortlist = TC_Common::sepstr<string>(locator,"|;");
        for (size_t i = 0; i < ipPortlist.size(); i++) {
            vector<string> ipPort = TC_Common::sepstr<string>(ipPortlist[i],":");
            if(ipPort.size() < 2) {
                cerr<<FILE_FUN<< ipPortlist[i]<<" is invalid"<<endl;
                continue;
            }
            
            string endPoint= "tcp -h " + ipPort[0] + " -p " + ipPort[1];
            if (tmpList != "") {
                tmpList += ":" + endPoint;
            } else {
                tmpList = endPoint;
            }
        }

        if(tmpList.empty()) {
            cerr<<"invalid locator param:"<<locator<<endl;
            exit(-1);
        }
        locator = TSEERSERVER_MODULE + ".QueryObj@"+tmpList;
    }

    //主要是要注册到路由中心方便云端管理，所以这里必须是内网ip
    std::string innerIp = paramConf.get("/server<localip>", ip);
    
    //默认是绑定本地127.0.0.1,提供给api访问
    std::string routerIp = paramConf.get("/server<routerip>", "127.0.0.1");
    if(routerIp.empty()) {
        //监听127.0.0.1
        routerIp = "127.0.0.1";
    }

    std::string confPath = TC_File::simplifyDirectory(sInstallpatch + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME+"/conf");
    if (!TC_File::isFileExistEx(confPath, S_IFDIR))
    {
        bool mkResult = TC_File::makeDirRecursive(confPath);
        if(!mkResult)
        {
            cerr<<" mkdir \""<<confPath<<"\" failure,errno," << strerror(errno)<<endl;
            exit(-1);
        }
    }
    
    g_app.g_installPath = sInstallpatch;
    g_app.g_configFile  = confPath + "/."+TSEERAGENT_SERVERNAME + ".conf";
    g_app.g_innerIp     = innerIp;
    
    TC_Config           newConf;
    map<string, string> m;
    
    string sModuleName = TSEERAGENT_APPNAME + "." + TSEERAGENT_SERVERNAME;

    //server config
    m["app"]      = TSEERAGENT_APPNAME;
    m["server"]   = TSEERAGENT_SERVERNAME;
    m["localip"]  = innerIp;
    // #服务的可执行文件
    m["basepath"] = TC_File::simplifyDirectory(sInstallpatch + "/"+TSEERAGENT_APPNAME+ "/" + TSEERAGENT_SERVERNAME+"/bin/");
    //服务的数据目录
    m["datapath"] = TC_File::simplifyDirectory(sInstallpatch + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME+"/data");
    m["logpath"]  = TC_File::simplifyDirectory(sInstallpatch + "/" + TSEERAGENT_APPNAME + "/" + TSEERAGENT_SERVERNAME +"/app_log");
    m["logLevel"] = paramConf.get("/server<logLevel>", "DEBUG");
    //滚动日志大小和个数
    m["logsize"]  = TC_Common::tostr(1024*1024*15);
    m["closecout"] = "1";
    newConf.insertDomainParam( "/tars/application/server", m, true);

    //servant config
    m.clear();
    m["endpoint"] = "udp -h " + routerIp +" -p 8865 -t 60000";
    m["maxconns"] = "10000";
    m["threads"]  = "8";
    m["queuecap"] = "10000";
    m["protocol"] = "tars";
    m["queuetimeout"] = "60000";
    m["servant"]  = sModuleName + ".RouterObj";
    m["allow"]    = "";
    m["handlegroup"] = "RouterObjAdapter";
    newConf.insertDomainParam( "/tars/application/server/RouterObjAdapter", m, true);

    if(!g_app.g_innerIp.empty())
    {
        m.clear();
        m["endpoint"] = "tcp -h " + g_app.g_innerIp +" -p 9765 -t 60000";
        m["maxconns"] = "10000";
        m["threads"]  = "5";
        m["queuecap"] = "10000";
        m["protocol"] = "tars";
        m["queuetimeout"] = "60000";
        m["servant"]  = sModuleName + ".UpdateObj";
        m["allow"]    = "";
        m["handlegroup"] = "UpdateObjAdapter";
        newConf.insertDomainParam( "/tars/application/server/UpdateObjAdapter", m, true);
    }

    m.clear();
    //client config
    m["locator"] = locator;
    m["modulename"] =sModuleName;
    m["sync-invoke-timeout"] = "5000";
    m["async-invoke-timeout"] = "20000";
    m["timeout-queue-size"] = "100";
    m["sendthread"] = "1";
    m["asyncthread"] = "3";
    //m["stat"]= "tars.tarsStat.StatObj";
    //m["property"]= "tars.tarsproperty.PropertyObj";
    //向路由中心自检更新
    m["update"] =TSEERSERVER_MODULE + ".RegistryObj";
    newConf.insertDomainParam( "/tars/application/client", m, true);

    m.clear();
    //seer update config
    m["autoupdate"]= "true";
    newConf.insertDomainParam( "/tars/node", m, true);

    if(tars::TC_File::isFileExistEx(g_app.g_configFile))
    {
        TC_Config oldConfig;
        oldConfig.parseFile(g_app.g_configFile);
        //备份
        std::string oldConfigBak = g_app.g_configFile + ".bak";
        TC_File::save2file(oldConfigBak, oldConfig.tostr());
    }
    
    //新的配置生成配置文件
    string newConfigContent  = TC_Common::replace(newConf.tostr(),"\\s"," ");
    TC_File::save2file(g_app.g_configFile,newConfigContent);

    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        g_app.g_installPath = "/usr/local/app/";
        if (parseConfig(argc, argv) == 0)
        {
            //only one args ./bin --config=file
            int myArgc = 2;
            string configFile = "--config=" + g_app.g_configFile;
            char** myArgv = static_cast<char**>(malloc(( myArgc + 1 ) * sizeof(char*)));
            myArgv[0] = argv[0];  //bin
            myArgv[1] = strdup(configFile.c_str());
            myArgv[2] = 0;
            g_app.main(myArgc, myArgv);
            g_app.waitForShutdown();
        }
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}
