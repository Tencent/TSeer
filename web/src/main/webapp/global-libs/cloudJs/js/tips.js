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
 * 提示组件
 */
cloudjs.define({
	tips: function(options){
		var defaults = {
			track: false, // 当鼠标在元素内移动时，tips是否跟随着鼠标移动，默认是false不随着移动
			title: '', // tips的内容，一般情况下是string，也可以是function，当是function时支持回传一个title参数
			position: '', // 表示tips出现的位置，可选的有up、down、right、left，分别表示上方，下方，右方和左方
			hover: false, // 是否支持鼠标移动到tips上时，tips不消失。 false表示不支持失，true表示支持
			hideOnFocus: true, // 鼠标单击目标元素时，提示信息是否消失（此时鼠标还在目标元素区域内）
			isContainer: false, //是否是容器
			width: '',
			height: '',
			maxWidth: '',
			loading: 'loading', //title为回调函数时显示加载的文本内容
			showDelay: 200, // 鼠标移动到目标元素后tips多长时间内出现，默认是200表示延迟200ms出现
			hideDelay: 300 //当鼠标离开目标元素后tips多长时间后消失，默认是300表示300ms后消失
		};

		var _self = this,
			ARROW_WIDTH = 5,
			ARROW_DOWN = 'down',
			ARROW_LEFT = 'left',
			ARROW_RIGHT = 'right',
			ARROW_UP = 'up',
			ARROW_DOWN_LEFT = 'down_left', //下方偏左
			ARROW_DOWN_RIGHT = 'down_right', //下方偏右
			ARROW_UP_LEFT = 'up_left', //上方偏左
			ARROW_UP_RIGHT = 'up_right', //上方偏右
			LEFT_EDG = 14,
			RIGHT_EDG = 10,
			TITLE = 'title',
			INIT_TITLE = 'init-title',
			OWN_TITLE = 'own-title',
			ACTIVE_ELE = 'tips_active',
			OBJ_ID = 'data-tips-obj-id',
			SPACING = 10, //track为false时箭头和元素的距离
			X_SPACING = 35, //track为true时tips距离鼠标的x轴距离
			Y_SPACING = 20, //track为true时tips距离鼠标的y轴距离
			_args0,
			_args1,
			_objsMap,
			pagePosition;

		_objsMap = cloudjs.tips._objsMap = cloudjs.tips._objsMap || {};

		if(!options || $.isPlainObject(options)){
			$.extend(true, defaults, options);
			_resolveEle(TITLE);
			_init();
		}else{
			_args0 = arguments[0];
			_args1 = arguments[1] || {};
			$.extend(true, defaults, _args1);
			if(_args0 === 'setData'){
				_resolveEle(TITLE);
				_update(_args1.change, true, !!_args1.forceShow);
			}else if(_args0 === 'destroy'){
				_resolveEle(OBJ_ID);
				_destroyTips(_self);
			}
		}

		cloudjs.callback.call();

		/**
		 * 更新title
		 * @param {Boolean} change 是否改变元素的title
		 * @param {Boolean} isAction 是否是方法
		 */
		function _update(change, isAction, forceShow){
			$.each(_self, function(i, ele, tipsObj, uuid){
				ele = $(ele);
				uuid = ele.attr(OBJ_ID);
				if(uuid){
					tipsObj = _objsMap[uuid];
					if(tipsObj.tips){
						tipsObj.update(defaults.title, change, isAction, forceShow);
					}else{
						ele.attr('title', defaults.title);
					}
				}
			});
		}

		/**
		 * 初始化
		 */
		function _init(){
			$.each(_self, function(i, ele, tipsObj, uuid){
				ele = $(ele);
				if(ele.attr(OBJ_ID)){
					_destroyTips(ele);
				}
				uuid = '_' + cloudjs.uniq();
				tipsObj = {
					ele: ele,
					init: function(title){
						_initTips(this);
						_setWidthAndHeight(this);
						this.update(title, true, false, true);
					},
					update: function(title, change, isAction, forceShow){
						if(change){
							this.ele.data(INIT_TITLE, title);
						}
						if(this.tips){
							_setTipsTitle(this, title);
							_calculatePosition(this);
							_resetTipsPosition(this);
							if(forceShow){
								_showTips(this, isAction);
							}
							_calculateOffset(this, false);
						}
					}
				};

				_objsMap[uuid] = tipsObj;
				ele.attr(OBJ_ID, uuid);

				ele.bind('mouseenter.' + ele.attr(OBJ_ID), function(){
					_hideOtherTips(tipsObj);
					ele.addClass(ACTIVE_ELE);
					clearTimeout(cloudjs.tips._timeout1);
					clearTimeout(cloudjs.tips._timeout2);
					if(_initTitle(tipsObj)){// 如果title存在，才会显示tips
						if(defaults.track){
							_startMoveTips(tipsObj);
						}
						tipsObj.tips.css('zIndex', defaults.zIndex||cloudjs.zIndex());
					}
					_setOwnTitle(tipsObj);
				}).bind('mouseleave.' + ele.attr(OBJ_ID), function(){
                    pagePosition = undefined;
					_reverTitle(tipsObj);
					clearTimeout(cloudjs.tips._timeout2);
//						clearTimeout(cloudjs.tips._timeout1);
					_hideTips(tipsObj);
					ele.removeClass(ACTIVE_ELE);
					_stopMoveTips(tipsObj);
				});
			});
		}

		function isPositionFixed(ele){
			var css_position = ele.css('position');
			while(ele && css_position != 'fixed' ){
				if(ele[0] == $('body')[0]){
					ele = null;
				}else{
					ele = ele.parent();
					css_position = ele.css('position');
				}
			}

			if(ele){
				return true;
			}
		}

		/**
		 * 设置元素宽高
		 * @param {Object} tipsObj tips对象
		 */
		function _setWidthAndHeight(tipsObj){
			var content = tipsObj.tips.find('.tips_content').first();
			if(defaults.width){
				content.width(defaults.width);
			}
			if(defaults.height){
				content.height(defaults.height);
			}
//			if(!defaults.width && !defaults.height){
////				content.css('maxWidth', '300px');
//			}
			if(defaults.maxWidth){
				content.css('maxWidth', defaults.maxWidth);
			}
		}

		/**
		 * 保存元素最原始的title
		 * @param {Object} tipsObj tips对象
		 */
		function _setOwnTitle(tipsObj){
			var ele = tipsObj.ele;
			ele.data(OWN_TITLE, ele.attr(TITLE));
			ele.attr(TITLE, '');
		}

		/**
		 * 还原元素最原始的title到元素
		 * @param {Object} tipsObj tips对象
		 */
		function _reverTitle(tipsObj){
			var ele = tipsObj.ele;
			ele.attr(TITLE, ele.data(OWN_TITLE));
		}

		/**
		 * 销毁tips
		 * @param {Object} eles 要销毁的对象
		 */
		function _destroyTips(eles){
			$.each(eles, function(i, ele, tipsObj, uuid){
				ele = $(ele);
				uuid = ele.attr(OBJ_ID);
				if(uuid){
					tipsObj = _objsMap[uuid];
					_reverTitle(tipsObj);
					tipsObj.tips && tipsObj.tips.remove();
					ele.unbind('mouseenter.' + ele.attr(OBJ_ID))
						.unbind('mouseleave.' + ele.attr(OBJ_ID))
						.unbind('mousemove.' + ele.attr(OBJ_ID));
					ele.data(INIT_TITLE, '');
					ele.removeAttr(OBJ_ID);
				}
			});
		}

		/**
		 * 解析元素，可能是一个container
		 * @param {String} attr 元素属性
		 */
		function _resolveEle(attr){
			var newSelf = $('');
			if(defaults.isContainer){
				$.each(_self.find('[' + attr + ']'), function(i, ele){
					if($(ele).attr(attr)){
						[].push.call(newSelf, ele);
					}
				});
				$.each(_self, function(i, ele){
					if($(ele).attr(attr)){
						[].push.call(newSelf, ele);
					}
				});
				_self = newSelf;
			}
		}

		/**
		 * 隐藏其他tips
		 * @param {Object} tipsObj tips对象
		 */
		function _hideOtherTips(tipsObj){
			$.each($('.tips_div'), function(i, tips){
				if(!tipsObj.tips || tipsObj.tips[0] !== tips){
					$(tips).hide();
				}
			});
		}

		/**
		 * 计算适合的方位
		 * @param {Object} tipsObj tips对象
		 */
		function _calculatePosition(tipsObj, point){
			var ele = tipsObj.ele,
				tips = tipsObj.tips,
				position = defaults.position,
				positions,
				scrollTop,
				screenWidth,
				screenHeight,
				eleLeft,
				eleTop,
				eleWidth,
				eleHeight,
				tipsWidth,
				tipsHeight,
				_position;

			if(position !== ARROW_DOWN
				&& position !== ARROW_UP
				&& position !== ARROW_RIGHT
				&& position !== ARROW_LEFT
				&& position !== ARROW_UP_LEFT
				&& position !== ARROW_UP_RIGHT
				&& position !== ARROW_DOWN_LEFT
				&& position !== ARROW_DOWN_RIGHT){
				position = undefined;
			}

			var scrollLeft = $(window).scrollLeft();
			scrollTop = $(window).scrollTop();
			screenWidth = $(window).width();
			screenHeight = $(window).height();
			eleLeft = ele.offset().left - scrollLeft;
			eleTop = ele.offset().top - scrollTop;
			eleWidth = ele.outerWidth();
			eleHeight = ele.outerHeight();
			var tipsWidthAndHeight = getTipsWidthAndHeight(tipsObj);
			tipsWidth = tipsWidthAndHeight.width;
			tipsHeight = tipsWidthAndHeight.height;

			if(point){
				point.x = point.x - scrollLeft;
				point.y = point.y - scrollTop;
			}

			positions = {};

			positions[ARROW_UP] = function(){
				return _uu() && _lr();
			};

			positions[ARROW_DOWN] = function(){
				return _dd() && _lr();
			};

			positions[ARROW_LEFT] = function(){
				return _ll() && _ud();
			};

			positions[ARROW_RIGHT] = function(){
				return _rr() && _ud();
			};

			positions[ARROW_UP_LEFT] = function(){
				return _uu() && _cl();
			};

			positions[ARROW_UP_RIGHT] = function(){
				return _uu() && _cr();
			};

			positions[ARROW_DOWN_LEFT] = function(){
				return _dd() && _cl();
			};

			positions[ARROW_DOWN_RIGHT] = function(){
				return _dd() && _cr();
			};

			function _cl(){
				if(point){
					return (point.x - tipsWidth + ARROW_WIDTH > 0);
				}else{
					return (eleLeft + eleWidth / 2 - tipsWidth + ARROW_WIDTH > 0);
				}
			}

			function _cr(){
				if(point){
					return (point.x + tipsWidth - ARROW_WIDTH < screenWidth);
				}else{
					return (eleLeft + eleWidth / 2 + tipsWidth - ARROW_WIDTH < screenWidth);
				}
			}

			function _uu(){
				if(point){
					return (point.y - tipsHeight - ARROW_WIDTH - SPACING > 0);
				}else{
					return (eleTop - tipsHeight - ARROW_WIDTH - SPACING > 0);
				}
			}

			function _dd(){
				if(point){
					return (point.y + tipsHeight + ARROW_WIDTH + SPACING < screenHeight);
				}else{
					return (eleTop + eleHeight + tipsHeight + ARROW_WIDTH + SPACING < screenHeight);
				}
			}

			function _ll(){
				if(point){
					return (point.x - tipsWidth - ARROW_WIDTH - SPACING - X_SPACING > 0);
				}else{
					return (eleLeft - tipsWidth - ARROW_WIDTH - SPACING > 0);
				}
			}

			function _rr(){
				if(point){
					return (point.x + tipsWidth + ARROW_WIDTH + SPACING + X_SPACING< screenWidth);
				}else{
					return (eleLeft + eleWidth + tipsWidth + ARROW_WIDTH + SPACING < screenWidth);
				}
			}

			function _lr(){
				if(point){
					return (point.x - tipsWidth / 2 > 0) && (point.x + tipsWidth / 2 < screenWidth);
				}else{
					return (eleLeft + eleWidth / 2 - tipsWidth / 2 > 0) && (eleLeft + eleWidth / 2 + tipsWidth / 2 < screenWidth);
				}
			}

			function _ud(){
				if(point){
					return (point.y - tipsHeight / 2 > 0) && (point.y + tipsHeight / 2 < screenHeight);
				}else{
					return (eleTop + eleHeight / 2 - tipsHeight / 2 > 0) && (eleTop + eleHeight / 2 + tipsHeight / 2 < screenHeight);
				}
			}

			if(position && positions[position]()){
				_position = position;
			}

			if(!_position){
				for(var p in positions){
					if(!_position && positions[p]()){
						_position = p;
					}
				}
			}

			tipsObj.position = _position || position || ARROW_DOWN;
		}

		function getTipsWidthAndHeight(tipsObj, isMove){
			if(isMove){
				tipsObj.tips.css({left:'0px', top:'0px'});
			}
			return {width:tipsObj.tips.outerWidth(), height:tipsObj.tips.outerHeight()};
		}

		/**
		 * 根据方位计算位置坐标
		 * @param {Object} tipsObj tips对象
		 * @return {Object} 返回位置坐标值
		 */
		function _getOffset(tipsObj){
			var tipsWidthAndHeight = getTipsWidthAndHeight(tipsObj);
			var offset = {},
				ele = tipsObj.ele,
				position = tipsObj.position,
				eleWidth = tipsObj.ele.outerWidth(),
				eleHeight = tipsObj.ele.outerHeight(),
				tipsWidth = tipsWidthAndHeight.width,
				tipsHeight = tipsWidthAndHeight.height;

			switch(position){
				case ARROW_LEFT:
					offset.left = ele.offset().left - tipsWidth - ARROW_WIDTH - SPACING;
					offset.top = ele.offset().top + eleHeight / 2 - tipsHeight / 2;
					break;
				case ARROW_RIGHT:
					offset.left = ele.offset().left + eleWidth + ARROW_WIDTH + SPACING;
					offset.top = ele.offset().top + eleHeight / 2 - tipsHeight / 2;
					break;
				case ARROW_UP:
					offset.left = ele.offset().left + eleWidth / 2 - tipsWidth / 2;
					offset.top = ele.offset().top - tipsHeight - ARROW_WIDTH - SPACING;
					break;
				case ARROW_DOWN:
					offset.left = ele.offset().left + eleWidth / 2 - tipsWidth / 2;
					offset.top = ele.offset().top + eleHeight + ARROW_WIDTH + SPACING;
					break;
				case ARROW_UP_LEFT:
					offset.left = ele.offset().left + eleWidth / 2 - tipsWidth + ARROW_WIDTH + LEFT_EDG;
					offset.top = ele.offset().top - tipsHeight - ARROW_WIDTH - SPACING;
					break;
				case ARROW_UP_RIGHT:
					offset.left = ele.offset().left + eleWidth / 2 - ARROW_WIDTH - RIGHT_EDG;
					offset.top = ele.offset().top - tipsHeight - ARROW_WIDTH - SPACING;
					break;
				case ARROW_DOWN_LEFT:
					offset.left = ele.offset().left + eleWidth / 2 - tipsWidth + ARROW_WIDTH + LEFT_EDG;
					offset.top = ele.offset().top + eleHeight + ARROW_WIDTH + SPACING;
					break;
				case ARROW_DOWN_RIGHT:
					offset.left = ele.offset().left + eleWidth / 2 - ARROW_WIDTH - RIGHT_EDG;
					offset.top = ele.offset().top + eleHeight + ARROW_WIDTH + SPACING;
					break;
				default:
				//不会到这里
			}

			var isFixed = isPositionFixed(ele);
			if(isFixed){
				var scrollTop = $(window).scrollTop();
				var scrollLeft = $(window).scrollLeft();
				offset.top = offset.top - scrollTop;
				offset.left = offset.left - scrollLeft;
				tipsObj.tips.css('position', 'fixed');
			}

			return offset;
		}

		/**
		 * 计算相对位置，设置tips跟随鼠标移动的时候掉会用到这个方法
		 * @param {Object} tipsObj tips对象
		 * @return {Object} 返回相对位置
		 */
		function _calculateOffset(tipsObj, isMove){
			var tipsWidthAndHeight = getTipsWidthAndHeight(tipsObj, isMove);
			var width = tipsWidthAndHeight.width,
				height = tipsWidthAndHeight.height,
				offsetX,
				offsetY;

			switch(tipsObj.position){
				case ARROW_UP:
					offsetX = -width / 2;
					offsetY = -(height + ARROW_WIDTH + Y_SPACING);
					break;
				case ARROW_DOWN:
					offsetX = -width / 2;
					offsetY = ARROW_WIDTH + Y_SPACING;
					break;
				case ARROW_RIGHT:
					offsetX = ARROW_WIDTH + X_SPACING;
					offsetY = -height / 2;
					break;
				case ARROW_LEFT:
					offsetX = -(width + ARROW_WIDTH + X_SPACING);
					offsetY = -height / 2;
					break;
				case ARROW_UP_LEFT:
					offsetX = -width + ARROW_WIDTH + LEFT_EDG;
					offsetY = -(height + ARROW_WIDTH + Y_SPACING);
					break;
				case ARROW_UP_RIGHT:
					offsetX = - ARROW_WIDTH - RIGHT_EDG;
					offsetY = -(height + ARROW_WIDTH + Y_SPACING);
					break;
				case ARROW_DOWN_LEFT:
					offsetX = -width + ARROW_WIDTH + LEFT_EDG;
					offsetY = ARROW_WIDTH + Y_SPACING;
					break;
				case ARROW_DOWN_RIGHT:
					offsetX = - ARROW_WIDTH - RIGHT_EDG;
					offsetY = ARROW_WIDTH + Y_SPACING;
					break;
				default:
					offsetX = 0;
					offsetY = 0;
			}

			tipsObj.offset = {
				x: offsetX,
				y: offsetY
			};
		}

		/**
		 * 绑定tips的事件
		 * @param {Object} tipsObj tips对象
		 */
		function _bindTipsEvent(tipsObj){
			if(tipsObj.tips && defaults.hover){
				tipsObj.tips.bind('mouseenter.' + tipsObj.ele.attr(OBJ_ID), function(){
					clearTimeout(cloudjs.tips._timeout1);
				}).bind('mouseleave.' + tipsObj.ele.attr(OBJ_ID), function(){
					_hideTips(tipsObj);
				});
			}
		}

		/**
		 * 初始化title
		 * @param {Object} tipsObj tips对象
		 * @return {Boolean} 返回是否初始化成功
		 */
		function _initTitle(tipsObj){
			var title;
			if(!($.trim(title = tipsObj.ele.data(INIT_TITLE)))){
				if($.isFunction(title = defaults.title)){// 回调的方式
					tipsObj.init(defaults.loading);
				}else if(title = defaults.title){// 设置了title属性
					tipsObj.init(title);
				}else if(title = tipsObj.ele.attr(TITLE)){// 采用元素默认的title
					tipsObj.init(title);
				}else {// 没有设置title，不显示
					return false;
				}
				tipsObj.eleTitle = title;
				_bindTipsEvent(tipsObj);
			}else {// 直接更新tips
				tipsObj.update(title, false, false, true);
			}

			if($.isFunction(title = defaults.title)){// 回调的方式
				title(tipsObj.ele, function(title){
					getTipsWidthAndHeight(tipsObj, true);
					_setTipsTitle(tipsObj, title);
					_calculatePosition(tipsObj, pagePosition);
					_resetTipsPosition(tipsObj);
				});
			}

			return true;
		}

		/**
		 * 绑定tips跟随鼠标移动的事件
		 * @param {Object} tipsObj tips对象
		 */
		function _startMoveTips(tipsObj){
			var tips = tipsObj.tips, ele = tipsObj.ele;
			ele.unbind('mousemove.' + ele.attr(OBJ_ID)).bind('mousemove.' + ele.attr(OBJ_ID), function(e){
				pagePosition = {x:e.pageX, y:e.pageY};
				_calculatePosition(tipsObj, pagePosition);
				_resetTipsPosition(tipsObj);
				_calculateOffset(tipsObj, true);
				tips.css({
					left: e.pageX + tipsObj.offset.x + 'px',
					top: e.pageY + tipsObj.offset.y + 'px'
				});
			});
		}

		/**
		 * 停止监听mousemove事件
		 * @param {Object} tipsObj tips对象
		 */
		function _stopMoveTips(tipsObj){
			tipsObj.ele.unbind('mousemove.' + tipsObj.ele.attr(OBJ_ID));
		}

		/**
		 * 当文本变化时重新设置出现的位置
		 * @param {Object} tipsObj tips对象
		 */
		function _resetTipsPosition(tipsObj){
			var offset = _getOffset(tipsObj);
			tipsObj.tips.find('.tips_arrow').removeClass().addClass('tips_arrow tips_arrow_' + tipsObj.position);
			tipsObj.tips.css({
				left: offset.left + 'px',
				top: offset.top + 'px'
			});
		}

		/**
		 * 初始化tips
		 * @param {Object} tipsObj tips对象
		 */
		function _initTips(tipsObj){
			var tips;
			if(!tipsObj.tips){
				tips = '';
				tips += '<div class="tips_div">';
				tips +=	'<span class="tips_content"></span>';
				tips += '<span class="tips_arrow"><i></i></span>';
				tips += '</div>';
				tips = $(tips).appendTo('body').hide();

				tipsObj.tips = tips;
			}
		}

		/**
		 * 设置tips的内容
		 * @param {Object} tipsObj tips对象
		 * @param {String} title tips的内容
		 */
		function _setTipsTitle(tipsObj, title){
			tipsObj.tips.find('.tips_content').html(title);
		}

		/**
		 * 显示tips
		 * @param {Object} tipsObj tips对象
		 * @param {Boolean} isAction 是否是方法
		 */
		function _showTips(tipsObj, isAction){
			if(!tipsObj.tips.is(':visible')){
				if(isAction){
					tipsObj.tips.show();
					_hideTips(tipsObj);
				}else{
					cloudjs.tips._timeout2 = setTimeout(function(){
						tipsObj.tips.show();
					}, defaults.showDelay || 0);
				}
			}
		}

		/**
		 * 隐藏tips
		 * @param {Object} tipsObj tips对象
		 * @param {Number} hideTime 隔多长时间隐藏
		 */
		function _hideTips(tipsObj, hideTime){
			cloudjs.tips._timeout1 = setTimeout(function(){
				tipsObj.tips && tipsObj.tips.hide();
			}, hideTime || defaults.hideDelay);
		}

		$('body').mousedown(function(e){
			var ta = $(e.target);
			if(!ta.hasClass('tips_div') && !ta.closest('.tips_div').length){//  假如点击的是tips，忽略，因为此时支持拷贝了
				if(((!ta.hasClass(ACTIVE_ELE) && !ta.closest('.' + ACTIVE_ELE).length) || defaults.hideOnFocus) && !_args1){
					clearTimeout(cloudjs.tips._timeout2);
					$.each(_self, function(i, ele, tipsObj){
						tipsObj = _objsMap[$(ele).attr(OBJ_ID)];
						tipsObj && tipsObj.tips && tipsObj.tips.hide();
					});
				}
			}
		});
	}
});