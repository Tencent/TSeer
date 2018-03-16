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
	json: function(){		
	    var args = arguments, callback = odfl.callback, o, str2Json, json2Str;
	    
	    o = args[0];
	    
	    _2();
	    
	    function _1(){
	        if(typeof o === 'string'){
	            callback(str2Json(o));
	        }else{
	            callback(json2Str(o));
	        }
	    }
	    
	    function _2(){
	        
	        json2Str = JSON.stringify;
	        
	        if(typeof JSON !== 'undefined' && typeof JSON.parse === 'function'){
                str2Json = JSON.parse;
            }else{
                str2Json = $.parseJSON;
            }
	        
	        _1();
	    }
	},
	
	require: function(){
	    if(typeof JSON === 'undefined' || typeof JSON.stringify !== 'function'){
            return ['../../js/json2.js'];
        }
	}
});