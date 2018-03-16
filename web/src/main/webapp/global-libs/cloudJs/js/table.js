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
 * 表格组件
 */

cloudjs.define({
    table: function(options){
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
            ajaxUrl: '',              //请求的url，留空则需要在onBodyChange事件中发起ajax请求，不留空则组件自动请求ajaxUrl，直接在onBodyChange事件中通过data.data获取数据
            params: '',               //请求的参数，查询操作时通过setData方法设置，在onBodyChange事件中用data.params获取
            dataType: 'json',         //请求ajaxUrl返回的数据类型，如果ajaxUrl为空，这个参数也没有意义
            sizeKey: 'pageSize',      //组件向外传递参数pageSize的key值
            numKey: 'pageNum',        //组件向外传递参数pageNum的key值
            loadImg: 'http://3gimg.qq.com/mig_op/cloudJs/images/paging_loading.gif',  //蒙版层加载图片的路径
            pagingFlag: 0,            //是否分页，0表示静态分页，负数不分页，正数动态分页
            tableData: [],            //每一列的数据
            tableTitle: '',           //表格标题
            maskId: null,             //要加蒙版层的id，如果不为空，ajax请求返回数据前会在此id区域加一层蒙版效果
            emptyData: '',
            onBodyChange: $.noop,                         //重构tbody和tfoot后触发的事件
            onHeadChange: $.noop,                         //重构thead后触发的事件
            onAjax: function(data){                       //返回数据列表
                if($.isPlainObject(data)){
                    return data.data;
                }else{
                    return data;
                }
            },
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
            _pagingFlag = defaults.pagingFlag,
            _tableData = defaults.tableData,
            _emptyData = defaults.emptyData,
            _tableTitle = defaults.tableTitle,
            _onBodyChange = defaults.onBodyChange,
            _onHeadChange = defaults.onHeadChange,
            _onAjax = defaults.onAjax,
            _dataList = null,
            _tableFlag = this.is('table'),
            _thead = _tableFlag ? [1] : [];
        
        _init(_pageSize, _pageNum);
        _tableFlag && _sortTable();
        
        /**
        * 初始化组件
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        */
        function _init(s, n){
            n = _verify(s, n);
            if(_tableFlag){
                _totalCounts = _self.find('tbody').children().length;
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
            if((_pagingFlag <= 0 && _dataList) || _tableFlag){
                func(s, n);
            }else{
                var params = _params, url = _ajaxUrl + '?', mask = _createMask();
                if(_pagingFlag >= 0){
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
                }
                if(url.charAt(url.length-1) === '&') url = url.substring(0, url.length - 1);             
                $.post(_ajaxUrl, params, function(data){
                    _dataList = _onAjax(data) || [];
                    if(_pagingFlag <= 0){
                        _totalCounts = _dataList.length;
                    }else if($.isFunction(_totalCounts)){
                        _totalCounts = Number(_totalCounts(data));
                    }
                    func(s, n);
                    if(_totalCounts === 0 && _emptyData){
                        var col = $('thead th:last',_self).index()+1;
                        $('tbody',_self).html('<tr><td style="text-align:center" colspan='+col+'>'+_emptyData+'</td></tr>');
                    }
                    if(mask != null) mask.remove();
                }, defaults.dataType);
            }
        }
        
        /**
        * 添加蒙版效果
        * @return {Object} 蒙版的jquery对象
        */
        function _createMask(){
            if(_maskId === '') return null;
            var mask = $('<div class="table_mask_box" style="display:none;"><div class="paging_mask"></div><img class="paging_loading" src="' + defaults.loadImg + '"/></div>').appendTo('body'),
                dataObj = _maskId ? $('#' + _maskId) : _self,
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
                        /*
                        if(_pos + maskObj.height > _winht){
                            _top = (maskObj.top + _winht + _sctop) / 2;
                        }else{
                            _top = (maskObj.top + maskObj.height / 2);
                        }
                        */
                        _top = (maskObj.top + maskObj.height / 2);
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
        * 生成表格组件
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        */
        function _renderHtml(s, n){
            var pagesHtml = '', tbody = [], tfoot = '', start = 0, limit = _pageSize, len = _tableFlag ? _totalCounts : _dataList.length;
            if(_pagingFlag >= 0){
                pagesHtml = _getPagesHtml(s, n);
                tfoot = '<tfoot><tr><td class="tpage" colspan="' + _tableData.length + '">' + pagesHtml + '</td></tfoot>';
            }
            
            if(typeof _dataList !== 'string'){
                if(_pagingFlag === 0){
                    start = (_pageNum - 1) * _pageSize;
                    (len < _pageNum * _pageSize) && (limit = len % _pageSize);
                }else if(len < limit || _pagingFlag < 0){
                    limit = len;
                }
                
                if(_thead.length === 0){
                    _thead = _getTheadArray();
                    tbody = _getTbodyArray(start, limit);
                    _tableTitle && (_tableTitle = '<caption>' + _tableTitle + '</caption>');
                    _self.html('<table class="paging_table">' + _tableTitle + '<thead>' + _thead.join('') + '</thead><tbody>' + tbody.join('') + '</tbody>' + tfoot + '</table>');
                    _onHeadChange(_self.find('thead'));
                    _onBodyChange(_self.find('tbody'));
                    _bindSortEvent();
                }else{
                    if(_pagingFlag >= 0){
                        _self.find('tfoot .tpage').html(pagesHtml);
                    }
                    if(_tableFlag){
                        _self.find('tbody tr').hide().slice(start, start + limit).show();
						_onBodyChange(s,n);
                    }else{
                        tbody = _getTbodyArray(start, limit);
                        _self.find('tbody').html(tbody.join(''));
                        _onBodyChange(_self.find('tbody'));
                    }
                }
            }else{
                _self.html(_dataList).find('tfoot .tpage').html(pagesHtml);
                _sortTable();
                _onHeadChange(_self.find('thead'));
                _onBodyChange(_self.find('tbody'));
            }
            _bindEvent(s);
        }
        
        /**
        * 生成table中的thead数组代码
        * @return {Array} thead的html数组代码
        */
        function _getTheadArray(){
            var theadArr = ['<tr>'], ihtml = '<i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i>';
            for(var i = 0; i < _tableData.length; i++){
                var obj = _tableData[i], css = '', flag = true, sk;
                if(obj.width) css += ' style="width:' + obj.width + '"';
                if(obj.className) css += ' class="' + obj.className + '"';
                if(obj.defaultSort && flag){
                    if(obj.defaultSort > 0){
                        ihtml = '<i class="cloudjs_icon up thissort"></i><i class="cloudjs_icon down"></i>';
                    }else{
                        ihtml = '<i class="cloudjs_icon up"></i><i class="cloudjs_icon down thissort"></i>';
                    }
                    flag = false;
                    _dataList.sort(_sort(obj.key, obj.sort, obj.defaultSort));
                }
                if(obj.sort){
                    theadArr.push('<th' + css + '><div class="th_div"><span>' + obj.title + '</span><div class="sort_icon ' + obj.sort + '">' + ihtml + '</div></div></th>');
                }else{
                    theadArr.push('<th' + css + '>' + obj.title + '</th>');
                }
            }
            theadArr.push('</tr>');
            return theadArr;
        }
        
        /**
        * 生成table中的tbody数组代码
        * @param {Number} start 开始记录数
        * @param {Number} limit 显示的记录数
        * @return {Array} tbody的html数组代码
        */
        function _getTbodyArray(start, limit){
            var tbodyArr = [];
            for(var j = start; j < start + limit; j++){
                var dataObj = _dataList[j], tr = '';
                for(var i = 0; i < _tableData.length; i++){
                    var obj = _tableData[i], css = '', key = obj.key, txt, temp, cname = obj.className, result = [], rex;
                    if(cname) css += ' class="' + cname + '"';
                    
                    if(typeof key === 'string'){
                        txt = dataObj[key];
                        if(/integer/.test(cname)){
                            if(isNaN(txt) || txt == null){
                                txt = '0';
                            }else if(txt >= 1000){
                                txt = (txt + '').replace(/(\d{1,3})(?=(?:\d{3})+(?!\d))/g, '$1,'); 
                            }
                        }
                        if(/percent/.test(cname)){
                            if(isNaN(txt) || txt == null){
                                txt = '0.00';
                            }
                            txt += '%';
                        }else if(key === 'statis_day' && txt.length === 8){
                            txt = txt.substring(0,4)+'-'+txt.substring(4,6)+'-'+txt.substring(6);
                        }
                    }else if($.isFunction(key)){
                        txt = temp = key();
                        rex = /\{(\w+)\}/;
                        while(rex.test(temp)){
                            result.push(RegExp.$1);
                            temp = temp.replace(rex, '');
                        }
                        for(var k = 0; k < result.length; k++){
                            txt = txt.replace(rex, dataObj[result[k]]);
                        }
                    }
                    tr += '<td' + css + '>' + txt + '</td>';
                }
                tbodyArr.push('<tr>' + tr + '</tr>');
            }
            return tbodyArr;
        }
        
        /**
        * 生成所有页号的html
        * @param {Number} n 当前页码
        * @param {Array} nArr 所有页号的数字
        * @return {String} html代码
        */
        function _getNumHtml(n, nArr){
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
        * 生成表格组件的分页代码
        * @param {Number} s 每页展示的记录数
        * @param {Number} n 当前页码
        * @param {String} html代码
        */
        function _getPagesHtml(s, n){
            var htmlArr = [], pagesHtml = '';
            if(_first){
                htmlArr.push(_getBtnHtml([n, 1, 'first', _first]));
            }
            if(_prev){
                htmlArr.push(_getBtnHtml([n, 1, 'prev', _prev]));
            }
            if(_useSimple){
                htmlArr.push('<li class="ellipsis">' + n + '/' + Math.ceil(_totalCounts / s) + '</li>');
            }else if(_visiblePages > 0){
                htmlArr.push(_getNumHtml(n, _getPagesArr(s, n)));
            }
            if(_next){
                htmlArr.push(_getBtnHtml([n, Math.ceil(_totalCounts / s), 'next', _next]));
            }
            if(_last){
                htmlArr.push(_getBtnHtml([n, Math.ceil(_totalCounts / s), 'last', _last]));
            }
            pagesHtml = '<ul>' + htmlArr.join('') + '</ul>';
            if(_showDetails && !_useSimple) pagesHtml += _getDetailsHtml(s, n);
            return '<div class="paging_div">' + pagesHtml + '</div>';
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
            html += '<span class="cloudjs_btn" id="table_paing_btn0">确定</span>';
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
        function _bindEvent(s){
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
            }).end().find('#table_paing_btn0').bind('click', function(){
                _reinit(Number($(this).prevAll('.size').val()), Number($(this).prevAll('.num').val()));
            }).end().find('input[type=text]').keyup(function(event){
                if(event.keyCode === 13){
                    var _parent = $(this).parent();
                    _reinit(Number(_parent.find('.size').val()), Number(_parent.find('.num').val()));
                }
            });
        }
        
        /**
        * 绑定排序的点击事件
        */
        function _bindSortEvent(){
            _self.find('.sort_icon').bind('click', function(){
                var cl = 'string', index = $(this).closest('th').index(), key = _tableData[index].key;
                $(this).closest('tr').find('.sort_icon i').removeClass('thissort');
                $(this).addClass('thissort');
                if($(this).hasClass('number')) cl = 'number';

                $(this).parents('tr').find('.sort_icon').removeClass('down').removeClass('up');
                if($.isFunction(key)){
                    key = key();
                    if(/\>.*\{(\w+)/.test(key) || /\{(\w+)\}/.test(key)){ 
                        key = RegExp.$1;
                    }
                }
                if($(this).data('sort')=='down'){
                    _dataList.sort(_sort(key, cl, 1));
                    $(this).removeClass('down').addClass('up').data('sort','up');
                }else{
                    _dataList.sort(_sort(key, cl, -1));
                    $(this).removeClass('up').addClass('down').data('sort','down');
                }
                _renderHtml(_pageSize, _pageNum);
            });
        }

        function _sortTable(){
            _self.find('.sort_icon').bind('click', function(){
                var cl = 'string', col = $(this).closest('th').index(), rows = $('tbody tr',_self).get(), key = 'sortKey';
                if($(this).hasClass('number')) cl = 'number';
                $(this).parents('tr').find('.sort_icon').removeClass('down').removeClass('up');

                $.each(rows, function(i, row){
                    row.sortKey = $(row).children().eq(col).text().replace(/,/g, "");
                });
                if($(this).data('sort')=='down'){
                    rows.sort(_sort(key, cl, 1));
                    $(this).removeClass('down').addClass('up').data('sort','up');
                }else{
                    rows.sort(_sort(key, cl, -1));
                    $(this).removeClass('up').addClass('down').data('sort','down');
                }
                $.each(rows, function(i, row){
                    $('tbody', _self).append(row);
                });
                _tableFlag && _self.find('tbody tr').hide().slice(_pageSize*(_pageNum-1), _pageSize*_pageNum).show();
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
        
        /**
        * 排序
        * @param {String} key 排序对象的key值
        * @param {String} cl 排序类型，字符串或数字
        * @param {Number} v 升降排序
        */
        function _sort(key, cl, v){
            return function(a, b){
                if(cl === 'number'){
                    if(parseFloat(a[key]) > parseFloat(b[key])){
                        return v;
                    }else if(parseFloat(a[key]) < parseFloat(b[key])){
                        return -v;
                    }
                }else{
                    if(a[key] > b[key]){
                        return v;
                    }else if(a[key] < b[key]){
                        return -v;
                    }
                }
            };
        }

        return this;
    }
});