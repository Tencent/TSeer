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
 * 浏览器缓存组件
 */

cloudjs.define({
    cookie: function(options){
        if(options === 'setCookie'){
            _setCookie(arguments[1]);
            return this;
        }else if(options === 'getCookie'){
            cloudjs.callback(_getCookie(arguments[1]));
        }else if(options === 'getCookieObj'){
            var obj = _getCookie(arguments[1]);
            try{
                obj = $.parseJSON(obj);
            }catch(err){
                cloudjs.util.log(err);
            }
            cloudjs.callback(obj);
        }else if(options === 'removeCookie'){
            _setCookie({ name: arguments[1], value: '', expires: -1 });
            return this;
        }
        
        /**
        * 设置cookie
        * @param {Object} obj 包括4个参数name,value,allPath,expires
        */
        function _setCookie(obj){
            var val = obj.value, expires = '', path = obj.allPath ? '; path=/' : '', 
                domain = obj.domain ? '; domain=' + (obj.domain) : '',
                secure = obj.secure ? '; secure' : '';
            if(obj.expires && typeof obj.expires === 'number'){
                var date = new Date();
                date.setTime(date.getTime() + (obj.expires * 24 * 60 * 60 * 1000));
                expires = '; expires=' + date.toUTCString();
            }
            if(typeof val === 'object'){
                try{
                    val = JSON.stringify(val);
                }catch(err){
                    cloudjs.util.log(err);
                    return;
                }
            }
            document.cookie = [obj.name, '=', encodeURIComponent(val), expires, path, domain, secure].join('');
        }
        
        /**
        * 获取cookie
        * @param {String} name cookie名
        * @return {String} cookie值
        */
        function _getCookie(name){
            var cookieValue = null;
            if (document.cookie && document.cookie !== ''){
                var cookies = document.cookie.split(';');
                for (var i = 0; i < cookies.length; i++){
                    var cookie = jQuery.trim(cookies[i]);
                    if (cookie.substring(0, name.length + 1) === (name + '=')){
                        cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                        break;
                    }
                }
            }
            return cookieValue;
        }
    }
});