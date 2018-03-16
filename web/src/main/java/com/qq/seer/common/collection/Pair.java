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

package com.qq.seer.common.collection;

import java.io.Serializable;

public class Pair<A, B> implements Serializable {
	


	private static final long serialVersionUID = 1L;

	public final A first;

	public final B second;

	private Pair(A first, B second) {
		this.first = first;
		this.second = second;
	}

	/**
	 * 用于生成新对象的工厂静态方法
	 * @param first A类的对象
	 * @param second B类的对象
	 * @return 返回Pair<A, B>的对象
	 */
	public static <A, B> Pair<A, B> of(A first, B second) {
		return new Pair<A, B>(first, second);
	}

	@SuppressWarnings("unchecked")
	@Override
	public boolean equals(Object obj) {
		if (obj == null) {
			return false;
		}
		if (getClass() != obj.getClass()) {
			return false;
		}
		final Pair other = (Pair) obj;
		if (this.first != other.first && (this.first == null || !this.first.equals(other.first))) {
			return false;
		}
		return !(this.second != other.second && (this.second == null || !this.second.equals(other.second)));
	}

	@Override
	public int hashCode() {
		int hash = 7;
		hash = 37 * hash + (this.first != null ? this.first.hashCode() : 0);
		hash = 37 * hash + (this.second != null ? this.second.hashCode() : 0);
		return hash;
	}

	@Override
	public String toString() {
		return String.format("Pair[%s,%s]", first, second);
	}

}
