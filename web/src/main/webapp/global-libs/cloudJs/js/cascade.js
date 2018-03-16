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
 * 级联组件
 */

cloudjs.define({
    cascade: function(options){
        var defaults = {
            casType: 'combobox',  //级联类型，combobox表示combobox下拉框，select表示源生态的下拉框
            allData: null, //级联的所有数据，可以是jsonArray也可以是一个url，如果allData不为null，则忽略casData里面的url属性
            casData: null, //jsonArray，级联各级的属性，每个数组项为如下注释的casObj
			onAjax: function(data){ if($.isPlainObject(data)) return data.data; else return data; },
			onSubAjax: function(data){ if($.isPlainObject(data)) return data.data; else return data; },
            sort: 0  //下拉列表是否按照text值排序，0不排，1正序，-1逆序，如果提供的数据本身就排序了，这里保持默认值0就好
        };
        
        /*
        var casObj = {
            id: '',  //当前级id
            url: '', //当前级url
            isMulti: '', //是否多选
            name: '', //当前级name
            width: '', //当前级的宽度
            firstOpt: '',  //指定当前级的第一条记录
            defaultValue: '',  //指定当前级的默认值
            valueName: 'value',  //指定当前级数据的valueName
            textName: 'text',   //指定当前级数据的valueName
            onSelect: null    //选择列表时触发的事件
        }
        */
        
        $.extend(defaults, options);
        var _casType = defaults.casType,
            _sort = Number(defaults.sort),
            _allData = defaults.allData,
            _casData = defaults.casData,
			_onAjax = defaults.onAjax,
			_onSubAjax = defaults.onSubAjax,
            _data = [],
            _ids = [],
            _names = [],
            _widths = [],
            _fOpts = [],
            _dValues = [],
            _vNames = [],
            _tNames = [],
            _dIndexs = [],
            _jqObjs = [],
            _callbacks = [],
            _isMulti = [],
            _level = 0,
            _casLen,
            TEMPKEY = 'X@#$%^q';
        
        if(!$.isArray(_casData)){
            alert('casData参数不合法');
            return;
        }
        _casLen = _casData.length;    
        for(var i = 0; i < _casLen; i++){
            var obj = _casData[i];
            obj.id && _ids.push(obj.id);
            obj.url && _data.push(obj.url);
            obj.name && _names.push(obj.name);
            obj.width && _widths.push(obj.width);
            obj.firstOpt && _fOpts.push(obj.firstOpt);
            obj.defaultValue && _dValues.push(obj.defaultValue);
            obj.onSelect && _callbacks.push(obj.onSelect);
            _vNames.push(obj.valueName || 'value');
            _tNames.push(obj.textName || 'text');
            _isMulti.push(obj.isMulti || false);
        }
        if(_casLen === 0 || _casLen !== _ids.length){
            alert('casData参数不合法');
            return;
        }
        if(($.isArray(_allData) || typeof _allData === 'string') && _allData.length > 0){
            _data = _allData;
        }
        
        if(typeof _data === 'string'){  //级联所有数据为一个url，需先请求此url生成数据
            $.getJSON(_data, function(data){
                _data = _onAjax(data);
                if(!$.isArray(_data) || _data.length === 0) return;
                _init();
            });
        }else{
            if(!$.isArray(_data) || _data.length === 0) return;
            _init();
        }
        
        /**
         * 初始化
         */
        function _init(){
            var i;
            if(_names.length != _casLen) _names = _ids;
            _getJqObjs();
            if($.isPlainObject(_data[0])){//级联所有数据写在jsonArray
                var data = _data, index = 0, key;
                for(i = 0; i < _casLen; i++){
                    if(i > 0) data = data[index].children;
                    key = _dValues.length > i ? _dValues[i] : TEMPKEY;
                    index = _createOptions(data, i, key);
                    _onChange(_jqObjs[i], i, true);
                    if(index === -1) break;
                }
            }else{//级联数据每级放在单独的url中
                for(i = 0; i < _casLen; i++){
                    _onChange(_jqObjs[i], i, false);
                }
                _ajaxData(0, '');
            }
        }
        
        /**
         * 请求url生成对应的数据,递归的作用自动获取参数并请求下一级的url
         * @param {Number} level 第level层
         * @param {String} params 前面级别的参数，比如index=2(第3级),params就是第1和第2级的参数 
         */
        function _ajaxData(level, params){
            var url = _data[level];
            if(params) url =  url.split('?')[0] + '?' + params;
            $.getJSON(url, function(data){
                var key = _dValues.length > level ? _dValues[level] : TEMPKEY;
                data = _onSubAjax(data);
                _createOptions(data, level, key);
                // if(level < _casLen - 1 && $.isArray(data) && data.length > 0){
                //     level++;
                //     _ajaxData(level, _getParams(level));
                // }
            });
        }
        /**
         * 获取url中的参数
         * @param {Number} level 第level层
         * @return {String} 参数
         */
        function _getParams(level){
            var params, url2 = _data[level], i;
            if(/\?/.test(url2)){
                params = url2.replace(/[^?]+\?/,'');
            }else{
                params = '';
            }
            if(/\{\d+\}/.test(url2)){
                for(i = 0; i < level; i++){
                    var reg = new RegExp('\\{' + i + '\\}', 'g');
                    params = params.replace(reg, _dValues[i]);
                }
            }else{
                for(i = 0; i < level; i++){
                    params += '&' + _names[i] + '=' + _dValues[i];
                }
                (params.charAt(0) === '&') && (params = params.substring(1));
            }
            return params;
        }
        /**
         * 获取级联select元素的jquery对象保存到_jqObjs数组中
         */
        function _getJqObjs(){
            var i, o, s;
            if($('#' + _ids[0]).is('select')){
                for(i = 0; i < _casLen; i++){
                    o = $('#' + _ids[i]), name = o.attr('name');
                    if(name){
                        _names[i] = name;
                    }else{
                        o.attr('name', _names[i]);
                    }
                    if(_widths.length > i && _widths[i] > 10) o.css('width', _widths[i] + 'px');
                    _jqObjs.push(o);
                }
            }else{
                for(i = 0; i < _casLen; i++){
                    o = $('#' + _ids[i]).html('<select name="' + _names[i] + '"></select>'), s = o.find('>select');
                    if(_widths.length > i && _widths[i] > 10) s.css('width', _widths[i] + 'px');
                    _jqObjs.push(s);
                }
            }
        }
        
        /**
         * 生成option代码并插入到对应的selelct中
         * @param {JSONArray} data 第level层的数据
         * @param {Number} level 第level层
         * @param {String} key 默认选中的值
         * @return {Number} 当前层选中的元素的位置编号
         */
        function _createOptions(data, level, key){
            var i;
            if(!$.isArray(data) || data.length === 0){
                if($.isArray(_fOpts) && _fOpts.length > level && $.isPlainObject(_fOpts[level])){
                    data = [_fOpts[level]];
                }else{
                    for(i = level; i < _casLen; i++){
                        _jqObjs[level].html('');
                        _dIndexs[level] = -1;
                        _dValues[level] = '';
                        if(_casType === 'combobox') cloudjs(_jqObjs[level]).combobox({ checkValue: true, isMulti: _isMulti[level] });
                    }
                    return -1;
                }
            }else if($.isArray(_fOpts) && _fOpts.length > level && $.isPlainObject(_fOpts[level])){
                if(_sort !== 0) data.sort(_sortString);
                data.splice(0, 0, _fOpts[level]);
            }
            var index = 0, htmls = [], vname = _vNames[level], tName = _tNames[level]; _level = level;
            for(i = 0; i < data.length; i++){
                var obj = data[i], selected = '';
                if(key === obj[vname]){
                    index = i;
                    selected = ' selected';
                }
                htmls.push('<option value="' + obj[vname] + '"' + selected + '>' + obj[tName] + '</option>');
            }
            _jqObjs[level].html(htmls.join(''));
            if(_casType === 'combobox') cloudjs(_jqObjs[level]).combobox({ checkValue: true, isMulti: _isMulti[level] });
            _dIndexs[level] = index;
            _dValues[level] = index === 0 ? data[0][vname] : key;
            return index;
        }
        
        /**
         * 每层级联的change事件
         * @param {Jquery Object} o 当前的jquery对象
         * @param {Number} level 第level层
         * @param {Boolean} flag true表示页面提供级联总数据,false表示页面提供每一级的接口
         */
        function _onChange(o, level, flag){
            o.bind('change', function(){
                var params = '', paramObj = {}, data, index = level, i;
                _dValues[level] = this.value;
                _dIndexs[level] = o.find('option:selected').index();
                while(index < _casLen - 1){
                    index++;
                    _dValues[index] = TEMPKEY;
                }
                if(level < _casLen - 1){
                    if(flag){ //总数据是json数组
                        data = _data;
                        for(i = 0; i < _casLen - 1; i++){
                            data = $.isArray(data) ? data[_dIndexs[i]].children : undefined;
                            if(level <= i){
                                _createOptions(data, i + 1, TEMPKEY);
                            }
                        }
                    }else{ //每级数据是url
                        if(_dValues[level] !== ''){
                            _ajaxData(level + 1, _getParams(level + 1));
                        }else{
                            for(i = 0; i < _casLen - 1; i++){
                                data = undefined;
                                if(level <= i){
                                    _createOptions(data, i + 1, TEMPKEY);
                                }
                            }
                        }
                    }
                }
                if($.isFunction(_callbacks[level])){
                    for(i = 0; i < _dValues.length; i++){
                        paramObj[_names[i]] = _dValues[i];
                    }
                    _callbacks[level](paramObj);
                }
            });
        }

        /**
         *根据_sort值排序,_sort为正数为正序排序，负数为逆序排序
         */
        function _sortString(a, b){
            return (a[_tNames[_level]] > b[_tNames[_level]]) ? _sort : -_sort;
        }

        return this;
    }
});