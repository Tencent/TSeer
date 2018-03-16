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

cloudjs.define({
    combobox: function(options){
        var defaults = {
            isMulti: false,    //是否多选
            width: null,    //下拉列表宽度
            height: 200,    //下拉列表高度
            placeholder: '',    //输入提示信息
            separator: ';',    //多选时，数据分隔符
            defaultValue: '',    //默认选中选项
            checkValue: false,    //是否检查value值必须是data里面的value
            showIcon: true, //是否显示下拉图标
            showAll: true,    //是否显示全部下拉
            recordLen: 10,    //搜索时最多显示的记录数
            name: '',    //value隐藏域
            data: null,    //data数据
            dataType: 'json',  //url接口类型
            comValue: 'value',    //value字段名
            comText: 'text',    //text字段名
            checkAllText: '全选',    //全选文字
            unCheckAllText: '取消选择',    //取消选择文字
            showCheckAllText: true,    //是否显示全选文字
            showCheckbox: true, //是否显示复选框
            params: '',    //异步搜索下拉数据参数
            onCreate: $.noop,    //创建后回调
            onAjax: $.noop, //异步请求后的处理回调
            onOpen: $.noop, //监听下拉面板展开
            onClose: $.noop, //监听下拉面板关闭
            onSelect: $.noop, //监听选择数据改变事件
            onCheckAll: $.noop, //全选事件
            onUnCheckAll: $.noop  //取消选择
        },
        _args0 = options,
        _args1 = arguments[1],
        _self = this;

        if(_self.length === 0){
            cloudjs.util.error('调用combobox下拉组件失败，' + _self.selector + ' 找不到相应的元素!');
            return;
        }

        if(!options || $.isPlainObject(options)){
            if(options && typeof options.defaultValue !== 'undefined'){
                defaults.hasDefaultValue = true;
            }
            $.extend(defaults, options);
        }else if(typeof _args0 === 'string'){
            if(_args0 === 'getValue'){
                cloudjs.callback(_self[0]._cloudjsComboObj[_args0]());
            }else{
                _self[0]._cloudjsComboObj && _self[0]._cloudjsComboObj[_args0](_args1);
            }
            return;
        }

        _self.each(function(){
            var self = this;

            self._cloudjsComboObj = self._cloudjsComboObj ? $.extend(self._cloudjsComboObj, _getComboObj()) : _getComboObj();

            _init.call(self);
        });


        function _init(){
            var self = $(this),
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                optionEles,
                tempObj;

            cObj.self = self;
            if(self.hasClass('combo_text') || self.parent().hasClass('combo_container')){
                self.siblings().remove();
            }else{
                if(self.prop('disabled')){
                    self.wrap('<span class="combo_container combo_disabled"></span>');
                }else{
                    self.wrap('<span class="combo_container"></span>');
                }
            }

            if(self.is('select')){
                cfg.hasDefaultValue = true;
                cfg.isSelectTag = true;
                cfg.checkValue = options && typeof options.checkValue !== 'undefined' ? options.checkValue : true;
                self.hide();
                self.parent().append('<input type="text" class="combo_text" />');
                cObj.showEle = self.siblings('.combo_text');
                cObj.showEle.css({ 'width': self.outerWidth() - 28 });
                if(cfg.isMulti){
                    self.prop('multiple', true);
                }else{
                    cfg.isMulti = self.prop('multiple');
                }

                if(!cfg.data){ //如果没有设置data，才去取select下拉列表的数据
                    if(cfg.isMulti){
                        cfg.defaultValue = cfg.defaultValue || (self.val() ? self.val().join(cfg.separator) : '');
                    }else{
                        cfg.defaultValue = cfg.defaultValue && self.find('option[value="' + cfg.defaultValue + '"]').not('.not_data').length !== 0 ? cfg.defaultValue : self.val();
                    }

                    cfg.data = [];
                    optionEles = self.find('option').not('.not_data');
                    for(var i = 0; i < optionEles.length; i++){
                        if(!optionEles.eq(i).attr('value')){
                            cfg.placeholder = cfg.placeholder || optionEles.eq(i).text();
                            continue;
                        }
                        tempObj = {};
                        tempObj[cfg.comValue] = optionEles.eq(i).attr('value');
                        tempObj[cfg.comText] = optionEles.eq(i).text();
                        tempObj.disabled = optionEles.eq(i).prop('disabled') ? 'true' : 'false';
                        cfg.data.push(tempObj);
                    }
                }
            }else{
                !self.hasClass('combo_text') && self.addClass('combo_text');
                cObj.showEle = self;
            }
            cObj.showEle.attr('placeholder', cfg.placeholder).css('border', '0');
            cObj.comboEle = cObj.showEle.parent().css({ 'zIndex': cloudjs.zIndex() });

            if(cfg.showIcon){
                self.parent().append('<input type="hidden" class="combo_value" name="' + cfg.name + '" /><span class="combo_arrow cloudjs_icon"><span></span></span>');
            }else{
                self.parent().append('<input type="hidden" class="combo_value" name="' + cfg.name + '" />');
            }
            cObj.hideEle = self.siblings('.combo_value');

            _setDefaultValue.call(this);    //设置默认值
            _bindEvent.call(this);
            cObj.create();
        }

        /**
         * 设置默认值
         * @param  {Array}  data 数据
         */
        function _setDefaultValue(data){
            var self = $(this),
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                ajaxParams,
                defValue = '',
                defText = '',
                valueArr,
                textArr;

            if(data){
                cfg.data = data;
            }
            if(cfg.hasDefaultValue){
                if(cfg.data && typeof cfg.data === 'object'){
                    if(cfg.isMulti){
                        cfg.defaultText = _getMultiValue.call(this, cfg.data, cfg.defaultValue);
                    }else{
                        if($.isArray(cfg.data)){
                            for(var i = 0; i < cfg.data.length; i++){
                                if(cfg.defaultValue === cfg.data[i][cfg.comValue]){
                                    defText = cfg.data[i][cfg.comText];
                                    defValue = cfg.data[i][cfg.comValue];
                                }
                            }
                            cfg.defaultText = defText;
                            cfg.defaultValue = defValue;
                        }else if(typeof cfg.data === 'string'){
                            cfg.url = cfg.data;
                        }
                    }
                    cfg.defaultValue = cfg.checkValue && cfg.defaultText === '' ? '' : cfg.defaultValue;

                    cObj.showEle.val(cfg.defaultText);
                    cObj.hideEle.val(cfg.defaultValue);
                    if(cfg.defaultValue !== cfg.selectValue && cfg.defaultValue !== (cfg.selectValue + cfg.separator)){
                        cObj.select(cfg.defaultValue, cfg.defaultText);
                    }

                    return;
                }

                if(cfg.url){
                    if(cfg.params){
                        if(cfg.defaultValue instanceof Object){
                            defValue = cfg.defaultValue[cfg.comValue];
                            defText = cfg.defaultValue[cfg.comText];
                        }else if(typeof cfg.defaultValue === 'string'){
                            defValue = cfg.defaultValue;
                            defText = cfg.defaultValue;
                        }

                        valueArr = defValue.split(cfg.separator);
                        textArr = defText.split(cfg.separator);

                        defText = '';
                        for(var j = 0; j < valueArr.length; j ++){
                            if(valueArr[j] !== ''){
                                cfg.selectedStr = cfg.selectedStr || '';
                                cfg.selectedStr += encodeURIComponent(textArr[j]) + '^true#' + encodeURIComponent(valueArr[j]) + ',';
                                defText += textArr[j] + ';';
                            }
                        }

                        if(defValue.lastIndexOf(cfg.separator) !== defValue.length - 1){
                            defValue += cfg.separator;
                        }

                        cObj.showEle.val(defText);
                        cObj.hideEle.val(defValue);
                        cfg.selectValue = defValue;
                        cfg.selectText = defText;
                    }else{
                        ajaxParams = _getParams.call(this);

                        cObj.comboEle.addClass('combo_disabled');
                        cObj.showEle.prop('disabled', true);

                        _getAjaxData.call(this, _setDefaultValue, false, ajaxParams);
                    }
                }
            }else{
                cObj.hideEle.val('');
            }
        }

        /**
         * 获取默认value对于的text [多选的时候]
         * @param  {Json}  data 数据
         * @param  {String} value  value字符串
         * @return {String}      返回vakye对应的text字符串
         */
        function _getMultiValue(data, value){
            var self = $(this),
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                valueArr,
                textStr = '';
            cfg.defaultValue = '';

            if(value.lastIndexOf(cfg.separator) === value.length - 1){
                value = value.substring(0, value.length - 1);
            }
            valueArr = value.split(cfg.separator);

            if($.isArray(data)){
                for(var i = 0; i < data.length; i++){
                    for(var j = 0; j < valueArr.length; j++){
                        if((cfg.separator + cfg.defaultValue).indexOf(cfg.separator + valueArr[j] + cfg.separator) === -1 && valueArr[j] === data[i][cfg.comValue]){
                            cfg.defaultValue += data[i][cfg.comValue] + cfg.separator;
                            textStr += data[i][cfg.comText] + cfg.separator;
                        }
                    }
                }
            }
            return textStr;
        }

        /**
         * 通过ajax获取数据
         * @param  {Function} callback 回调函数
         * @param  {Boolen} isAsync 是否异步加载
         * @param  {String/Object} ajaxParams ajax参数
         */
        function _getAjaxData(callback, isAsync, ajaxParams){
            var self = this,
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                isSearch,
                json;
            ajaxParams = ajaxParams || '';
            isSearch = ajaxParams === '' ? false : true;
            cfg.ajaxIng = true;

            $.ajax({
                url: cfg.url,
                type: 'post',
                dataType: cfg.dataType,
                data: ajaxParams,
                async: isAsync,
                success: function(data){
                    cfg.ajaxIng = false;
                    
                    json = cfg.onAjax(data) || data;

                    if(!isSearch && json && $.isArray(json) && json.length !== 0){
                        cfg.data = json;
                    }

                    callback.call(self, json, isSearch);
                },
                complete: function(){
                    cObj.comboEle.removeClass('combo_disabled');
                    cObj.showEle.prop('disabled', false);
                }
            });
        }

        /**
         * 改变下拉列表的值
         * @param  {Object} ele 当前操作的下拉元素
         * @param  {String} targetName 当前操作元素名称
         * @param  {Boolean} isChangeEvent 是否触发onSelect事件，全选/取消选择时，只触发一次
         */
        function _changeSelect(ele, targetName, isChangeEvent){
            var self = $(this),
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                lastValue = $.trim(cObj.hideEle.val()),
                lastText = $.trim(cObj.showEle.val()),
                curValue,
                textArr = lastText.split(cfg.separator),
                text;
            isChangeEvent = typeof isChangeEvent === 'undefined' ? true : isChangeEvent;
            ele = typeof ele !== 'undefined' ? $(ele) : cObj.panelEle.find('.combo_item_hover');
            targetName = targetName || '';

            if(targetName !== 'input'){
                if(ele.hasClass('combo_item_disabled')){
                    return;
                }
                if(ele.find('.combo_check').prop('checked')){
                    ele.find('.combo_check').prop('checked', false);
                }else{
                    ele.find('.combo_check').prop('checked', true);
                }
            }else{
                ele = ele.parent();
            }

            curValue = ele.attr('combo-value');
            text = ele.text();

            if(!cfg.isMulti){
                ele.addClass('combo_item_selected').siblings().removeClass('combo_item_selected');
                cObj.panelEle.hide();
                cObj.close();
            }else{
                for(var i = 0; i < textArr.length; i++){
                    if((',' + cfg.strData).indexOf(',' + encodeURIComponent(textArr[i]) + '^') === -1 || textArr[i] === ''){
                        textArr.splice(i, 1);
                    }
                }
                lastText = textArr.join(cfg.separator);
                if(lastText !== ''){
                    lastText += cfg.separator;
                }

                if(ele.find('.combo_check').prop('checked')){
                    ele.addClass('combo_item_selected');
                    if(cfg.params){
                        _checkInput.call(self[0], curValue, text, 'check');
                        return;
                    }
                    if((cfg.separator + lastValue).indexOf(cfg.separator + curValue + cfg.separator) === -1){
                        curValue = lastValue + curValue + cfg.separator;
                        text = lastText + text + cfg.separator;
                    }else{
                        curValue = lastValue;
                        text = lastText;
                    }
                }else{
                    ele.removeClass('combo_item_selected');
                    if(cfg.params){
                        _checkInput.call(self[0], curValue, text, 'unCheck');
                        return;
                    }
                    curValue = (cfg.separator + lastValue).replace(cfg.separator + curValue + cfg.separator, cfg.separator).replace(cfg.separator, '');
                    text = (cfg.separator + lastText).replace(cfg.separator + text + cfg.separator, cfg.separator).replace(cfg.separator, '');
                }
            }

            cObj.showEle.val(text);
            cObj.hideEle.val(curValue);
            if(isChangeEvent && lastValue !== curValue){    //value值真正改变才触发onSelect
                cObj.select(curValue, text);
            }
        }

        /**
         * 获取数据，并生成下拉html
         * @param  {Object}  data 下拉数据
         * @param  {Boolean} isSearch 是否是搜索
         * @return {String}           生成的html
         */
        function _formatDataToHtml(data, isSearch){
            var self = $(this),
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                panelHtml = '',
                value,
                text,
                disabledStr,
                isDisabled = false,
                isSelected = false,
                dataLen = 0,
                checkStr = '';

            cfg.lastData = data;
            cfg.selectValue = cfg.isFirst && typeof cfg.selectValue === 'undefined' ? cfg.defaultValue : cfg.selectValue;
            cfg.selectText = cfg.isFirst && typeof cfg.selectText === 'undefined' ? cfg.defaultText : cfg.selectText;

            if($.isArray(data)){
                for(var i = 0; i < data.length; i++){
                    value = data[i][cfg.comValue];
                    text = data[i][cfg.comText];
                    disabledStr = (!data[i].disabled || data[i].disabled === 'false') ? false : true;
                    isDisabled = false;

                    if(cfg.isFirst){
                        cfg.strData += encodeURIComponent(text) + '^' + encodeURIComponent(disabledStr) + '#' + encodeURIComponent(value) + ',';
                    }
                    if(data[i].disabled && data[i].disabled === 'true'){
                        isDisabled = true;
                    }

                    if(!(!cfg.showAll && i >= cfg.recordLen)){
                        if(cfg.isMulti){
                            isSelected = ((cfg.selectValue === value && cObj.showEle.val() !== '')  || (cfg.separator + cfg.selectValue).indexOf(cfg.separator + value + cfg.separator) !== -1) ? true : false;
                            checkStr = '<input style="' + (cfg.showCheckbox ? '' : 'display: none;') + '" type="checkbox" ' + (isDisabled ? 'disabled="disabled"' : '') + ' class="combo_check" ' + (isSelected ? 'checked="checked"' : '') + ' />';
                            panelHtml += '<div class="combo_item ' + (isDisabled ? 'combo_item_disabled' : '') + (isSelected ? ' combo_item_selected' : '') + '" combo-value="' + value + '">' + checkStr + text + '</div>';
                        }else{
                            isSelected = (cfg.selectValue === value && cObj.showEle.val() !== '') ? true : false;
                            panelHtml += '<div class="combo_item ' + (isDisabled ? 'combo_item_disabled' : '') + (isSelected ? ' combo_item_selected' : '') + '" combo-value="' + value + '">' + text + '</div>';
                        }
                    }
                }
            }
            if(cfg.isMulti && cfg.showCheckAllText && data && data.length !== 0){
                panelHtml = '<div class="combo_head"><span class="combo_close_btn cloudjs_icon"></span><span class="cloudjs_btn_white combo_check_all ' + (!cfg.isCheckAll ? '' : 'combo_uncheck_all') + '">' + (!cfg.isCheckAll ? cfg.checkAllText : cfg.unCheckAllText) + '</span></div>' + panelHtml;
            }

            return panelHtml;
        }

        /**
         * 初始化下拉面板
         * @param  {Object} data 下拉面板数据
         * @param  {String} isSearch 是否是搜索
         */
        function _initSearchPanel(data, isSearch){
            var self = this,
                cObj = this._cloudjsComboObj,
                cfg = cObj.cfg,
                isHidden,
                retStr,
                comboLeft = parseInt(cObj.comboEle.css('border-left-width'), 10),
                comboHeight = cObj.comboEle.outerHeight() - parseInt(cObj.comboEle.css('border-bottom-width'), 10) - parseInt(cObj.comboEle.css('border-top-width'), 10),
                scrollTop = $(window).scrollTop(),
                comboTop = cObj.comboEle.offset().top,
                screenHeight = $(window).height();

            cObj.comboEle.css({ 'zIndex': cloudjs.zIndex() });

            data = data || cfg.data;
            cfg.width = cfg.width || cObj.comboEle.innerWidth();

            if(cObj.comboEle.hasClass('combo_disabled') || cObj.showEle.prop('disabled')){
                return;
            }

            if(cfg.isFirst){
                cObj.panelEle && cObj.panelEle.remove();
                cObj.panelEle = $('<div></div>').addClass('combo_panel').css({ 'width': cfg.width, 'max-height': cfg.height, 'top': comboHeight, 'left': -comboLeft });
                if(cfg.isMulti){
                    cObj.panelEle.css({ 'min-width': '105px' });
                }
                cObj.comboEle.append(cObj.panelEle);
            }
            isHidden = cObj.panelEle.is(':hidden');
            
            if(data && typeof data !== 'string'){
                if(!isSearch && !cfg.isFirst){
                    if(typeof JSON !== 'undefined'){
                        if(JSON.stringify(data) === '[]'){
                            return;
                        }
                        if(JSON.stringify(cfg.lastData) !== JSON.stringify(data)){
                            retStr = _formatDataToHtml.call(self, data, isSearch);
                            if(!retStr){
                                return;
                            }
                        }
                    }else{
                        retStr = _formatDataToHtml.call(self, data, isSearch);
                        if(!retStr){
                            return;
                        }
                    }
                }else{
                    retStr = _formatDataToHtml.call(self, data, isSearch);
                    if(!retStr){
                        cObj.panelEle.hide().find('.combo_item_hover').removeClass('combo_item_hover');
                        return;
                    }
                }
                cObj.panelEle.html(retStr);

                if(comboTop - scrollTop > cObj.panelEle.outerHeight() && screenHeight + scrollTop < comboHeight + comboTop + cObj.panelEle.outerHeight()){
                    cObj.panelEle.css({bottom: comboHeight, top: 'auto'});
                }else{
                    cObj.panelEle.css({bottom: 'auto', top: comboHeight});
                }

                cObj.comboEle.css({ 'zIndex': cloudjs.zIndex() });
                cObj.panelEle.show().unbind('click').bind('click', function(e){
                    var target =  e.target,
                        curEle;

                    cfg.isSelectBlur = true;
                    if($(target).hasClass('combo_item')){
                        _changeSelect.call(self, target, target.tagName.toLowerCase());
                    }else if(target.tagName.toLowerCase() === 'input'){
                        if(cfg.isMulti){
                            _changeSelect.call(self, target, target.tagName.toLowerCase());
                        }else{
                            _changeSelect.call(self, $(target).closest('.combo_item'), $(target).closest('.combo_item')[0].tagName.toLowerCase());
                        }
                    }else if($(target).closest('.combo_item').length !== 0){
                        _changeSelect.call(self, $(target).closest('.combo_item'), $(target).closest('.combo_item')[0].tagName.toLowerCase());
                    }

                    e.stopPropagation();
                }).find('.combo_head').bind('mouseenter', function(e) {
                    cObj.panelEle.find('.combo_item').removeClass('combo_item_hover');
                }).find('.combo_check_all').unbind().bind('click', function(e){  //全选、取消选择
                    if($(this).hasClass('combo_uncheck_all')){
                        $(this).removeClass('combo_uncheck_all').html(cfg.checkAllText);
                        cObj.unCheckAll();
                        cfg.isCheckAll = false;
                    }else{
                        $(this).addClass('combo_uncheck_all').html(cfg.unCheckAllText);
                        cObj.checkAll();
                        cfg.isCheckAll = true;
                    }
                    cObj.panelEle.find('.combo_item').not('.combo_item_disabled').each(function(i){
                        var isChangeEvent = false;
                        $(this).find('.combo_check').prop('checked', cfg.isCheckAll);

                        if(i === cObj.panelEle.find('.combo_item').not('.combo_item_disabled').length - 1){    //防止多次触发onSelect
                            isChangeEvent = true;
                        }
                        _changeSelect.call(self, $(this).find('.combo_check'), 'input', isChangeEvent);
                    });

                    e.stopPropagation();
                }).end().find('.combo_close_btn').unbind().bind('click', function(e){
                    cObj.panelEle.hide();
                    cObj.close();

                    e.stopPropagation();
                });
                isHidden && cObj.open();
                if(cObj.panelEle.find('.combo_item_selected').length === 0){
                    cObj.panelEle.find('.combo_item:eq(0)').addClass('combo_item_hover');
                }

                cObj.panelEle.find('.combo_item').unbind().bind('mouseenter', function(){
                    $(this).addClass('combo_item_hover');
                }).bind('mouseleave', function(){
                    $(this).removeClass('combo_item_hover');
                });

                cfg.isFirst = false;
                return;    //如果有本地数据，则即使有url，也不会去请求
            }

            if(cfg.url && !cfg.params && !cfg.ajaxIng){
                _getAjaxData.call(self, _initSearchPanel, true);
            }
        }

        /**
         * 验证内容
         */
        function _checkInput(value, text, type){
            var self = this,
                cObj = self._cloudjsComboObj,
                cfg = cObj.cfg,
                lastValue = $.trim(cObj.hideEle.val()),
                lastText = $.trim(cObj.showEle.val()),
                valueArr = lastValue.split(cfg.separator),
                textArr = lastText.split(cfg.separator),
                newTextArr = [],
                newValueArr = [],
                newText = '',
                newValue = '',
                tmpArr,
                newStr = '';

            cfg.selectedStr = cfg.selectedStr || '';
            for(var i = 0; i < textArr.length; i ++){
                if(textArr[i] !== '' && !(type && type === 'unCheck' && text === textArr[i]) && (',' + cfg.selectedStr).indexOf(',' + encodeURIComponent(textArr[i]) + '^') !== -1){
                    newTextArr.push(textArr[i]);

                    tmpArr = (',' + cfg.selectedStr).split(',' + encodeURIComponent(textArr[i]) + '^');
                    tmpArr = tmpArr[1].split('#');
                    tmpArr = tmpArr[1].split(',');
                    newValueArr.push(decodeURIComponent(tmpArr[0]));

                    newStr += encodeURIComponent(textArr[i]) + '^true#' + tmpArr[0] + ',';
                }
            }

            if(type && type === 'check'){
                newTextArr.push(text);
                newValueArr.push(value);

                newStr += encodeURIComponent(text) + '^true#' + encodeURIComponent(value) + ',';
            }

            if(newValueArr.length === 0){
                cObj.panelEle.find('.combo_item').removeClass('combo_item_selected').find('.combo_check').prop('checked', false);
                newText = '';
                newValue = '';
            }else{
                newText = newTextArr.join(cfg.separator) + cfg.separator;
                newValue = newValueArr.join(cfg.separator) + cfg.separator;
            }

            cObj.showEle.val(newText);
            cObj.hideEle.val(newValue);
            cfg.selectedStr = newStr;
            cfg.selectValue = newValue;
            cfg.selectText = newText;
        }

        /**
         * 获取ajax参数
         * @param  {String} searchStr 需要替换的字符串
         * @return {String/Object}    替换后的ajax参数
         */
        function _getParams(searchStr){
            var self = this,
                cfg = self._cloudjsComboObj.cfg,
                ajaxParams = '';
            searchStr = searchStr || '';
            if(typeof cfg.params === 'string' && cfg.params.indexOf('{%s}') !== -1){
                ajaxParams = cfg.params.replace(/{%s}/g, searchStr);
            }else if($.isPlainObject(cfg.params)){
                ajaxParams = {};
                for(var key in cfg.params){
                    ajaxParams[key] = cfg.params[key];
                    if(cfg.params[key].indexOf('{%s}') !== -1){
                        ajaxParams[key] = ajaxParams[key].replace(/{%s}/g, searchStr);
                    }
                }
            }
            return ajaxParams;
        }

        /**
         * 创建combobox对象和其操作方法
         * @return {Object} 返回创建好的对象
         */
        function _getComboObj(){
            var comboObj = {
                cfg: {},
                create: function(){
                    this.cfg.onCreate.apply(this, arguments);
                },
                open: function(){
                    this.cfg.onOpen.apply(this, arguments);
                },
                close: function(){
                    this.cfg.onClose.apply(this, arguments);
                    this.comboEle.find('.combo_item_hover').removeClass('combo_item_hover');
                },
                checkAll: function(){
                    this.cfg.onCheckAll.apply(this, arguments);
                },
                unCheckAll: function(){
                    this.cfg.onUnCheckAll.apply(this, arguments);
                },
                select: function(value, text){
                    var valueArr,
                        data = {};
                    data[this.cfg.comValue] = value;
                    data[this.cfg.comText] = text;
                    this.cfg.selectValue = this.cfg.selectValue || '';
                    if(value !== this.cfg.selectValue){
                        this.cfg.onSelect.call(this, data);
                        this.cfg.selectValue = value;
                        this.cfg.selectText = text;
                    }

                    if(this.cfg.isSelectTag){
                        if(this.cfg.isMulti){
                            valueArr = value.split(this.cfg.separator);
                            this.self.prop('multiple', true).find('option').not('.not_data').each(function(){
                                if(!this.disabled){
                                    $(this).prop('selected', $.inArray(this.value, valueArr) > -1);
                                }
                            });
                        }else{
                            this.self.prop('multiple', false).find('option[value="' + value + '"]').not('.not_data').prop('selected', 'selected');
                        }
                        this.self.change();
                    }
                },
                getValue: function(){
                    var data = {};
                    data[this.cfg.comValue] = $.trim(this.hideEle.val());
                    data[this.cfg.comText] = $.trim(this.showEle.val());
                    return data;
                },
                setValue: function(value, check){
                    check = typeof check !== 'undefined' ? check : false;
                    if(typeof value === 'number'){
                        value = value.toString();
                    }
                    if(typeof value !== 'string'){
                        cloudjs.util.error('设置的value值格式不正式。');
                        return;
                    }

                    var cfg = this.cfg,
                        jsonData = cfg.data,
                        valueArr = [],
                        valueArrLen,
                        text = '',
                        setvalueStr,
                        separator = cfg.separator;

                    if(cfg.isMulti){
                        valueArr = value.split(separator);
                    }
                    valueArrLen = valueArr.length;
                    if(value.lastIndexOf(separator) !== -1 && value.lastIndexOf(separator) === value.length - 1){
                        valueArrLen = valueArrLen - 1;
                    }

                    if($.isArray(jsonData)){
                        for(var i = 0; i < jsonData.length; i++){
                            if(cfg.isMulti){
                                for(var j = 0; j < valueArrLen; j++){
                                    if(jsonData[i][cfg.comValue] == valueArr[j] && jsonData[i].disabled !== 'true'){
                                        setvalueStr = setvalueStr || '';
                                        text += jsonData[i][cfg.comText] + separator;
                                        setvalueStr += valueArr[j] + separator;
                                    }
                                }
                            }else{
                                if(jsonData[i][cfg.comValue] == value && jsonData[i].disabled !== 'true'){
                                    setvalueStr = value;
                                    text = jsonData[i][cfg.comText];
                                }
                            }
                        }
                    }

                    if(value === ''){
                        setvalueStr = value;
                    }

                    if(setvalueStr || setvalueStr === '' || check){
                        this.cfg.isFirst = true;
                        this.showEle.val(text);
                        this.hideEle.val(setvalueStr);
                        if(setvalueStr !== this.cfg.selectValue){
                            this.select(setvalueStr, text);
                        }
                        if(setvalueStr === '' && this.cfg.isSelectTag){
                            if(this.self.find('option[value=""]').length === 0){
                                this.self.prepend('<option class="not_data" value=""></option>');
                            }
                            this.self.find('.not_data').prop('selected', 'selected');
                        }
                    }else{
                        cloudjs.util.error('设置的value值不在下拉列表中。');
                    }
                },
                addData: function(data){
                    var cfg = this.cfg,
                        jsonData = cfg.data,
                        extraData = [],
                        newData = [],
                        isExist = false;
                    if($.isArray(jsonData) && $.isArray(data)){
                        for(var j = 0; j < data.length; j++){
                            for(var i = 0; i < jsonData.length; i++){
                                if(jsonData[i][cfg.comValue] === data[j][cfg.comValue]){
                                    isExist = true;
                                    break;
                                }
                            }
                            !isExist && extraData.push(data[j]);
                        }
                    }else{
                        cloudjs.util.error('新增数据格式不正确。');
                        return;
                    }

                    if(extraData.length !== 0){
                        newData = jsonData.concat(extraData);
                        this.refresh({data: newData});
                    }
                },
                delData: function(value){
                    if(typeof value !== 'string'){
                        cloudjs.util.error('删除的value值格式不正式。');
                        return;
                    }

                    var jsonData = this.cfg.data,
                        separator = this.cfg.separator,
                        valueArr = value.split(separator);

                    if($.isArray(jsonData)){
                        for(var i = 0; i < jsonData.length; i++){
                            for(var j = 0; j < valueArr.length; j++){
                                if(jsonData[i][this.cfg.comValue] === valueArr[j]){
                                    jsonData.splice(i, 1);
                                    i--;
                                }
                            }
                        }
                    }
                    
                    this.refresh({data: jsonData});
                },
                refresh: function(obj){
                    this.cfg.isFirst = true;
                    this.cfg.strData = '';

                    var newCfg = {}, selValue = typeof this.cfg.selectValue !== 'undefined' ? this.cfg.selectValue : this.cfg.defaultValue;
                    
                    if(obj){
                        newCfg = obj;
                        if(!$.isPlainObject(newCfg)){
                            cloudjs.util.error('刷新的数据格式不正式。');
                            return;
                        }
                        this.cfg.data = newCfg.data || this.cfg.data;
                        this.cfg.isMulti = typeof newCfg.isMulti !== 'undefined' ? newCfg.isMulti : this.cfg.isMulti;
                    }else{
                        if(this.cfg.isSelectTag){
                            this.cfg.data = null;
                            this.cfg.isMulti = newCfg.isMulti = this.self.prop('multiple');
                        }
                    }

                    if(typeof newCfg.defaultValue !== 'undefined'){
                        this.cfg.defaultValue = newCfg.defaultValue;
                    }else{
                        this.cfg.defaultValue = this.hideEle.val();
                    }
                    $.extend(this.cfg, newCfg);
                    _init.call(this.self[0]);
                },
                disable: function(){
                    this.comboEle.addClass('combo_disabled');
                    this.showEle.prop('disabled', true);
                },
                enable: function(){
                    this.comboEle.removeClass('combo_disabled');
                    this.showEle.prop('disabled', false);
                }
            };
            $.extend(comboObj.cfg, defaults);
            comboObj.cfg.isFirst = true;
            comboObj.cfg.isSelectTag = false;
            comboObj.cfg.defaultText = '';
            comboObj.cfg.strData = '';
            comboObj.cfg.ajaxIng = false;
            comboObj.cfg.searchTimeout = null;
            comboObj.cfg.comboObj = null;
            comboObj.cfg.lastData = null;
            comboObj.cfg.isContextMenu = false;
            comboObj.cfg.isSelectBlur = false;
            comboObj.cfg.url = typeof comboObj.cfg.data === 'string' ? comboObj.cfg.data : '';

            return comboObj;
        }


        function _bindEvent(){
            var wSefl = this,
                wComboObj = wSefl._cloudjsComboObj,
                wCfg = wComboObj.cfg;

            wComboObj.comboEle.unbind('click').bind('click', function(e){
                var self = $(this).children().eq(0)[0],
                    cObj = self._cloudjsComboObj;

                cObj.showEle.focus();
                e.stopPropagation();
            });

            wComboObj.showEle.unbind().bind('focus', function(e){
                var self = $(this).parent().children().eq(0)[0];
                _initSearchPanel.call(self);
            }).bind('click', function(e){
                var self = $(this).parent().children().eq(0)[0];
                $('.combo_panel:visible').each(function(){
                    var sub_self = $(this).parent().children().eq(0)[0];

                    if(self !== sub_self){
                        sub_self._cloudjsComboObj.panelEle.hide();
                        sub_self._cloudjsComboObj.close();
                    }
                });
                e.stopPropagation();
            }).bind('keyup', function(e){
                var self = $(this).parent().children().eq(0)[0],
                    cObj = self._cloudjsComboObj,
                    cfg = cObj.cfg,
                    text = $.trim($(this).val()),
                    exg,
                    macthData = null,
                    dataArr = [],
                    ajaxParams,
                    searchFun,
                    tempObj;

                if(e.keyCode === 37 || e.keyCode === 38 || e.keyCode === 39 || e.keyCode === 40 || e.keyCode === 46 || e.keyCode === 13){
                    return;
                }

                clearTimeout(cfg.searchTimeout);
                searchFun = function(){
                    text = $.trim(cObj.showEle.val());
                    if(cfg.isMulti){
                        text = text.split(cfg.separator)[text.split(cfg.separator).length - 1];
                    }
                    exg = 'var exg = /[^,]*' + encodeURIComponent(text) + '[^,]*#[^,]*/gi';
                    eval(exg);

                    if(cfg.params){
                        ajaxParams = _getParams.call(self, text);
                        
                        _getAjaxData.call(self, _initSearchPanel, true, ajaxParams);
                        return;
                    }

                    if(decodeURIComponent(cfg.strData).toLowerCase().indexOf(text.toLowerCase()) === -1){
                        macthData = [];
                    }else{
                        macthData = cfg.strData.match(exg);
                    }
                    if(!macthData){
                        macthData = [];
                    }

                    for(var i = 0; i < macthData.length; i++){
                        tempObj = {};
                        tempObj[cfg.comValue] = decodeURIComponent(macthData[i].split('#')[1]);
                        tempObj[cfg.comText] = decodeURIComponent(macthData[i].split('#')[0]).split('^')[0];
                        tempObj.disabled = decodeURIComponent(macthData[i].split('#')[0]).split('^')[1];

                        if(tempObj[cfg.comText].toLowerCase().indexOf(text.toLowerCase()) !== -1){
                            dataArr.push(tempObj);
                        }
                    }
                    
                    _initSearchPanel.call(self, dataArr, text);
                };

                cfg.searchTimeout = setTimeout(searchFun, 300);
            }).bind('keydown', function(e){
                var self = $(this).parent().children().eq(0)[0],
                    cObj = self._cloudjsComboObj,
                    cfg = cObj.cfg,
                    panelElement = cObj.panelEle,
                    text = $.trim($(this).val()),
                    itemLen = cObj.panelEle.children().length,
                    hoverEle = cObj.panelEle.find('.combo_item_hover'),
                    curEle,
                    hoverIndex = hoverEle.length === 0 ? -1 : hoverEle.index();

                cfg.isSelectBlur = false;
                if(e.keyCode === 13){
                    if(hoverIndex === -1){
                        cObj.showEle.blur();
                        return false;
                    }

                    _changeSelect.call(self);
                    if(!cfg.isMulti){
                        cObj.showEle.blur();
                    }
                    return;
                }else if(e.keyCode === 38){
                    if(hoverIndex === -1){
                        hoverIndex = itemLen - 1;
                    }else{
                        --hoverIndex;
                    }
                    if(panelElement.children().eq(hoverIndex).hasClass('combo_head')){
                        --hoverIndex;
                    }
                    
                    panelElement.children().removeClass('combo_item_hover');
                    if(hoverIndex !== -1){
                        panelElement.children().eq(hoverIndex).addClass('combo_item_hover');
                    }else{
                        return;
                    }

                    //滚动条
                    if(panelElement.find('.combo_item_hover').position().top > panelElement.scrollTop()){
                        panelElement.scrollTop(panelElement.find('.combo_item_hover').position().top);
                    }else if(panelElement.find('.combo_item_hover').position().top < 0){
                        panelElement.scrollTop(panelElement.scrollTop() - panelElement.find('.combo_item_hover').outerHeight());
                    }
                }else if(e.keyCode === 40){
                    if(hoverIndex < itemLen){
                        ++hoverIndex;
                    }else{
                        hoverIndex = 0;
                    }
                    if(panelElement.children().eq(hoverIndex).hasClass('combo_head')){
                        ++hoverIndex;
                    }
                    
                    panelElement.children().removeClass('combo_item_hover');
                    if(hoverIndex !== itemLen){
                        panelElement.children().eq(hoverIndex).addClass('combo_item_hover');
                    }else{
                        return;
                    }

                    //滚动条
                    if(panelElement.find('.combo_item_hover').position().top >= panelElement.outerHeight() - panelElement.find('.combo_item_hover').outerHeight()){
                        panelElement.scrollTop(panelElement.scrollTop() + panelElement.find('.combo_item_hover').outerHeight());
                    }else if(panelElement.find('.combo_item_hover').position().top < 0){
                        panelElement.scrollTop(0);
                    }
                }
            }).bind('blur', function(){
                var self = $(this).parent().children().eq(0)[0],
                    cObj = self._cloudjsComboObj,
                    cfg = cObj.cfg,
                    text = $.trim(cObj.showEle.val()),
                    textIndex,
                    lastValue = $.trim(cObj.hideEle.val()),
                    lastText = cfg.selectText,
                    curValue = '',
                    curText = $.trim(cObj.showEle.val()),
                    valueArr = [],
                    valueArrLen,
                    arrLen;

                if(cfg.isSelectBlur){
                    cfg.isSelectBlur = false;
                    return;
                }

                if(!cfg.checkValue && !cfg.isMulti){
                    if(curText !== lastText){
                        curValue = curText;
                        if(cfg.isSelectTag){
                            $(self).prepend('<option class="not_data" value="' + curValue + '">' + curText + '</option>');
                            $(self).find('.not_data').prop('selected', 'selected');
                        }
                    }else{
                        return;
                    }
                }else{
                    if(!text){
                        cObj.hideEle.val('');
                        if(cfg.isSelectTag){
                            if($(self).find('option[value=""]').length === 0){
                                $(self).prepend('<option class="not_data" value=""></option>');
                            }
                            $(self).find('.not_data').prop('selected', 'selected');
                        }
                    }else{
                        if(cfg.isMulti){
                            if(cfg.params){
                                _checkInput.call(self);
                                return;
                            }

                            valueArr = lastValue.split(cfg.separator);
                            valueArrLen = valueArr.length;
                            if(lastValue.lastIndexOf(cfg.separator) !== -1 && lastValue.lastIndexOf(cfg.separator) === lastValue.length - 1){
                                valueArrLen = valueArrLen - 1;
                            }

                            
                            text = '';

                            for(var i = 0; i < valueArrLen; i++){
                                if((',' + cfg.strData).indexOf('#' + encodeURIComponent(valueArr[i]) + ',') !== -1){
                                    arrLen = cfg.strData.split('#' + encodeURIComponent(valueArr[i]) + ',')[0].split(',').length;
                                    if((cfg.separator + curText + cfg.separator).indexOf(cfg.separator + decodeURIComponent(cfg.strData.split('#' + encodeURIComponent(valueArr[i]) + ',')[0].split(',')[arrLen - 1].split('^')[0]) + cfg.separator) !== -1){
                                        text += decodeURIComponent(cfg.strData.split('#' + encodeURIComponent(valueArr[i]) + ',')[0].split(',')[arrLen - 1].split('^')[0]) + cfg.separator;
                                        curValue += valueArr[i] + cfg.separator;
                                    }else{
                                        cObj.panelEle.find('.combo_item[combo-value="' + valueArr[i] + '"] .combo_check').removeAttr('checked');
                                        cObj.panelEle.find('.combo_item[combo-value="' + valueArr[i] + '"]').removeClass('combo_item_selected');
                                    }
                                }else{
                                    cObj.panelEle.find('.combo_item[combo-value="' + valueArr[i] + '"] .combo_check').removeAttr('checked');
                                    cObj.panelEle.find('.combo_item[combo-value="' + valueArr[i] + '"]').removeClass('combo_item_selected');
                                }
                            }
                        }else{
                            textIndex = (',' + cfg.strData).indexOf(',' + encodeURIComponent(text) + '^');
                            if(textIndex !== -1){
                                curValue = decodeURIComponent((',' + cfg.strData).split(',' + encodeURIComponent(text) + '^')[1].split(',')[0].split('#')[1]);
                                text = cObj.showEle.val();
                            }else{
                                curValue = '';
                                text = '';
                                if(cfg.isSelectTag){
                                    if($(self).find('option[value=""]').length === 0){
                                        $(self).prepend('<option class="not_data" value=""></option>');
                                    }
                                    $(self).find('.not_data').prop('selected', 'selected');
                                }
                            }
                        }
                        cObj.showEle.val(text);
                    }
                }

                if(curValue !== lastValue){
                    cObj.hideEle.val(curValue);
                    cObj.select(curValue, text);
                }
            }).bind('input', function(){
                $(this).keyup();
            }).bind('contextmenu', function(){
                wCfg.isContextMenu = true;
            });

            if('onpropertychange' in this._cloudjsComboObj.showEle[0]){
                wComboObj.showEle[0].onpropertychange = function(){
                    var self = $(this).parent().children().eq(0)[0],
                        cObj = self._cloudjsComboObj,
                        cfg = cObj.cfg;

                    if(cfg.isContextMenu && !cfg.isFirst){
                        cObj.showEle.keyup();
                        cfg.isContextMenu = false;
                    }
                };
            }

            $('html').bind('click', function(){
                $('.combo_panel:visible').each(function(){
                    var self = $(this).parent().children().eq(0)[0];

                    self._cloudjsComboObj.panelEle.hide();
                    self._cloudjsComboObj.close();
                });
            });
        }

        return this;
    }
});