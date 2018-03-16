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
    mask: function(options){
        var defaults = {
            maskId: '',    //蒙版ID
            maskHtml: '<img src="'+loadingGif+'" /><br /><span style="color: #fff; font-size: 24px;">Loading...</span>',    //蒙版上的内容
            modal: true,    //是否加蒙版
            relative: ''    //需要加蒙版的元素
        };

        if(options && $.isPlainObject(options)){
            $.extend(defaults, options);
        }else if(options === 'destroy'){
            if(arguments[1]){
                _destroy(arguments[1]);
            }else{
                _destroy.call(this);
            }
            return;
        }

        var _self = this,
            _maskId = defaults.maskId,
            _maskHtml = defaults.maskHtml,
            _modal = defaults.modal,
            _relative = defaults.relative,
            _maskEle;

        _init();

        /**
         * 初始化蒙版组件
         */
        function _init(){
            if(_self.length !== 0 && _self !== window){
                if(_self.hasClass('mask_disabled') || _self.prop('disabled')){
                    return;
                }
                if(_maskId){
                    if($('#' + _maskId).length !== 0){
                        return;
                    }
                    _self.addClass('mask_' + _maskId);
                }
                _self.addClass('mask_disabled').prop('disabled', true);
            }

            if(_modal){
                _maskEle = $('<div class="mask_div" id="' + _maskId + '"><div class="mask_modal" style="z-index: ' + cloudjs.zIndex() + '"></div><div class="mask_con"></div></div>');
                _maskEle.find('.mask_con').append($(_maskHtml));

                $('body').append(_maskEle);
                _setStyle(_maskEle);
                if(_self.length !== 0 && _self !== window){
                    _self[0].maskEle = _maskEle;
                }
            }
        }

        /**
         * 设置蒙版的样式
         * @param {Object} ele 蒙版元素
         */
        function _setStyle(ele){
            ele = ele || _maskEle;
            var maskConEle = ele.find('.mask_con'),
                maskModalEle = ele.find('.mask_modal'),
                height = maskConEle.height(),
                width = maskConEle.width(),
                modalTop,
                modalLeft,
                modalHeight,
                modalWidth,
                relativeEle,
                sTop = $(window).scrollTop(),
                wHeight = $(window).height(),
                vHeight = 0,    //可视高度
                cTop,
                visualHeight;

            maskConEle.css({ 'margin-top': -height / 2 + 'px', 'margin-left': -width / 2 + 'px', 'z-index': cloudjs.zIndex() });

            if((_relative && _relative.length !== '') || ele.hasClass('mask_relative')){
                relativeEle = $(_relative);
                ele.addClass('mask_relative');
                modalTop = relativeEle.offset().top;
                modalLeft = relativeEle.offset().left;
                modalHeight = relativeEle.outerHeight();
                modalWidth = relativeEle.outerWidth();
                vHeight = modalHeight;
                cTop = modalTop + modalHeight / 2 + 'px';

                maskModalEle.css({ top: modalTop + 'px', left: modalLeft + 'px', height: modalHeight + 'px', width: modalWidth + 'px', position: 'absolute' });
                maskConEle.css({ top: cTop, left: modalWidth / 2 + modalLeft + 'px', position: 'absolute' });

                if(!(modalTop > sTop + wHeight || modalTop + modalHeight < sTop)){    //relativeEle 在可视窗口内
                    if(modalTop < sTop){    //上边超出可视上边界
                        vHeight = modalHeight - (sTop - modalTop);
                        vHeight = vHeight < 150 ? 150 : vHeight;
                        cTop = modalTop + vHeight / 2 + (modalHeight - vHeight) + 'px';
                    }
                    if(modalTop + modalHeight > sTop + wHeight){    //下边超出可视下边界
                        vHeight = modalHeight - (modalTop + modalHeight - sTop - wHeight);
                        vHeight = vHeight < 150 ? 150 : vHeight;
                        cTop = modalTop + vHeight / 2 + 'px';
                    }
                    maskConEle.css({ top: cTop });
                }
            }
        }

        /**
         * 销毁蒙版
         * @param {string} id 关闭的蒙版组件id，若不传id，则关闭当前，若id为‘all’，则关闭全部消息组件
         */
        function _destroy(id){
            var self = this;
            if(!id){
                if(self && self !== window){
                    self[0].maskEle && self[0].maskEle.remove();
                    self.removeClass('mask_disabled').prop('disabled', false);
                }else{
                    $('.mask_div').remove();
                    $('.mask_disabled').prop('disabled', false).removeClass('mask_disabled');
                }
            }else{
                $('#' + id).remove();
                $('.mask_' + id).removeClass('mask_disabled mask_' + id).prop('disabled', false);
            }
        }

        $(window).bind('resize', function(){
            $('.mask_div.mask_relative').each(function(){
                _setStyle($(this));
            });
        });

        return this;
    },

    require: ['./loadingGif.js']
});