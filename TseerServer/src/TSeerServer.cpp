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

#include "TSeerServer.h"
#include "reg_roll_logger.h"
#include "StoreProxy.h"
#include "ApiRegImp.h"
#include "util.h"
#include "util/tc_option.h"
#include "UpdateThread.h"

/**
 * 若要修改代码发布，请修改此版本号。
 * 升级时，主版本号与副版本号均转换成数值大小比较。
 */
#define TSEERSERVER_VER "v0.02"
/**
 * OS_VERSION 在编译时指定
 */
#define TSEERSERVER_VERSION "TseerServer_" OS_VERSION "_" TSEERSERVER_VER

TSeerServer g_app;
TC_Config * g_pconf;

using namespace std;
using namespace Tseer;

struct HttpProtocol
{
public:


    /**
     * 解析http请求
     * @param in
     * @param out
     *
     * @return int
     */
    static int parseHttp(string&in, string&out)
    {
        try
        {
            /* 判断请求是否是HTTP请求 */
            bool    b = TC_HttpRequest ::checkRequest(in.c_str(), in.length());
            APIIMP_LOG <<FILE_FUN<< "in coming-----------------------" <<   b << endl;
            /* 完整的HTTP请求 */
            if(b)
            {
                APIIMP_LOG <<FILE_FUN << in.substr(0, in.find("\r\n\r\n")) << endl;
                out = in;
                in  = "";
                return(TC_EpollServer::PACKET_FULL);
            }
            else{
                return(TC_EpollServer::PACKET_LESS);
            }
        }
        catch(exception&ex)
        {
            TLOGERROR(FILE_FUN << "error:" << ex.what() << endl);
            return(TC_EpollServer::PACKET_ERR);
        }

        return(TC_EpollServer::PACKET_LESS); /* 表示收到的包不完全 */
    }
};


void TSeerServer::initialize()
{
    try
    {
        //滚动日志也打印毫秒
        TarsRollLogger::getInstance()->logger()->modFlag(TC_DayLogger::HAS_MTIME);

        RollLoggerManager::getInstance()->setLogInfo(ServerConfig::Application,
                ServerConfig::ServerName, ServerConfig::LogPath,
                ServerConfig::LogSize, ServerConfig::LogNum);
        
        extern TC_Config * g_pconf;
        if(STOREPROXY->init(g_pconf) != 0)
        {
            cerr << "STOREPROXY initialize exception"<< endl;
            exit(-1);
        }

        
        /* 
          加载registry对象的端口信息
         */
        loadServantEndpoint();

        //注册registry各个obj的基础服务信息
        if(registerSelf() != 0)
        {
            cerr << "Register self to store failed!" << endl;
            exit(-1);
        }
        
        _installScript = "";
        loadInstallScript();

        //启动线程
        UPDATETHREAD->start();

        
        /* 轮询线程 */
        _reapThread.init();
        _reapThread.start();

        /* 供agent访问的对象 */
        addServant<RegistryImp>(TSEERSERVER_REGISTRYOBJ);
        
        /* 供服务client查询路由数据的对象 */
        addServant<QueryImp>(TSEERSERVER_QUERYOBJ);

        /* 供业务注册和查询路由管理信息的接口,HTTP+JSON API*/
        addServant<ApiRegImp>(TSEERSERVER_APIOBJ);
        addServantProtocol(TSEERSERVER_APIOBJ, &HttpProtocol::parseHttp);
    }
    catch(TC_Exception& ex)
    {
        TLOGERROR(FILE_FUN<<"initialize exception:" << ex.what() << endl);
        cerr << "initialize exception:" << ex.what() << endl;
        exit(-1);
    }

    TLOGDEBUG(FILE_FUN<<"initialize OK!" << endl);
}

void TSeerServer::loadInstallScript()
{
    try
    {
        string sScriptFile = "installscript.tgz";
        string sFilePath = TC_File::simplifyDirectory(ServerConfig::BasePath + "/" + sScriptFile);

        //加载文件到内存
        ifstream inFile;
        inFile.open(sFilePath.c_str(), ios::in);
        if (!inFile.is_open())
        {
            cerr<<FILE_FUN << "|open file fail, file name|" << sFilePath << endl;
            exit(1);
        }
        else
        {
            //读取成功 更新到缓存
            istreambuf_iterator<char> beg(inFile), end;
            _installScript.assign(beg, end);
        }
        inFile.close();
    }catch(exception& ex)
    {
        cerr<<FILE_FUN << "|open file exception:"<<ex.what()<< endl;
        exit(1);
    }
}

const string& TSeerServer::getSeerInstallScript() const
{
    return _installScript;
}
void TSeerServer::destroyApp()
{
    TLOGDEBUG(FILE_FUN<<"ok" << endl);
}

