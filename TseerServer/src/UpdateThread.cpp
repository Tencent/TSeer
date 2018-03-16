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

#include "UpdateThread.h"
#include "servant/Application.h"
#include "util/tc_md5.h"
#include "util/tc_file.h"
#include "StoreProxy.h"
#include "util.h"

using namespace Tseer;
using namespace std;

UpdateThread::UpdateThread() :_terminate(false), _interval(60)
{
}

UpdateThread::~UpdateThread()
{
}

void UpdateThread::run()
{
    while (!_terminate)
    {
        checkChange();
        {
            TC_ThreadLock::Lock lock(*this);
            timedWait(_interval * 1000);
        }
    }
}

/*
 * 获取正式发布包
 */
bool UpdateThread::getFormalPackageData(const string &ostype, PackageData &data)
{
    TC_ThreadLock::Lock lock(*this);

    map<string, PackageData>::const_iterator it = _formalAgentPackage.find(ostype);
    if (it != _formalAgentPackage.end())
    {
        data = it->second;
        UPDATEPACKAGE_LOG<<FILE_FUN<<"find formal packet:"<<data.packageName<<"|"<<data.version<< "|ostype=" << ostype << endl;
        return true;
    }
    UPDATEPACKAGE_LOGWARN<<FILE_FUN<<"ostype=" << ostype<<"|can't find this formal packet" << endl;
    return false;
}

/*
 *获取灰度发布包
 */
bool UpdateThread::getGrayPackageData(const string &ostype, PackageData &data)
{
    TC_ThreadLock::Lock lock(*this);
    map<string, PackageData>::iterator it = _grayAgentPackage.find(ostype);
    if (it != _grayAgentPackage.end())
    {
        data = it->second;
        UPDATEPACKAGE_LOG<<FILE_FUN<<"find gray packet:"<<data.packageName<<"|"<<data.version<< "|ostype=" << ostype << endl;
        return true;
    }
    UPDATEPACKAGE_LOGWARN<<FILE_FUN<<"ostype=" << ostype<<"|can't find this packet" << endl;
    return false;

}

bool UpdateThread::getFormalInfo(const PackageInfo &reqInfo, PackageInfo &rspInfo)
{
    TC_ThreadLock::Lock lock(*this);
    map<string, PackageInfo>::const_iterator it = _formalAgentInfoList.find(reqInfo.ostype);
    if (it != _formalAgentInfoList.end())
    {
        rspInfo = it->second;
        UPDATEPACKAGE_LOG<<FILE_FUN << display(reqInfo) <<"|find formal packageName|"<<display(rspInfo)<< endl;
        return true;
    }
    UPDATEPACKAGE_LOGWARN<< FILE_FUN << display(reqInfo)<< "|can't find this packet"<<"|_formalAgentInfoList size=" << _formalAgentInfoList.size()<< endl;
    return false;
}

bool UpdateThread::getGrayInfo(const PackageInfo &reqInfo, PackageInfo &rspInfo)
{
    TC_ThreadLock::Lock lock(*this);
    UPDATEPACKAGE_LOG<<FILE_FUN<< "reqInfo.packageName=" << reqInfo.packageName<< "|_grayAgentInfoList size=" << _grayAgentInfoList.size()<< endl;

    map<string, PackageInfo>::const_iterator it = _grayAgentInfoList.find(reqInfo.ip);
    if (it != _grayAgentInfoList.end())
    {
        rspInfo = it->second;
        UPDATEPACKAGE_LOG<<FILE_FUN << display(reqInfo) <<"|find formal packageName|"<<display(rspInfo)<< endl;
        return true;
    }

    UPDATEPACKAGE_LOGWARN<< FILE_FUN << display(reqInfo)<< "|can't find this packet"<<"|_formalAgentInfoList size=" << _formalAgentInfoList.size()<< endl;
    return false;
}


