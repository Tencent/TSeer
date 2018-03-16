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
 * 格式校验组件
 */
cloudjs.define({
    validate: function(options){
        var args = arguments;
        var defaults = {
            warn: true, // 是否需要页面显示错误提示
            warnType:1, //tip的样式 1错误、2警告、3提示、4成功
            rule: _defaultRule(), // 默认的校验规则和错误提示信息
            success: $.noop, // 校验通过调用的方法
            fail: $.noop, // 校验不通过的调用方法
            checkType: 'empty', // 元素默认的校验类型
            errmsgType: 1, // 页面提示错误信息的方式，1对应弹tip，2对应直接跟在元素后面
            position: '', // 默认的tips提示位置，在元素下方
            delay: 0, // 提示信息出现多久后消失，默认无动作触发时不消失
            context: 'body',
            zIndex: cloudjs.zIndex()
        };

        var _self = this===window?undefined:this,
            DATA_FLAG = 'data-check',
            DATA_POSITION = 'data-position',
            DATA_FORMAT = 'data-format',
            DATA_WRAPER = 'data-wraper',
            ARROW_WIDTH = 5,
            ARROW_DOWN = 'down',
            ARROW_LEFT = 'left',
            ARROW_RIGHT = 'right',
            ARROW_UP = 'up',
            SPACING = 2,
            DATE_INVALID = 'Invalid Date',
            WARN_TYPE = ['','error','warn','info','succ'];

        if(!options || $.isPlainObject(options)){
            $.extend(true, defaults, options);
            cloudjs(_self).validate('removeErrmsg');
        }else if(options === 'removeErrmsg'){ // 调用的是方法
            if(_self){
                $.each(_self||[], function(i, obj){
                    $.each(_formFilter($(obj)), function(j, el){
                        _removeEleErrmsg(el[0]||el);
                    });
                });
            }else{
                $.extend(true, defaults, args[1]||{});
                _removeErrmsgs(defaults.context);
            }
            return;
        }else if(options === 'autoFit'){
            autoFit();
        }

        _init(defaults.context, defaults.position, defaults.warnType);

        cloudjs.callback(defaults); // 初始化完毕

        /**
         * 初始化方法
         */
        function _init(context, posiiton, warnType){
            var i, len, results = [], result, fail = defaults.fail, success = defaults.success;

            clearTimeout(cloudjs.validate._timeout1);
            //_removeErrmsgs();

            $.each(_self, function(i, ele){
                ele = _formFilter($(ele));
                $.each(ele, function(j, el){
                    if(result = _validate($(el))){
                        results.push(result);
                    }
                });
            });
            if(len = results.length){
                for(i = 0; i < len; i++){
                    result = results[i];
                    if(defaults.warn){
                        if(defaults.errmsgType === 2){
                            _showTexts(result.ele, result.errmsg);
                        }else{
                            _showTips(result.ele, result.errmsg, context, posiiton, warnType);
                        }
                        _bindFocus(result.ele);
                    }
                }
                if(defaults.warn && defaults.delay){
                    cloudjs.validate._timeout1 = setTimeout(function(){
                        _removeErrmsgs(context);
                    }, defaults.delay);
                }
                fail(results);
            }else{
                success();
            }
        }

        /**
         * 处理表单的情况
         * @param {Object} ele 校验的元素
         * @return {Array} form下面的input=text的表单元素，如果不是form，则将该元素包在数组内返回
         */
        function _formFilter(ele){
            if(ele[0].tagName.toLowerCase() === 'form'){
                return ele.find('input[type="file"],input[type="text"],textarea');
            }else{
                return [ele];
            }
        }

        /**
         * 绑定鼠标focus到元素时删除提示
         * @param {Object} ele 校验的元素
         */
        function _bindFocus(ele){
            if(ele.data('focus-binded')){
                return;
            }
            ele.data('focus-binded', 1);
            ele.focus(function(){
                _removeEleErrmsg(this);
            });
        }

        /**
         * 删除某个元素的错误提示
         * @param {HTMLObject} obj 要删除提示的dom元素
         */
        function _removeEleErrmsg(obj){
            obj.errmsgObj && obj.errmsgObj.remove();
            delete obj.errmsgObj;
        }

        /**
         * 校验具体某个元素
         * @param {Object} ele 校验的元素
         * @return {Object} 假如有错误，就会返回一个错误对象，没错误返回undefined
         */
        function _validate(ele){
            var check = ele.attr(DATA_FLAG), i, len, ch, rule, result;
            if(!check){
                check = defaults.checkType;
            }
            check = check.split(';');
            for(i = 0, len = check.length; i < len; i++){
                if(ch = check[i]){
                    rule = defaults.rule[ch];
                    if(rule && !rule.check(ele)){
                        result = {
                            ele: ele,
                            errmsg: rule.errmsg(ele)
                        };
                        break;
                    }
                }
            }

            return result;
        }

        /**
         * 正则匹配
         * @param {String} s 被匹配的内容
         * @param {Reg} reg 正则表达式
         * @return {Boolean} 返回是否匹配
         */
        function _match(s, reg){
            s = $.trim(s);
            return !s || reg.test(s);
        }

        /**
         * 删除全局的错误提示信息
         */
        function _removeErrmsgs(context){
            $(context).find('.validate_div,.validate_span').remove();
        }

        /**
         * 错误提示信息的第二种展示方式
         * @param {Object} ele 要提示的元素
         * @param {String} content 提示的内容
         */
        function _showTexts(ele, content){
            var wraperId, wraper = $(ele.attr(DATA_WRAPER));
            var validate_wraper = 'validate_wraper';
            if(!wraper.length){
                wraperId = 'id_' + cloudjs.uniq();
                wraper = $('<span id="' + wraperId + '" class="'+validate_wraper+'"></span>').insertAfter(ele);
                ele.attr(DATA_WRAPER, '#' + wraperId);
            }
            wraper.find('.validate_span').remove();
            ele[0].errmsgObj = $('<span class="validate_span"><span class="validate_icon"></span>' + content + '</span>').appendTo(wraper);
        }

        /**
         * 显示tips提示信息
         * @param {Object} ele 要提示的元素
         * @param {String} content 提示的内容
         * @param {String} context
         * @param {String} position
         */
        function _showTips(ele, content, context, position, warnType){
            var tips, offset;

            position = ele.attr(DATA_POSITION)||position;

            tips = _createTip(content, context, warnType);

            offset = _getOffset(ele, tips, _getPosiiton(ele, tips, position), context);

            if(!offset){
                return;
            }

            tips.show().css({
                left: offset.left + 'px',
                top: offset.top + 'px'
            }).find('.validate_arrow').addClass('validate_arrow_' + offset.position);

            ele[0].errmsgObj = tips;

            if(tips[0] && !tips[0].windowResiezeBind){
                tips[0].windowResiezeBind = 'y';
                cloudjs.validate.onResizeFuns = cloudjs.validate.onResizeFuns||[];
                cloudjs.validate.onResizeFuns.push(function(){
                    setTimeout(function(){
                        if(tips.is(':visible') && ele[0]){
                            var offset = _getOffset(ele, tips, _getPosiiton(ele, tips, position), context);
                            if(!offset){
                                return;
                            }
                            tips.css({
                                left: offset.left + 'px',
                                top: offset.top + 'px'
                            }).find('.validate_arrow').removeClass().addClass('validate_arrow validate_arrow_' + offset.position);
                        }
                    }, 10);
                });
                if(!cloudjs.validate.bindWindowResize){
                    cloudjs.validate.bindWindowResize = true;
                    $(window).resize(autoFit);
                }
            }
        }

        function autoFit(){
            var funs = [].concat(cloudjs.validate.onResizeFuns||[]);
            while(funs.length){
                funs.pop()();
            }
        }

        /**
         * 根据传入的tips指向计算出tips该出现在哪个位置
         * @param {Object} ele 需要显示tips的元素
         * @param {Object} tips tips的jquery对象
         * @param {String} position tips出现的方向
         * @return {Object} 返回一个包含了left和top坐标的对象
         */
        function _getOffset(ele, tips, position, context){

            var relative_parent = ele.parent();
            var css_position = relative_parent.css('position');
            var ok_position = {absolute:1, relative:1};
            while(relative_parent[0] && !ok_position[css_position] && relative_parent[0] !== $('body')[0]){
                relative_parent = relative_parent.parent();
                css_position = relative_parent.css('position');
            }

            if(!relative_parent[0]){
                cloudjs(ele).validate('removeErrmsg');
                return;
            }

            var parent_x = relative_parent.offset().left;
            var parent_y = relative_parent.offset().top;

            if(context === 'body'){
                parent_x = parent_y = 0;
            }

            var scr_x = relative_parent.scrollLeft();
            var src_y = relative_parent.scrollTop();

            if(relative_parent[0] == $('body')[0]){
                scr_x = src_y = 0;
            }

            var offset = {
                position: position
            };
            switch(position){
                case ARROW_RIGHT:
                    offset.left = ele.offset().left + ele.outerWidth() + ARROW_WIDTH + SPACING;
                    offset.top = ele.offset().top + ele.outerHeight() / 2 - tips.outerHeight() / 2;
                    break;
                case ARROW_LEFT:
                    offset.left = ele.offset().left - tips.outerWidth() - ARROW_WIDTH - SPACING;
                    offset.top = ele.offset().top + ele.outerHeight() / 2 - tips.outerHeight() / 2;
                    break;
                case ARROW_UP:
                    offset.left = ele.offset().left + ele.outerWidth() / 2 - tips.outerWidth() / 2;
                    offset.top = ele.offset().top - tips.outerHeight() - ARROW_WIDTH - SPACING;
                    break;
                case ARROW_DOWN:
                    offset.left = ele.offset().left + ele.outerWidth() / 2 - tips.outerWidth() / 2;
                    offset.top = ele.offset().top + ele.outerHeight() + ARROW_WIDTH + SPACING;
                    break;
                default:
                //一定会是上面四种情况，不会有这种情况
            }

            offset.left = offset.left - parent_x + scr_x;
            offset.top = offset.top - parent_y + src_y;

            return offset;
        }

        /**
         * 创建tips
         * @param {String} content tips显示的内容
         * @return {Object} 返回tips的jquery对象
         */
        function _createTip(content, context, warnType){
            var tips = '';
            tips += '<div class="validate_div validate_div_' + (WARN_TYPE[warnType]||WARN_TYPE[0]) + '" style="z-index:' + defaults.zIndex + '">';
            tips += '<span class="validate_content"><i class="validate_icon"></i>';
            tips += (content || ' ');
            tips += '</span>';
            tips += '<span class="validate_arrow"><i></i></span>';
            tips += '</div>';

            return $(tips).appendTo(context).hide();
        }

        /**
         * 自动计算tips应该出现的位置，防止tips超出页面边界
         * @param {Object} ele 需要显示tips的元素
         * @param {Object} tips tips的jquery对象
         * @param {String} position tips指定的方向，如果指定了方向，则不会自动计算
         * @return {String} 返回tips的指向
         */
        function _getPosiiton(ele, tips, position){
            var scrollTop, screenWidth, screenHeight, eleLeft, eleTop, eleWidth, eleHeight, tipsWidth, tipsHeight, positions;

            if(position !== ARROW_DOWN && position !== ARROW_UP && position !== ARROW_RIGHT && position !== ARROW_LEFT){
                position = undefined;
                scrollTop = $(window).scrollTop();
                screenWidth = $(window).width();
                screenHeight = $(window).height();
                eleLeft = ele.offset().left;
                eleTop = ele.offset().top - scrollTop;
                eleWidth = ele.outerWidth();
                eleHeight = ele.outerHeight();
                tipsWidth = tips.outerWidth();
                tipsHeight = tips.outerHeight();
                positions = [
                    {
                        name: ARROW_DOWN,
                        isFix: function(){return (eleTop + eleHeight + tipsHeight + ARROW_WIDTH + SPACING < screenHeight) && _lr();}
                    },
                    {
                        name: ARROW_UP,
                        isFix: function(){return (eleTop - tipsHeight - ARROW_WIDTH - SPACING > 0) && _lr();}
                    },
                    {
                        name: ARROW_RIGHT,
                        isFix: function(){return (eleLeft + eleWidth + ARROW_WIDTH + SPACING < screenWidth && _ud());}
                    },
                    {
                        name: ARROW_LEFT,
                        isFix: function(){return (eleLeft - tipsWidth - ARROW_WIDTH - SPACING > 0) && _ud();}
                    }
                ];

                $.each(positions, function(i, p){
                    if(!position && p.isFix()){
                        position = p.name;
                    }
                });
            }

            function _lr(){
                return (eleLeft + eleWidth / 2 - tipsWidth / 2 > 0) && (eleLeft + eleWidth / 2 + tipsWidth / 2 < screenWidth);
            }

            function _ud(){
                return (eleTop + eleHeight / 2 - tipsHeight / 2 > 0) && (eleTop + eleHeight / 2 + tipsHeight / 2 < screenHeight);
            }

            return position || ARROW_UP;
        }

        /**
         * 设置默认的检验规则
         * @return {Object} 返回组件默认的校验规则
         */
        function _defaultRule(){
            var rule = {
                empty: {
                    check: function(ele){
                        return !!$.trim(ele.val());
                    },
                    errmsg: function(ele){
                        return '不能为空';
                    }
                },
                date: {
                    check: function(ele){
                        var date = $.trim(ele.val()), result, reg, format = ele.attr(DATA_FORMAT);

                        this.format = parseInt(format||this.format);
                        this._f = 'yyyy-mm-dd';

                        if(this.format === 1){
                            reg = /^(\d{4})-(\d{2})-(\d{2})$/;
                            this._f = 'yyyy-mm-dd';
                        }else if(this.format === 2){
                            reg = /^(\d{4})\/(\d{2})\/(\d{2})$/;
                            this._f = 'yyyy/mm/dd';
                        }else if(this.format === 3){
                            reg = /^(\d{4})(\d{2})(\d{2})$/;
                            this._f = 'yyyymmdd';
                        }

                        result = date.match(new RegExp(reg));

                        date = !date || DATE_INVALID;

                        if(result){
                            date = new Date(result[1]+'-'+result[2]+'-'+result[3]).toString();
                        }

                        if(date === DATE_INVALID){
                            return false;
                        }else{
                            return true;
                        }
                    },
                    errmsg: function(ele){
                        return '日期错误，格式为' + this._f;
                    },
                    format: 1 // 1对应 yyyy-mm-dd 2对应 yyyy/mm/dd  3对应yyyymmdd
                },
                time: {
                    check: function(ele){
                        var time = $.trim(ele.val()), result, reg, format = ele.attr(DATA_FORMAT);

                        this.format = parseInt(format||this.format);
                        this._f = 'yyyy-mm-dd hh:mm:ss';

                        if(this.format === 1){
                            reg = /^(\d{4})-(\d{2})-(\d{2})\s(\d{2}:\d{2}:\d{2})$/;
                            this._f = 'yyyy-mm-dd hh:mm:ss';
                        }else if(this.format === 2){
                            reg = /^(\d{4})\/(\d{2})\/(\d{2})\s(\d{2}:\d{2}:\d{2})$/;
                            this._f = 'yyyy/mm/dd hh:mm:ss';
                        }else if(this.format === 3){
                            reg = /^(\d{4})(\d{2})(\d{2})\s(\d{2}:\d{2}:\d{2})$/;
                            this._f = 'yyyymmdd hh:mm:ss';
                        }

                        result = time.match(new RegExp(reg));

                        time = !time || DATE_INVALID;

                        if(result){
                            time = new Date(result[1] + '-' + result[2] + '-' + result[3] + ' ' + result[4]).toString();
                        }

                        if(time === DATE_INVALID){
                            return false;
                        }else{
                            return true;
                        }
                    },
                    errmsg: function(ele){
                        return '时间错误，格式为' + this._f;
                    },
                    format: 1 // 1对应 yyyy-mm-dd hh:mm:ss 2对应 yyyy/mm/dd hh:mm:ss 3对应yyyymmdd hh:mm:ss
                },
                url: {
                    check: function(ele){
                        var value = $.trim(ele.val()).split('');
                        value.pop();
                        value = value.join('');
                        if(cloudjs.util.isStartWith('http://', value) || cloudjs.util.isStartWith('https://', value) || cloudjs.util.isStartWith('ftp://', value)){
                            return true;
                        }else{
                            return false;
                        }
                    },
                    errmsg: function(ele){
                        return '链接格式错误';
                    }
                },
                num: {
                    check: function(ele){
                        return !isNaN($.trim(ele.val()));
                    },
                    errmsg: function(ele){
                        return '必须为数字';
                    }
                },
                inum: {
                    check: function(ele){
                        var reg = /^-?[0-9]\d*$/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '必须为整数';
                    }
                },
                uinum: {// unsigned int number
                    check: function(ele){
                        var reg = /^[1-9]\d*$/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '必须为正整数';
                    }
                },
                ginum: {// greate int number
                    check: function(ele){
                        var reg = /^0$|(^[1-9]\d*$)/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '必须为非负整数';
                    }
                },
                ip: {
                    check: function(ele){
                        //var reg = /^(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.)(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.){2}([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))$/;
                        var reg = /^(([01]?[\d]{1,2})|(2[0-4][\d])|(25[0-5]))(\.(([01]?[\d]{1,2})|(2[0-4][\d])|(25[0-5]))){3}$/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return 'ip格式错误';
                    }
                },
                ips: {
                    check: function(ele){
                        var value = ele.val(), i, len;
                        //var reg = /^(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.)(([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))\.){2}([1-9]|([1-9]\d)|(1\d\d)|(2([0-4]\d|5[0-5])))$/;
                        var reg = /^(([01]?[\d]{1,2})|(2[0-4][\d])|(25[0-5]))(\.(([01]?[\d]{1,2})|(2[0-4][\d])|(25[0-5]))){3}$/;
                        value = value.replace(/;/g, ',');
                        value = value.split(',');
                        for(i = 0, len = value.length; i < len; i++){
                            if($.trim(value[i]) && !_match(value[i], reg)){
                                return false;
                            }
                        }
                        return true;
                    },
                    errmsg: function(ele){
                        return '多ip格式错误';
                    }
                },
                email: {
                    check: function(ele){
                        var reg = /^([a-zA-Z0-9_-])+@([a-zA-Z0-9_-])+(.[a-zA-Z0-9_-])+/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '邮箱格式错误';
                    }
                },
                phone: {
                    check: function(ele){
                        var reg = /^1\d{10}$/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '手机号码格式错误';
                    }
                },
                naming: {
                    check: function(ele){
                        var reg = /^[a-zA-Z\$][\w\d\$]*$/;
                        return _match(ele.val(), reg);
                    },
                    errmsg: function(ele){
                        return '命名只能由字符、数字、下划线或$组成';
                    }
                }
            };

            for( var i = 0; i < 200; i++){
                (function(i){
                    rule['maxlen' + i] = {
                        check: function(ele){
                            if($.trim(ele.val()).length > i){
                                return false;
                            }
                            return true;
                        },
                        errmsg: function(ele){
                            return '最大长度不能超过' + i;
                        }
                    };
                    rule['minlen' + i] = {
                        check: function(ele){
                            if($.trim(ele.val()).length < i){
                                return false;
                            }
                            return true;
                        },
                        errmsg: function(ele){
                            return '最小长度不能小于' + i;
                        }
                    };
                })(i);
            }

            rule.maxlen = rule.maxlen20;
            rule.minlen = rule.minlen6;

            rule.never = {
                check: function(){
                    return true;
                }
            };
            return rule;
        }
    }
});