int TSeerServer::loadServantEndpoint()
{
    map<string, string> mapAdapterServant;
    mapAdapterServant = ServantHelperManager::getInstance()->getAdapterServant();

    map<string, string>::iterator iter;
    for(iter = mapAdapterServant.begin(); iter != mapAdapterServant.end(); iter++)
    {
        TC_Endpoint ep = getEpollServer()->getBindAdapter(iter->first)->getEndpoint();

        _mapServantEndpoint[iter->second] = ep.toString();

        TLOGDEBUG(FILE_FUN<<"registry obj: " << iter->second << " = " << ep.toString() << endl);
    }

    return(0);
}


TC_Endpoint TSeerServer::getAdapterEndpoint(const string& name) const
{
    TC_Endpoint locator;
    try
    {
        locator = getEpollServer()->getBindAdapter(name)->getEndpoint();
    }catch(exception& ex)
    {
        TLOGERROR(FILE_FUN << " exception: " << ex.what() << endl);
    }

    return(locator);
}

int TSeerServer::registerSelf()
{
    try
    {
        TC_Endpoint locator;
        locator.parse(_mapServantEndpoint[TSEERSERVER_QUERYOBJ]);
        string locatorId = locator.getHost() + ":" + TC_Common::tostr<int>(locator.getPort());

        return STOREPROXY->addBaseServiceInfo(locatorId, _mapServantEndpoint);
    }catch(exception& ex)
    {
        TLOGERROR(FILE_FUN << " exception: " << ex.what() << endl);
    }
    return -1;
}

void useAge()
{
        cerr<<"Usage: TSeerServer [OPTIONS]"<<endl;
        cerr<<endl;
        cerr<<"Options:" <<endl;
        cerr<<"--help"<<endl;
        cerr<<"--version"<<endl;
        cerr<<"--config=<path>\t the config file"<<endl;
        exit(0);    
}

