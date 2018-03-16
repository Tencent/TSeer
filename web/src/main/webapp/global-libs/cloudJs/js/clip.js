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
 * 复制到粘贴板组件
 */

cloudjs.define({
    clip: function(options){
        if($.isPlainObject(options)){
            var defaults = {
                swfPath: 'http://3gimg.qq.com/mig_op/cloudJs/js/Clipboard.swf',   //falsh文件路径
                hoverClass: 'hover',    //鼠标经过的className
                activeClass: 'active',  //鼠标点击时的className
                zIndex: 99,     //flash文件的zIndex
                onCopy: $.noop, //函数，要复制的内容，通过return返回
                beforeCopy: $.noop, //函数，复制前要进行的操作
                afterCopy: $.noop //函数，复制后要进行的操作
            };
            $.extend(defaults, options);
            
            return this.each(function(){
                var self = $(this);
                if (self.is(':visible')){
                    ZeroClipboard.setDefaultValue(defaults);
                    var clip = new ZeroClipboard.Client();
    
                    self.bind('cloudJs_clip_copy', defaults.onCopy);
                    self.bind('cloudJs_clip_beforeCopy', defaults.beforeCopy);
                    self.bind('cloudJs_clip_afterCopy', defaults.afterCopy);
        
                    clip.setHandCursor(true);
                    clip.setCSSEffects(true);
                    clip.addEventListener('mouseOver', function(){
                        self.trigger('mouseenter');
                    });
                    clip.addEventListener('mouseOut', function(){
                        self.trigger('mouseleave');
                    });
                    clip.addEventListener('mouseDown', function(){
                        clip.setText(self.triggerHandler('cloudJs_clip_copy'));                        
                        self.trigger('cloudJs_clip_beforeCopy');
                    });
                    clip.addEventListener('complete', function(client, text){
                        self.trigger('cloudJs_clip_afterCopy');
                    });
                    clip.glue(self[0], self.parent()[0]);
                    $(window).bind('load resize',function(){
                        clip.reposition();
                    });
                }
            });
        }else{
            return this.each(function(){
                var self = $(this);
                var clipId = self.data('clipId');
                var clipElm = $('#' + clipId + '.clip_div');
                if(options === 'destroy'){
                    self.remove();
                    clipElm.remove();
                }else if(options === 'hide'){
                    self.hide();
                    clipElm.hide();
                }else if(options === 'show'){
                    self.show();
                    clipElm.show();
                }
            });
        }

        return this;
    }   
});

var ZeroClipboard = {
    clients: {},
    index: 1,
    $: function(thingy){
        if(typeof(thingy) === 'string') thingy = document.getElementById(thingy);
        if(!thingy.addClass){
            thingy.hide = function(){
                this.style.display = 'none';
            };
            thingy.show = function(){
                this.style.display = '';
            };
            thingy.addClass = function(name){
                this.removeClass(name);
                this.className += ' ' + name;
            };
            thingy.removeClass = function(name){
                var classes = this.className.split(/\s+/);
                var idx = -1;
                for(var k = 0; k < classes.length; k++){
                    if(classes[k] === name){
                        idx = k;
                        k = classes.length;
                    }
                }
                if(idx > -1){
                    classes.splice(idx, 1);
                    this.className = classes.join(' ');
                }
                return this;
            };
            thingy.hasClass = function(name){
                return !!this.className.match(new RegExp('\\s*' + name + '\\s*'));
            };
        }
        return thingy;
    },
    setDefaultValue: function(opts){
        this.swfPath = opts.swfPath;
        this.hoverClass = opts.hoverClass;
        this.activeClass = opts.activeClass;
        this.zIndex = opts.zIndex;
    },
    dispatch: function(id, eventName, args){   
        var client = this.clients[id];
        if(client){
            client.receiveEvent(eventName, args);
        }
    },
    register: function(id, client){
        this.clients[id] = client;
    },
    getDOMObjectPosition: function(obj, stopObj){
        var info = {
            left: 0,
            top: 0,
            width: obj.width ? obj.width : obj.offsetWidth,
            height: obj.height ? obj.height : obj.offsetHeight
        };

        if(obj && (obj != stopObj)){
            info.left += obj.offsetLeft;
            info.top += (obj.offsetTop - 5);
        }
        return info;
    },
    Client: function(ele){
        this.handlers = {};
        this.id = ZeroClipboard.index++;
        this.movieId = 'ClipboardSwf_' + this.id;
        ZeroClipboard.register(this.id, this);
        if(ele) this.glue(ele);
    }
};

