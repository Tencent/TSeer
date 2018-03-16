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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/syscall.h>
#include "gc.h"
#include "router_manager.h"
#include "stat_manager.h"
#include "registry_ep_manager.h"

#ifndef PATH_MAX
#define PATH_MAX    4096
#endif

namespace Tseerapi
{

GC::GC(int maxCacheNum): _maxCacheNum(maxCacheNum), _cacheObjNum(0)
{
    _head.tid = syscall(SYS_gettid);
    _head.type = OBJ_TYPE_MIN;
    _head.data = NULL;
    _head.next = NULL;
}

static void obj_release(GC_OBJ *obj)
{
    switch(obj->type)
    {
    case OBJ_ROUTE:
        delete ((RouterManager *)obj->data);
        break;
    case OBJ_STAT_MGR:
        delete ((StatManager *)obj->data);
        break;
    case OBJ_REGISTRY_EP_MGR:
        delete ((RegistryEpManager *)obj->data);
        break;
    default:
        break;
    }

    delete obj;
    obj = NULL;
}

GC::~GC()
{
    volatile GC_OBJ *obj = NULL, *next = NULL;
    for( obj = _head.next; obj != NULL; )
    {
        next = obj->next;
        obj_release((GC_OBJ*)obj);
        obj = next;
    }
    _head.next = NULL;
}


int GC::addObj(OBJ_TYPE type, void *obj, unsigned int tid)
{
    GC_OBJ *gc = NULL;
    
    if(0 == tid)
    {
        tid = syscall(SYS_gettid);
    }

    if(type <= OBJ_TYPE_MIN || type >= OBJ_TYPE_MAX)
    {
        return -1;
    }

    gc = new(std::nothrow) GC_OBJ;
    if(!gc)
    {
        return -1;
    }

    gc->tid = tid;
    gc->type = type;
    gc->data = obj;
    gc->next = NULL;

    //看是否需要清理资源
    if(_maxCacheNum < (__sync_fetch_and_add(&_cacheObjNum, 1)))
    {
        releaseExitThreadObj();
    }

    //头部插入
    do
    {
        gc->next = _head.next;
    }while(!__sync_bool_compare_and_swap(&(_head.next), gc->next, gc));

    return 0;
}

//获取所有的线程ID
static int get_all_tids(unsigned long *tids, unsigned int count, unsigned int *real_get)
{
    struct dirent *ent = NULL;
    struct dirent d;
    DIR *dir; 
    char path[PATH_MAX];
    unsigned int index = 0;

    snprintf(path, sizeof(path), "/proc/%u/task", getpid());
    path[sizeof(path) - 1] = 0;

    if((dir = opendir(path)) == NULL)
    {
        return -1;
    }


    while(1)
    {
        ent = NULL;
        if(readdir_r(dir, &d, &ent))
        {
            closedir(dir);
            return -1;
        }

        if(ent == NULL)
        {
            break;
        }

        if('.' == ent->d_name[0])
            continue;

        tids[index++] = atol(ent->d_name);
        if(index == count)
        {
            closedir(dir);
            return -1;
        }
    }

    *real_get = index;
    closedir(dir);
    return 0;
}

void GC::releaseExitThreadObj()
{
    unsigned int index, find = 0;
    volatile GC_OBJ  *obj, *prev_obj;
    GC_OBJ  tmp;
    unsigned long tids[1024];
    unsigned int real_count = 0;

    tmp.next = NULL;

    do
    {
        tmp.next = _head.next;
    }while(!__sync_bool_compare_and_swap(&(_head.next), tmp.next, NULL));

    if(get_all_tids(tids, sizeof(tids) / sizeof(unsigned long), &real_count))
    {
        if(NULL == tmp.next)
            return ;
        prev_obj = &tmp;

        while(NULL != prev_obj->next)
            prev_obj = prev_obj->next;

        do
        {
            prev_obj->next = _head.next;
        }while(!__sync_bool_compare_and_swap(&(_head.next), prev_obj->next, tmp.next));

        return ;
    }

    prev_obj = &tmp;
    for(obj = prev_obj->next; obj != NULL; obj = prev_obj->next)
    {
        find = 0;
        for(index = 0; index < real_count; index++)
        {
            if(tids[index] == obj->tid)
            {
                find = 1;
                break;
            }
        }

        if(find)
        {
            prev_obj = obj;
            continue;
        }
        prev_obj->next = obj->next;

        obj_release((GC_OBJ*)obj);

        __sync_fetch_and_sub(&_cacheObjNum, 1);
    }

    if(NULL == tmp.next)
        return ;

    do
    {
        prev_obj->next = _head.next;
    }while(!__sync_bool_compare_and_swap(&(_head.next), prev_obj->next, tmp.next));
}

void GC::deleteObj(unsigned int tid)
{
    volatile GC_OBJ  *obj, *prev_obj;
    GC_OBJ  tmp;
    if( 0 == tid)
        tid = syscall(SYS_gettid);

    tmp.next = NULL;
    do
    {
        tmp.next = _head.next;
    }while(!__sync_bool_compare_and_swap(&(_head.next), tmp.next, NULL));

    prev_obj = &tmp;
    for(obj = prev_obj->next; obj != NULL; obj = prev_obj->next)
    {
        if(tid != obj->tid)
        {
            prev_obj = obj;
            continue;
        }
        prev_obj->next = obj->next;

        obj_release((GC_OBJ*)obj);

        __sync_fetch_and_sub(&_cacheObjNum, 1);
    }

    if(NULL == tmp.next)
        return ;

    do
    {
        prev_obj->next = _head.next;
    }while(!__sync_bool_compare_and_swap(&(_head.next), prev_obj->next, tmp.next));
}

}
