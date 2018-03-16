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
    calendar: function(options){
        var defaults = {
            months: 2,                  // 同时显示的月份数
            width: 220,                 // 单个月时日历控件的宽度
            zIndex: cloudjs.zIndex(),   // 日历控件的层级
            beginYear: -40,             // 开始年份
            endYear: +10,               // 结束年份
            monSelect: true,            // 是否可选择年份和月份
            showIcon: true,             // 是否显示图标
            placeholder: '',            // 输入框中显示的placeholder信息
            timeFormat: 'yyyy-mm-dd',   // 输出到输入框的日期格式
            minDate: null,              // 可选的最小日期，用于范围选择
            maxDate: null,              // 可选的最大日期，用于范围选择
            multiSelect: false,         // 选择一段时间,另一种形式的范围选择
            startDate: '',              // 时间段的默认开始日期
            endDate: '',                // 时间段的默认结束日期
            startShowMonth: 0,          // 开始显示的月份，默认为0既从当月开始显示。
            startDateName: 'startDate', // 设置时间段开始日期隐藏域的name值
            endDateName: 'endDate',     // 设置时间段结束日期隐藏域的name值
            onSelect: $.noop,           // 选择日期后触发的事件
            onClose: $.noop,            // 关闭日历控件后触发的事件
            onChange: $.noop,           // 日历控件的值改变后触发的事件
            onCreate: $.noop            // 日历控件初始化完后触发的事件
        };
        if($.isPlainObject(options) || !options){
            $.extend(defaults, options);
        } else if(typeof options === 'string'){
            if(options === 'update' && $.isPlainObject(arguments[1])){
                var obj = $(this).data('calendar');
                $.extend(obj.defaults, arguments[1]);
                return true;
            }else if(options === 'destroy'){
                this.each(function(){
                    var obj = $(this).data('calendar');
                    obj.destroy();
                });
                return true;
            }
        }
        
        function Calendar(){
            this.init.apply(this, arguments);
        }
        
        Calendar.prototype = {
            init: function(ele){
                var self = this,
                    calendarIcon, eleVal, startDate, endDate, startDateHidden, endDateHidden;
                self.defaults = defaults;
                if(self.defaults.showIcon){
                    calendarIcon = document.createElement('span');
                    calendarIcon.className = 'calendar_icon cloudjs_icon';
                    ele.wrap('<span class="calendar_inputs_wrap"></span>');
                    ele.after(calendarIcon);
                    ele.addClass('cloudjs_input');
                    self.calendarIcon = $(calendarIcon);
                }
                self.createCalendarBox();
                startDate = _formatToStr(self.getRangeDate(self.defaults.startDate), self.defaults.timeFormat) || '';
                endDate = _formatToStr(self.getRangeDate(self.defaults.endDate), self.defaults.timeFormat) || '';
                self.ele = ele;
                self.timer = self.defaults.timeFormat.toLowerCase().indexOf('hh');
                self.rangeArr = [];
                self.beginYear = new Date().getFullYear() + parseInt(self.defaults.beginYear);
                self.endYear = new Date().getFullYear() + parseInt(self.defaults.endYear);
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
                
                if(self.defaults.multiSelect){
                    ele.attr('readonly', 'readonly');
                    startDateHidden = document.createElement('input');
                    startDateHidden.type = 'hidden';
                    startDateHidden.value = startDate;
                    startDateHidden.className = 'startDate';
                    startDateHidden.name = self.defaults.startDateName;
                    ele.after(startDateHidden);
                    endDateHidden = document.createElement('input');
                    endDateHidden.type = 'hidden';
                    endDateHidden.value = endDate;
                    endDateHidden.className = 'endDate';
                    endDateHidden.name = self.defaults.endDateName;
                    ele.after(endDateHidden);
                    ele.data('startDateHd', startDateHidden);
                    ele.data('endDateHd', endDateHidden);
                    if(self.defaults.endDate && self.defaults.startDate){
                        ele.val(startDate + ' 至 ' + endDate);
                    }
                }
                eleVal = ele.val() !== '' ? ele.val().split(' 至 ')[0] : '';
                if(eleVal != ''){
                    self.months = _formatToDay(eleVal).getFullYear() * 12 + _formatToDay(eleVal).getMonth() + parseInt(self.defaults.startShowMonth);
                    ele.data('date',eleVal);
                }else{
                    self.months = new Date().getFullYear() * 12 + new Date().getMonth() + parseInt(self.defaults.startShowMonth);
                    if(startDate){
                        ele.val(startDate);
                        ele.data('date',startDate);
                    }
                }
                ele.attr('placeholder', self.defaults.placeholder);
                self.oldVal = ele.val();
                self.listen();
                self.defaults.onCreate.call(ele[0]);
            },
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
             * 销毁事件
             */
            destroy: function(){
                var self = this;
                $(document).unbind('click', self.handleEvent.documentHandle);
                $(window).unbind('resize', self.handleEvent.documentHandle);
                self.calendarIcon.unbind('click', self.handleEvent.showCalendar);
                self.ele.unbind('click', self.handleEvent.showCalendar);
                self.ele.unbind('blur', self.handleEvent.hideCalendar);
                self.ele.removeData('calendar');
                self.ele.removeData('date');
                self.ele.removeData('handler');
                self.ele.parents().eq(1).append(self.ele);
                self.ele.siblings('.calendar_inputs_wrap').remove();
            },
            
            /**
             * 绘制单个日历面板
             * @param {Number} n    绘制面板的个数
             */
            draw: function(n){
                var self = this,
                    tmpWeeks = '',
                    tmpDays = '',
                    calendarCon = document.createElement('div'),
                    yearSelect = '',
                    monSelect = '',
                    currTime = self.months + n,
                    i = 0,
                    currYear = parseInt(currTime / 12),
                    currMon = currTime - currYear * 12 + 1,
                    calendarTitle, calendarDays, calendarWeek;
                for(; i < 7; i++){
                    tmpWeeks += '<span>' + '日一二三四五六'.charAt(i) + '</span>';
                }
                for(i = 0; i < 42; i++){
                    tmpDays += '<a class="calendar_disabled calendar_day" href="javascript:;"></a>';
                }
                for(i = 1; i <= 12; i++){
                    if(i === currMon){
                        monSelect += '<option value="' + i + '" selected="selected">' + i + '月</option>';
                    }else {
                        monSelect += '<option value="' + i + '">' + i + '月</option>';
                    }
                }
                for(i = self.endYear; i >= self.beginYear; i--){
                    if(i === currYear){
                        yearSelect += '<option value="' + i + '" selected="selected">' + i + '</option>';
                    }else {
                        yearSelect += '<option value="' + i + '">' + i + '</option>';
                    }
                }
                if(self.defaults.monSelect && n === 0){
                    calendarTitle = '<div class="calendar_title"><div class="select_box"><select class="year_select">' + yearSelect + '</select>';
                    calendarTitle += ' <select class="mon_select">' + monSelect + '</select></div></div>';
                }else{
                    calendarTitle = '<div class="calendar_title"><em class="curr_year">' + currYear + '年</em> <em class="curr_mon">' + currMon + '月</em></div></div>';
                }
               
                calendarWeek = '<p class="calendar_week">' + tmpWeeks + '</p>';
                calendarDays = '<div class="calendar_days">' + tmpDays + '</div>';
                calendarCon.className = 'calendar_form';
                calendarCon.innerHTML += calendarTitle + calendarWeek + calendarDays;
                self.panel.append(calendarCon);
                calendarCon.style.width = self.defaults.width + 'px';
                self.bindData(calendarCon, currYear, currMon);  
            },
            
            /**
             * 绑定数据
             * @param {Element} box
             * @param {Number} year
             * @param {Number} months
             */
            bindData: function(box, year, months){
                var self = this,
                    dateArray = _getMonthViewDateArray(year, months - 1),
                    today = new Date().toDateString(),
                    val = $.trim(self.ele.val()),
                    selectedDay = _formatToDay(val) ? _formatToDay(val).toDateString() : undefined,
                    $box = $(box), startDate, endDate,
                    tds = $box.find('.calendar_day'), currDate, i = 0, $tds;
                $box.find('.calendar_today').removeClass('calendar_today').find('.calendar_current').removeClass('calendar_current');
                for(; i < tds.length; i++){
                    if(i > dateArray.length - 1) continue;
                    if(dateArray[i]){
                       currDate = new Date(year, months - 1, dateArray[i]).toDateString();
                       $tds = tds.eq(i);
                       $tds.attr('data-value', currDate);
                       if (today === currDate){
                           $tds.addClass('calendar_today');
                       }
                       if(selectedDay === currDate){
                           $tds.addClass('calendar_current');
                       }
                       if(self.defaults.multiSelect){
                            startDate = self.ele.data('startDateHd').value;
                            endDate = self.ele.data('endDateHd').value === '' ? startDate : self.ele.data('endDateHd').value;
                            if(new Date(currDate) > _formatToDay(startDate) && new Date(currDate) < _formatToDay(endDate)){
                                $tds.addClass('calendar_range');
                            }
                            if(startDate && endDate){
                                if(currDate === _formatToDay(startDate).toDateString() || currDate === _formatToDay(endDate).toDateString()){
                                    $tds.addClass('calendar_current');
                                }
                            }
                       }
                       $tds.html(dateArray[i]);
                       self.setEnable(tds[i]);
                    }
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
                    dateCount = _formatToDay(dateValue),
                    start,end;
                // 处理开始日期
                start = self.getRangeDate(self.defaults.minDate);
                if(typeof start === 'undefined'){
                    start = _formatToDay(self.beginYear + ',01,01');
                }
                // 处理结束日期
                end = self.getRangeDate(self.defaults.maxDate);
                if(typeof end === 'undefined'){
                    end = _formatToDay(self.endYear + ',12,31');
                }
                
                if(dateCount >= start && dateCount <= end){
                    $tds.removeClass('calendar_disabled');
                }
            },
            
            /**
             * 获得所设置最大值或最小值的时间对象
             * @param {String} str  设置的最大值或最小值
             */
            getRangeDate: function(str){
                str = $.trim(str);
                if(!str){
                    return undefined;
                }else if(/^(\d{4})([,-/])?(\d{2})([,-/])?(\d{2})(\s\d\d)?(:\d\d)?(:\d\d)?$/g.test(str)){
                    return _formatToDay(str);
                }else{
                    str = str.toLowerCase();
                    var arrStr = str.split(','),
                        now = new Date(),
                        nowYear = now.getFullYear(),
                        nowMonth = now.getMonth(),
                        nowDate = now.getDate(),
                        newDate, i = 0;
                    for(; i < arrStr.length; i++){
                        if(/([-+]?\d+)([ymd])/g.test(arrStr[i])){
                            switch (RegExp.$2){
                                case 'y' :
                                    nowYear += Number(RegExp.$1);
                                    break;
                                case 'm' :
                                    nowMonth += Number(RegExp.$1);
                                    break;
                                case 'd' :
                                    nowDate += Number(RegExp.$1);
                                    break;
                            }
                            newDate = new Date(nowYear, nowMonth, nowDate);
                        }
                    }
                    return newDate;
                }
            },
            
            /**
             * 创建日历容器
             */
            createCalendarBox: function(){
                var self = this,
                    calendarWarpper, calendarBox;
                if(cloudjs._calendarHasDraw){
                    self.panel = $('.calendar_panel');
                    self.calendarWarpper = $('.calendar_warpper');
                    return;
                }
                cloudjs._calendarHasDraw = true;
                calendarWarpper = document.createElement('div');
                calendarBox = document.createElement('div');
                calendarWarpper.id = 'calendar_warpper';
                calendarWarpper.className = 'calendar_warpper';
                calendarWarpper.style.display = 'none';
                self.calendarWarpper = $(calendarWarpper);
                calendarBox.className = 'calendar_panel';
                calendarWarpper.appendChild(calendarBox);
                self.panel = $(calendarBox);
                document.body.appendChild(calendarWarpper);
            },
            
            /**
             * 填充日历面板
             */
            fillPanel: function(){
                var self = this,
                    tmpStr = '<a class="calendar_pre_mon"><i class="cloudjs_icon"></i></a><a class="calendar_next_mon"><i class="cloudjs_icon"></i></a>',
                    i = 0,
                    len = self.defaults.months;
                self.panel.html(tmpStr);
                for(; i < len; i++){
                    self.draw(i);
                }
                self.drawTimeBox();
                self.bindEvent();
            },
            
            /**
             * 绘制时间面板
             */
            drawTimeBox: function(){
                var self = this,
                    d = _formatToDay(self.ele.val()),
                    timeTpl, timeStr, timeCon, startDate, endDate,
                    minSelectDay = '',
                    maxSelectDay = '';
                timeCon = document.createElement('div');
                self.panel.append(timeCon);
                timeCon.className = 'calendar_time_box';
                if(self.defaults.multiSelect){
                    startDate = self.ele.data('startDateHd').value;
                    endDate = self.ele.data('endDateHd').value;
                    self.timer = -1;
                    if(startDate && self.rangeArr.length ===0){
                        self.rangeArr.push(startDate);
                    }
                    if(endDate && !self.rangeArr[1]){
                        self.rangeArr.push(endDate);
                    }
                    minSelectDay = _formatToStr(_formatToDay(startDate), 'yyyy-mm-dd') || '';
                    maxSelectDay = _formatToStr(_formatToDay(endDate), 'yyyy-mm-dd') || '';
                    timeTpl = '<div style="float:left"><input class="min_date multi_input" value="' + minSelectDay + '" type="text" readonly="readonly">-';
                    timeTpl += '<input class="max_date multi_input" type="text" value="' + maxSelectDay + '" readonly="readonly"></div>';
                    timeTpl += '<input type="button" class="multi_submit cloudjs_btn" value="确定">';
                    timeCon.innerHTML = timeTpl;
                }
                if(self.timer > -1){
                    timeStr = self.defaults.timeFormat.substring(self.timer);
                    timeTpl = '<div style="float:left;white-space:nowrap">时间：<input class="hour_select date_input" type="number" value="0" min="0" max="23">h ';
                    if(timeStr.indexOf('hh:mm') > -1){
                        timeTpl += '<input type="number" class="min_select date_input" value="0" min="0" max="59">m ';
                    }
                    if(timeStr.indexOf('hh:mm:ss') > -1){
                        timeTpl += '<input type="number" class="sec_select date_input" value="0" min="0" max="59">s';
                    }
                    timeTpl += '</div>';
                    timeTpl += '<input type="button" class="time_submit cloudjs_btn" value="确定">';
                    timeCon.innerHTML = timeTpl;
                    if(d){
                        $('.hour_select').val(d.getHours());
                        $('.min_select').val(d.getMinutes());
                        $('.sec_select').val(d.getSeconds());
                    }
                }
                if(!self.defaults.multiSelect){
                    $(timeCon).append('<input type="button" class="today_submit cloudjs_btn_white" value="今天">');
                    if(self.defaults.minDate && self.getRangeDate(self.defaults.minDate) > new Date()){
                        $('.today_submit').attr('disabled', 'disabled');
                    }else if(self.defaults.maxDate && self.getRangeDate(self.defaults.maxDate) < new Date()){
                        $('.today_submit').attr('disabled', 'disabled');
                    }
                }
            },
            show: function(){
                var self = this;
                self.fillPanel();
                self.calendarWarpper.show();
                self.setPosition();
                self.shown = true;
            },
            
            /**
             * 隐藏日历面板
             */
            hide: function(){
                var self = this,
                    $calendarWarpper = self.calendarWarpper,
                    selfVal = self.ele.val(),
                    startDate,
                    endDate;
                self.shown = false;
                $calendarWarpper.hide();
                $calendarWarpper.undelegate('click');
                $calendarWarpper.unbind('click');
                $calendarWarpper.unbind('mouseenter');
                $calendarWarpper.unbind('mouseleave');
                if(self.defaults.multiSelect && self.rangeArr.length === 1){
                    self.rangeArr[1] = new Date().toDateString();
                    self.rangeArr.sort(function(a, b){
                        return _formatToDay(a) - _formatToDay(b);
                    });
                    startDate = new Date(self.rangeArr[0]);
                    endDate = new Date(self.rangeArr[1]);
                    self.ele.val(_formatToStr(startDate, self.defaults.timeFormat) + ' 至 ' + _formatToStr(endDate, self.defaults.timeFormat));
                    self.ele.data('startDateHd').value = _formatToStr(startDate, self.defaults.timeFormat);
                    self.ele.data('endDateHd').value = _formatToStr(endDate, self.defaults.timeFormat);
                }
                if(selfVal !== ''){
                    selfVal = $.trim(selfVal.split(' 至 ')[0]);
                    if(_formatToDay(selfVal) == 'Invalid Date'){
                        self.ele.val('');
                    }else{
                        self.months = _formatToDay(selfVal).getFullYear() * 12 + _formatToDay(selfVal).getMonth() + self.defaults.startShowMonth;
                    }
                }else{
                    self.ele.val('');
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
                self.panel.width(self.defaults.width * self.defaults.months);
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
                self.calendarWarpper.css({
                    width: (self.defaults.width + 1) * self.defaults.months,
                    zIndex: (cloudjs.zIndex() > self.defaults.zIndex ? cloudjs.zIndex() : self.defaults.zIndex),
                    left: l,
                    top: t
                });
            },
            
            /**
             * 显示前一月
             */
            goPrevMonth: function(){
                var self = this;
                if(self.months <= self.beginYear * 12){
                    return;
                }
                self.months--;
                self.fillPanel();
            },
           
            /**
             * 显示后一月
             */
            goNextMonth: function(){
                var self = this;
                if(self.months >= self.endYear * 12 + 11){
                    return;
                }
                self.months++;
                self.fillPanel();
            },
            
            /**
             * 绑定事件
             */
            bindEvent: function(){
                var self = this,
                    $calendarWarpper = self.calendarWarpper,
                    $yearSelect = $calendarWarpper.find('.year_select'),
                    $monSelect = $calendarWarpper.find('.mon_select');
                $calendarWarpper.undelegate('click');
                $calendarWarpper.unbind('click');
                $yearSelect.unbind('change');
                $monSelect.unbind('change');
                $calendarWarpper.unbind('mouseenter');
                $calendarWarpper.unbind('mouseleave');
                $calendarWarpper.bind('click', function(e){
                    e.stopPropagation();
                });
                $calendarWarpper.bind('mouseenter', function(){
                    self.mouseover = true;
                });
                $calendarWarpper.bind('mouseleave', function(){
                    self.mouseover = false;
                });
                // IE6,7,8,9不支持表单事件冒泡，所以delegate事件委托会失效
                $yearSelect.bind('change', function(){
                    self.months = $(this).val() * 12 + parseInt($monSelect.val())-1;
                    self.fillPanel();
                });
                $monSelect.bind('change', function(){
                    self.months = $yearSelect.val() * 12 + parseInt($(this).val())-1;
                    self.fillPanel();
                });
                $calendarWarpper.delegate('.calendar_pre_mon', 'click', function(){
                    self.goPrevMonth();
                });
                $calendarWarpper.delegate('.calendar_next_mon', 'click', function(){
                    self.goNextMonth();
                });
                $calendarWarpper.delegate('.time_submit', 'click', function(){
                    var arrDate = [],timeArray,
                        selects = $('.calendar_time_box').find('.date_input'),
                        currDay = self.ele.data('date') ? _formatToStr(_formatToDay(self.ele.data('date')),'yyyy,mm,dd') : _formatToStr(new Date(),'yyyy,mm,dd');
                    selects.each(function(){
                        if(!Number(this.value)){
                            this.value = 0;
                        }
                        if(this.value > 59){
                            this.value = 59;
                        }
                        if(this.value < 0){
                            this.value = 0;
                        }
                        if(selects[0].value > 23){
                            selects[0].value = 23;
                        }
                        arrDate.push(this.value);
                    });
                    timeArray=currDay.split(',').concat(arrDate);
                    timeArray[1]-=1;
                    if(timeArray.length<6){
                        for(var i=timeArray.length;i<6;i++){
                            if(!timeArray[i] && typeof timeArray[i]=='undefined') timeArray[i]=0;
                        }
                    }
                    self.ele.val(_formatToStr(new Date(timeArray[0],timeArray[1],timeArray[2],timeArray[3],timeArray[4],timeArray[5]), self.defaults.timeFormat));
                    self.hide();
                    self.shown = false;
                });
                $calendarWarpper.delegate('.multi_submit', 'click', function(){
                    var inputs = $('.calendar_time_box').find('.multi_input'),
                        val1 = _formatToStr(_formatToDay(inputs.eq(0).val()), self.defaults.timeFormat),
                        val2 = _formatToStr(_formatToDay(inputs.eq(1).val()), self.defaults.timeFormat);
                    if(inputs.eq(0).val()==='' && inputs.eq(1).val()==='') return;
                    if(inputs.eq(1).val() === ''){
                        self.rangeArr[1] = new Date().toDateString();
                        self.rangeArr.sort(function(a, b){
                            return _formatToDay(a) - _formatToDay(b);
                        });
                        val1 = _formatToStr(new Date(self.rangeArr[0]), self.defaults.timeFormat);
                        val2 = _formatToStr(new Date(self.rangeArr[1]), self.defaults.timeFormat);
                    }
                    self.ele.val(val1 + ' 至 ' + val2);
                    self.ele.data('startDateHd').value = val1;
                    self.ele.data('endDateHd').value = val2;
                    self.hide();
                    self.shown = false;
                });
                $calendarWarpper.delegate('.today_submit', 'click', function(){
                    self.ele.val(_formatToStr(new Date(), self.defaults.timeFormat));
                    self.hide();
                    self.shown = false;
                });
                $calendarWarpper.delegate('.calendar_day', 'click', function(){
                    var $this = $(this),
                        $calendarTimeBox = $('.calendar_time_box'),
                        selects = $calendarTimeBox.find('.date_input'),
                        multiInputs = $calendarTimeBox.find('.multi_input'),
                        _selfVal = _formatToStr(new Date($this.attr('data-value')), self.defaults.timeFormat),
                        dateStr = '', startDate, endDate, dateStrLength = 0;
                    if($this.hasClass('calendar_disabled')){
                        return;
                    }
                    if(!self.defaults.multiSelect){
                        selects.each(function(){
                            dateStr += _formatDate(this.value) + ':';
                        });
                        dateStr = dateStr.replace(/:$/g, '');
                        _selfVal = _selfVal.split(/\s/)[0];
                        dateStrLength = $.trim(_selfVal + ' ' + dateStr).length;
                        self.ele.val($.trim(_selfVal + ' ' + dateStr)+self.defaults.timeFormat.substring(dateStrLength));
                        self.ele.data('date', $this.attr('data-value'));
                        $calendarWarpper.find('.calendar_current').removeClass('calendar_current');
                    }else{
                        if(self.rangeArr.length >= 2){
                            self.rangeArr = [];
                            $calendarWarpper.find('.calendar_current').removeClass('calendar_current');
                            $calendarWarpper.find('.calendar_range').removeClass('calendar_range');
                            multiInputs.val('');
                            self.ele.val('');
                        }
                        if(self.rangeArr.length === 0){
                            self.rangeArr.push($this.attr('data-value'));
                            startDate = new Date(self.rangeArr[0]);
                            multiInputs.eq(0).val(_formatToStr(startDate, 'yyyy-mm-dd'));
                            self.ele.data('startDateHd').value = _formatToStr(startDate, self.defaults.timeFormat);
                            self.ele.data('endDateHd').value = '';
                            self.ele.val(_formatToStr(startDate, self.defaults.timeFormat));
                        }else if(self.rangeArr.length === 1){
                            self.rangeArr.push($this.attr('data-value'));
                            self.rangeArr.sort(function(a, b){
                                return _formatToDay(a) - _formatToDay(b);
                            });
                            startDate = new Date(self.rangeArr[0]);
                            endDate = new Date(self.rangeArr[1]);
                            multiInputs.eq(0).val(_formatToStr(startDate, 'yyyy-mm-dd'));
                            multiInputs.eq(1).val(_formatToStr(endDate, 'yyyy-mm-dd'));
                            self.ele.val(_formatToStr(startDate, self.defaults.timeFormat) + ' 至 ' + _formatToStr(endDate, self.defaults.timeFormat));
                            self.ele.data('startDateHd').value = _formatToStr(startDate, self.defaults.timeFormat);
                            self.ele.data('endDateHd').value = _formatToStr(endDate, self.defaults.timeFormat);
                        }
                    }
                    $this.addClass('calendar_current');
                    self.defaults.onSelect.call(self.ele[0]);
                    if(self.timer>-1 || (self.defaults.multiSelect && !self.rangeArr[1])){
                        return;
                    }
                    self.hide();
                    self.shown = false;
                });
            }
        };
        
        this.each(function(){
            var self = $(this);
            self.data('calendar', new Calendar(self));
        });
        
       
        /**
         * 格式化日期
         * @param {Number} n 日期或月数
         * @return {String} n 返回格式化后的字符串
         */
        function _formatDate(n){
            return (n < 10 ? '0' + n : n).toString();
        }
       
        /**
         * 将Date对象变成字符串
         * @param {Object} date
         * @param {String} format
         * @return {String} date 返回经指定格式格式化后的时间字符串
         */
        function _formatToStr(date, format){
            if(!date || date=='Invalid Date') return;
            return format.replace(/yyyy/gi, date.getFullYear().toString())
                  .replace(/MM/i, _formatDate(date.getMonth() + 1))
                  .replace(/dd/gi, _formatDate(date.getDate()))
                  .replace(/hh/gi, _formatDate(date.getHours()))
                  .replace(/mm/gi, _formatDate(date.getMinutes()))
                  .replace(/ss/gi, _formatDate(date.getSeconds()));
        }
       
        /**
         * 将日期字符串变成Date对象
         * @param {String} date  时间字符串
         * @return {Date} date 返回指定时间的Date对象
         */
        function _formatToDay(date){
            var regFormat = /^(\d{4})([,-/])?(\d{2})([,-/])?(\d{2})(\s\d\d)?(:\d\d)?(:\d\d)?$/g,
                hour, minute, seconds;
            if(!date) return;
            if(regFormat.test(date)){
                hour = RegExp.$6 ? parseInt(RegExp.$6) : 0;
                minute = RegExp.$7 ? parseInt(RegExp.$7.substring(1)) : 0;
                seconds = RegExp.$8 ? parseInt(RegExp.$8.substring(1)) : 0;
                return new Date(RegExp.$1, RegExp.$3 - 1, RegExp.$5, hour, minute, seconds);
            }
            return new Date(date);
        }
       
        /**
         * 生成星期对应天数数组
         * @param {Number} y 年
         * @param {Number} m 月
         * @return {Array} dateArray 返回星期对应天数数组
         */
        function _getMonthViewDateArray(y, m){
            var dateArray = new Array(42),
                dayOfFirstDate = new Date(y, m, 1).getDay(),
                dateCountOfMonth = new Date(y, m + 1, 0).getDate(),
                i = 0;
            for(; i < dateCountOfMonth; i++){
                dateArray[i + dayOfFirstDate] = i + 1;
            }
            return dateArray;
        }

        return this;
    }
});