void UpdateThread::checkChange()
{
    __TRY__
    
    /*
     * 加载正式数据包 
     */
    vector<AgentPackageInfo> agentInfoList;
    int iRet = STOREPROXY->getAgentInfo(TSEER_PACKAGE_FORMAL_TYPE, agentInfoList);
    if (iRet == 0)
    {
        for (size_t i = 0; i < agentInfoList.size(); i++)
        {
            PackageInfo newInfo;
            //找到最新版本
            newInfo.version = agentInfoList[i].version;
            newInfo.packageName = agentInfoList[i].package_name;
            newInfo.md5 = agentInfoList[i].md5;
            newInfo.ostype = agentInfoList[i].ostype;
            newInfo.gray = false;
    
            {
                TC_ThreadLock::Lock lock(*this);
                map<string, PackageInfo>::iterator it = _formalAgentInfoList.find(newInfo.ostype);
                UPDATEPACKAGE_LOG<< FILE_FUN << "|formal info size=" << _formalAgentInfoList.size()<<"|os="<<newInfo.ostype<< "|packageName=" << newInfo.packageName << endl;
                if (it != _formalAgentInfoList.end())
                {
                    //一个ostype只允许有一个正式版本
                    string oldVersion = it->second.version;
                    UPDATEPACKAGE_LOG<< FILE_FUN <<"os="<<newInfo.ostype<< "|oldVersion=" <<oldVersion << "|new version=" << newInfo.version << endl;
                    if (needUpdate(oldVersion,newInfo.version,std::greater_equal<int>()))
                    {
                        continue;
                    }
                }
            }
            UPDATEPACKAGE_LOG<< FILE_FUN <<"os="<<newInfo.ostype<< "|packageName=" << newInfo.packageName<< "|new version=" << newInfo.version << endl;
            //没找到或者找到 但是版本不同, 更新发布包到内存中
            update(newInfo);
        }
    }
    else
    {
        UPDATEPACKAGE_LOGWARN<< FILE_FUN << "load formal agent package info failure"<< endl;
    }

    /*
    * 加载灰度版本信息
    */
    bool finish(false);
    map<string, bool> delAgentPacket; //key:ostype; value:是否删除 有一个节点在使用 不可清理发布包

    do
    {
        vector <GrayAgentInfo> grayInfoList;
        iRet = STOREPROXY->getGrayAgentInfo(grayInfoList);
        if (iRet != 0)
        {
            //记录日志
            UPDATEPACKAGE_LOGWARN<< FILE_FUN << "load gray agent package info failure"<< endl;
            return;
        }

        for (size_t i = 0; i < grayInfoList.size(); i++)
        {
            PackageInfo newInfo;
            newInfo.packageName = grayInfoList[i].package_name;
            newInfo.version = grayInfoList[i].grayscale_version;
            newInfo.ostype = grayInfoList[i].ostype;
            newInfo.md5 = grayInfoList[i].md5;
            newInfo.gray = true;
            newInfo.ip = grayInfoList[i].ip;
            
            string grayscale_state = grayInfoList[i].grayscale_state; //区分是否灰度

            if (newInfo.ip.empty())
            {
                UPDATEPACKAGE_LOG<< FILE_FUN << newInfo.packageName<< "|has invalid data" << endl;
                continue;
            }

            UPDATEPACKAGE_LOG<<FILE_FUN<< "gray info,packageName=" << newInfo.packageName<< "|version=" << newInfo.version << "|ostype=" << newInfo.ostype 
                        << "|status=" << grayscale_state<< "|grayIP=" << newInfo.ip << endl;


            //查看本地是否已经存在该文件  不存在则加载
            delAgentPacket[newInfo.ostype] = false;
            if (grayscale_state == TSEER_PACKAGE_GRAY_TYPE)
            {
                {
                    TC_ThreadLock::Lock lock(*this);
                    map<string, PackageData>::iterator it = _grayAgentPackage.find(newInfo.ostype);
                    if (it != _grayAgentPackage.end())
                    {
                        if (needUpdate(it->second.version,newInfo.version,std::equal_to<int>()))
                        {
                            _grayAgentInfoList[newInfo.ip] = newInfo;
                            UPDATEPACKAGE_LOG<< FILE_FUN << "Local exist file name|"<< it->second.packageName << "|new ip|" << newInfo.ip
                                << "|_grayAgentInfoList.size()=" << _grayAgentInfoList.size() << endl;
                            continue;
                        }
    
                        UPDATEPACKAGE_LOG<< FILE_FUN << "new filename|" << newInfo.packageName << "|local file name|" << it->second.packageName << "| new inner ip|" << newInfo.ip << endl;
                    }
                }
                update(newInfo);
            }
            else if (grayscale_state == TSEER_PACKAGE_FORMAL_TYPE)
            {
                //内存有包 再设置是否删除数据
                if (_grayAgentPackage.find(newInfo.ostype) != _grayAgentPackage.end())
                {
                    map<string, bool>::iterator it = delAgentPacket.find(newInfo.ostype);
                    if (it == delAgentPacket.end())
                    {
                        delAgentPacket[newInfo.ostype] = true;
                    }
                }

                //灰度结束,清理本地内存
                {
                    TC_ThreadLock::Lock lock(*this);
                    map<string, PackageInfo>::iterator it = _grayAgentInfoList.find(newInfo.ip);
                    if (it != _grayAgentInfoList.end())
                    {
                        _grayAgentInfoList.erase(it);
                        UPDATEPACKAGE_LOG<< FILE_FUN << "clean gray info, node ip|" << newInfo.ip << endl;
                    }
                }
            }
            else
            {
                UPDATEPACKAGE_LOGWARN<< FILE_FUN << "data error, status|" << grayscale_state << endl;
            }
        }
        
        if (grayInfoList.size() == 0)
        {
            break;
        }
        finish = ((grayInfoList.size() % 5000) == 0) ? false : true;

    } while (!finish);


    //清理不用的灰度包
    UPDATEPACKAGE_LOG<< FILE_FUN << "delAgentPacket size=" << delAgentPacket.size() << endl;
    map<string, bool>::iterator itDel = delAgentPacket.begin();
    for (; itDel != delAgentPacket.end(); itDel++)
    {
        UPDATEPACKAGE_LOG<< FILE_FUN << "agent os ver|" << itDel->first<< "|bDelete|" << std::boolalpha<<itDel->second<< endl;
        if (itDel->second)
        {
            TC_ThreadLock::Lock lock(*this);
            map<string, PackageData>::iterator itGray = _grayAgentPackage.find(itDel->first);
            if (itGray != _grayAgentPackage.end())
            {
                UPDATEPACKAGE_LOG<< FILE_FUN << "agent delete os ver|" << itGray->first<< "|_grayAgentPackage size|" << _grayAgentPackage.size() << endl;
                _grayAgentPackage.erase(itGray);
            }
        }
    }
    __CATCH__
}

