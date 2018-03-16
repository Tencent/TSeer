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
    slide: function(options){
        var defaults = {
            direction: 'x', //轮播的方向
            duration: 500, //动画持续时间
            effects: 'slide', //动画效果，可选'slide','fade','cutout'
            isAuto: true, //是否自动轮播
            order: 'forward', //设置自动轮播的顺序，可选'forward','reverse'
            interval: 5000, //自动轮播间隔
            isCycle: true, //是否循环轮播
            startIndex: 0, //开始展示的索引位置
            isShowAnchor: true, //是否展示轮播锚点
            isShowArrow: true, //是否显示先前向后箭头
            anchorOffsetLeft: null, //锚点偏移左上角横向距离
            anchorOffsetTop: null, //锚点偏移左上角竖向距离
            enableMouseEnter:true, //控制鼠标移入是否暂停自动轮播
            onChange: $.noop //每次轮播后执行的回调
        };

        var _self = this,
            _direction,
            _duration,
            _effects,
            _isAuto,
            _order,
            _interval,
            _startIndex,
            _isCycle,
            _isShowArrow,
            _isShowAnchor,
            _anchorOffsetLeft,
            _anchorOffsetTop,
            _enableMouseEnter,
            _onChange;


        if(!options || $.isPlainObject(options)){
            $.extend(defaults, options);
            $(_self).each(function(i, ele){
                this._cloudjsSlideObj = {
                    count: 0,
                    activeIndex: 0,
                    slideInteval: null,
                    arrPrev: null,
                    arrNext: null,
                    anchorBox: null,
                    anchors: null,
                    content: 0,
                    items: 0,
                    isMouseEnter: false,
                    toNext: _toNext,
                    toPrev: _toPrev,
                    toIndex: _toIndex
                };
            });
        }else{
            var args = arguments;
            if(options === 'toNext' || options === 'toPrev'){
                $(_self).each(function(i, ele){
                    this._cloudjsSlideObj && this._cloudjsSlideObj[options] && this._cloudjsSlideObj[options].call(this);
                });
                return _self;
            }else if(options === 'toIndex'){
                $(_self).each(function(i, ele){
                    if(args[1] !== undefined && args[1] !== null){
                        this._cloudjsSlideObj && this._cloudjsSlideObj[options] && this._cloudjsSlideObj['toIndex'].call(this, args[1]);
                    }
                });
                return _self;
            }else{
                return;
            }
        }
        _direction = cloudjs.util.indexOf(['x', 'y'], defaults.direction) !== -1 ? defaults.direction : 'x';
        _duration = defaults.duration;
        _effects = cloudjs.util.indexOf(['slide', 'fade', 'cutout'], defaults.effects) !== -1 ? defaults.effects : 'slide';
        _isAuto = defaults.isAuto;
        _order = cloudjs.util.indexOf(['forward', 'reverse'], defaults.order) !== -1 ? defaults.order : 'forward';
        _interval = defaults.interval >= 0 ? defaults.interval : 5000;
        _startIndex = defaults.startIndex ? defaults.startIndex : 0;
        _isCycle = defaults.isCycle;
        _isShowArrow = defaults.isShowArrow || false;
        _isShowAnchor = defaults.isShowAnchor || false;
        _anchorOffsetLeft = defaults.anchorOffsetLeft;
        _anchorOffsetTop = defaults.anchorOffsetTop;
        _enableMouseEnter = defaults.enableMouseEnter;
        _onChange = $.isFunction(defaults.onChange) ? defaults.onChange : $.noop;

        _init();
        return _self;

        /**
         * 初始化组件
         */
        function _init(){
            $(_self).each(function(){
                (function(){
                    var self = this, cObj = self._cloudjsSlideObj;
                    cObj.items = $(self).children('div').not('.slide_nav');
                    cObj._count = cObj.items.length;
                    if(!cObj._count){
                        return;
                    }
                    _renderHtml.call(this);
                    _setStyle.call(this);
                    _setInterval.call(this);
                    _bindEvent.call(this);
                }).call(this);
            });
        }

        /**
         * 重新生成html元素
         */
        function _renderHtml(){
            var self = this, cObj = self._cloudjsSlideObj, anchorDom, len, i;
            $(self).addClass('slide_div').show();
            cObj.items.addClass('slide_item');
            cObj.content = $('<div/>').addClass('slide_content');
            anchorDom = $(self).children('.slide_nav');
            cObj.items.each(function(i, ele){
                $(this).appendTo(cObj.content);
            });
            $(self).append(cObj.content);
            if(_isShowArrow){
                cObj.arrPrev = $('<div/>').addClass('slide_arrow slide_arrow_prev').appendTo($(self));
                cObj.arrNext = $('<div/>').addClass('slide_arrow slide_arrow_next').appendTo($(self));
                cObj.arrPrev.add(cObj.arrNext).html('<div class="slide_arrow_icon cloudjs_icon"></div>');
            }
            if(_isShowAnchor){
                if(anchorDom.length){
                    cObj.anchorBox = anchorDom;
                    cObj.anchors = anchorDom.children();
                    len = cObj.anchors.length;
                    for(i = 0; i < cObj._count; i++){
                        if(i >= len){
                            break;
                        }
                        cObj.anchors.eq(i).addClass('slide_anchor').data('index', i);
                    }
                    cObj.anchorBox.appendTo($(self));
                }else{
                    cObj.anchorBox = $('<div/>').addClass('slide_nav slide_anchor_box');
                    var anchorHtml='';
                    for(i = 0; i < cObj._count; i++){
                        anchorHtml += '<div class="slide_anchor" data-index="' + i + '" title="' + (i + 1) + '">&nbsp;&nbsp;</div>';
                    }
                    cObj.anchorBox.html(anchorHtml);
                    cObj.anchorBox.appendTo($(self));
                    cObj.anchors = cObj.anchorBox.children();
                }

            }
        }

        /**
         * 初始化样式
         */
        function _setStyle(){
            var self = this, cObj = self._cloudjsSlideObj;
            _startIndex = _startIndex >= cObj._count ? cObj._count - 1 : _startIndex;
            cObj.items.eq(_startIndex).addClass('slide_active');
            _isShowAnchor && cObj.anchors.eq(_startIndex).addClass('slide_anchor_active');
            if(!_isCycle && _isShowArrow){
                (_startIndex === 0) && (cObj.arrPrev.addClass('slide_disable').hide());
                (_startIndex === cObj._count - 1) && (cObj.arrNext.addClass('slide_disable').hide());
            }
            cObj._activeIndex = _startIndex;
            if(_direction === 'x'){
                $(self).addClass('slide_x');
                cObj.content.width(cObj._count * 100 + '%').height('100%');
                cObj.items.width(parseFloat(100 / cObj._count) + '%').height('100%');

            }else{
                $(self).addClass('slide_y');
                cObj.content.width('100%').height(cObj._count * 100 + '%');
                cObj.items.width('100%').height(parseFloat(100 / cObj._count) + '%');
            }
            var style = _getNewStyle.call(self, _startIndex);
            cObj.content.css(style);
            _resetAbsStyle.call(self);
        }

        /**
         * 设置箭头和锚点的位置
         */
        function _resetAbsStyle(){
            var self = this, cObj = self._cloudjsSlideObj;
            var oWidth = $(self).innerWidth(), oHeight = $(self).innerHeight(), anchorSytle = {};
            if(_isShowArrow){
                var arrWidth = cObj.arrNext.width() + parseInt(cObj.arrNext.css('paddingLeft')) * 2,
                    arrHeight = cObj.arrNext.height() + parseInt(cObj.arrNext.css('paddingTop')) * 2,
                    top, left;
                if(_direction === 'x'){
                    top = (oHeight - arrHeight) / 2;
                    cObj.arrPrev.css({ top: top, left: 0 });
                    cObj.arrNext.css({ top: top, left: oWidth - arrWidth });
                }else{
                    left = (oWidth - arrWidth) / 2;
                    cObj.arrPrev.css({ top: 0, left: left });
                    cObj.arrNext.css({ top: oHeight - arrHeight, left: left });
                }
            }
            if(_isShowAnchor){
                if(cObj.anchorBox.hasClass('slide_anchor_box')){
                    if(_anchorOffsetLeft !== null){
                        anchorSytle.left = _anchorOffsetLeft;
                    }
                    if(_anchorOffsetTop !== null){
                        anchorSytle.top = _anchorOffsetTop;
                    }
                    var ancWidth = cObj.anchorBox.outerWidth(), ancHeight = cObj.anchorBox.outerHeight();
                    if(anchorSytle.left === undefined){
                        anchorSytle.left = oWidth - ancWidth;
                        if(_direction === 'x'){
                            anchorSytle.left = anchorSytle.left / 2;
                        }
                    }
                    if(anchorSytle.top === undefined){
                        anchorSytle.top = oHeight - ancHeight;
                        if(_direction === 'y'){
                            anchorSytle.top = anchorSytle.top / 2;
                        }
                    }
                    cObj.anchorBox.css(anchorSytle);
                }else{
                    if(_anchorOffsetLeft !== null){
                        cObj.anchorBox.css({ left: _anchorOffsetLeft });
                    }
                    if(_anchorOffsetTop !== null){
                        cObj.anchorBox.css({ top: _anchorOffsetTop });
                    }
                }
            }
        }

        /**
         * 设置箭头,锚点点击事件
         */
        function _bindEvent(){
            var self = this, cObj = self._cloudjsSlideObj;
            $(self).mouseenter(function(){
                cObj.arrPrev.add(cObj.arrNext).not('.slide_disable').stop().show().animate({ opacity: 0.7 }, 200);
            }).mouseleave(function(){
                cObj.arrPrev.add(cObj.arrNext).not('.slide_disable').stop().animate({ opacity: 0 }, 500, function(){
                    $(this).hide();
                });
            });
            if(_isShowArrow){
                cObj.arrPrev.add(cObj.arrNext).mouseenter(function(){
                    $(this).stop().animate({ opacity: 1 }, 300);
                }).mouseleave(function(){
                    $(this).stop().animate({ opacity: 0.7 }, 300);
                });
                cObj.arrPrev.click(function(){
                    _toPrev.call(self);
                });
                cObj.arrNext.click(function(){
                    _toNext.call(self);
                });
            }
            if(_isShowAnchor){
                cObj.anchorBox.children('.slide_anchor').click(function(e){
                    e.stopPropagation();
                    var index = $(this).data('index');
                    _toIndex.call(self, index);
                });
            }
            $(window).resize(function(){
                var style = _getNewStyle.call(self, cObj._activeIndex);
                cObj.content.css(style);
                _resetAbsStyle.call(self);
            });
            if(_enableMouseEnter){
                $(self).mouseenter(function(){
                    cObj.isMouseEnter = true;
                    _clearInterval.call(self);
                }).mouseleave(function(){
                    cObj.isMouseEnter = false;
                    _setInterval.call(self);
                });
            }
        }

        /**
         * 清除自动轮播定时器
         */
        function _clearInterval(){
            var self = this, cObj = self._cloudjsSlideObj;
            if(_isAuto){
                if(cObj._slideInteval){
                    clearInterval(cObj._slideInteval);
                    cObj._slideInteval = null;
                }
            }
        }

        /**
         * 添加自动轮播定时器
         */
        function _setInterval(){
            var self = this, cObj = self._cloudjsSlideObj;
            if(_isAuto){
                _clearInterval.call(self);
                if(!cObj.isMouseEnter){
                    cObj._slideInteval = setInterval(function(){
                        if(_order === 'forward'){
                            _toNext.call(self, 1);
                        }else{
                            _toPrev.call(self, 1);
                        }
                    }, _interval);
                }
            }
        }

        /**
         * 往上一页轮播
         * @param {boolean} autoFlag 区分是自动轮播的还是用户触发的向下轮播
         * @return {boolean} 若上一页索引超出且不允许循环，返回false；否则返回true
         */
        function _toPrev(autoFlag){
            var self = this, cObj = self._cloudjsSlideObj;
            autoFlag = autoFlag || 0;
            var preIndex;
            if(cObj._activeIndex - 1 >= 0){
                preIndex = cObj._activeIndex - 1;
            }else{
                if(_isCycle){
                    preIndex = cObj._count - 1;
                }else{
                    return false;
                }
            }
            _toIndex.call(self, preIndex, autoFlag);
            return true;
        }

        /**
         * 往下一页轮播
         * @param {boolean} autoFlag 区分是自动轮播的还是用户触发的向上轮播
         * @return {boolean} 若下一页索引超出且不允许循环，返回false；否则返回true
         */
        function _toNext(autoFlag){
            var self = this, cObj = self._cloudjsSlideObj;
            autoFlag = autoFlag || 0;
            var nextIndex;
            if(cObj._activeIndex + 1 < cObj._count){
                nextIndex = cObj._activeIndex + 1;
            }else{
                if(_isCycle){
                    nextIndex = 0;
                }else{
                    return false;
                }
            }
            _toIndex.call(self, nextIndex, autoFlag);
            return true;
        }

        /**
         * 通过指定索引位置获取新的位移量
         * @param {number} index 轮播到的索引位置
         * @return {object} 新的样式对象
         */
        function _getNewStyle(index){
            var self = this, cObj = self._cloudjsSlideObj;
            if(index === undefined || index === null || isNaN(index) || index < 0 || index >= cObj._count){
                return false;
            }
            var style = {};
            if(_direction === 'x'){
                style.marginLeft = -index * cObj.items.eq(0).width();
            }else{
                style.marginTop = -index * cObj.items.eq(0).height();
            }
            return style;
        }

        /**
         * 轮播到指定索引位置
         * @param {number} index 轮播到的索引位置 ,
         * @param {boolean} autoFlag 区分是自动轮播的还是用户触发的轮播
         */
        function _toIndex(index, autoFlag){
            index = parseInt(index);
            var self = this, cObj = self._cloudjsSlideObj;
            var style = _getNewStyle.call(self, index);
            if(!style){
                return false;
            }
            if(!_isCycle && _isShowArrow){
                cObj.arrPrev.add(cObj.arrNext).removeClass('slide_disable').show();
                if(index === 0){
                    cObj.arrPrev.addClass('slide_disable').hide();
                }else if(index === cObj._count - 1){
                    cObj.arrNext.addClass('slide_disable').hide();
                }
            }
            if(_isShowAnchor){
                cObj.anchors.removeClass('slide_anchor_active');
                cObj.anchors.eq(index).addClass('slide_anchor_active');
            }
            cObj.items.removeClass('slide_active');
            cObj.items.eq(index).addClass('slide_active');
            cObj._activeIndex = index;
            if(!autoFlag){
                _setInterval.call(self);
            }
            if(_effects === 'slide'){
                cObj.content.stop(true, true).animate(style, _duration, function(){
                    _onChange.call(self, index);
                });
            }else if(_effects === 'fade'){
                cObj.content.children().css({ opacity: 0 }).stop(true, true).animate({ opacity: 1 }, _duration, function(){
                    _onChange.call(self, index);
                });
                cObj.content.css(style);
            }else{
                cObj.content.css(style);
                _onChange.call(self, index);
            }
        }
    }
});