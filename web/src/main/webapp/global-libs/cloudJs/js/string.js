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

/**
 * 字符串大小写转换组件
 */
cloudjs.define({
    string: function(options){
        var action = arguments[0],
            str = arguments[1],
            callback = cloudjs.callback,
            result;
        
        if(!str || Object.prototype.toString.call(str) !== '[object String]'){
            callback(result);
            return;
        }
        
        if(action === 'toLowerCase'){
            result = str.toLowerCase();
        }else if(action === 'toUpperCase'){
            result = str.toUpperCase();
            
            result = result + 'a';//假如这里多加个a，就通不过测试
        }
        
        callback(result);
        
    }
});