ZeroClipboard.Client.prototype = {
    id: 0,
    ready: false,
    movie: null,
    clipText: '',
    handCursorEnabled: true,
    cssEffects: true,
    handlers: null,
    glue: function(ele, appendElem, stylesToAdd){
        this.domElement = ZeroClipboard.$(ele);
        var zIndex = ZeroClipboard.zIndex;
        if(this.domElement.style.zIndex){
            zIndex = parseInt(this.domElement.style.zIndex, 10) + 1;
        }
        if(typeof(appendElem) === 'string'){
            appendElem = ZeroClipboard.$(appendElem);
        }else if(typeof(appendElem) === 'undefined'){
            appendElem = document.getElementsByTagName('body')[0];
        }

        var box = ZeroClipboard.getDOMObjectPosition(this.domElement, appendElem);
        this.div = document.createElement('div');
        this.div.className = 'clip_div';
        this.div.id = 'clip_' + this.movieId;
        $(this.domElement).data('clipId', 'clip_' + this.movieId);
        var style = this.div.style;
        style.position = 'absolute';
        style.left = box.left + 'px';
        style.top = box.top + 'px';
        style.width = box.width + 'px';
        style.height = box.height + 'px';
        style.zIndex = zIndex;

        if(typeof(stylesToAdd) === 'object'){
            for(var addedStyle in stylesToAdd){
                style[addedStyle] = stylesToAdd[addedStyle];
            }
        }
        appendElem.appendChild(this.div);
        this.div.innerHTML = this.getHTML(box.width, box.height);
    },
    getHTML: function(width, height){
        var html = '', flashvars = 'id=' + this.id + '&width=' + width + '&height=' + height;
        if(navigator.userAgent.match(/MSIE/)){
            var protocol = location.href.match(/^https/i) ? 'https://' : 'http://';
            html += '<object classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="' + protocol + 'download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,0,0" width="' + width + '" height="' + height + '" id="' + this.movieId + '" align="middle"><param name="allowScriptAccess" value="always" /><param name="allowFullScreen" value="false" /><param name="movie" value="' + ZeroClipboard.swfPath + '" /><param name="loop" value="false" /><param name="menu" value="false" /><param name="quality" value="best" /><param name="bgcolor" value="#ffffff" /><param name="flashvars" value="' + flashvars + '"/><param name="wmode" value="transparent"/></object>';
        }else {
            html += '<embed id="' + this.movieId + '" src="' + ZeroClipboard.swfPath + '" loop="false" menu="false" quality="best" bgcolor="#ffffff" width="' + width + '" height="' + height + '" name="' + this.movieId + '" align="middle" allowScriptAccess="always" allowFullScreen="false" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" flashvars="' + flashvars + '" wmode="transparent" />';
        }
        return html;
    },
    hide: function(){
        if(this.div) this.div.style.left = '-2000px';
    },
    show: function(){
        this.reposition();
    },
    reposition: function(ele){
        if(ele){
            this.domElement = ZeroClipboard.$(ele);
            if(!this.domElement) this.hide();
        }
        if(this.domElement && this.div){
            var box = ZeroClipboard.getDOMObjectPosition(this.domElement);
            var style = this.div.style;
            style.left = box.left + 'px';
            style.top = box.top + 'px';
        }
    },
    setText: function(newText){
        this.clipText = newText;
        if(this.ready) this.movie.setText(newText);
    },
    addEventListener: function(eventName, func){
        eventName = eventName.toString().toLowerCase().replace(/^on/, '');
        if(!this.handlers[eventName]) this.handlers[eventName] = [];
        this.handlers[eventName].push(func);
    },
    setHandCursor: function(enabled){
        this.handCursorEnabled = enabled;
        if(this.ready) this.movie.setHandCursor(enabled);
    },
    setCSSEffects: function(enabled){
        this.cssEffects = !! enabled;
    },
    receiveEvent: function(eventName, args){
        eventName = eventName.toString().toLowerCase().replace(/^on/, '');
        switch (eventName){
        case 'load':
            this.movie = document.getElementById(this.movieId);
            var self = this;
            if(!this.movie){
                setTimeout(function(){
                    self.receiveEvent('load', null);
                }, 1);
                return;
            }

            if(!this.ready && navigator.userAgent.match(/Firefox/) && navigator.userAgent.match(/Windows/)){
                setTimeout(function(){
                    self.receiveEvent('load', null);
                }, 100);
                this.ready = true;
                return;
            }

            this.ready = true;
            try{ this.movie.setText(this.clipText); }catch(e){}
            try{ this.movie.setHandCursor(this.handCursorEnabled); }catch(e){}
            break;

        case 'mouseover':
            if(this.domElement && this.cssEffects){
                this.domElement.addClass(ZeroClipboard.hoverClass);
                if(this.recoverActive){
                    this.domElement.addClass(ZeroClipboard.activeClass);
                }
            }
            break;

        case 'mouseout':
            if(this.domElement && this.cssEffects){
                this.recoverActive = false;
                if(this.domElement.hasClass(ZeroClipboard.activeClass)){
                    this.domElement.removeClass(ZeroClipboard.activeClass);
                    this.recoverActive = true;
                }
                this.domElement.removeClass(ZeroClipboard.hoverClass);
            }
            break;

        case 'mousedown':
            if(this.domElement && this.cssEffects){
                this.domElement.addClass(ZeroClipboard.activeClass);
            }
            break;

        case 'mouseup':
            if(this.domElement && this.cssEffects){
                this.domElement.removeClass(ZeroClipboard.activeClass);
                this.recoverActive = false;
            }
            break;
        }
        
        if(this.handlers[eventName]){
            for(var idx = 0, len = this.handlers[eventName].length; idx < len; idx++){
                var func = this.handlers[eventName][idx];
                if(typeof(func) === 'function'){
                    func(this, args);
                }else if((typeof(func) === 'object') && (func.length === 2)){
                    func[0][func[1]](this, args);
                }else if(typeof(func) === 'string'){
                    window[func](this, args);
                }
            }
        }
    }
};  