int parseConfig(int argc, char *argv[])
{
    TC_Option tOp;
    string configFile = "";
    tOp.decode(argc, argv);
    if (tOp.hasParam("version"))
    {
        std::cout << "Tseer Server version: " << TSEERSERVER_VERSION << endl;
        exit(0);
    }
    if (tOp.hasParam("help"))
    {
        useAge();
    }
    
    if (tOp.hasParam("config"))
    {
        configFile = tOp.getValue("config");
    }
    else
    {
        useAge();
    }

    //读取配置文件
    if(!TC_File::isFileExistEx(configFile))
    {
        cerr << "Config file " + configFile + " don't exist!" << endl;
        cerr << "Please start SeerServer by 'SeerServer --config=<path>'" << endl;
        useAge();
        exit(-1);
    }

    TC_Config conf;
    try
    {
        conf.parseFile(configFile);
    }catch(exception& ex)
    {
        cerr<<"Config file has invalid content,"<<ex.what()<<endl;
        useAge();
        exit(-1);
    }

    string installPath = conf.get("/server<installpath>", "/usr/local");
    installPath += ("/" + TSEER_APPNAME + "/" + TSEER_SERVERNAME); 

    string store = TC_Common::lower(conf.get("/server<store>", "etcd"));
    if(store == "mysql")
    {
        #ifndef USE_MYSQL
            cerr << "You didn't complie TSeerServer by \"USE_MYSQL=ON\"" << endl;
            cerr << "If you want to use mysql, please recompile TSeerServer by typing \"cmake -DUSE_MYSQL=ON\"" <<endl;
            exit(0);
        #endif
    }

    string sInnerIp = conf.get("/server<localip>", "127.0.0.1");
    string regPort = conf.get("/server<regport>", "9902");
    string queryPort = conf.get("/server<queryport>", "9903");
    string apiPort = conf.get("/server<apiport>", "9904");

    g_app.g_installPath = installPath;
    
    std::string sConfPath = TC_File::simplifyDirectory(installPath + "/conf");
    if (!TC_File::isFileExistEx(sConfPath, S_IFDIR))
    {
        bool mkResult = TC_File::makeDirRecursive(sConfPath);
        if(!mkResult)
        {
            cerr<<"mkdir \""<<sConfPath<<"\" failure,errno," << strerror(errno)<<endl;
            exit(-1);
        }
    }
    
    g_app.g_configFile = sConfPath + "/." + TSEER_SERVERNAME + ".conf";
    
    TC_Config tNewConf;
    map<string, string> m;
    
    //server config
    m["app"]= TSEER_APPNAME;
    m["server"]= TSEER_SERVERNAME;
    m["localip"]= sInnerIp;
    // #服务的可执行文件
    m["basepath"]= TC_File::simplifyDirectory(installPath + "/bin/");
    //服务的数据目录
    m["datapath"]= TC_File::simplifyDirectory(installPath + "/data");
    m["logpath"]= TC_File::simplifyDirectory(installPath + "/app_log");
    m["logLevel"]= conf.get("/server<logLevel>", "DEBUG");
    //滚动日志大小和个数
    m["logsize"]= TC_Common::tostr(1024*1024*15);
    m["closecout"] = "0";
    m["store"] = store;
    tNewConf.insertDomainParam( "/tars/application/server", m, true);

    //servant config
    m.clear();
    m["endpoint"]= "tcp -h  "+ sInnerIp +" -p " + queryPort + " -t 50000";
    m["maxconns"]= "409600";
    m["threads"]= "8";
    m["queuecap"]= "10000";
    m["protocol"] = "tars";
    m["queuetimeout"] = "60000";
    m["servant"] = TSEERSERVER_QUERYOBJ;
    m["allow"] = "";
    m["handlegroup"] = "QueryAdapter";
    tNewConf.insertDomainParam( "/tars/application/server/QueryAdapter", m, true);

    m.clear();
    m["endpoint"]= "tcp -h  "+ sInnerIp +" -p " + regPort + " -t 30000";
    m["maxconns"]= "409600";
    m["threads"]= "8";
    m["queuecap"]= "10000";
    m["protocol"] = "tars";
    m["queuetimeout"] = "60000";
    m["servant"] = TSEERSERVER_REGISTRYOBJ;
    m["allow"] = "";
    m["handlegroup"] = "RegistryAdapter";
    tNewConf.insertDomainParam( "/tars/application/server/RegistryAdapter", m, true);

    m.clear();
    m["endpoint"]= "tcp -h  "+ sInnerIp +" -p " + apiPort + " -t 52000";
    m["maxconns"]= "409600";
    m["threads"]= "5";

    m["queuecap"]= "10000";
    m["protocol"] = "not_tars";
    m["queuetimeout"] = "60000";
    m["servant"] = TSEERSERVER_APIOBJ;
    m["allow"] = "";
    m["handlegroup"] = "ApiRegObjAdapter";
    tNewConf.insertDomainParam( "/tars/application/server/ApiRegObjAdapter", m, true);

    m.clear();
    //client config
    m["locator"] = TSEERSERVER_QUERYOBJ + "@tcp -h  "+ sInnerIp +" -p " + queryPort;
    m["modulename"] = TSEERSERVER_NAME;
    m["sync-invoke-timeout"] = "5000";
    m["async-invoke-timeout"] = "20000";
    m["timeout-queue-size"] = "100";
    m["sendthread"] = "1";
    m["asyncthread"] = "6";
    tNewConf.insertDomainParam( "/tars/application/client", m, true);

    if(store == "mysql")
    {
        m.clear();
        m["dbhost"] = conf.get("/mysql<dbhost>", "127.0.0.1");
        m["dbuser"] = conf.get("/mysql<dbuser>", "root");
        m["dbpass"] = conf.get("/mysql<dbpass>", "");
        m["dbname"] = conf.get("/mysql<dbname>", "seer");
        m["charset"] = conf.get("/mysql<charset>", "utf8");
        m["dbport"] = conf.get("/mysql<dbport>", "3306");
        tNewConf.insertDomainParam( "/tars/application/mysql", m, true);
    }
    else
    {
        m.clear();
        m["host"] = conf.get("/etcd<host>", "127.0.0.1:2379");
        tNewConf.insertDomainParam( "/tars/application/etcd", m, true);
    }
    
    if(TC_File::isFileExistEx(g_app.g_configFile))
    {
        TC_Config oldConfig;
        oldConfig.parseFile(g_app.g_configFile);
        //备份
        std::string oldConfigBak = g_app.g_configFile + ".bak";
        TC_File::save2file(oldConfigBak, oldConfig.tostr());
    }
    
    //新的配置生成配置文件
    string sNewConfigContent = TC_Common::replace(tNewConf.tostr(),"\\s"," ");
    TC_File::save2file(g_app.g_configFile,sNewConfigContent);
    return 0;
}

int main(int argc, char *argv[])
{
    try
    {
        if (parseConfig(argc, argv) == 0)
        {
            int myArgc = 2;
            string sConfigFile = "--config=" + g_app.g_configFile;
            char** myArgv = static_cast<char**>(malloc(( myArgc + 1 ) * sizeof(char*)));
            myArgv[0] = argv[0];
            //Tars服务启动需要的配置信息
            myArgv[1] = strdup(sConfigFile.c_str());
            myArgv[2] = 0;
            g_pconf =  & g_app.getConfig();

            g_app.main(myArgc, myArgv);
            g_app.waitForShutdown();
        }
    }
    catch(exception &ex)
    {
        cerr<< ex.what() << endl;
    }
    return 0;
}