bool UpdateThread::update(const PackageInfo &newInfo)
{
    __TRY__
    string directory  = ServerConfig::DataPath;
    string filePath = TC_File::simplifyDirectory(directory + "/" + newInfo.packageName);

    //加载文件到内存
    ifstream inFile;
    string buff;
    inFile.open(filePath.c_str(), ios::in);
    if (!inFile.is_open())
    {
        UPDATEPACKAGE_LOGERROR<< FILE_FUN << "open file fail, file name|" << filePath << endl;
        return false;
    }
    else
    {
        //读取成功 更新到缓存
        istreambuf_iterator<char> beg(inFile), end;
        buff.assign(beg, end);
    }
    inFile.close();

   UPDATEPACKAGE_LOG<< FILE_FUN<< "osversion=" <<newInfo.ostype<<"|agent packageName="<< newInfo.version 
                                << "|md5" << newInfo.md5 << "|file name=" << newInfo.packageName << "|file size=" << buff.size()<< endl;

    //将最新数据 存入buff
    PackageData data;
    data.dataBuff = buff;
    data.version = newInfo.version;
    data.ostype = newInfo.ostype;
    data.packageName = newInfo.packageName;
    data.md5 = newInfo.md5;

    if (newInfo.gray)
    {
        TC_ThreadLock::Lock lock(*this);
        _grayAgentPackage[newInfo.ostype] = data;
        _grayAgentInfoList[newInfo.ip] = newInfo;
        UPDATEPACKAGE_LOG<< FILE_FUN <<"gray ip="<<newInfo.ip<< "|update osversion=" << newInfo.ostype <<"|agent packageName="<<newInfo.version 
                    << "|md5|" << newInfo.md5<< "|gray agent size|" << _formalAgentInfoList.size()<< endl;
    }
    else
    {
        TC_ThreadLock::Lock lock(*this);
        _formalAgentPackage[newInfo.ostype] = data;
        _formalAgentInfoList[newInfo.ostype] = newInfo;
        UPDATEPACKAGE_LOG<< FILE_FUN << "update osversion=" << newInfo.ostype <<"|agent packageName="<<newInfo.version << "|md5|" << newInfo.md5
                    << "|formal agent size|" << _formalAgentInfoList.size()<< endl;
    }
    return true;
    __CATCH__
    return false;
}

