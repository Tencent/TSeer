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
    monthpicker: function(options){
        var defaults = {
            width: 220,                     // 单个月时月控件的宽度
            zIndex: cloudjs.zIndex(),       // 月控件的层级
            beginYear: -40,                 // 开始年份
            endYear: +10,                   // 结束年份
            placeholder: '',                // 输入框中显示的placeholder信息
            timeFormat: 'yyyy-mm',          // 输出到输入框的日期格式
            showIcon: true,                 // 是否显示图标
            multiSelect: false,             // 范围选择
            startMonth: '',                 // 默认开始月份
            endMonth: '',                   // 默认结束月份
            startMonthName: 'startMonth',   // 设置开始月份隐藏域的name值
            endMonthName: 'endMonth',       // 设置结束月份隐藏域的name值
            minDate: null,                  // 可选的最小月份，用于范围选择
            maxDate: null,                  // 可选的最大月份，用于范围选择
            onSelect: $.noop,               // 选择日期后触发的事件
            onClose: $.noop,                // 关闭月控件后触发的事件
            onChange: $.noop                // 月控件的值改变后触发的事件
        };
       
        if ($.isPlainObject(options) || !options){
            $.extend(defaults, options);
        } else if(typeof options === 'string'){
            if (options === 'update' && $.isPlainObject(arguments[1])){
                var obj = this.data('monthpicker');
                if(!obj) return;
                $.extend(obj.defaults, arguments[1]);
                return;
            }else if(options === 'destroy'){
                this.each(function(){
                    var obj = $(this).data('monthpicker');
                    obj.destroy();
                });
            }
        }
        
        function MonthPicker(){
            this.init.apply(this, arguments);
        }
        
        MonthPicker.prototype={
            init: function(ele){
                var self = this, calendarIcon, startMonthHidden, endMonthHidden, eleVal, startMonth, endMonth;
                self.defaults = defaults;
                if(self.defaults.showIcon){
                    calendarIcon = document.createElement('span');
                    calendarIcon.className = 'calendar_icon cloudjs_icon';
                    ele.wrap('<span class="calendar_inputs_wrap"></span>');
                    ele.after(calendarIcon);
                    ele.addClass('cloudjs_input');
                    self.calendarIcon = $(calendarIcon);
                }
                self.createMonthBox();
                self.handleEvent = {
                    showCalendar: function(e){
                        e.stopPropagation();
                        self.show();
                        self.shown = true;
                    },
                    hideCalendar: function(){
                        if(!self.mouseover){
                            self.hide();
                            self.shown = false;
                        }
                    },
                    documentHandle: function(){
                        if(self.shown){
                            self.hide();
                            self.shown = false;
                        }
                    }
                };
                startMonth = _formatToStr(self.getRangeDate(self.defaults.startMonth), self.defaults.timeFormat);
                endMonth = _formatToStr(self.getRangeDate(self.defaults.endMonth), self.defaults.timeFormat);
                self.ele = ele;
                ele.attr('placeholder', self.defaults.placeholder);
                if(self.defaults.multiSelect){
                    ele.attr('readonly', 'readonly');
                    startMonthHidden = document.createElement('input');
                    startMonthHidden.type = 'hidden';
                    startMonthHidden.value = startMonth;
                    startMonthHidden.className = self.defaults.startMonthName;
                    startMonthHidden.name = self.defaults.startMonthName;
                    ele.after(startMonthHidden);
                    endMonthHidden = document.createElement('input');
                    endMonthHidden.type = 'hidden';
                    endMonthHidden.value = endMonth;
                    endMonthHidden.className = self.defaults.endMonthName;
                    endMonthHidden.name = self.defaults.endMonthName;
                    ele.after(endMonthHidden);
                    ele.data('startMonthHd' , startMonthHidden);
                    ele.data('endMonthHd' , endMonthHidden);
                    if(self.defaults.startMonth && self.defaults.endMonth){
                        ele.val(startMonth + ' 至 ' + endMonth);
                    }
                }
                self.listen();
                eleVal = ele.val() !== '' ? ele.val().split(' 至 ')[0] : '';
                if(eleVal != ''){
                    self.months = _formatToMonth(eleVal);
                }else{
                    self.months = new Date().getFullYear() * 12 + new Date().getMonth();
                    if(startMonth){
                        ele.val(startMonth);
                    }
                }
                self.oldVal = ele.val();
                self.rangeArr = [];
                self.beginYear = new Date().getFullYear() + parseInt(self.defaults.beginYear);
                self.endYear = new Date().getFullYear() + parseInt(self.defaults.endYear);
            },
            
            /**
             * 绘制单个日历面板
             */
            draw: function(){
                var self = this,
                    monthCon = document.createElement('div'),
                    tmpMons = '',
                    currTime = self.months,
                    i = 1,
                    currYear = parseInt(currTime / 12),
                    yearSelect='', monthTitle, monthBox;
                for (; i <= 12; i++){
                    tmpMons += '<a class="calendar_disabled calendar_mon" data-value="' + currYear + ',' + _formatDate(i) + '" href="javascript:;">' + i + '</a>';
                }
                for(i = self.endYear; i >= self.beginYear; i--){
                    if(i === currYear){
                        yearSelect += '<option value="' + i + '" selected="selected">' + i + '</option>';
                    }else {
                        yearSelect += '<option value="' + i + '">' + i + '</option>';
                    }
                }
                monthTitle = '<div class="calendar_title"><div class="select_box"><select class="year_select">' + yearSelect + '</select></div></div>';
                monthBox = '<div class="calendar_months">' + tmpMons + '</div>';
                monthCon.className = 'calendar_form';
                monthCon.innerHTML += monthTitle + monthBox;
                self.panel.append(monthCon);
                monthCon.style.width = self.defaults.width + 'px';
                self.bindData(monthCon, currYear);
            },
            
            /**
             * 创建日历容器
             */
            createMonthBox: function(){
                var self = this, 
                    calendarWarpper, calendarBox;
                if(cloudjs._calendarHasDraw){
                    self.panel = $('.calendar_panel');
                    self.monthWarpper = $('.calendar_warpper');
                    return;
                }
                cloudjs._calendarHasDraw = true;
                calendarWarpper = document.createElement('div');
                calendarBox = document.createElement('div');
                calendarWarpper.id = 'calendar_warpper';
                calendarWarpper.className = 'calendar_warpper';
                calendarWarpper.style.display = 'none';
                self.monthWarpper = $(calendarWarpper);
                calendarBox.className = 'calendar_panel';
                calendarWarpper.appendChild(calendarBox);
                self.panel = $(calendarBox);
                document.body.appendChild(calendarWarpper);
            },
            
            /**
             * 为input框、窗口、图标绑定事件
             */
            listen: function(){
                var self = this;
                $(document).bind('click', self.handleEvent.documentHandle);
                $(window).bind('resize', self.handleEvent.documentHandle);
                self.ele.bind('click', self.handleEvent.showCalendar);
                self.ele.bind('blur', self.handleEvent.hideCalendar);
                if(self.calendarIcon){
                    self.calendarIcon.bind('click', self.handleEvent.showCalendar);
                }
            },
            /**
             * 绑定数据
             * @param {Element} box
             */
            bindData: function(box){
                var self = this,
                    currMonth = new Date().getFullYear() * 12 + new Date().getMonth(),
                    selectedMonth = _formatToStr(_formatToMonth(self.ele.val()), 'yyyy,mm'),
                    $box = $(box), startMonth, endMonth,
                    tds = $box.find('.calendar_mon'), currDate, i = 0, $tds;
                $box.find('.currMonth').removeClass('currMonth').find('.calendar_current').removeClass('calendar_current');
                for (; i < tds.length; i++){
                    $tds = tds.eq(i);
                    currDate = $tds.attr('data-value');
                    if(_formatToMonth(currDate) === currMonth){
                        $tds.addClass('currMonth');
                    }
                    if(selectedMonth && selectedMonth === currDate){
                        $tds.addClass('calendar_current');
                    }
                    if(self.defaults.multiSelect){
                        startMonth = self.ele.data('startMonthHd').value;
                        endMonth = self.ele.data('endMonthHd').value === '' ? startMonth : self.ele.data('endMonthHd').value;
                        if(_formatToMonth(currDate) > _formatToMonth(startMonth) && _formatToMonth(currDate) < _formatToMonth(endMonth)){
                            $tds.addClass('calendar_range');
                        }
                        if(_formatToMonth(currDate) === _formatToMonth(startMonth) || _formatToMonth(currDate) === _formatToMonth(endMonth)){
                            $tds.addClass('calendar_current');
                        }
                   }
                   self.setEnable(tds[i]);
                }
            },
            /**
             * 设置时间范围
             * @param {Element} tds  面板中的每一个日期元素
             */
            setEnable: function(tds){
                var self = this,
                    $tds = $(tds),
                    dateValue = tds.getAttribute('data-value'),
                    dateCount = _formatToMonth(dateValue),
                    start,end;
                // 处理开始月
                start = self.getRangeDate(self.defaults.minDate);
                if (typeof start === 'undefined'){
                    start = _formatToMonth(self.beginYear + ',01');
                }
                // 处理结束月
                end = self.getRangeDate(self.defaults.maxDate);
                if (typeof end === 'undefined'){
                    end = _formatToMonth(self.endYear + ',12');
                }
                if (dateCount >= start && dateCount <= end){
                    $tds.removeClass('calendar_disabled');
                }
            },
            /**
             * 获得所设置最大值或最小值的时间对象
             * @param {String} str  设置的最大值或最小值
             */
            getRangeDate: function(str){
                str = $.trim(str);
                if (!str){
                    return;
                } else if(/^\d{4}[,-/]?\d{1,2}$/g.test(str)){
                    return _formatToMonth(str);
                } else {
                    str = str.toLowerCase();
                    var arrStr = str.split(','),
                        now = new Date(),
                        nowYear = now.getFullYear() * 12,
                        nowMonth = nowYear + now.getMonth(),
                        i = 0;
                    for (; i < arrStr.length; i++){
                        if (/([-+]?\d+)([ym]?)/g.test(arrStr[i])){
                            switch (RegExp.$2){
                                case 'y' :
                                    nowMonth += Number(RegExp.$1) * 12;
                                    break;
                                case 'm' :
                                    nowMonth += Number(RegExp.$1);
                                    break;
                            }
                        }
                    }
                    return nowMonth;
                }
            },
            
            /**
             * 填充日历面板
             */
            fillPanel: function(){
                var self = this,
                    tmpStr = '<a class="calendar_pre_mon"><i class="cloudjs_icon"></i></a><a class="calendar_next_mon"><i class="cloudjs_icon"></i></a>';
                self.panel.html(tmpStr);
                self.draw();
                if(self.defaults.multiSelect){
                    self.drawRangeBox();
                }
                self.bindEvent();
            },
            
            /**
             * 绘制范围面板
             */
            drawRangeBox: function(){
                var self = this,
                    minSelectMonth = '',
                    maxSelectMonth = '',
                    startMonth, endMonth, timeCon, timeTpl;
                timeCon = document.createElement('div');
                self.panel.append(timeCon);
                timeCon.className = 'calendar_time_box';
                if(self.defaults.multiSelect){
                    startMonth = self.ele.data('startMonthHd').value;
                    endMonth = self.ele.data('endMonthHd').value;
                    if(startMonth && self.rangeArr.length ===0){
                        self.rangeArr.push(startMonth);
                    }
                    if(endMonth && !self.rangeArr[1]){
                        self.rangeArr.push(endMonth);
                    }
                    minSelectMonth = _formatToStr(_formatToMonth(startMonth), self.defaults.timeFormat) || '';
                    maxSelectMonth = _formatToStr(_formatToMonth(endMonth), self.defaults.timeFormat) || '';
                    timeTpl = '<div style="float:left"><input class="min_date multi_input" value="' + minSelectMonth + '" type="text" readonly="readonly">-';
                    timeTpl += '<input class="max_date multi_input" type="text" value="' + maxSelectMonth + '" readonly="readonly"></div>';
                    timeCon.innerHTML = timeTpl;
                }
            },
            
            /**
             * 显示日历面板
             */
            show: function(){
                var self = this;
                self.fillPanel();
                self.monthWarpper.show();
                self.setPosition();
                self.shown = true;
            },
            
            /**
             * 隐藏日历面板
             */
            hide: function(){
                var self = this,startMonth, endMonth,
                    $monthWarpper = self.monthWarpper,
                    _selfVal = self.ele.val();
                self.shown = false;
                $monthWarpper.hide();
                $monthWarpper.undelegate('click');
                $monthWarpper.unbind('click');
                $monthWarpper.unbind('mouseenter');
                $monthWarpper.unbind('mouseleave');
                if(self.defaults.multiSelect && self.rangeArr.length === 1){
                    self.rangeArr[1] = new Date().getFullYear() * 12 + new Date().getMonth();
                    self.rangeArr.sort(function(a, b){
                        return a - b;
                    });
                    startMonth = self.rangeArr[0];
                    endMonth = self.rangeArr[1];
                    self.ele.val(_formatToStr(startMonth, self.defaults.timeFormat) + ' 至 ' + _formatToStr(endMonth, self.defaults.timeFormat));
                    self.ele.data('startMonthHd').value = _formatToStr(startMonth, self.defaults.timeFormat);
                    self.ele.data('endMonthHd').value = _formatToStr(endMonth, self.defaults.timeFormat);
                }
                if(_selfVal !== ''){
                    _selfVal = _selfVal.split(' 至 ')[0];
                    _formatToMonth(_selfVal);
                }
                if(self.oldVal !== self.ele.val()){
                    self.oldVal = self.ele.val();
                    self.defaults.onChange.call(self.ele[0]);
                }
                self.defaults.onClose.call(self.ele[0]);
            },
            
            /**
             * 设置位置
             */
            setPosition: function(){
                var self = this,
                    offset = self.ele.offset(),
                    h = self.ele.outerHeight(),
                    t, l;
                self.panel.width(self.defaults.width);
                if(offset.top + h + self.panel.outerHeight() > $(window).height() + $(window).scrollTop() && offset.top - self.panel.outerHeight() > $(window).scrollTop()){
                    t = offset.top - self.panel.outerHeight();
                }else{
                    t = offset.top + h;
                }
                if(offset.left + self.panel.outerWidth() > $(window).width()){
                    l = $(window).width() - self.panel.outerWidth() -5;
                }else{
                    l = offset.left;
                }
                self.monthWarpper.css({
                    width: self.defaults.width,
                    zIndex: (cloudjs.zIndex() > self.defaults.zIndex ? cloudjs.zIndex() : self.defaults.zIndex),
                    left: l,
                    top:t
                });
            },
            
            /**
             * 显示前一月
             */
            goPrevYear: function(){
                var self = this;
                if (self.months <= self.beginYear * 12){
                    return;
                }
                self.months -= 12;
                self.fillPanel();
            },
            
            /**
             * 显示后一月
             */
            goNextYear: function(){
                var self = this;
                if (self.months >= self.endYear * 12 + 11){
                    return;
                }
                self.months += 12;
                self.fillPanel();
            },
            
            /**
             * 给日历面板元素绑定事件
             */
            bindEvent: function(){
                var self = this,
                    $monthWarpper = self.monthWarpper,
                    $yearSelect = $monthWarpper.find('.year_select');
                $monthWarpper.undelegate('click');
                $monthWarpper.unbind('click');
                $yearSelect.unbind('change');
                $monthWarpper.unbind('mouseenter');
                $monthWarpper.bind('click', function(e){
                    e.stopPropagation();
                });
                $monthWarpper.bind('mouseenter', function(){
                    self.mouseover = true;
                });
                $monthWarpper.bind('mouseleave', function(){
                    self.mouseover = false;
                });
                // IE6,7,8,9不支持表单事件冒泡，所以delegate事件委托会失效
                $yearSelect.bind('change', function(){
                    self.months = $(this).val() * 12;
                    self.fillPanel();
                });
                $monthWarpper.delegate('.calendar_pre_mon', 'click', function(){
                    self.goPrevYear();
                });
                $monthWarpper.delegate('.calendar_next_mon', 'click', function(){
                    self.goNextYear();
                });
                $monthWarpper.delegate('.calendar_mon', 'click', function(){
                    var $this = $(this), startMonth, endMonth, dateCount,
                        multiInputs = $('.calendar_time_box').find('.multi_input'),
                        $tds = $monthWarpper.find('.calendar_mon'),
                        i = 0, len = $tds.length, 
                        val1, val2;
                    if ($this.hasClass('calendar_disabled')){
                        return;
                    }
                    if(self.defaults.multiSelect){
                        if(self.rangeArr.length >= 2){
                            self.rangeArr = [];
                            $monthWarpper.find('.calendar_current').removeClass('calendar_current');
                            $monthWarpper.find('.calendar_range').removeClass('calendar_range');
                            multiInputs.val('');
                            self.ele.val('');
                        }
                        if(self.rangeArr.length === 0){
                            self.rangeArr.push(_formatToMonth($this.attr('data-value')));
                            startMonth = self.rangeArr[0];
                            val1 = _formatToStr(startMonth, self.defaults.timeFormat);
                            multiInputs.eq(0).val(val1);
                            self.ele.data('startMonthHd').value = val1;
                            self.ele.data('endMonthHd').value = '';
                            self.ele.val(val1);
                        }else if(self.rangeArr.length === 1){
                            self.rangeArr.push(_formatToMonth($this.attr('data-value')));
                            self.rangeArr.sort(function(a, b){
                                return a - b;
                            });
                            startMonth = self.rangeArr[0];
                            endMonth = self.rangeArr[1];
                            val1 = _formatToStr(startMonth, self.defaults.timeFormat);
                            val2 = _formatToStr(endMonth, self.defaults.timeFormat);
                            multiInputs.eq(0).val(val1);
                            multiInputs.eq(1).val(val2);
                            for(; i < len; i++){
                                dateCount = _formatToMonth($tds.eq(i).attr('data-value'));
                                if(dateCount > startMonth && dateCount < endMonth){
                                    $tds.eq(i).addClass('calendar_range');
                                }
                            }
                            self.ele.val(val1 + ' 至 ' + val2);
                            self.ele.data('startMonthHd').value = val1;
                            self.ele.data('endMonthHd').value = val2;
                        }
                    }else{
                        self.ele.val(_formatToStr(_formatToMonth($this.attr('data-value')), self.defaults.timeFormat));
                        self.ele.data('month', $this.attr('data-value'));
                        $monthWarpper.find('.calendar_current').removeClass('calendar_current');
                        $this.addClass('calendar_current');
                    }
                    $this.addClass('calendar_current');
                    self.defaults.onSelect.call(self.ele[0]);
                    if(self.defaults.multiSelect && !self.rangeArr[1]){
                        return;
                    }
                    self.hide();
                    self.shown = false;
                });
            }
        };
        
        this.each(function(){
            var self = $(this);
            self.data('monthpicker', new MonthPicker(self));
        });
       
        /**
         * 处理输出到input框中的时间格式
         * @param {Number} date
         * @param {String} format
         */
        function _formatToStr(date, format){
            if(!date) return;
            return format.replace(/yyyy/gi, parseInt(date/12))
                  .replace(/MM/i, _formatDate(date % 12 + 1));
        }
       
        /**
         * 格式化日期
         * @param {Number} n 日期或月数
         */
        function _formatDate(n){
            return (n < 10 ? '0' + n : n).toString();
        }
       
        /**
         * 将字符串变成月份数字
         * @param {String} date  字符串
         */
        function _formatToMonth(date){
            var tmpDate = date.split(/[,-/]/g);
            // IE7在这里有个BUG,既会把以0开头的数字字符串解析为八进制数字: parseInt('08')==0
            return (parseInt(tmpDate[0] , 10) * 12 + (parseInt(tmpDate[1] , 10) - 1));
        }

        return this;
    }
});
