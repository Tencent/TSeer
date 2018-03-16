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
 * 进度条组件
 */
cloudjs.define({
    progressbar: function(options){
        var defaults = {
            duration: 5000, //进度条动画的时长，组件自动算出每段时间对应的百分比
            step: 1, //步长，如每次依次增加5%，或10%等，默认步长为1%
            autoClose: true, //是否到达100%后自动关闭
            delay: 0, //延迟多少时间之后出现进度条，因为有的时候，比如500毫秒内有响应的基本不需要进度条的，突然有个进度条一闪而过也不大友好
            isDialog: true, //进度条以弹框的形式打开和非弹框形式打开。
            closable: false, //进度条是否可关闭的
            width: 200, //进度条的宽
            height: 16, //进度条的高
            onCreate: $.noop, //当进度条被创建时调用，此时可能还没开始计算进度
            onOpen: $.noop, //当进度条打开时调用
            onStart: $.noop, //当进度条开始计算进度时调用
            onChange: $.noop, //当进度条进度发生变化时调用，传入的参数是当前进度条的进度
            onComplete: $.noop, //当进度条到达100%时调用
            onClose: $.noop, //当进度条关闭时的回调方法
            onDestroy: $.noop //当进度条被销毁时调用的方法
        };

        var _self = this,
	        _progressbarObj,
	        _args0 = arguments[0],
	        _args1 = arguments[1],
	        _callback = cloudjs.callback;
        
        if(_self[0]){
            _progressbarObj = _self[0]._progressbarObj;
            if(!options || $.isPlainObject(options)){
                _self[0]._progressbarObj = _progressbarObj = undefined;
                options = options || {};
                if(_self[0]){
                    $.extend(true, defaults, options);
                    _init();
                    _self[0]._progressbarObj = _progressbarObj;
                }
            }else if(_progressbarObj){
                defaults = _progressbarObj.defaults;
                if(_args0 === 'destroy'){
                    _progressbarObj.destroy();
                    clearTimeout(_self[0]._progressTimeout);
                }else if(_args0 === 'close'){
                    _progressbarObj.close();
                    clearTimeout(_self[0]._progressTimeout);
                }else if(_args0 === 'open'){
                    _progressbarObj.open();
                }else if(_args0 === 'getValue'){
                    _callback.call(_progressbarObj, _progressbarObj.getValue());
                }else if(_args0 === 'setValue'){
                    if(isNaN(_args1)){
                        cloudjs.util.error('setValue参数错误！');
                    }else{
                        _args1 = parseInt(_args1);
                        if(_args1 > 100){
                            _args1 = 100;
                        }
                        _progressbarObj.setValue(_args1);
                    }
                }else if(_args0 === 'getText'){
                    _callback.call(_progressbarObj, _progressbarObj.getText());
                }else if(_args0 === 'setText'){
                    _progressbarObj.setText(_args1);
                }else if(_args0 === 'pause'){
                	_progressbarObj.pause();
                }else if(_args0 === 'run'){
                	_progressbarObj.run();
                }
            }
        }else if(_self === window){
            $.extend(true, defaults, options);
            cloudjs('<div>').progressbar($.extend(true, defaults, { isDialog: false }), _callback);
        }else{
            cloudjs.util.error('调用progress组件失败， ' + _self.selector + ' 找不到相应的元素!');
        }

        /**
         * 初始化方法
         */
        function _init(){
            _progressbarObj = _progressbarObj || _getProgressbarObj();
            _self[0]._progressTimeout = setTimeout(function(){
                if(defaults.isDialog){
                    cloudjs(_self).dialog({
                        modal: true,
                        title: '系统提示',
                        resizable: false,
                        onClose: function(){
                            _progressbarObj.close();
                            defaults.onClose.call(_progressbarObj);
                        }
                    }, function(dialogObj){
                        if(!defaults.closable){
                            dialogObj.dialog.find('.dialog_div_head_close_btn').hide();
                        }
                        defaults.onCreate.call(_progressbarObj);
                        defaults.onOpen.call(_progressbarObj);
                        _run();
                        _callback.call(_progressbarObj);
                    });
                }else{
                    _self.show();
                    _run();
                    defaults.onCreate.call(_progressbarObj);
                    defaults.onOpen.call(_progressbarObj);
                    _callback.call(_progressbarObj);
                }
            }, defaults.delay);
        }

        /**
         * 进度条开始计时
         */
        function _run(){
        	var duration = parseInt(defaults.duration), f, progress;
        	
        	_progressbarObj._paused = false;
        	clearInterval(_self[0]._progressInterval);
        	
        	if(defaults.duration === 0){
        		_progressbarObj.setValue(0);
        	}else if(duration > 0){
        		f = defaults.duration * defaults.step / 100, progress;
                defaults.onStart.call(_progressbarObj);
                _progressbarObj.setValue(0);
                _self[0]._progressInterval = setInterval(function(){
                	if(!_progressbarObj._paused){
                		progress = _progressbarObj.getValue() + defaults.step;
                        if(progress >= 100){
                            progress = 100;
                            clearInterval(_self[0]._progressInterval);
                        }
                        _progressbarObj.setValue(progress);
                	}
                }, f);
        	}else{
        		cloudjs.util.error('progressbar 组件出错，duration 必须为非负数');
        	}
        }

        /**
         * 获取进度条对象的方法
         * @return {Object} 返回封装好的进度条对象
         */
        function _getProgressbarObj(){
            var progressbarObj, progress = '', progressbar, progressbarProgress, progressbarValue, progressbarText, temp;

            progress += '<div class="progressbar_div">';
            progress += '<div class="progressbar_progress"></div>';
            progress += '<span class="progressbar_value">0%</span>';
            progress += '</div>';

            _self.find('.progressbar_div').remove();
            temp = _self.html();
            _self.empty();
            _self.html('<div class="progressbar_text">' + temp + '</div>');
            _self.hide();

            progressbar = $(progress).appendTo(_self);
            progressbarProgress = progressbar.find('.progressbar_progress');
            progressbarValue = progressbar.find('.progressbar_value');
            progressbarText = _self.find('.progressbar_text');

            if(defaults.height < 10){
                defaults.height = 10;
            }

            _self.width(defaults.width);
            progressbar.height(defaults.height);
            progressbar.width(defaults.width);
            progressbarObj = {
                defaults: defaults,
                destroy: function(){
                    if(defaults.isDialog){
                        cloudjs(_self).dialog('destroy');
                    }else{
                        _self.remove();
                    }
                    defaults.onDestroy.call(progressbarObj);
                    progressbarObj = null;
                },
                close: function(){
                    if(defaults.isDialog){
                        cloudjs(_self).dialog('close');
                    }else{
                        _self.hide();
                        defaults.onClose.call(progressbarObj);
                    }
                    clearInterval(_self[0]._progressInterval);
                },
                open: function(){
                    cloudjs(_self).dialog('open');
                    defaults.onOpen.call(progressbarObj);
                    _run();
                },
                pause: function(){
                    progressbarObj && (progressbarObj._paused = true);
                },
                run: function(){
                    progressbarObj && (progressbarObj._paused = false);
                },
                getValue: function(){
                    return parseInt(progressbarValue.text());
                },
                setValue: function(value){
                    progressbarProgress.width(value + '%');
                    progressbarValue.html(value + '%');
                    defaults.onChange.call(progressbarObj, value);

                    if(value == 100){
                    	clearInterval(_self[0]._progressInterval);
                        defaults.onComplete.call(progressbarObj);
                        if(defaults.autoClose){
                            progressbarObj.close();
                        }
                    }
                },
                getText: function(){
                    return progressbarText.text();
                },
                setText: function(text){
                    progressbarText.html(text);
                }
            };
            return progressbarObj;
        }

        return this;
    }
});