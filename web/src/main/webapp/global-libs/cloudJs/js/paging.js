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

﻿/**
 * 分页组件
 */
 
cloudjs.define({
    paging: function(options){
        var defaults = {
            first: '',                //首页文字，留空则不显示首页
            prev: '上一页',           //上一页文字，留空则不显示上一页
            next: '下一页',           //下一页文字，留空则不显示下一页
            last: '',                 //末页文字，留空则不显示末页
            showDetails: true,        //是否显示详细的分页即分页后面那一串内容
            showIcons: false,         //首末上下页是否用图标
            useSimple: false,         //是否采用简单的分页
            totalCounts: 1000,        //总记录数
            pageSize: 20,             //每页展示的记录数
            pageNum: 1,               //当前页码
            visiblePages: 5,          //显示的页码数量
            disableClass: 'paging_disabled', //不可点击时的样式
            activeClass: 'paging_active',    //当前页码的样式
            ajaxUrl: '',              //请求的url，留空则需要在onPageChange事件中发起ajax请求，不留空则组件自动请求ajaxUrl，直接在onPageChange事件中通过data.data获取数据
            params: '',               //请求的参数，查询操作时通过setData方法设置，在onPageChange事件中用data.params获取
            dataType: 'json',         //请求ajaxUrl返回的数据类型，如果ajaxUrl为空，这个参数也没有意义
            sizeKey: 'pageSize',      //组件向外传递参数pageSize的key值
            numKey: 'pageNum',        //组件向外传递参数pageNum的key值
            maskId: '',               //要加蒙版层的id，如果不为空，ajax请求返回数据前会在此id区域加一层蒙版效果
            loadImg: 'http://3gimg.qq.com/mig_op/cloudJs/images/paging_loading.gif',  //蒙版层加载图片的路径
            onPageChange: $.noop                         //改变分页时触发，向外传递data.totalCounts,data.pageSize,data.pageNum,data.params和data.data
        };
        
        if($.isPlainObject(options) || !options){
            $.extend(defaults,options);
            $(this).data('initData', defaults);
        }else if(options === 'setData'){
            defaults = $.extend($(this).data('initData'), arguments[1]);
        }else if(options === 'getData'){
            cloudjs.callback($(this).data('initData'));
            return;
        }
        
        var _self = this,
            _first = defaults.first,
            _prev = defaults.prev,
            _next = defaults.next,
            _last = defaults.last,
            _showDetails = defaults.showDetails,
            _showIcons = defaults.showIcons,
            _useSimple = defaults.useSimple,
            _totalCounts = defaults.totalCounts,
            _pageSize = Number(defaults.pageSize),
            _pageNum = Number(defaults.pageNum),
            _visiblePages = Number(defaults.visiblePages),
            _disableClass = defaults.disableClass,
            _activeClass = defaults.activeClass,
            _ajaxUrl = defaults.ajaxUrl,
            _params = defaults.params,
            _sizeKey = defaults.sizeKey,
            _numKey = defaults.numKey,
            _maskId = defaults.maskId,
            _onPageChange = defaults.onPageChange;

        if(_totalCounts === 0){
            _self.empty();
            return;
        }
        
        _init(_pageSize, _pageNum);
        
        /**
        * 初始化组件
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        */
        function _init(s, n){
            n = _verify(s, n);
            if(n == 0){
                alert("error：totalCounts|pageSize|pageNum|visiblePages参数设置错误");
                return;
            }
            _ajaxFunc(s, n, _renderHtml);
        }
        
        /**
        * 参数校验
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        * @return {Number} 当前页码
        */
        function _verify(s, n){
            if(isNaN(s) || isNaN(n) || isNaN(_visiblePages) || s < 1) return 0;
            if(n < 1) n = 1;
            return n;
        }
        
        /**
        * 加载组件
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        * @param {Function} func ajax请求完成后要加载的函数
        */
        function _ajaxFunc(s, n, func){
            
            if(_ajaxUrl){
                var params = _params, url = _ajaxUrl + '?', mask = _createMask();
                if($.isPlainObject(params)){
                    var obj = {}; obj[_sizeKey] = s; obj[_numKey] = n;
                    params = $.extend(_params, obj);
                    for( var key in params){
                        url += key + '=' + params[key] + '&';
                    }
                }else{
                    params = _sizeKey + '=' + s + '&' + _numKey + '=' + n + '&' + _params;
                    url += params;
                }
                if(url.charAt(url.length-1) === '&') url = url.substring(0, url.length - 1);
                                
                $.post(_ajaxUrl, params, function(data){
                    if($.isFunction(_totalCounts)) _totalCounts = Number(_totalCounts(data));
                    if(_totalCounts === 0){
                        _self.empty();
                    }else{
                        func(s, n);
                    }
                    var result = { totalCounts: _totalCounts, params: _params };
                    result[_sizeKey] = s; result[_numKey] = n;
                    if($.isPlainObject(data)){
                        _onPageChange($.extend(result, data));
                    }else{
                        result.data = data;
                        _onPageChange(result);
                    }
                    if(mask != null) mask.remove();
                }, defaults.dataType);
            }else{
                _totalCounts = Number(_totalCounts);
                func(s, n);
                _onPageChange({ totalCounts: _totalCounts, pageSize: s, pageNum: n, params: _params });
            }
        }
        
        /**
        * 添加蒙版效果
        * @return {Object} 蒙版的jquery对象
        */
        function _createMask(){
            if(!_maskId) return null;
            var mask = $('<div style="display:none;"><div class="paging_mask"></div><img class="paging_loading" src="' + defaults.loadImg + '"/></div>').appendTo('body'),
                dataObj = $('#' + _maskId),
                imgObj = mask.find('img'),
                maskObj = { height: dataObj.height() };
            if(maskObj.height > 99){
                mask.show();
                $.extend(maskObj, { width:dataObj.width() }, dataObj.offset());
                if(!dataObj.is("body")){
                    var _winht = $(window).height(), _sctop = $(document).scrollTop(), _top, _pos = maskObj.top - _sctop;
                    //mask对象的top大于$(document).scrollTop()
                    if(_pos > 0){
                        //mask对象bottom值超出屏幕
                        if(_pos + maskObj.height > _winht){
                            _top = (maskObj.top + _winht + _sctop) / 2;
                        }else{
                            _top = (maskObj.top + maskObj.height / 2);
                        }
                    }else{
                        _top = (maskObj.top + maskObj.height + _sctop) / 2;
                    }
                    imgObj.css({ top: _top + 'px', left: (maskObj.left + maskObj.width / 2) + 'px', position: 'absolute' })
                    .prev().css({ width: maskObj.width + 'px', height: maskObj.height + 'px', top: maskObj.top + 'px', left: maskObj.left + 'px', position: 'absolute' });
                }
            }
            return mask;
        }
        
        /**
        * 生成分页组件的html代码放入到当前对象中
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        */
        function _renderHtml(s, n){
            var htmlArr = [], html = '';
            if(_first){
                htmlArr.push(_getBtnHtml([n, 1, 'first', _first]));
            }
            if(_prev){
                htmlArr.push(_getBtnHtml([n, 1, 'prev', _prev]));
            }
            if(_useSimple){
                htmlArr.push('<li class="ellipsis">' + n + '/' + Math.ceil(_totalCounts / s) + '</li>');
            }else if(_visiblePages > 0){
                htmlArr.push(_getPagesHtml(n, _getPagesArr(s, n)));
            }
            if(_next){
                htmlArr.push(_getBtnHtml([n, Math.ceil(_totalCounts / s), 'next', _next]));
            }
            if(_last){
                htmlArr.push(_getBtnHtml([n, Math.ceil(_totalCounts / s), 'last', _last]));
            }
            html = '<ul>' + htmlArr.join('') + '</ul>';
            if(_showDetails && !_useSimple) html += _getDetailsHtml(s, n);
            _self.html('<div class="paging_div">' + html + '</div>');
            _bindEvnet(s);
        }
        
        /**
        * 生成所有页号的html
        * @param {Number} n 当前页码
        * @param {Array} nArr 所有页号的数字
        * @return {String} html代码
        */
        function _getPagesHtml(n, nArr){
            var html = '';
            for(var i = 0; i < nArr.length; i++){
                var _num = nArr[i];
                if(n == _num){
                    html += '<li class="page ' + _activeClass + '"><a>' + _num + '</a></li>';
                }else if(_num == '...'){
                    html += '<li class="ellipsis">' + _num + '</li>';
                }else{
                    html += '<li class="page"><a>' + _num + '</a></li>';
                }
            }
            return html;
        }
        
        /**
        * 生成所有页号后面那一串html
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        * @return {String} html代码
        */
        function _getDetailsHtml(s, n){
            var html = '<div class="paging_div_right" style="white-space:nowrap"><span>共' + _totalCounts + '条记录 , </span>';
            html += '<span>每页</span><input type="text" class="size cloudjs_input" value="' + s + '"/><span>条 , </span>';
            html += '<span>到第</span><input type="text" class="num cloudjs_input" value="' + n + '"/><span>页</span> ';
            html += '<span class="cloudjs_btn">确定</span>';
            return html;
        }
        
        /**
        * 获取所有页号保存到数组中
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        * @return {Array} 要展示的所有页号
        */
        function _getPagesArr(s, n){
            var arr = [], total = Math.ceil(_totalCounts / s);
            if(_visiblePages == 0) return [];
            if(_visiblePages > total){
                _visiblePages = total;
            }
            var _half = Math.floor(_visiblePages / 2),
                _start = n - _half + 1 - _visiblePages % 2,
                _end = n + _half;
            if(_start < 1) {
                _start = 1;
                _end = _visiblePages;
            }
            if(_end > total) {
                _end = total;
                _start = 1 + total - _visiblePages;
            }
            var _index = _start;
            while(_index <= _end) {
                arr.push(_index);
                _index++;
            }
            if(_start > 1){
                if(_start > 2) arr.unshift('...');
                arr.unshift(1);
            }
            if(_end < total){
                if(_end < total - 1) arr.push('...');
                arr.push(total);
            }
            return arr;
        }
        
        /**
        * 生成(上/下/首/末)页按钮的html代码
        * @param {Array} arr [当前页码,当前按钮变灰时的页码,'当前按钮的class值','当前按钮的文本值']
        * @return {String} html代码
        */
        function _getBtnHtml(arr){
            var pclass = arr[2], aclass = '';
            if(_showIcons){
                aclass = ' class="paging_icons"';
                arr[3] = '<i class="cloudjs_icon"></i>';
            }
            if(arr[0] == arr[1]){
                pclass += ' ' + _disableClass;
            }
            return '<li class="' + pclass + '"><a' + aclass + '>' + arr[3] + '</a></li>';
        }
        
        /**
        * 绑定分页组件所有按钮的点击事件
        */
        function _bindEvnet(s){
            _self.find('li:not(.ellipsis)').bind('click', function(){
                var num;
                if($(this).hasClass(_disableClass) || $(this).hasClass(_activeClass)) return;
                if($(this).hasClass('first')){
                    num = _pageNum = 1;
                }else if($(this).hasClass('prev')){
                    num = --_pageNum;
                }else if($(this).hasClass('next')){
                    num = ++_pageNum;
                }else if($(this).hasClass('last')){
                    num = _pageNum = Math.ceil(_totalCounts / s);
                }else{
                    num = _pageNum = Number($(this).text());
                }
                $.extend(_self.data('initData'), { pageNum: num });
                _init(_pageSize, num);
            }).end().find('.cloudjs_btn').bind('click', function(){
                _reinit(Number($(this).prevAll('.size').val()), Number($(this).prevAll('.num').val()));
            }).end().find('input[type=text]').keyup(function(event){
                if(event.keyCode === 13){
                    var _parent = $(this).parent();
                    _reinit(Number(_parent.find('.size').val()), Number(_parent.find('.num').val()));
                }
            });
        }
        
        /**
        * 改变pageSize后重新赋值并初始化组件
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        */
        function _reinit(s, n){
            _pageSize = s;
            _pageNum = n;
            $.extend(_self.data('initData'), { pageSize: s, pageNum: n });
            _init(s, n);
        }

        return this;
    }
});