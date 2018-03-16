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

var odfl = ($.odfl||odfl);
odfl.define({
	jsonp: function(obj){
		
	    var $this = this, url, data, type, async, dataType, jsonpCallback, complete, error, success, exception, disableClass = 'odfl_btn_disable', tempError, tempSuccess, uuid = odfl.uniq();
	    //记录开始时间.
	    
	    var startTime = 0;
	    if($this && typeof $this.hasClass === 'function'){
            if($this.hasClass('odfl_btn_disable')){
                return;
            }else{
                $this.addClass('odfl_btn_disable');
                _();
            }
        }else{
            _();
        }
	    
	    function _(){
	        obj = obj||{};
	        
	        url = obj.url;
	        if(!url){
	            __.log('url is empty');
	            return;
	        }
	        
	        complete = function(){
	            __.log('request complete!' + url);
	        };
	        
	        error = function(){
	            __.log('request error!' + url);
	        };
	        
	        success = function(resp){
	            if(resp && resp.ret_code=='200'){
	                __.log('request success!' + url);
	            }else{
	                __.log('request fail!' +resp.err_msg);
	            }
	        };
	        
	        exception = function(err){
	            __.log('exception!'+err.message);
	        };
	        
	        data = obj.data||{};
	        data.timestamp = data.timestamp||new Date().getTime();
	        
	        type = obj.type||'get';
	        async = obj.async===false?false:true;
	        dataType = obj.dataType||'jsonp';
	        
	        if(obj.jsonpCallback){
	            jsonpCallback = obj.jsonpCallback;
	        }else{
	            jsonpCallback = 'callback'+uuid;
	            if(url.indexOf('?')>=0){
	                url = url + '&call_back_name=callback'+uuid;
	            }else{
	                url = url + '?call_back_name=callback'+uuid;
	            }
	        }
	        
	        complete = obj.complete||complete;
	        
            tempError = error = obj.error||error;
            tempSuccess = success = obj.success||success;
            
            error = function(){
                if($this && typeof $this.removeClass === 'function'){
                    $this.removeClass(disableClass);
                }
                tempError.apply(this, arguments);
            };
            
            success = function(){
                if($this && typeof $this.removeClass === 'function'){
                    $this.removeClass(disableClass);
                }
                tempSuccess.apply(this, arguments);
                if(obj.monitor){
	                var endTime = new Date().getTime();
    	            var dua = endTime - startTime, rs = '3';
    	            if(dataType == 'json'){
    	            	if(arguments[0].ret_code){
    	            		if(arguments.ret_code == 200){
    	            			rs = '1';
    	            		}
    	            		else{
    	            			rs = '2';
    	            		}
    	            	}
    	            }
    	            var pms = {url:obj.url,type:'time',rs:rs,time:dua};
    	            if(obj.username){
    	            	pms.username = obj.username;
    	            }
    	            odfl.monitor(pms);
                }
            };
	        startTime = new Date().getTime();
	        _2();
	    }
		function _2(){
		    try{
                $.ajax({
                     type: type,
                     data: _3(data),  
                     async: async, 
                     dataType: dataType,
                     jsonpCallback: jsonpCallback,
                     url: url,
                     complete: complete,
                     error: error,  
                     success: success
                 });
             }catch(err){
                 exception(err);
             }
		}
		
		function _3(data){
            if(typeof data === 'object'){
                for(var p in data){
                    if(data.hasOwnProperty(p) && (typeof data[p] === 'string')){
                        data[p] = $.trim(data[p]);
                    }
                }
            }
            return data;
        }
	},
	
	require: ['./css/jsonp.css']
});