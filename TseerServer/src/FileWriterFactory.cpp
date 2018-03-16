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

#include "FileWriterFactory.h"
#include "util/tc_md5.h"
#include "servant/TarsLogger.h"
#include "util.h"

FileWriterFactory::FileWriterFactory() : _hashf(tars::hash_new<string>())
{
}

int FileWriterFactory::updateAgentInfo(const Tseer::PushPackageInfo& pushPkInfo)
{
    try
    {
        static TC_ThreadLock _Lock;
        TC_ThreadLock::Lock lock(_Lock);

        AgentPackageInfo agentInfo;
        STOREPROXY->convertStruct(pushPkInfo, agentInfo);
        TLOGDEBUG(FILE_FUN << "agent info|" <<agentInfo.ostype<< "|" << agentInfo.package_type\
                        << "|" << agentInfo.package_name<< "|" << agentInfo.version << endl);
        int iRet = STOREPROXY->add_agent_packageinfo(agentInfo);

        return iRet;

    }catch(exception& ex)
    {
        TLOGERROR(FILE_FUN << ex.what() << endl);
    }catch(...)
    {
        TLOGERROR(FILE_FUN << "exception unknow" << endl);
    }
    return -1;
}

int FileWriterFactory::write(const Tseer::PushPackageInfo & pushPkInfo,string& result)
{
    int ret = 0;
    FileTask rTask;
    string key = pushPkInfo.packageName;
    do{
        try{
            //初始化任务并缓存起来文件句柄
            {
                TC_ThreadLock::Lock lock(*this);
                map<string,FileTask>::iterator it = _taskList.find(key);
                if(it != _taskList.end())
                {
                    rTask = it->second;
                    if(it->second.pos > pushPkInfo.pos)
                    {
                        //写位置不合法
                        result = "give invalid pos=" + TC_Common::tostr(pushPkInfo.pos);
                        ret =  -1;
                        break;
                    }
                }
                else
                {
                    //初始化任务
                    string  directory  = ServerConfig::DataPath;

                    FileTask task;
                    task.path = pushPkInfo.packageName;
                    task.size = pushPkInfo.size;
                    task.md5 = pushPkInfo.md5;
                    task.pos = pushPkInfo.pos;

                    task.filePath = tars::TC_File::simplifyDirectory(directory + "/" + pushPkInfo.packageName);
                    task.tInTime = TNOW;
                    task.lock = getRecLock(task);

                    string filepath = tars::TC_File::extractFilePath(task.filePath);
                    if(!TC_File::isFileExistEx(filepath, S_IFDIR))
                    {
                        bool mkResult = TC_File::makeDirRecursive(filepath);
                        if(!mkResult)
                        {
                            result = "External error, mkdir \""+filepath+"\" failure,errno," + strerror(errno);
                            TLOGERROR(FILE_FUN<< pushPkInfo.packageName << "|"<<result<< endl);
                                ret= -1;
                            break;
                        }
                    }

                    //重复上传的，自动删除
                    if(TC_File::isFileExist(task.filePath))
                    {
                        if(TC_File::removeFile(task.filePath,true) != 0)
                        {
                            result = "External error, mkdir \""+filepath+"\" failure,errno," + strerror(errno);
                            ret =-1;
                            break;
                        }
                        TLOGDEBUG(FILE_FUN<< rTask.filePath<<"|del file succ"<<endl);
                    }

                    task.fp = fopen(task.filePath.c_str(), "wb");
                    if (!task.fp)
                    {
                        result = task.filePath + " can not open file " + task.filePath + ",errno=" + strerror(errno);
                        ret = -1;
                        break;
                    }
                    _taskList[key] = task;
                    rTask = _taskList[key];
                    TLOGDEBUG(FILE_FUN<< rTask.filePath<<"|open file succ"<<endl);
                }
            }

            //写文件
            {
                TC_ThreadRecLock::Lock lock(*rTask.lock);
                size_t r = fwrite((void*)&pushPkInfo.vb[0], 1, pushPkInfo.vb.size(), rTask.fp);
                if (r == 0)
                {
                    TLOGERROR(FILE_FUN<< "fwrite file '" + rTask.filePath+ "' error!" << endl);
                    result += "fwrite file '" + rTask.filePath + "' error!errno=" + strerror(errno);;
                    ret = -1;
                    break;
                }

                _taskList[key].pos += r;

                rTask.pos += r;
                if(rTask.pos == rTask.size)
                {
                    fclose(rTask.fp);
                    rTask.fp = NULL;
                    _taskList[key].fp = NULL;
                    tars::Int64 tmpSize = tars::TC_File::getFileSize(rTask.filePath);
                    if(tmpSize != rTask.size)
                    {
                        result = "receivelen=" + TC_Common::tostr(tmpSize) + ",srclen=" +TC_Common::tostr(rTask.size) + "|size invalid";
                        TLOGERROR(FILE_FUN<< rTask.filePath<<"|"<<result<< endl);
                        ret = -1;
                        break;
                    }

                    string tmpMd5      = tars::TC_MD5::md5file(rTask.filePath);
                    if(tmpMd5 == rTask.md5)
                    {
                        TLOGDEBUG(FILE_FUN<< rTask.filePath<<"|receive finish,len="<<rTask.pos<<",tmpmd5="<<tmpMd5<<",srcmd5="<<rTask.md5<< endl);

                                         if(updateAgentInfo(pushPkInfo) != 0)
                        {
                            result = pushPkInfo.packageName + " update result to db failure";
                            TLOGERROR(FILE_FUN<< rTask.filePath<<"|"<<pushPkInfo.packageName<< endl);
                            ret = -1;
                            break;
                        }
                        return 0;
                    }
                    else
                    {
                        result = "tmpmd5=" + tmpMd5 + ",srcmd5=" + rTask.md5 + "|md5 invalid";
                        TLOGERROR(FILE_FUN<< rTask.filePath<<"|"<<result<< endl);
                        ret = -1;
                        break;
                    }
                }
                else
                {
                    TLOGDEBUG(FILE_FUN<< rTask.filePath<<"|receive len="<<pushPkInfo.vb.size() <<"|totallen="<<rTask.pos<< endl);
                }
            }
            return 0;
        }catch(exception& e)
        {
            result = string(e.what());
            TLOGERROR(FILE_FUN << e.what() << endl);
        }
        catch(...)
        {
            result = "unkonw exception";
            TLOGERROR(FILE_FUN << result<< endl);
        }
        ret = -1;
    } while(0);

    if(rTask.fp)
    {
        fclose(rTask.fp);
    }

    TC_ThreadLock::Lock lock(*this);
    map<string,FileTask>::iterator it = _taskList.find(key);
    if(it != _taskList.end())
    {
        TLOGDEBUG(FILE_FUN<< rTask.filePath<<"|erase key="<<key<< endl);
        _taskList.erase(it);
    }

    return ret;
}


void FileWriterFactory::timeout()
{
    __TRY__
    TC_ThreadLock::Lock lock(*this);
    for(map<string,FileTask>::iterator it = _taskList.begin();it != _taskList.end();it++)
    {
        bool bDel = (TNOW - it->second.tInTime) > 600;
        if(bDel)
        {
            TLOGDEBUG(FILE_FUN<< "task timeout:"<<it->second.filePath<<endl);
            TC_ThreadRecLock::Lock lock(*it->second.lock);
            __TRY__
            if(it->second.fp)
            {
                fclose(it->second.fp);
            }
            __CATCH__
            _taskList.erase(it);
        }
    }
    __CATCH__
}


