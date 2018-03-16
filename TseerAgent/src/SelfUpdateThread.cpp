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

#include "servant/Application.h"
#include "util/tc_md5.h"
#include "util/tc_file.h"
#include "util/tc_common.h"
#include "RollLogger.h"
#include "util.h"
#include "TseerAgentServer.h"
#include "SelfUpdateThread.h"

using namespace tars;
using namespace std;


SelfUpdateThread::SelfUpdateThread(bool autoUpdate, string path)
: _terminate(false)
, _tInterval(1 * 60)
, _autoUpdate(autoUpdate)
, _packageName("router_agent")
{
    _updtePrx = NULL;
    _path = path;

    g_app.getVersion(_serName, _servVersion, _osVersion);
}

void SelfUpdateThread::terminate()
{
    _terminate = true;
}

int SelfUpdateThread::init()
{
    string sUpdateObj = Application::getCommunicator()->getProperty("update");
    if(!sUpdateObj.empty())
    {
        _updtePrx = Application::getCommunicator()->stringToProxy<Tseer::RegistryPrx>(sUpdateObj);
    }

    LOG->debug() << FILE_FUN << "NodeVer: " << _servVersion << endl;

    return(0);
}

bool SelfUpdateThread::needUpdate(const Tseer::PackageInfo &reqInfo, const Tseer::PackageInfo &rspInfo)
{
    /**
     * 版本号转换成数值大小比较
     */

    if(!_autoUpdate)
    {
        return false;
    }

    vector<string> oldVerList = TC_Common::sepstr<string>(reqInfo.version, ".");
    vector<string> newVerList = TC_Common::sepstr<string>(rspInfo.version, ".");

    if(oldVerList.size() != 2 || newVerList.size() != 2)
    {
        LOG->debug() << FILE_FUN << "bad version format" << "|localVer=" << reqInfo.version << "|remoteVer=" << rspInfo.version << endl;
        return false;
    }

    string mReqVer = oldVerList[0].substr(1, oldVerList[0].length() - 1);
    string mRspVer = newVerList[0].substr(1, newVerList[0].length() - 1);

    if(!TC_Common::isdigit(mReqVer) ||
       !TC_Common::isdigit(oldVerList[1]) ||
       !TC_Common::isdigit(mRspVer) ||
       !TC_Common::isdigit(newVerList[1]))
    {
        LOG->debug() << FILE_FUN << "not digits"  << "|localVer=" << reqInfo.version << "|remoteVer=" << rspInfo.version << endl;
        return false;
    }
    
    if(TC_Common::strto<int>(mReqVer) == TC_Common::strto<int>(mRspVer))
    {
        if(TC_Common::strto<int>(oldVerList[1]) < TC_Common::strto<int>(newVerList[1]))
        {
            return true;
        }
    }

    if(TC_Common::strto<int>(mReqVer) < TC_Common::strto<int>(mRspVer))
    {
        return true;
    }

    return false;
}


void SelfUpdateThread::run()
{
    LOG->debug() << FILE_FUN << "running, start Ver: " << _osVersion + "_" + _servVersion
        << ", autoUpdate: " << _autoUpdate << endl;

    while (!_terminate)
    {
        do
        {
            try
            {
                Tseer::PackageInfo reqInfo, rspInfo;

                reqInfo.version = _servVersion;
                reqInfo.packageName = _packageName;
                reqInfo.ostype = _osVersion;
                reqInfo.ip = g_app.g_innerIp;
                reqInfo.netID = "router_agent";

                LOG->debug() << FILE_FUN << "req:" << display(reqInfo)
                    << "|md5=" << reqInfo.md5
                    << "|serVer=" << reqInfo.version
                    << "|osVer=" << reqInfo.ostype
                    << "|filename=" << reqInfo.packageName
                    << endl;
                if (!_updtePrx)
                {
                    LOG->error() << FILE_FUN << "|req:" << display(reqInfo) << "|_updtePrx not init" << endl;
                    break;
                }

                int iRet = _updtePrx->checkChange(reqInfo, rspInfo);
                if (iRet != 0)
                {
                    LOG->error() << FILE_FUN << "|req:" << display(reqInfo) << "|iRet|" << iRet << "|check change fail!" << endl;
                    break;
                }
                LOG->debug() << FILE_FUN << "rsp:" << display(rspInfo) << endl;

                rspInfo.version = TC_Common::lower(rspInfo.version);
                if (needUpdate(reqInfo, rspInfo))
                {
                    LOG->debug() << FILE_FUN << "|req:" << display(reqInfo) << "|rsp:" << display(rspInfo) << "|need update" << endl;
                    if (download(_path, rspInfo) == true)
                    {
                        _servVersion = rspInfo.version;

                        LOG->debug() << FILE_FUN << "update over, newVer: " << _serName + "_" + _osVersion + "_" + _servVersion << endl;
                        g_app.updateBinOver();
                    }
                    else
                    {
                        TSEER_LOG(UPDATE_LOG)->debug() << FILE_FUN << "|req:" << display(reqInfo) << "|rsp:" << display(rspInfo) << "|not need update" << endl;
                        LOG->debug() << FILE_FUN << "|req:" << display(reqInfo) << "|rsp:" << display(rspInfo) << "|not need update" << endl;
                    }
                }
                else
                {
                    TSEER_LOG(UPDATE_LOG)->debug() << FILE_FUN << "|req:" << display(reqInfo) << "|rsp:" << display(rspInfo) << "|not need update" << endl;
                    LOG->debug() << FILE_FUN << "|req:" << display(reqInfo) << "|rsp:" << display(rspInfo) << "|not need update" << endl;
                }
            }
            catch (exception&ex)
            {
                LOG->error() << FILE_FUN << ex.what() << endl;
            }
            catch (...)
            {
                LOG->error() << FILE_FUN << "unknown exception" << endl;
            }
        } while (0);

        {
            TC_ThreadLock::Lock lock(*this);
            timedWait(_tInterval * 1000);
        }
    }
}


