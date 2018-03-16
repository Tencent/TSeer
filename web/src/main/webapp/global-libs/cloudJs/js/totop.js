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
 * 返回顶部组件
 */

cloudjs.define({
    totop: function(options){
        var defaults = {
            bottom: 0,  //相对屏幕底部的距离
            right: 0,   //相对屏幕右侧的距离
            isScroll: true,  //是否在滚动条向下滚动时才出现
            scrollHeight: 200,  //滚动到多高时才出现
            img: '../../images/totop.png',  //默认展示的图片
            hoverImg: ''   //hover状态下展示的图片
        };
        $.extend(defaults,options);
        var _self,
            _scrollHeight = defaults.scrollHeight,
            _img = defaults.img,
            _hoverImg = defaults.hoverImg,
            _totopHtml = '<a href="#" style="dispaly:none;z-index:99999;position:fixed;bottom:' + defaults.bottom + 'px;right:' + defaults.right + 'px';
        if(_hoverImg){
            _totopHtml += '">';
        }else{
            _totopHtml += ';opacity:0.7;filter:alpha(opacity=70);">';
        }
        _totopHtml += '<img style="float:left;" src="' + _img + '" /></a>';
        
        _self = $(_totopHtml).appendTo('body');
        
        if(defaults.isScroll){
            if($(document).scrollTop() > _scrollHeight){
                _self.show();
            }else{
                _self.hide();
            }
            $(window).scroll(function(){ 
                
                if($(document).scrollTop() > _scrollHeight){
                    _self.show();
                }else{
                    _self.hide();
                }
            });
        }else{
            _self.show();
        }
        _self.hover(
            function(){
                hoverTop(this, 1);
            },
            function(){
                hoverTop(this, 0.7);
            }
        );
        
        /**
         * hover效果
         * @param {Object} obj hover对象
         * @param {Number} alpha 透明度
         */
        function hoverTop(obj, alpha){
            if(_hoverImg){
                if(alpha === 1){
                    $(obj).find('img').attr('src', _hoverImg);
                }else{
                    $(obj).find('img').attr('src', _img);
                }
            }else{
                $(obj).css({ opacity: alpha, filter: 'alpha(opacity=' + alpha*100 + ')' });
            }
        }

        return this;
    }
});