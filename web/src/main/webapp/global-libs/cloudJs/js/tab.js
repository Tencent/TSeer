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
    tab: function(options){
        var defaults = {
            axis: 'x', //tab的方向，横向或竖向
            enableHover: false, //是否允许鼠标hover的时候选中tab
            hoverDelay: 200, //鼠标移上去之后的延迟时间
            selectIndex: 0, //默认选中的位置
            isAuto: false,  //是否按时自动切换
            inteval: 5000,  //自动切换时间间隔
            onChange: $.noop //切换时候的回调
        };

        var _self = this,
            _axis,
            _enableHover,
            _hoverDelay,
            _selectIndex,
            _isAuto,
            _inteval,
            _onChange;


        if(!options || $.isPlainObject(options)){
            $.extend(defaults, options);
        }else{
            return;
        }
        _axis = cloudjs.util.indexOf(['x', 'y'], defaults.axis) !== -1 ? defaults.axis : 'x';
        _enableHover = defaults.enableHover || false;
        _hoverDelay = (defaults.hoverDelay || defaults.hoverDelay < 0) || 200;
        _selectIndex = defaults.selectIndex || 0;
        _isAuto = defaults.isAuto || false;
        _inteval = (defaults.inteval || defaults.inteval < 0) || 2000;
        _onChange = defaults.onChange || $.noop;

        _init();
        return self;

        /**
         * 初始化消息组件
         */
        function _init(){
            _self.each(function(){
                var ul = $(this).children('ul.tab_head'),
                    lis = ul.children('li.tab_tab'),
                    panels = $(this).children('div.tab_panel');

                this._cloudjsTabObj = {
                    ul: ul,
                    lis: lis,
                    panels: panels,
                    selectIndex: _selectIndex,
                    length: lis.length,
                    hoverTimeout: null,
                    switchInteval: null
                };
                _setStyle.call(this);
                _bindEvent.call(this);
            });
        }


        /**
         * 初始化样式
         */
        function _setStyle(){
            var self = this, cObj = self._cloudjsTabObj;
            if(_axis === 'x'){
                $(self).addClass('tab_div tab_div_x');
            }else{
                $(self).addClass('tab_div tab_div_y');
            }
            cObj.ul.addClass('tab_head');
            cObj.lis.addClass('tab_tab');
            (cObj.length <= cObj.selectIndex) && (cObj.selectIndex = 0);
            cObj.lis.eq(cObj.selectIndex).addClass('tab_selected');
            cObj.panels.addClass('tab_panel').hide().eq(cObj.selectIndex).show();
            _setVerticalH.call(self);
        }

        /**
         * 垂直轴向的情况下自适应高度
         */

        function _setVerticalH(){
            var self = this, cObj = self._cloudjsTabObj, height;
            if(_axis === 'y'){
                height = $(self).height();
                cObj.ul.height(height);
                cObj.panels.each(function(i, ele){
                    $(this).height(height - parseInt($(this).css('borderTop')) - parseInt($(this).css('borderBottom')));
                });
            }
        }

        /**
         * 绑定事件
         */
        function _bindEvent(){
            var self = this, cObj = self._cloudjsTabObj, target, index;
            _setSwitchInteval.call(self);
            if(_enableHover){
                cObj.lis.bind('mouseenter', function(e){
                    clearTimeout(cObj.hoverTimeout);
                    target = $(e.currentTarget);
                    index = cObj.lis.index(target);
                    cObj.hoverTimeout = setTimeout(function(){
                        _swithTo.call(self, index);
                    }, _hoverDelay);
                }).bind('mouseleave', function(e){
                    clearTimeout(cObj.hoverTimeout);
                });
            }
            cObj.lis.bind('click', function(e){
                clearTimeout(cObj.hoverTimeout);
                target = $(e.currentTarget);
                index = cObj.lis.index(target);
                _swithTo.call(self, index);
            });
            $(self).bind('mouseenter', function(){
                _clearSwithInteval.call(self);
            }).bind('mouseleave', function(){
                _setSwitchInteval.call(self);
            });
        }

        /**
         * 设置自动切换定时器
         */
        function _setSwitchInteval(){
            var self = this, cObj = self._cloudjsTabObj, index;
            if(_isAuto){
                _clearSwithInteval.call(self);
                cObj.switchInteval = setInterval(function(){
                    index = (cObj.selectIndex + 1) % cObj.length;
                    _swithTo.call(self, index);
                }, _inteval);
            }
        }

        /**
         * 清除自动切换定时器
         */
        function _clearSwithInteval(){
            var self = this, cObj = self._cloudjsTabObj;
            clearInterval(cObj.switchInteval);
        }

        /**
         * 设置自动切换定时器
         * @param {number} index 切换到的选项卡索引
         */
        function _swithTo(index){
            var self = this, cObj = self._cloudjsTabObj, target;
            target = cObj.lis.eq(index);
            cObj.lis.removeClass('tab_selected');
            target.addClass('tab_selected');
            cObj.panels.hide().eq(index).show();
            cObj.selectIndex = index;
            _setVerticalH.call(self);
            _onChange.call(self, index);
        }
    }
});