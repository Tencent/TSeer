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

package com.qq.seer.common.string;

import java.util.*;

public class StringUtils {

    public static final String EMPTY = "";
    
    /**
     * Represents a failed index search.
     * @since 2.1
     */
    public static final int INDEX_NOT_FOUND = -1;

    /**
     * 若为null，则返回空串
     *
     * @param s
     * @return
     */
    public static String toString(String s) {
        if (null == s) {
            return "";
        }

        return s;
    }

    /**
     * 若为空串或者null，则返回指定默认值
     *
     * @param s
     * @param defaultValue
     * @return
     */
    public static String toString(String s, String defaultValue) {
        if (nullOrEmpty(s)) {
            return defaultValue;
        }

        return s;
    }

    /**
     * 去掉前导空白和尾部空白，包含\t,\n,\r\n等的过滤，若输入为null，则返回""<br/>
     * 该方法在beta35之后增加对全角空格的支持
     *
     * @param s
     * @return
     */
    public static String trimEx(String s) {
        if (null == s || s.length() < 1) {
            return "";
        }
        s = s.trim();

        char[] charArray = s.toCharArray();
        for (int i = 0; i < charArray.length; ++i) {
            if (charArray[i] == 12288) {
                s = s.substring(1);
            } else {
                break;
            }
        }

        charArray = s.toCharArray();
        for (int i = charArray.length - 1; i >= 0; --i) {
            if (charArray[i] == 12288) {
                s = s.substring(0, s.length() - 1);
            } else {
                break;
            }
        }

        return s.trim();
    }


    /**
     * 判断输入参数是否有效，以下输入均视为无效：
     * null、""、"null"（忽略大小写）、"undefined"（忽略大小写）
     *
     * @param input 入参
     * @return 有效则返回true，否则返回false
     */
    public static boolean isInputValid(String input) {
        return !(null == input || input.length() <= 0 || "null".equalsIgnoreCase(input) || "undefined".equalsIgnoreCase(input));
    }

    /**
     * 判断字符串是否为null、空字符串，只包含空格的空白字符串。该方法内部逻辑在beta35之后进行改进，对全角空格以及\t\n\x0B\f\r进行了过滤。
     *
     * @param s 要判断的字符串
     * @return 如果字符串为null,"",只包含空格或\t的空白字符串则返回true.
     */
    public static boolean nullOrEmpty(String s) {
        if (null == s) {
            return true;
        }

        return s.replace((char) 12288, ' ').replaceAll("\\t", "").replace(" ", "").length() < 1;
    }

    /**
     * 将字符串参数作为有符号的十进制整数进行解析,返回一个整数,出现异常则返回0
     *
     * @param s 包含要解析的 int 表示形式的 String
     * @return 返回用十进制参数表示的整数值, 出现异常则返回0
     * @deprecated beta35之后使用toInt
     */
    public static int parseInt(String s) {
        int i = 0;
        try {
            i = Integer.parseInt(s);
        } catch (NumberFormatException e) {

        }
        return i;
    }

    /**
     * 将字符串参数作为有符号的十进制整数进行解析,返回一个整数,出现异常则返回默认指定的值
     *
     * @param s            包含要解析的 int 表示形式的 String
     * @param defaultValue 解析出现异常默认返回的值
     * @return 返回用十进制参数表示的整数值, 出现异常则返回默认指定的值
     * @deprecated beta35之后使用toInt
     */
    public static int parseInt(String s, int defaultValue) {
        try {
            defaultValue = Integer.parseInt(s);
        } catch (NumberFormatException e) {

        }
        return defaultValue;
    }

    /**
     * 将字符串参数作为有符号的十进制整数进行解析,返回一个整数
     *
     * @param s 包含要解析的 int 表示形式的 String
     * @return 返回用十进制参数表示的整数值, 如果 String不包含可解析的int,抛出NumberFormatException
     * @deprecated beta35之后使用toInt
     */
    public static int parseIntWithException(String s) throws NumberFormatException {
        int i = 0;
        try {
            i = Integer.parseInt(s);
        } catch (Exception e) {
            throw new NumberFormatException();
        }
        return i;
    }

