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
    ctab: function(options){

        var defaults = {
            width: null, //外层容器的宽度
            height: null, //外层容器的高度
            panels: [], //打开的tab对象，可为object，也可为array
            showCloseOtherIcon: false, //是否显示关闭其他的按钮
            disableClose: false, //不允许显示tab关闭按钮
            showCopyIcon: false, //是否显示复制iframe链接的url
            enableHover: false, //是否允许鼠标hover的时候选中tab
            hoverDelay: 200, //允许hover情况下，鼠标移入选中的延迟时间
            isSortable: false, //是否允许tab拖动排序
            maxNum: Infinity, //允许打开的最多tab个数
            fit: false, //自动适应父元素的宽高
            onInitTab: $.noop, //初始化tab之后的回调
            onChangeTab: $.noop //tab变化之后的回调
        };

        var _self = this,
            _args = arguments,
            _CTAB_MAX_WIDTH = 150,
            _CTAB_BORDER_WIDTH = 2,
            _CTAB_MARGIN= 5,
            _CTAB_ZINDEX = 10,
            _CTAB_DRAGING_ZINDEX = 1000,
            _defaultPanel = {
                type: 'html',
                title: '',
                html: '',
                url: '',
                showCloseIcon: true,
                showRefreshIcon: false,
                isSelected: false
            },
            _width,
            _height,
            _panels,
            _disableClose,
            _showCloseOtherIcon,
            _enableHover,
            _hoverDelay,
            _isSortable,
            _maxNum,
            _fit,
            _onInitTab,
            _onChangeTab;


        (cloudjs.ctab._id === undefined) && (cloudjs.ctab._id = 0);

        if(!options || $.isPlainObject(options)){
            $.extend(defaults, options);
            _self.each(function(i, ele){
                this._cloudjsCtabObj = {
                    panels: [],
                    maxNum: Infinity,
                    resizeTimeout: null,
                    hoverTimeout: null,
                    hoverDelay: null,
                    onChangeTab: null
                };
            });
        }else if(options === 'addTab'){
            _self.each(function(i, ele){
                if(!_args[1]){
                    return;
                }
                var panels, onAddTab = $.noop;
                if(!$.isArray(_args[1]) && _args[1].panels){
                    panels = _args[1].panels;
                    onAddTab = _args[1].onAddTab || $.noop;
                }else{
                    panels = _args[1];
                }
                var result = _doAddTab.call(this, panels);
                onAddTab.call(this, result);
            });
            return true;
        }else if(options === 'selectTab'){
            _self.each(function(i, ele){
                var indexs = _matchCondition.call(this, _args[1]);
                var result = _doSelectTab.call(this, indexs[0]);
                var onSelectTab = $.noop;
                if(_args[1] && $.isPlainObject(_args[1])){
                    onSelectTab = _args[1].onSelectTab || $.noop;
                }
                onSelectTab.call(this, result);
            });
            return true;
        }else if(options === 'removeTab'){
            _self.each(function(i, ele){
                var indexs = _matchCondition.call(this, _args[1]);
                var result = _doRemoveTab.call(this, indexs);
                var onRemoveTab = $.noop;
                if(_args[1] && $.isPlainObject(_args[1])){
                    onRemoveTab = _args[1].onRemoveTab || $.noop;
                }
                onRemoveTab.call(this, result);
            });
            return true;
        }else if(options === 'refreshTab'){
            _self.each(function(i, ele){
                var indexs = _matchCondition.call(this, _args[1]);
                var result = _doRefreshTab.call(this, indexs);
                var onRefreshTab = $.noop;
                if(_args[1] && $.isPlainObject(_args[1])){
                    onRefreshTab = _args[1].onRefreshTab || $.noop;
                }
                onRefreshTab.call(this, result);
            });
            return true;
        }else if(options === 'getTab'){
            _self.each(function(i, ele){
                var indexs = _matchCondition.call(this, _args[1]);
                var result = _doGetTab.call(this, indexs);
                var onGetTab = $.noop;
                if(_args[1] && $.isPlainObject(_args[1])){
                    onGetTab = _args[1].onGetTab || $.noop;
                }
                onGetTab.call(this, result);
            });
            return true;
        }else if(options === 'updateTab'){
            _self.each(function(i, ele){
                var indexs = _matchCondition.call(this, _args[1]);
                var result = _doUpdateTab.call(this, indexs, _args[1].panel || {});
                var onUpdateTab = $.noop;
                if(_args[1] && $.isPlainObject(_args[1])){
                    onUpdateTab = _args[1].onUpdateTab || $.noop;
                }
                onUpdateTab.call(this, result);
            });
            return true;
        }else{
            return;
        }

        _width = parseInt(defaults.width);
        _height = parseInt(defaults.height);
        _panels = defaults.panels;
        _disableClose = defaults.disableClose || false;
        _showCloseOtherIcon = defaults.showCloseOtherIcon || false;
        _enableHover = defaults.enableHover || false;
        _hoverDelay = defaults.hoverDelay || 200;
        _isSortable = defaults.isSortable || false;
        _maxNum = defaults.maxNum || Infinity;
        _fit = defaults.fit || false;
        _onInitTab = defaults.onInitTab || $.noop;
        _onChangeTab = defaults.onChangeTab || $.noop;

        _init();
        return self;

        /**
         * 初始化消息组件
         */
        function _init(){
            _self.each(function(){
                (function(){
                    var self = this, panels =  _initArgs(_panels);
                    self._cloudjsCtabObj = {
                        width: _width,
                        height: _height,
                        maxNum: _maxNum,
                        enableHover: _enableHover,
                        isSortable: _isSortable,
                        hoverDelay: _hoverDelay,
                        onChangeTab: _onChangeTab,
                        panels: [],
                        tabHead: null,
                        tabUl: null,
                        tabBody: null,
                        selectedList:[]
                    };
                    _getDataFromDom.call(self, panels);
                    _renderHtml.call(self);
                    _setStyle.call(self);
                    _bindWholeEvent.call(self);

                    var result = _doAddTab.call(self, panels);
                    _onInitTab.call(self, result);
                }).call(this);
            });
        }

        /**
         * 初始化数据
         * @param {object | array} panels 每个tab节点的数据
         * @return {array} 初始化过的节点数据列表
         */
        function _initArgs(panels){
            if($.isPlainObject(panels)){
                panels = [panels];
            }else if(!$.isArray(panels)){
                panels = [];
            }
            var temp, tempArr = [];
            for(var i = 0, len = panels.length; i < len; i++){
                if($.isPlainObject(panels[i])){
                    temp = $.extend({}, _defaultPanel);
                    tempArr.push($.extend(temp, panels[i]));
                }
            }
            return tempArr;
        }

        /**
         *  从dom节点中获取tab的数据
         *  @param {array} panels tab节点初始化数据
         */
        function _getDataFromDom(panels){
            var self = this, cObj = self._cloudjsCtabObj,
                ul = $(self).children('ul'),
                div = $(self).children('div'),
                lis = ul.children('li'),
                divs = div.children('div'),
                title, panelHtml, result = false, panelExist;
            for(var i = 0, len = lis.length; i < len; i++){
                result = true;
                title = lis.eq(i).html();
                panelHtml = '';
                panelExist = !!divs.eq(i).length;
                if(panelExist){
                    panelHtml = divs.eq(i).html();
                }
                if(i < panels.length){
                    (!panels[i].title) && (panels[i].title = title);
                    (!panels[i].html) && (panels[i].html = panelHtml);
                }else{
                    panels.push({ title: title, html: panelHtml });
                }
                if(panelExist){
                    panels[i].panelDiv = divs.eq(i).clone(true);
                }
            }
            return true;
        }

        /**
         * 生成html节点，无默认节点的情况下
         */
        function _renderHtml(){
            var self = this, cObj = self._cloudjsCtabObj, baseHtml;
            $(self).addClass('ctab_div').show();
            baseHtml = '<div class="ctab_wrap' + (_showCloseOtherIcon ? ' ctab_wrap_with_icon' : '') + '">'
                + '<ul class="ctab_head"></ul>'
                + (_showCloseOtherIcon ? '<div class="ctab_close_other_icon" title="关闭其他"><span class="ctab_icon_span cloudjs_icon"></span></div>' : '')
                + '</div>'
                + '<div class="ctab_body"></div>';
            $(self).html(baseHtml);
        }

        /**
         * 绑定全局的事件，如关闭其他按钮事件
         */
        function _bindWholeEvent(){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self);
            if(_showCloseOtherIcon){
                $('.ctab_close_other_icon', myTab.tabHead).bind('click', function(e){
                    e.stopPropagation();
                    _closeOtherTabs.call(self);
                });
            }
        }

        /**
         * 绑定tab上的事件
         * @param {object} li 相应的tab对应的li元素的jquery对象
         */
        function _bindEachEvent(li){
            var self = this, cObj = self._cloudjsCtabObj,
                index, target, crtTarget;
            li.bind('click', function(e){
                target = $(e.target);
                crtTarget = $(e.currentTarget);
                index = crtTarget.index();
                if(target.hasClass('ctab_close_icon')){
                    _doRemoveTab.call(self, [index]);
                }else if(target.hasClass('ctab_refresh_icon')){
                    _doRefreshTab.call(self, [index]);
                }
                e.stopPropagation();
            }).bind('mousedown', function(e){
                target = $(e.target);
                if(target.hasClass('ctab_close_icon')){
                    return;
                }else if(target.hasClass('ctab_refresh_icon')){
                    return;
                }else{
                    crtTarget = $(e.currentTarget);
                    index = crtTarget.index();
                    _doSelectTab.call(self, index);
                }
            });
            if(cObj.enableHover){
                li.bind('mouseenter', function(e){
                    clearTimeout(cObj.hoverTimeout);
                    cObj.hoverTimeout = setTimeout(function(){
                        crtTarget = $(e.currentTarget);
                        index = crtTarget.index();
                        e.stopPropagation();
                        _doSelectTab.call(self, index);
                    }, cObj.hoverDelay);
                }).bind('mouseleave', function(){
                    clearTimeout(cObj.hoverTimeout);
                });
            }
            if(cObj.isSortable){
                _doDragTab.call(self, li);
            }
        }

        /**
         * 获取tab div上的各个元素的jquery对象,部分jq对象已缓存
         */
        function _getTabJqNode(){
            var self = this, cObj = self._cloudjsCtabObj,
                tabHead = cObj.tabHead || ($(self).children('.ctab_wrap')),
                tabUl = cObj.tabUl || ($(self).find('>.ctab_wrap>.ctab_head')),
                tabLis = tabUl.children('li'),
                tabBody = cObj.tabBody || ($(self).children('.ctab_body')),
                tabPanels = tabBody.children('.ctab_panel');
            return { tabHead: tabHead, tabUl: tabUl, tabLis: tabLis, tabBody: tabBody, tabPanels: tabPanels };
        }

        /**
         *  添加tab选项卡
         *  @param {array} panels 初始化过的每个tab节点的数据
         *  @return {array} 添加的tab相关数据
         */
        function _doAddTab(panels){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                item, liHtml, selectIndex = -1, nowNum, length, eachWidth, newLis = [], newLi, tabId, indexs=[], i, len;
            panels = _initArgs.call(self, panels);
            nowNum = cObj.panels.length || 0;
            length = Math.min(panels.length, cObj.maxNum - nowNum);
            if(length > 0){
                panels.splice(length);
                cObj.panels = cObj.panels.concat(panels);
                for(i = 0; i < length; i++){
                    tabId = cloudjs.ctab._id++;
                    item = panels[i];
                    if(item.isSelected){
                        selectIndex = i;
                    }
                    liHtml = '<li id = "ctab_' + tabId + ' " class="ctab_tab">';
                    liHtml += _renderTab(item);
                    liHtml += '</li>';
                    newLi = $(liHtml).appendTo(myTab.tabUl);
                    newLis.push(newLi);
                    if(item.panelDiv){
                        if(item.type === 'html'){
                            item.panelDiv.data('tabloaded', true);
                        }
                        item.panelDiv.addClass('ctab_panel').attr('id', 'ctab_panel_' + tabId).hide().appendTo(myTab.tabBody);
                        delete(item.panelDiv);
                    }else{
                        $('<div id="ctab_panel_' + tabId + '" class="ctab_panel" style="display: none"></div>').appendTo(myTab.tabBody);
                    }

                    indexs.push(nowNum + i);
                    cObj.selectedList.push(nowNum + i);
                }
                myTab.tabLis = myTab.tabUl.children('li');
                //绑定事件
                for(i = 0, len = newLis.length; i < len; i++){
                    _bindEachEvent.call(self, $(newLis[i]));
                }

                eachWidth = _getTabWidth.call(self, myTab.tabUl.width(), nowNum + length);
                myTab.tabLis.css({ width: eachWidth });

                if(selectIndex === -1){
                    selectIndex = myTab.tabLis.length - 1;
                }
                selectIndex += nowNum;
                _doSelectTab.call(self, selectIndex);
            }
            return _doGetTab.call(self, indexs);
        }

        /**
         *  渲染tab的html元素
         *  @param {object} panel 选项卡的参数
         *  @return {string} 选项卡的html字符串
         */
        function _renderTab(panel){
            var liHtml = '<div href="javascript:void(0)" class="ctab_tab_inner">';
            if(panel.showRefreshIcon){
                liHtml += '<a class="ctab_refresh_icon cloudjs_icon" title="刷新"></a>';
            }
            liHtml += '<a class="ctab_title" title="' + _formatHtmlToText(panel.title) + '">';
            liHtml +=  panel.title + '</a>';
            if(panel.showCloseIcon && !_disableClose){
                liHtml += '<a class="ctab_close_icon cloudjs_icon"></a>';
            }
            liHtml += '</div>';
            return liHtml;
        }

        /**
         * 将带html标签的字符串转换成text
         * @param {string} str 待转换的字符串
         * @return {string} 转换后的字符串
         */
        function _formatHtmlToText(str){
            var n = $('<p/>'), t;
            n.html(str);
            t = n.text();
            return t;
        }

        /**
         *  重新加载第index个tab panel
         *  @param {number, string} index 需重新加载的tab索引, panelId 对应面板的id
         */
        function _doLoadPanel(index, panelId){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                item = cObj['panels'][index],
                panel;
            if(!item){
                return;
            }
            panel = myTab.tabBody.children('#' + panelId);
            if(item.type === 'iframe'){
                panel.html('<iframe frameborder="0" src="' + item.url + '" class="slide_iframe" style="display: block;"></iframe>');
                panel.css({ 'overflow': 'hidden' });
            }else if(item.type === 'ajax'){
                panel.load(item.url);
            }else{
                panel.html(item.html);
            }
            _setPanelStyle.call(self);
        }

        /**
         *  选中第index个tab
         *  @param {number} index 选中的索引
         *  @return {array} 选中的tab相关数据
         */
        function _doSelectTab(index){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                panel, panelId, goalTab, result, i, len, temp;
            if(index === undefined || index === null || isNaN(index) || (index >= myTab.tabLis.length)){
                return [];
            }
            result = _doGetTab.call(self, [index]);
            if(result && result.length){
                i = 0 ; len = cObj.selectedList.length;
                for(i = 0; i< len; i++){
                    if(cObj.selectedList[i] == index){
                        break;
                    }
                }
                if(i !== len){
                    temp = cObj.selectedList.splice(i, 1);
                    cObj.selectedList.unshift(temp[0]);
                }
            }

            if(!myTab.tabLis.eq(index).hasClass('ctab_selected')){
                myTab.tabLis.removeClass('ctab_selected');
                goalTab = myTab.tabLis.eq(index);
                goalTab.addClass('ctab_selected');
                panelId = goalTab.attr('id').replace('ctab_', 'ctab_panel_');
                panel = myTab.tabBody.children('#' + panelId);
                myTab.tabPanels.hide();
                panel.show();
                if(!panel.data('tabloaded')){
                    _doLoadPanel.call(self, index, panelId);
                    panel.data('tabloaded', true);
                }
                cObj.onChangeTab.call(self, result);
            }
            return result;
        }

        /**
         *  关闭并销毁index位置的tab框
         *  @param {array} indexList 关闭的tab索引列表
         *  @return {array} 关闭的tab相关数据
         */
        function _doRemoveTab(indexList){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                li, isSelected = false, index, id, result;
            result = _doGetTab.call(self, indexList);
            indexList = indexList.reverse();
            for(var i= 0, len = indexList.length; i < len; i++){
                index = indexList[i];

                //从选中排序列表中删除自己
                var selectedIndex = cloudjs.util.indexOf(cObj.selectedList, index);
                cObj.selectedList.splice(selectedIndex, 1);
                for(var j = 0 ; j < cObj.selectedList.length; j++){
                    if(cObj.selectedList[j] > index){
                        cObj.selectedList[j]--;
                    }
                }

                li = myTab.tabLis.eq(index);
                if(li.hasClass('ctab_selected')){
                    isSelected = true;
                }
                id = li.attr('id');
                li.unbind();
                li.remove();
                id = id.replace('ctab_', 'ctab_panel_');
                myTab.tabBody.children('#' + id).remove();
                cObj.panels.splice(index, 1);
            }
            if(cObj.resizeTimeout){
                clearTimeout(cObj.resizeTimeout);
            }
            cObj.resizeTimeout = setTimeout(function(){
                _setTabStyle.call(self);
                cObj.resizeTimeout = null;
            }, 800);
            if(isSelected){
                //默认选中最近打开的tab
                _doSelectTab.call(self, cObj.selectedList[0]);
            }
            return result;
        }

        /**
         * 绑定tab上的拖动
         * @param {object} li 相应的tab对应的li元素的jquery对象
         */
        function _doDragTab(li){
            var self = this, cObj, myTab, index, preIndex, temp;
            cloudjs(li).draggable({
                axis: 'x',
                start:function(){
                    cObj = self._cloudjsCtabObj;
                    myTab = _getTabJqNode.call(self);
                    index = li.index();
                    preIndex = -1;
                    $(this).css({ zIndex: _CTAB_DRAGING_ZINDEX });
                },
                stop:function(){
                    myTab = _getTabJqNode.call(self);
                    $(this).css({ zIndex: _CTAB_ZINDEX });
                    var i = 0, myLeft, lis = myTab.tabLis, len = lis.length;
                    for(; i < len; i++){
                        myLeft = $(this).offset().left;
                        if(myLeft - lis.eq(i).offset().left > 0){
                            preIndex = i;
                        }
                    }
                    temp = cObj.panels.splice(index, 1)[0];
                    cObj.panels.splice(preIndex + 1, 0, temp);
                    if(preIndex === -1){
                        li.prependTo(myTab.tabUl).css( { left: 0 });

                    }else{
                        li.insertAfter(myTab.tabLis.eq(preIndex)).css({ left: 0 });
                    }
                }
            });
        }

        /**
         *  刷新index位置的tab框
         *  @param {array} indexList 刷新的tab索引列表
         *  @return {array} 刷新的tab相关数据
         */
        function _doRefreshTab(indexList){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self), panelId;
            for(var i= 0, len = indexList.length; i < len; i++){
                panelId = myTab.tabLis.eq(indexList[i]).attr('id').replace('ctab_', 'ctab_panel_');
                _doLoadPanel.call(self, indexList[i], panelId);
            }
            return _doGetTab.call(self, indexList);
        }

        /**
         *  获取相应index位置的tab数据，包括tabId，panelId，index，tab参数
         *  @param {array} indexList 刷新的tab索引列表
         *  @return {array} 获取到的tab相关数据
         */
        function _doGetTab(indexList){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self), tabId, panelId, tempObj, index, result = [];
            for(var i= 0, len = indexList.length; i < len; i++){
                index = indexList[i];
                tabId = myTab.tabLis.eq(index).attr('id');
                panelId = tabId.replace('ctab_', 'ctab_panel_');
                tempObj = {};
                $.extend(tempObj, cObj.panels[index]);
                result.push({ index: index, tabId: tabId, panelId: panelId, panel: tempObj });
            }
            return result;
        }

        /**
         *  更新index位置的tab数据
         *  @param {array, object} indexList 更新的tab索引列表，panel 更新的内容
         *  @return {array} 更新后的tab相关数据
         */
        function _doUpdateTab(indexList, panel){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                index,
                nowPanel,
                li,
                reloadFlag;
            if(!$.isPlainObject(panel)){
                return [];
            }
            for(var i = 0, len = indexList.length; i < len; i++){
                index = indexList[i];
                nowPanel = cObj.panels[index];
                reloadFlag = false;
                for(var k in nowPanel){
                    if(panel[k] !== undefined){
                        if(k === 'type' && nowPanel[k] !== panel[k]){
                            reloadFlag = true;
                        }else if(k === 'html' && nowPanel['type'] === 'html' && nowPanel[k] !== panel[k]){
                            reloadFlag = true;
                        }else if(k === 'url' && nowPanel['type'] !== 'html' && nowPanel[k] !== panel[k]){
                            reloadFlag = true;
                        }
                        nowPanel[k] = panel[k];
                    }
                }
                li = $(myTab.tabLis[index]);
                li.html(_renderTab.call(self, nowPanel));
                _setTabStyle.call(self);
                if(reloadFlag){
                    _doLoadPanel.call(self, index, li.attr('id').replace('ctab_', 'ctab_panel_'));
                }
            }
            return _doGetTab.call(self, indexList);
        }

        /**
         * 初始化组件整体样式
         */
        function _setStyle(){
            var self = this, cObj = self._cloudjsCtabObj, pNode, width, height;
            _width && !isNaN(_width) && $(self).width(_width);
            _height && !isNaN(_height) && $(self).height(_height);
            if(_fit){
                pNode = $(self).parent();
                width = pNode.width();
                if(pNode.is($('body'))){
                    $('body').css('overflow', 'hidden');
                    height = $(window).height() - parseFloat(pNode.css('marginTop')) - parseFloat(pNode.css('marginBottom'))
                        - parseFloat(pNode.css('paddingTop')) - parseFloat(pNode.css('paddingBottom'));
                }else{
                    height = pNode.height();
                }
                $(self).width(width).height(height);
                if(!self.fitTimeout){
                    self.fitTimeout = $(window).resize(function(){
                        setTimeout(function(){
                            _setStyle.call(self);
                            _setTabStyle.call(self);
                            _setPanelStyle.call(self);
                        },100);
                    });
                }
            }
        }

        /**
         * 设置选项卡的样式
         */
        function _setTabStyle(){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                eachWidth = _getTabWidth.call(self, myTab.tabUl.width(), myTab.tabLis.length);
            myTab.tabLis.width(eachWidth);
            /*fuck*/
        }

        /**
         * 设置面板的样式
         */
        function _setPanelStyle(){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self);
            setTimeout(function(){
                var height = $(self).height() - myTab.tabHead.outerHeight(true) - parseInt(myTab.tabBody.css('borderBottomWidth'));
                myTab.tabBody.height(height);
                myTab.tabPanels.each(function(i, ele){
                    $(this).height(height);
                    var childIfm = $(this).children('iframe.slide_iframe');
                    if(childIfm.length){
                        childIfm.css({ width: '100%' });
                        childIfm.height($(this).height());
                    }
                });
            });
        }

        /**
         * 计算当前tab的合适宽度
         * @param {number, number} width 头部ul的宽度，count 当前tab个数
         * @return {number} 返回tab的合适宽度
         */
        function _getTabWidth(width, count){
            /*fuck*/
            width = parseFloat(width);
            var eachWidth = (width - 10) / count - _CTAB_MARGIN - _CTAB_BORDER_WIDTH;
            return eachWidth < _CTAB_MAX_WIDTH ? eachWidth : _CTAB_MAX_WIDTH;

//            _CTAB_MAX_WIDTH
//            return (eachWidth - _CLOSE_BTN_WIDTH - _REFRESH_BTN_WIDTH > _ctab_MAX_WIDTH ? _ctab_MAX_WIDTH : (eachWidth - _CLOSE_BTN_WIDTH - _REFRESH_BTN_WIDTH));
        }

        /**
         * 关闭其他tab
         */
        function _closeOtherTabs(){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                length = myTab.tabLis.length,
                index = $('.ctab_selected', myTab.tabUl).index(),
                indexs = [];
            for(var i = 0; i < length; i++){
                (i !== index) && (indexs.push(i));
            }
            _doRemoveTab.call(self, indexs);
        }

        /**
         * 通过args条件获取符合条件的tab的索引列表
         * @param {object} args 筛选的条件
         * @return {array} 返回符合条件的tab的索引列表
         */
        function _matchCondition(args){
            var self = this, cObj = self._cloudjsCtabObj,
                myTab = _getTabJqNode.call(self),
                panels = cObj.panels, i,
                index = -1, length = cObj.panels.length, indexArr = [], temp=[];
            if(!args || !$.isPlainObject(args)){
                return indexArr;
            }

            for(i = 0; i<length; i++){
                indexArr[i] = i;
            }

            if(args.selected !== undefined){
                temp = [];
                index = myTab.tabUl.children('.ctab_selected').index();
                if(args.selected === false){
                    for(i = 0; i<length; i++){
                        if(indexArr[i] !== index){
                            temp.push(indexArr[i]);
                        }
                    }
                }else{
                    temp = [index];
                }
                indexArr = temp;
                length = indexArr.length;
                if(!length){
                    return indexArr;
                }
            }
            if(args.index !== undefined){
                temp = [];
                for (i = 0; i < length; i++){
                    if (args.index === indexArr[i]){
                        temp.push(indexArr[i]);
                    }
                }
                indexArr = temp;
                length = indexArr.length;
                if (!length) {
                    return indexArr;
                }
            }
            if(args.title !== undefined){
                temp = [];
                for (i = 0; i < length; i++){
                    index = indexArr[i];

                    if(matchStr.call(self, args.title, cObj.panels[index].title)){
                        temp.push(index);
                    }
                }
                indexArr = temp;
                length = indexArr.length;
                if (!length) {
                    return indexArr;
                }
            }
            if(args.url !== undefined){
                temp = [];
                for (i = 0; i < length; i++){
                    index = indexArr[i];
                    if(matchStr.call(self, args.url, cObj.panels[index].url)){
                        temp.push(index);
                    }
                }
                indexArr = temp;
                length = indexArr.length;
                if (!length){
                    return indexArr;
                }
            }
            return indexArr;

        }

        /**
         * 判断字符串是否匹配
         * @param {string|RegExp, string} a 标准字符串或正则表达式  b 需比对的字符串
         * @return {boolean} 返回匹配结果
         */
        function matchStr(a, b){
            if(a === undefined || b === undefined){
                return false;
            }
            if(a.constructor === RegExp){
                return a.test(b);
            }else{
                return a === b;
            }
        }

        return this;
    }
});