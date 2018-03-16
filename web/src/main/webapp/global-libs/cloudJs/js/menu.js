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
 * 菜单组件
 */

cloudjs.define({
    menu: function(options){
        var defaults = $.extend({},options),
            _self = this,
            _menuLi,
            _data,
            _selectedName,
            _selectedValue,
            _isHideOther,
            _html = '',
            _type = defaults.type,
            _levelClass = (_type || 'horizontal'),
            _css = $.isPlainObject(defaults.css) ? defaults.css : {},
            _objMap = cloudjs.menu._objMap = cloudjs.menu._objMap || {},
            _onClick = $.isFunction(defaults.onClick) ? defaults.onClick : $.noop,
            _onCreate = $.isFunction(defaults.onCreate) ? defaults.onCreate : $.noop,
            DELAY_TIME = 300,
            HOVER_CLASS = 'hover',
            OBJ_ID = 'uuid';

        //第一种菜单，必须是ul元素，数据格式必须json数组
        if(_type === 'tree'){
            _isHideOther = typeof defaults.isHideOther === 'boolean' ? defaults.isHideOther : true;
            _selectedName = defaults.selectedName || 'href';
            _selectedValue = defaults.selectedValue;
            
            _data = defaults.data;
            if(_self.is('ul')){
                _self.addClass('menu_tree_ul').find('li').each(function(index){
                    _html = '';
                    _createHtml(_data[index]);
                    $('<ul>' + _html + '</ul>').appendTo(this);
                });
            }else{
                _html = '';
                _createHtml(_data);
                _self.append('<ul class="menu_tree_ul">' + _html + '</ul>');
            }
            _self.find('a').bind('click', function(){
                var li = $(this).parent();
                if(li.hasClass(HOVER_CLASS)){
                    li.removeClass(HOVER_CLASS).children('ul').hide();
                }else{
                    li.addClass(HOVER_CLASS).children('ul').show();
                    if(_isHideOther && $(this).next().is('ul')){
                        li.siblings().removeClass(HOVER_CLASS).children('ul').hide();
                    }
                }
                _onClick(this);
            });
        }else if(_self.is('ul')){
            _data = defaults.data;
            _menuLi = _self.addClass('menu_data_ul menu_data_' + _levelClass + '_ul').find('>li').addClass('li_level0');
            if(defaults.isPreLoad){
                _menuLi.each(function(index){
                    var self = $(this);
                    _init(self);
                });
            }
            _menuLi.bind('mouseenter', function(){
                var self = $(this), uuid = getUuid(self);
                _init(self);
                clearTimeout(_objMap['hide_' + uuid]);
               _showObj(uuid, self.find('>ul'), self, self.find('>a'));
            }).bind('mouseleave', function(){
                var self = $(this), uuid = getUuid(self);
                clearTimeout(_objMap['show_' + uuid]);
                _hideObj(uuid, self.find('>ul'), self, self.find('>a'));
            });
        }else if(_self.children().eq(0).is('ul')){ //第二种菜单，第一个元素必须是ul
            _self.each(function(){
                $(this).addClass('menu_fixed_div').children().eq(0).addClass('menu_fixed_title').nextAll().addClass('menu_fixed_hide');
            });
            _bindFixedMouse(_createFixedObj(_self));
        }else{ //第三种菜单，必须含两个子元素，前一个显示标题，后一个隐藏内容
            _self.each(function(){
                var nextObj = $(this).addClass('menu_simple_div').children().eq(0).addClass('menu_simple_title').next().addClass('menu_simple_hide');
                if(defaults.position === 'right'){
                    nextObj.css('right', 0);
                }else if(defaults.position === 'center'){
                    nextObj.css('left', ($(this).outerWidth() - nextObj.outerWidth())/2 + 'px');
                }
            });
            _self.bind('mouseenter', function(){
                var obj = $(this).children().eq(0), uuid = getUuid($(this));
                clearTimeout(_objMap['hide_' + uuid]);
                _showObj(uuid, obj.next(), obj);
            }).bind('mouseleave', function(){
                var obj = $(this).children().eq(0), uuid = getUuid($(this));
                clearTimeout(_objMap['show_' + uuid]);
                _hideObj(uuid, obj.next(), obj);
            });
        }
        _onCreate();

        /**
         * 初始化
         * @param {Jquery Object} self 菜单的li对象
         */
        function _init(self){
            var index = self.index(), data = _data[index], top1 = 0, left1 = 0;
            if(defaults.type === 'vertical'){ //纵向菜单
                left1 = self.outerWidth();
                top1--;
            }else{
                left1--; 
                top1 = self.outerHeight();
            }
            top1 = (typeof _css.top1 === 'number' ? _css.top1 + top1 : top1) + 'px';
            left1 = (typeof _css.left1 === 'number' ? _css.left1 + left1 : left1) + 'px';

            if(!$.isArray(data)) return;
            _html = '';
            _createHtml(data);
            $('<ul class="menu_div' + index + '" style="display:none;">' + _html + '</ul>').css({ top: top1, left: left1 })
                .appendTo(self).find('li').bind('mouseenter', function(){
                    var self = $(this),
                        uuid = getUuid(self),
                        top2 = (typeof _css.top2 === 'number' ? _css.top2 : -1) + 'px',
                        left2 = (typeof _css.left2 === 'number' ? _css.left2 + self.outerWidth() : self.outerWidth()) + 'px';
                    clearTimeout(_objMap['hide_' + uuid]);
                    _showObj(uuid, self.find('>ul').css({ top: top2, left: left2 }), self, self.find('>a'));
                }).bind('mouseleave', function(){
                    var self = $(this), uuid = getUuid(self);
                    clearTimeout(_objMap['show_' + uuid]);
                    _hideObj(uuid, self.find('>ul'), self, self.find('>a'));
                });
            _data[index] = null;
        }

        /**
         * 获取元素唯一id值
         * @param {Jquery Object} self jquery对象
         * @return {String} 当前的jquery对象的OBJ_ID属性值
         */
        function getUuid(self){
            var uuid = self.attr(OBJ_ID);
            if(!uuid){
                self.attr(OBJ_ID, cloudjs.uniq());
                uuid = self.attr(OBJ_ID);
            }
            return uuid;
        }

        /**
         * 绑定鼠标事件
         * @param {Jquery Object} self jquery对象
         */
        function _bindFixedMouse(self){
            self.div.css({minHeight: self.height + 'px', width: self.width + 'px'}).find('>div').hide();
            self.li.bind('mouseenter', function(){
                var top = self.top;
                self.mIndex = $(this).index();
                $(this).addClass(HOVER_CLASS);
                if(defaults.relativeTop === 'current') top += $(this).outerHeight() * self.mIndex;
                self.div.eq(self.mIndex).css({top: top + 'px', left: (self.left - 1) + 'px'}).show();
            }).bind('mouseleave', function(){
                $(this).removeClass(HOVER_CLASS);
                self.div.hide();
            });
            self.div.bind('mouseenter', function(){
                $(this).show();
                self.li.eq(self.mIndex).addClass(HOVER_CLASS);
            }).bind('mouseleave', function(){
                $(this).hide();
                self.li.eq(self.mIndex).removeClass(HOVER_CLASS);
            });
        }

        /**
         * 绑定鼠标事件
         * @param {Jquery Object} self jquery对象
         * @return {Object} 对象
         */
        function _createFixedObj(self){
            var ul = self.find('>ul'), height = defaults.relativeTop === 'current' ? 50 : ul.outerHeight() - 2;
            return {
                ul: ul,
                li: ul.find('>li'),
                div: self.find('>div'),
                width: _css.width || 500,
                height: _css.height || height,
                top: _css.top || 0,
                left: _css.left || ul.outerWidth(),
                mIndex: null
            };
        }

        /**
         * 把对象数组生成对应的html元素
         * @param {Array} data 对象数组
         */
        function _createHtml(data){
            for(var i = 0; i < data.length; i++){
                var obj = data[i],
                    selected = obj.isOpen ? HOVER_CLASS : '',
                    className = obj.className ? ' class="' + obj.className + ' ' + selected + '"' : ' class="' + selected + '"',
                    href = obj.href ? ' href="' + obj.href + '"' : '',
                    target = obj.target ? ' target="' + obj.target + '"' : '',
                    children = obj.children;
                if(_selectedValue && _selectedValue === obj[_selectedName]) target += ' class="selected"';
                _html += '<li' + className + '><a' + href + target + '>' + obj.text + '</a>';
                if(!$.isArray(children) || children.length === 0){
                    _html += '</li>';
                }else{
                    if(_type !== 'tree') _html += '<span class="cloudjs_icon"></span>';
                    if(obj.isOpen){
                        _html += '<ul>';
                    }else{
                        _html += '<ul style="display:none;">';
                    }
                    _createHtml(children);
                    _html += '</ul></li>';
                }
            }
        }

        /**
         * 延迟显示对象
         * @param {String} uuid 唯一id
         * @param {Jquery Object} self 要显示的jquery对象
         * @param {Jquery Object}  要添加HOVER_CLASS的jquery对象，可以是多个
         */
        function _showObj(uuid, self){
            var args = arguments, len = args.length;
            _objMap['show_' + uuid] = setTimeout(function(){
                self.show();
                while(len > 2){
                    len--;
                    args[len].addClass(HOVER_CLASS);
                }
            }, DELAY_TIME);
        }

        /**
         * 延迟隐藏对象
         * @param {String} uuid 唯一id
         * @param {Jquery Object} self要隐藏的jquery对象
         * @param {Jquery Object}  要移除HOVER_CLASS的jquery对象，可以是多个
         */
        function _hideObj(uuid, self){
            var args = arguments, len = args.length;
            _objMap['hide_' + uuid] = setTimeout(function(){
                self.hide();
                while(len > 2){
                    len--;
                    args[len].removeClass(HOVER_CLASS);
                }
            }, DELAY_TIME);
        }
        
        cloudjs.callback(_self);
    }
});