    /**
     * 返回一个新的 double值,该值被初始化为用指定 String表示的值.出现异常则返回0.0
     *
     * @param s 要解析的字符串
     * @return 由字符串参数表示的double值, 出现异常则返回0.0
     * @deprecated beta35之后使用toDouble
     */
    public static double parseDouble(String s) {
        double d = 0.0;
        try {
            d = Double.parseDouble(s);
        } catch (NumberFormatException e) {

        }
        return d;
    }

    /**
     * 返回一个新的 double值,该值被初始化为用指定 String表示的值.出现异常则返回默认指定的值
     *
     * @param s            要解析的字符串
     * @param defaultValue 解析出现异常默认返回的值
     * @return 由字符串参数表示的double值, 出现异常则返回默认指定的值
     * @deprecated beta35之后使用toDouble
     */
    public static double parseDouble(String s, double defaultValue) {
        try {
            defaultValue = Double.parseDouble(s);
        } catch (NumberFormatException e) {

        }
        return defaultValue;
    }

    /**
     * 返回一个新的 double值,该值被初始化为用指定 String表示的值
     *
     * @param s 要解析的字符串
     * @return 由字符串参数表示的double值, 如果字符串不包含可解析的double值, 抛出NumberFormatException
     * @deprecated beta35之后使用toDouble
     */
    public static double parseDoubleWithException(String s) {
        double d = 0.0;
        try {
            d = Double.parseDouble(s);
        } catch (Exception e) {
            throw new NumberFormatException();
        }
        return d;
    }

    /**
     * 判断字符串是否为空
     *
     * @param s 字符串
     * @return 为空返回true，否则返回false
     */
    public static boolean isEmpty(String s) {
        return s == null || s.length() == 0;
    }

    /**
     * 判断字符串序列是否全为空
     *
     * @param s 字符串序列
     * @return 全为空返回true，否则返回false
     */
    public static boolean isEmpty(String... s) {
        for (String p : s) {
            if (!isEmpty(p)) {
                return false;
            }
        }
        return true;
    }

    /**
     * 判断字符串是否为空
     *
     * @param s 字符串
     * @return 为空返回true，否则返回false
     */
    public static boolean isBlank(String s) {
		int strLen;
		if (s == null || (strLen = s.length()) == 0) {
			return true;
		}
		for (int i = 0; i < strLen; i++) {
			if (!Character.isWhitespace(s.charAt(i))) {
				return false;
			}
		}
		return true;
    }

    /**
     * 判断字符串序列是否全为空
     *
     * @param s 字符串序列
     * @return 全为空返回true，否则返回false
     */
    public static boolean isBlank(String... s) {
        for (String p : s) {
            if (!isBlank(p)) {
                return false;
            }
        }
        return true;
    }

    /**
     * 判断字符串是否不为空
     *
     * @param s 字符串
     * @return 不为空返回true，否则返回false
     */
    public static boolean isNotBlank(String s) {
		int strLen;
		if (s == null || (strLen = s.length()) == 0) {
			return false;
		}
		for (int i = 0; i < strLen; i++) {
			if (!Character.isWhitespace(s.charAt(i))) {
				return true;
			}
		}
		return false;
    }

    /**
     * 判断字符串序列是否全不为空
     *
     * @param s 字符串序列
     * @return 全不为空返回true，否则返回false
     */
    public static boolean isNotBlank(String... s) {
        for (String p : s) {
            if (!isNotBlank(p)) {
                return false;
            }
        }
        return true;
    }

    public static String trim(String s) {
    	return s == null ? s : s.trim();
    }

    public static String trimToEmpty(String str) {
    	return str == null ? EMPTY : str.trim();
    }

