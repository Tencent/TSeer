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

﻿(function($, w, undefined){
	w.Util = {
		openDlg:function(type, mes, okFunc, noFunc){
			if (type === "confirm") {
				cloudjs.dialog('confirm', {
					content: mes,
					ok: function(){
						okFunc && typeof okFunc === 'function' && okFunc();
						this.close();
					},
					no: function(){
						noFunc && typeof noFunc === 'function' && noFunc();
						this.close();
					}
				});
			} else {
				cloudjs.dialog('alert', {
					content: mes,
					ok: function(){
						okFunc && typeof okFunc === 'function' && okFunc();
						this.close();
					}
				});
			}
		},
		showMsg: function(type, msg, duration, closeBtn){
			duration = duration == undefined ? 5000: duration;
			closeBtn = closeBtn || false;
			cloudjs.message('close');
			cloudjs.message({
				content: msg,
				type: type,
				duration: duration,
				showCloseIcon: closeBtn,
				showLeftIcon: true
			});
		},
		//展示蒙版
		showMask:function(text){
			cloudjs.mask('destroy');
			text = (text == undefined || text == null || text == '') ?'正在加载' : text;
			cloudjs.mask({
				maskHtml: '<div style="border-radius:10px;padding:15px 20px;background:#565656;color:#CCC;font-size:14px;"><img src="/images/loading_big.gif" /><p style="margin-top:5px;">' + text + '</p></div>',
				zIndex: 12000
			});
		},
		hideMask:function(){
			cloudjs.mask('destroy');
		},
		getUrlParams:function(key, url) {
			if(arguments.length == 0) return null;
			if (url == undefined || url ==null ) url = location.href;
			var href = url;
			var index = href.indexOf('?');
			if (index === -1) return null;
			var search = href.substring(index);
			var start = search.indexOf('&' + key + '=');
			if (start == -1)start = search.indexOf('?' + key + '=');
			var value = '';
			if (start != -1) {
				start += (key.length + 2);
				var end = search.indexOf('&', start);
				value = search.substring(start, end == -1 ? search.length : end);
			} else {
				return null;
			}
			var cutPos = value.indexOf('#');
			if (cutPos !== -1)value = value.substring(0, cutPos);
			return value;
		},
		setUrlParams: function(key, value, url) {
			if (!url || !key) return null;
			if (getUrlParams(key, url) === null) {
				url = url + (url.indexOf('?') === -1 ? '?' : '&') + key + '=' + value;
			} else {
				var href = url;
				var index = href.indexOf('?');
				if (index === -1)index = href.length;
				var path = href.substring(0, index);
				var search = href.substring(index);
				var start = search.indexOf('&' + key + '=');
				if (start == -1)start = search.indexOf('?' + key + '=');
				if (start != -1) {
					start += (key.length + 2);
					var end = search.indexOf('&', start);
					url = path + search.substring(0, start) + value + search.substring(end == -1 ? search.length : end);
				}
			}
			return url;
		},
		validaterName:function(name){
			name = name||'';
			var exp = /^[a-zA-Z][a-zA-Z\d]*$/;
			var reg = name.match(exp);
			if(reg==null){
				return false;
			}else{
				return true;
			}
		},

		//页面打开上报和用户动作上报
		actReport: function(name, type, node, method){
			var report = function(name, type){
				if(!name || !type){
					return;
				}
				$.ajax({
					url:'/interfacenew?interface_name=event_stat&interface_params={"target_name":"'+name+'","event_name":"'+type+'"}',
					dataType:'json'
				});
			};
			if(node){
				method = method || 'click';
				$(node).each(function(){
					var actReportName = name || $(this).data('act-report-name') || '';
					$(this).bind(method, function(){
						report(encodeURIComponent(actReportName), encodeURIComponent(type));
					});
				});
			}else{
				report(encodeURIComponent(name), encodeURIComponent(type));
			}
		},

		clone: function(obj){
			if (obj === null) return null;
			var o = Object.prototype.toString.call(obj) === '[object Array]' ? [] : {};
			for (var i in obj) {
				o[i] = (obj[i] instanceof Date) ? new Date(obj[i].getTime()) : (typeof obj[i] === "object" ? arguments.callee(obj[i]) : obj[i]);
			}
			return o;
		}
	};
})(jQuery, this);