bool SelfUpdateThread::download(const string &dstPath, const Tseer::PackageInfo &info)
{
    bool finish = false;

    string tmpDir = tars::TC_File::simplifyDirectory(string("/tmp/") + info.packageName + "_update");
    string tgzFile = tars::TC_File::simplifyDirectory(tmpDir + "/" + TC_Common::now2str() + ".tgz");

    LOG->debug() << FILE_FUN << "tmpDir:" << tmpDir << "|tgzFile:" << tgzFile << "|filename:" << info.packageName << endl;

    LOG->debug() << FILE_FUN << "extract path:" << TC_File::extractFilePath(tmpDir) << endl;

    TC_File::makeDirRecursive(TC_File::extractFilePath(tgzFile));
    if(TC_File::isFileExist(tgzFile))
    {
        TC_File::removeFile(tgzFile, false);
    }

    FILE *fp = fopen(tgzFile.c_str(), "wb");
    if(!fp)
    {
        LOG->error() << FILE_FUN << tgzFile << " can not write" << endl;
        return false;
    }

    Tseer::UpdateReq  req;
    req.version = info.version;
    req.ostype = info.ostype;
    req.gray = info.gray;
    req.packageName = info.packageName;
    req.offset = 0;

    string downfile_md5 = "";

    //循环下载文件到本地
    while(!finish)
    {
        Tseer::UpdateRsp  rsp;
        int     iRetry = 3;

        while(iRetry-- > 0)
        {
            try
            {
                if(!_updtePrx)
                {
                    LOG->error() << FILE_FUN << display(req) << "|_updtePrx not init" << endl;
                    goto err;
                }

                int iRet = _updtePrx->updatePacket(req, rsp);
                if(iRet == -1)
                {
                    LOG->error() << FILE_FUN << display(req) << "|download packet error" << endl;
                    goto err;
                }

                if(iRet == 0)
                {
                    break;
                }
                else
                {
                    LOG->error() << FILE_FUN << display(req) << "|download packet data fail,iRet|" << iRet << endl;
                }

            }
            catch(exception&ex)
            {
                LOG->error() << FILE_FUN << display(req) << "|download packet data ex|" << ex.what() << endl;
            }
        }

        if(iRetry <= 0)
        {
            /* to do update on next time; */
            LOG->error() << FILE_FUN << display(req) << "|download fatal error" << endl;

            goto err;
        }

        if(rsp.buff.size() > 0)
        {
            size_t r = fwrite((void*)rsp.buff.c_str(), 1, rsp.buff.size(), fp);
            if(r == 0)
            {
                LOG->error() << FILE_FUN << "fwrite file '" + tgzFile + "' error!" << endl;
                goto err;
            }

            req.offset += r;
        }

        finish = rsp.finish; //下载完成标志
        LOG->debug() << FILE_FUN << display(req) << "|downloading|offset:" << req.offset << "|Finish=" << std::boolalpha << finish << endl;
    }

    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }

    downfile_md5 = tars::TC_MD5::md5file(tgzFile);
    if(downfile_md5 != info.md5)
    {
        LOG->error() << FILE_FUN << "md5 of download file not matched: " << downfile_md5 << endl;
        goto err;
    }

    try
    {
        string cmdTar = "tar -zxf " + tgzFile + " -C " + tmpDir;
        popen_sendMsg(cmdTar);
        LOG->debug() << FILE_FUN << "cmdTar: " << cmdTar << endl;
        string unTarPath = tmpDir + "/" + TSEERAGENT_APPNAME + "/" +TSEERAGENT_SERVERNAME;
         
        //dstpath=$installpath/TSEERAGENT_APPNAME/TSEERAGENT_SERVERNAME
        string cmdCpBin = "cp -f " + unTarPath + "/bin/" +TSEERAGENT_SERVERNAME + " " + dstPath + "/bin/"+TSEERAGENT_SERVERNAME;
        cmdCpBin += " && chmod 744 " + dstPath + "/bin/"+TSEERAGENT_SERVERNAME;
        
        popen_sendMsg(cmdCpBin);
        LOG->debug() << FILE_FUN << "cmdCpBin: " << cmdCpBin << endl;

        try
        {
            string monitorscript = "mon_" + TSEERAGENT_SERVERNAME + ".sh";
            string cmdCpMonsh = "cp -f " + unTarPath + "/util/" + monitorscript + " " + dstPath + "/util/"+monitorscript;
            cmdCpMonsh += " && chmod 744 " + dstPath + "/util/" + monitorscript;
            popen_sendMsg(cmdCpMonsh);
            LOG->debug() << FILE_FUN << "cmdCpMonsh: " << cmdCpMonsh << endl;
        }
        catch(...)
        {
            LOG->error() << FILE_FUN << "cmdCpTestBin exception" << endl;
        }

        TC_File::removeFile(tmpDir, true);
        return true;
    }
    catch(exception&ex)
    {
        /* save to file failed */
        LOG->error() << FILE_FUN << "back up file exeception: " << ex.what() << endl;
    }

err:
    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }
    unlink(tgzFile.c_str());
    return false;
}