    public static String trimToNull(String str) {
        if (str == null) {
            return null;
        }
        str = strip(str, null);
        return str.isEmpty() ? null : str;
    }
    public static String strip(String str, final String stripChars) {
        if (isEmpty(str)) {
            return str;
        }
        str = stripStart(str, stripChars);
        return stripEnd(str, stripChars);
    }
    public static String stripEnd(final String str, final String stripChars) {
        int end;
        if (str == null || (end = str.length()) == 0) {
            return str;
        }

        if (stripChars == null) {
            while (end != 0 && Character.isWhitespace(str.charAt(end - 1))) {
                end--;
            }
        } else if (stripChars.isEmpty()) {
            return str;
        } else {
            while (end != 0 && stripChars.indexOf(str.charAt(end - 1)) != INDEX_NOT_FOUND) {
                end--;
            }
        }
        return str.substring(0, end);
    }
    public static String stripStart(final String str, final String stripChars) {
        int strLen;
        if (str == null || (strLen = str.length()) == 0) {
            return str;
        }
        int start = 0;
        if (stripChars == null) {
            while (start != strLen && Character.isWhitespace(str.charAt(start))) {
                start++;
            }
        } else if (stripChars.isEmpty()) {
            return str;
        } else {
            while (start != strLen && stripChars.indexOf(str.charAt(start)) != INDEX_NOT_FOUND) {
                start++;
            }
        }
        return str.substring(start);
    }
    /**
     * SQL编码，防注入
     *
     * @param sql 字段值
     * @return 编码结果
     */
    public static String encodeSQL(String sql) {
        if (null == sql) {
            return EMPTY;
        }

        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < sql.length(); ++i) {
            char c = sql.charAt(i);
            switch (c) {
                case '\\':
                    sb.append("\\\\");
                    break;
                case '\r':
                    sb.append("\\r");
                    break;
                case '\n':
                    sb.append("\\n");
                    break;
                case '\t':
                    sb.append("\\t");
                    break;
                case '\b':
                    sb.append("\\b");
                    break;
                case '\'':
                    sb.append("\'\'");
                    break;
                case '\"':
                    sb.append("\\\"");
                    break;
                default:
                    sb.append(c);
            }
        }
        return sb.toString();
    }

    /**
     * 分割字符串
     *
     * @param s     待分隔字符串
     * @param split 分隔字符串
     * @return 分割之后的字符串集合, 忽略空字符串，如果参数s为null则返回空集合
     */
    public static Set<String> splitToSet(String s, String split) {
        return splitToSet(s, split, true);

    }

    /**
     * 分割字符串
     *
     * @param s           待分隔字符串
     * @param split       分隔字符串
     * @param ignoreEmpty 是否忽略空字符串
     * @return 分割之后的字符串集合，如果参数s为null则返回空集合
     */
    public static Set<String> splitToSet(String s, String split, boolean ignoreEmpty) {
        if (null == s) {
            return new HashSet<String>();
        }
        if (null == split || split.length() == 0) {
            throw new IllegalArgumentException("split is null or empty");
        }

        Set<String> rtnValue = new HashSet<String>();
        String[] tokens = s.split(split);
        for (String token : tokens) {
            token = trimToEmpty(token);
            if (ignoreEmpty && isEmpty(token)) {
                continue;
            }
            rtnValue.add(token);
        }
        return rtnValue;
    }


    /**
     * 分割字符串
     *
     * @param s           待分隔字符串
     * @param split       分隔字符串
     * @param ignoreEmpty 是否忽略空字符串
     * @return 分割之后的字符串列表，如果参数s为null则返回空列表
     */
    public static List<String> splitToList(String s, String split, boolean ignoreEmpty) {
        if (null == s) {
            return new LinkedList<String>();
        }
        if (null == split || split.length() == 0) {
            throw new IllegalArgumentException("split is null or empty");
        }

        List<String> rtnValue = new LinkedList<String>();
        String[] tokens = s.split(split);
        for (String token : tokens) {
            token = trimToEmpty(token);
            if (ignoreEmpty && isEmpty(token)) {
                continue;
            }
            rtnValue.add(token);
        }
        return rtnValue;
    }

    /**
     * 分割字符串
     *
     * @param s     待分隔字符串
     * @param split 分隔字符串
     * @return 分割之后的字符串列表，忽略空字符串，如果参数s为null则返回空列表
     */
    public static String[] splitToArray(String s, String split) {
        return splitToArray(s, split, true);
    }

    /**
     * 分割字符串
     *
     * @param s           待分隔字符串
     * @param split       分隔字符串
     * @param ignoreEmpty 是否忽略空字符串
     * @return 分割之后的字符串列表，如果参数s为null则返回空列表
     */
    public static String[] splitToArray(String s, String split, boolean ignoreEmpty) {
        List<String> result = splitToList(s, split, ignoreEmpty);
        return result.toArray(new String[result.size()]);
    }

    /**
     * 拼接字符串
     *
     * @param data 需要拼接的数据
     * @param join 拼接字符
     * @return 拼接出的字符串，忽略null和空字符串, 如果参数data为null则返回空字符串<br/>
     * 当data中的元素类型为String、StringBuilder、StringBuffer、CharSequence时认为是字符串
     */
    public static <T> String join(Collection<T> data, String join) {
        return join(data, join, true);
    }

    /**
     * 拼接字符串
     *
     * @param data        需要拼接的数据
     * @param join        拼接字符
     * @param ignoreBlank 是否忽略null和空字符串<br/>
     *                    当data中的元素类型为String、StringBuilder、StringBuffer、CharSequence时认为是字符串
     * @return 拼接出的字符串，如果参数data为null则返回空字符串
     */
    public static <T> String join(Collection<T> data, String join, boolean ignoreBlank) {
        if (null == data) {
            return "";
        }
        if (null == join) {
            throw new IllegalArgumentException("join is null");
        }

        StringBuilder sb = new StringBuilder();
        for (T t : data) {
            if (ignoreBlank) {
                if (null == t
                        || (t instanceof String && ((String) t).length() == 0)
                        || (t instanceof StringBuilder && ((StringBuilder) t).length() == 0)
                        || (t instanceof StringBuffer && ((StringBuffer) t).length() == 0)
                        || (t instanceof CharSequence && ((CharSequence) t).length() == 0)) {
                    continue;
                }
            }
            sb.append(t).append(join);
        }
        deleteLast(sb, join);
        return sb.toString();
    }

    /**
     * 拼接字符串
     *
     * @param data 需要拼接的数据
     * @param join 拼接字符
     * @return 拼接出的字符串，忽略null和空字符串，如果参数data为null则返回空字符串
     */
    public static String join(String[] data, String join) {
        return join(data, join, true);
    }

    /**
     * 拼接字符串
     *
     * @param data        需要拼接的数据
     * @param join        拼接字符
     * @param ignoreBlank 是否忽略null和空字符串
     * @return 拼接出的字符串，如果参数data为null则返回空字符串
     */
    public static String join(String[] data, String join, boolean ignoreBlank) {
        if (null == data) {
            return "";
        }
        if (null == join) {
            throw new IllegalArgumentException("join is null");
        }

        StringBuilder sb = new StringBuilder();
        for (String s : data) {
            if (ignoreBlank && isBlank(s)) {
                continue;
            }
            sb.append(s).append(join);
        }
        deleteLast(sb, join);
        return sb.toString();
    }

    /**
     * 拼接字符串
     *
     * @param join 拼接字符
     * @param s    需要拼接的数据
     * @return 拼接出的字符串，忽略null和空字符串，如果参数data为null则返回空字符串
     */
    public static String join(String join, String... s) {
        return join(s, join, true);
    }

    /**
     * 拼接字符串
     *
     * @param join        拼接字符
     * @param s           需要拼接的数据
     * @param ignoreBlank 是否忽略null和空字符串
     * @return 拼接出的字符串，如果参数data为null则返回空字符串
     */
    public static String join(String join, boolean ignoreBlank, String... s) {
        return join(s, join, ignoreBlank);
    }


    /**
     * 删除字符串后缀，非贪心模式
     *
     * @param sb     字符串
     * @param suffix 后缀
     * @return 删除后缀后的结果
     */
    public static StringBuilder deleteLast(StringBuilder sb, String suffix) {
        if (null == sb) {
            throw new IllegalArgumentException("sb is null");
        }
        if (null == suffix) {
            throw new IllegalArgumentException("suffix is null");
        }
        if (sb.length() < suffix.length()) {
            return sb;
        }
        int sbLength = sb.length();
        int suffixLength = suffix.length();
        for (int i = 1; i <= suffixLength; i++) {
            if (sb.charAt(sbLength - i) != suffix.charAt(suffixLength - i)) {
                return sb;
            }
        }
        return sb.delete(sbLength - suffixLength, sbLength);
    }


    /**
     * 判断字符串中是否包含子串
     *
     * @param s     字符串
     * @param split 分隔符
     * @param find  字串
     * @return 是否包含子串，如果find为null则返回false
     */
    public static boolean contains(String s, String split, String find) {
        return null != split && splitToSet(s, split).contains(find);
    }
}
