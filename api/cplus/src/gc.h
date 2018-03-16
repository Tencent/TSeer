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

#ifndef __TSEER_API_GC_H_
#define __TSEER_API_GC_H_

/***************************************************************
* 文件定义了垃圾回收工具，它跟踪着各线程的路由以及调用状态信息
* 当线程退出时，就回收该线程下的路由和调用状态信息
***************************************************************/

#ifndef OBJ_MAX_CACHE
#define OBJ_MAX_CACHE    32
#endif

namespace Tseerapi
{

//对象类型
enum OBJ_TYPE
{
    OBJ_TYPE_MIN            = 0,
    OBJ_ROUTE                = 1,        //路由
    OBJ_STAT_MGR            = 2,        //调用状态信息
    OBJ_REGISTRY_EP_MGR        = 4,        //Tseer服务端IP管理对象
    OBJ_TYPE_MAX            = 5
};

struct GC_OBJ
{
    unsigned int                    tid;
    OBJ_TYPE                        type;
    void                            *data;
    volatile struct GC_OBJ            *next;
};

class GC
{
public:
    GC(int maxCacheNum = OBJ_MAX_CACHE);

    virtual ~GC();

    int addObj(OBJ_TYPE type, void *obj, unsigned int tid = 0);

    void deleteObj(unsigned int tid = 0);

private:
    GC(const GC&);

    GC& operator=(const GC&);

    //释放退出线程的资源
    void releaseExitThreadObj();

    //最大缓存数量
    //当超过这个数量时会清理不用的资源
    unsigned int                _maxCacheNum;

    //已缓存的对象数量
    volatile unsigned int        _cacheObjNum;

    //GC回收链表头部，里面并不包含实际的data数据
    GC_OBJ                        _head;
};

}

#endif
