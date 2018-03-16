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

// **********************************************************************
// TUP version 1.0.2 by WSRD Tencent.
// **********************************************************************

#ifndef _TUP_H_
#define _TUP_H_
#include <map>
#include <string>
#include <vector>
#include <sstream>

#ifdef __APPLE__
    #include "RequestF.h"
#elif defined ANDROID  // android
    #include "RequestF.h"
#else
    #include "RequestF.h"
#endif




#ifdef __GNUC__
#   if __GNUC__ >3 || __GNUC_MINOR__ > 3
#       include <ext/pool_allocator.h>
#   endif
#endif

namespace Tseerapi
{

//存放tars返回值的key
const string STATUS_RESULT_CODE = "STATUS_RESULT_CODE";
const string STATUS_RESULT_DESC = "STATUS_RESULT_DESC"; 

/////////////////////////////////////////////////////////////////////////////////
// 属性封装类

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator >
        //template<typename> class Alloc = __gnu_cxx::__pool_alloc >
class UniAttribute
{
    typedef vector<char,Alloc<char> > VECTOR_CHAR_TYPE;
    typedef map<string, VECTOR_CHAR_TYPE, less<string>,Alloc< pair<string,VECTOR_CHAR_TYPE > > > VECTOR_CHAR_IN_MAP_TYPE;
    typedef map<string, VECTOR_CHAR_IN_MAP_TYPE, less<string>,Alloc< pair<string,VECTOR_CHAR_IN_MAP_TYPE > > >   TUP_DATA_TYPE;

public:
    /**
     * 构造函数
     */
    UniAttribute()
    {
        _iVer = 2;
    }

    void setVersion(short iVer)
    {
        _iVer = iVer;
    }
    /**
     * 添加属性值
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值
     */
    template<typename T> void put(const string& name, const T& t)
    {
        //TarsOutputStream<TWriter> os;
        os.reset();

        os.write(t, 0);

        //_data[name][Class<T>::name()] = os.getByteBuffer();
        if(_iVer == 3) 
        {
            VECTOR_CHAR_TYPE & v = _new_data[name];
            v.assign(os.getBuffer(), os.getBuffer() + os.getLength());
        }
        else
        {
            VECTOR_CHAR_TYPE & v = _data[name][TarsClass<T>::name()];
            v.assign(os.getBuffer(), os.getBuffer() + os.getLength());
        }
    }

    void putUnknown(const string& name, const string& value)
    {
        //TarsOutputStream<TWriter> os;
        os.reset();
        os.writeUnknownV2(value);

        //_data[name][Class<T>::name()] = os.getByteBuffer();
        if(_iVer == 3)
        {
            VECTOR_CHAR_TYPE & v = _new_data[name];
            v.assign(os.getBuffer(), os.getBuffer() + os.getLength());
        }
        else
        {
            throw runtime_error(string("putUnknown supported version 3"));
        }
    }

    void getUnknown(const string& name, string& value)
    {
        typename VECTOR_CHAR_IN_MAP_TYPE::iterator mit;

        mit = _new_data.find(name);

        if (mit != _new_data.end() && mit->second.size()>2)
        {
            //去掉DataHead::eStructBegin,DataHead::eStructEnd
            value = string(&mit->second[0]+1, mit->second.size()-2);
            return;

        }
        throw runtime_error(string("UniAttribute not found key:") +  name);
    }

    /**
     * 获取属性值，属性不存在则抛出异常
     * 
     * @throw runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:   属性值输出参数
     */
    template<typename T> void get(const string& name, T& t)
    {
        //map<string, map<string, vector<char> > >::iterator mit;
        if(_iVer == 3) 
        {
            typename VECTOR_CHAR_IN_MAP_TYPE::iterator mit;
    
            mit = _new_data.find(name);
    
            if (mit != _new_data.end())
            {
                is.reset();

                is.setBuffer(mit->second);

                is.read(t, 0, true);

                return;
            
            }
            throw runtime_error(string("UniAttribute not found key:") +  name);
        }
        else
        {
            typename TUP_DATA_TYPE::iterator mit;
    
            mit = _data.find(name);
    
            if (mit != _data.end())
            {
                string type = TarsClass<T>::name();
    
                typename VECTOR_CHAR_IN_MAP_TYPE::iterator mmit = mit->second.find(type);
    
                if (mmit == mit->second.end() && mit->second.size() > 0)
                {
                    string firstType = mit->second.begin()->first;
                    
                    if (firstType.find_first_of("?") != string::npos) 
                    {
                        mmit = mit->second.begin();
                    }
                }
                if (mmit != mit->second.end())
                {
                    //TarsInputStream<TReader> is;
                    is.reset();
    
                    is.setBuffer(mmit->second);
    
                    is.read(t, 0, true);
    
                    return;
                }
                ostringstream os;
    
                os  << "UniAttribute type match fail,key:" << name << ",type:" << TarsClass<T>::name() << ",";
    
                if (mit->second.size() > 0)
                {
                    os << "may be:" << mit->second.begin()->first;
                }
                throw runtime_error(os.str());
            }
            throw runtime_error(string("UniAttribute not found key:") +  name + ",type:" + TarsClass<T>::name());
        }
    }
    /**
     * 获取属性值，属性不存在则抛出异常
     * 
     * @throw runtime_error
     * @param T:   属性类型
     * @param name:属性名称
     * @return T:  属性值
     */
    template<typename T> T get(const string& name)
    {
        T t;

        get<T>(name, t);

        return t;
    }
    /**
     * 获取属性值，忽略异常，不存在的属性返回缺省值
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param t:  属性值输出参数
     * @param def:     默认值
     */
    template<typename T> void getByDefault(const string& name, T& t, const T& def)
    {
        try
        {
            get<T>(name, t);
        }
        catch (runtime_error& e)
        {
            t = def;
        }
    }
    /**
     * 获取属性值(忽略异常，def为缺省值)
     * 
     * @param T:   属性类型
     * @param name:属性名称
     * @param:     默认值
     * @return T:  属性值
     */
    template<typename T> T getByDefault(const string& name, const T& def)
    {
        T t;

        getByDefault<T>(name, t, def);

        return t;
    }

    /**
     *清除全部属性值
     */
    void clear() 
    { 
        _data.clear(); 
        _new_data.clear();
    }

    /** 编码
     * 
     * @param buff： 编码结果输出参数
     */
    void encode(string& buff)
    {
        //TarsOutputStream<TWriter> os;
        os.reset();

        if(_iVer == 3)
        {
            os.write(_new_data, 0);
        }
        else
        {
            os.write(_data, 0);
        }
        buff.assign(os.getBuffer(), os.getLength());
    }

    /** 编码
     * 
     * @param buff： 编码结果输出参数
     */
    void encode(vector<char>& buff)
    {
        os.reset();

        if(_iVer == 3)
        {
            os.write(_new_data, 0);
        }
        else
        {
            os.write(_data, 0);
        }

        buff.assign(os.getBuffer(), os.getBuffer() + os.getLength());
    }

    /** 编码
     * 
     * @throw runtime_error
     * @param buff：输出存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char* buff, size_t & len)
    {   
        os.reset();

        if(_iVer == 3)
        {
            os.write(_new_data, 0);
        }
        else
        {
            os.write(_data, 0);
        }

        if(len < os.getLength()) throw runtime_error("encode error, buffer length too short");
        memcpy(buff, os.getBuffer(), os.getLength());
        len =  os.getLength();
    }

    /** 解码
     * 
     * @throw runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */
    void decode(const char* buff, size_t len)
    {
        is.reset();

        is.setBuffer(buff, len);

        if(_iVer == 3)
        {
            _new_data.clear();
    
            is.read(_new_data, 0, true);
        }
        else
        {
            _data.clear();
    
            is.read(_data, 0, true);
        }
    }
    /**
     * 解码
     * 
     * @throw runtime_error
     * @param buff： 待解码的字节流
     */
    void decode(const vector<char>& buff)
    {
        is.reset();

        is.setBuffer(buff);
        if(_iVer == 3)
        {
            _new_data.clear();
    
            is.read(_new_data, 0, true);
        }
        else
        {
            _data.clear();
    
            is.read(_data, 0, true);
        }
    }
    /**
     * 获取已有的属性
     * 
     * @return const map<string,map<string,vector<char>>>& : 属性map
     */
    const map<string, map<string, vector<char> > >& getData() const
    {
        return _data;
    }

    const map<string, vector<char> >& getNewData() const
    {
        return _new_data;
    }

    /**
     * 判断属性集合是否为空
     * 
     * @return bool:属性是否为空
     */
    bool isEmpty()
    {
        if(_iVer == 3) 
        {
            return _new_data.empty();
        }
        
        return _data.empty();
    }

    /**
     * 获取属性集合大小
     * 
     * @return size_t:  集合大小
     */
    size_t size()
    {
        if(_iVer == 3)
        {
            return _new_data.size();
        }
        
        return _data.size();
    }

    /**
     * 判断属性是否存在
     * 
     * @param key:属性名称
     * @return bool:是否存在
     */
    bool containsKey(const string & key)
    {
        if(_iVer == 3)
        {
            return _new_data.find(key) != _new_data.end();
        }
        
        return _data.find(key) != _data.end();
    }

protected:
    TUP_DATA_TYPE _data;
    VECTOR_CHAR_IN_MAP_TYPE _new_data;
    short _iVer;

public:
    TarsInputStream<TReader>     is;
    TarsOutputStream<TWriter>    os;
};

/////////////////////////////////////////////////////////////////////////////////
// 请求、回应包封装类

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator >
struct UniPacket : protected RequestPacket, public UniAttribute<TWriter, TReader, Alloc>
{
public:
    /**
     * 构造函数
     */
    UniPacket() 
    {
        iVersion = 2; cPacketType = 0; 

        iMessageType = 0; iRequestId = 0; 

        sServantName = ""; funcName = ""; 

        timeOut = 0; buffer.clear(); 

        context.clear(); status.clear(); 

        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVersion;

        UniAttribute<TWriter, TReader,Alloc>::_data.clear();

        UniAttribute<TWriter, TReader,Alloc>::_new_data.clear();
    }

    /**
     * 拷贝构造
     * @param tup
     */
    UniPacket(const UniPacket &tup)  { *this = tup;}

    void setVersion(short iVer)
    {
        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVer;
        iVersion = iVer;
    }

    /**
     * 由请求包生成回应包基本结构，回填关键的请求信息
     * 
     * @return UniPacket： 回应包
     */
    UniPacket createResponse()
    {
        UniPacket respPacket;

        respPacket.sServantName = sServantName;
        respPacket.funcName    = funcName;
        respPacket.iRequestId   = iRequestId;

        return respPacket;
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * 
     * @throw runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(string& buff)
    {
        TarsOutputStream<TWriter> &os = UniAttribute<TWriter, TReader,Alloc>::os;

        os.reset();
        
        doEncode(os);

        Tseerapi::Int32 headerLen = htonl(sizeof(Tseerapi::Int32) + os.getLength());
        buff.assign((const char*)&headerLen, sizeof(Tseerapi::Int32));

        buff.append(os.getBuffer(), os.getLength());
    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * 
     * @throw runtime_error
     * @param buff： 编码结果输出参数
     */
    void encode(vector<char>& buff)
    {
        TarsOutputStream<TWriter> & os = UniAttribute<TWriter, TReader,Alloc>::os;

        os.reset();

        doEncode(os);

        Tseerapi::Int32 headerLen = htonl(sizeof(Tseerapi::Int32) + os.getLength());

        buff.resize(sizeof(Tseerapi::Int32) + os.getLength());
        memcpy(&buff[0], &headerLen, sizeof(Tseerapi::Int32));
        memcpy(&buff[sizeof(Tseerapi::Int32)], os.getBuffer(), os.getLength());

    }

    /**
     * 编码，结果的包头4个字节为整个包的长度，网络字节序
     * @throw runtime_error
     * @param buff：存放编码结果的buffer指针
     * @param len： 输入buff长度，输出编码结果长度
     */
    void encode(char* buff, size_t & len)
    {
        TarsOutputStream<TWriter> &os = UniAttribute<TWriter, TReader,Alloc>::os;

        os.reset();

        doEncode(os);

        Tseerapi::Int32 headerLen = htonl(sizeof(Tseerapi::Int32) + os.getLength());
        if(len < sizeof(Tseerapi::Int32) + os.getLength()) throw runtime_error("encode error, buffer length too short");

        memcpy(buff, &headerLen, sizeof(Tseerapi::Int32));
        memcpy(buff + sizeof(Tseerapi::Int32), os.getBuffer(), os.getLength());

        len = sizeof(Tseerapi::Int32) + os.getLength();
    }

    /** 解码
     * 
     * @throw runtime_error
     * @param buff：待解码字节流的buffer指针
     * @param len： 待解码字节流的长度
     */

    void decode(const char* buff, size_t len)
    {
        if(len < sizeof(Tseerapi::Int32)) throw runtime_error("packet length too short");
    
        TarsInputStream<TReader> &is = UniAttribute<TWriter, TReader,Alloc>::is;

        is.reset();

        is.setBuffer(buff + sizeof(Tseerapi::Int32), len - sizeof(Tseerapi::Int32));

        readFrom(is);

        UniAttribute<TWriter, TReader,Alloc>::_iVer = iVersion;

        is.reset();

        is.setBuffer(buffer);

        if(iVersion == 3) 
        {
            UniAttribute<TWriter, TReader,Alloc>::_new_data.clear();
    
            is.read(UniAttribute<TWriter, TReader,Alloc>::_new_data, 0, true);
        }
        else
        {
            UniAttribute<TWriter, TReader,Alloc>::_data.clear();
    
            is.read(UniAttribute<TWriter, TReader,Alloc>::_data, 0, true);
        }
    }
public:
    /**
     * 获取消息version
     * @return tars::Short
     */
    Tseerapi::Short getVersion() const { return iVersion; }
    /**
     * 获取消息ID
     * @return tars::Int32
     */
    Tseerapi::Int32 getRequestId() const { return iRequestId; }
    /**
     * 设置请求ID
     * @param value
     */
    void setRequestId(Tseerapi::Int32 value) { iRequestId = value; }
    /**
     * 获取对象名称
     * @return const std::string&
     */
    const std::string& getServantName() const { return sServantName; }
    /**
     * 设置对象名称
     * @param value
     */
    void setServantName(const std::string& value) { sServantName = value; }
    /**
     * 获取方法名
     * @return const std::string&
     */
    const std::string& getFuncName() const { return funcName; }
    /**
     * 设置方法名
     * @param value
     */
    void setFuncName(const std::string& value) { funcName = value; }

protected:
    /**
     * 内部编码
     */
    void doEncode(TarsOutputStream<TWriter>& os)
    {
        //ServantName、FuncName不能为空
        if(sServantName.empty()) throw runtime_error("ServantName must not be empty");
        if(funcName.empty())    throw runtime_error("FuncName must not be empty");

        os.reset();

        if(iVersion == 3)
        {
            os.write(UniAttribute<TWriter, TReader,Alloc>::_new_data, 0);
        }
        else
        {
            os.write(UniAttribute<TWriter, TReader,Alloc>::_data, 0);
        }

        buffer.assign(os.getBuffer(), os.getBuffer() + os.getLength());

        os.reset();

        writeTo(os);
    }
};

/////////////////////////////////////////////////////////////////////////////////
// 调用TARS的服务时使用的类

template<typename TWriter = BufferWriter, typename TReader = BufferReader,template<typename> class Alloc = std::allocator>
struct TarsUniPacket: public UniPacket<TWriter, TReader,Alloc>
{
public:
    TarsUniPacket(){};
    TarsUniPacket(const UniPacket<TWriter, TReader,Alloc> &tup) 
    : UniPacket<TWriter, TReader,Alloc>(tup) {};

    /**
     * 设置协议版本
     * @param value
     */
    void setTarsVersion(Tseerapi::Short value) { UniPacket<TWriter, TReader,Alloc>::setVersion(value); }

    /**
     * 设置调用类型
     * @param value
     */
    void setTarsPacketType(Tseerapi::Char value) { this->cPacketType = value; }

    /**
     * 设置消息类型
     * @param value
     */
    void setTarsMessageType(Tseerapi::Int32 value) { this->iMessageType = value; }

    /**
     * 设置超时时间
     * @param value
     */
    void setTarsTimeout(Tseerapi::Int32 value) { this->timeOut = value; }

    /**
     * 设置参数编码内容
     * @param value
     */
    void setTarbuffer(const vector<Tseerapi::Char>& value) { this->buffer = value; }

    /**
     * 设置上下文
     * @param value
     */
    void setTarsContext(const map<std::string, std::string>& value) { this->context = value; }

    /**
     * 设置特殊消息的状态值
     * @param value
     */
    void setTarsStatus(const map<std::string, std::string>& value) { this->status = value; }

    /**
     * 获取协议版本
     * @return tars::Short
     */
    Tseerapi::Short getTarsVersion() const { return this->iVersion; }

    /**
     * 获取调用类型
     * @return tars::Char
     */
    Tseerapi::Char getTarsPacketType() const { return this->cPacketType; }

    /**
     * 获取消息类型
     * @return tars::Int32
     */
    Tseerapi::Int32 getTarsMessageType() const { return this->iMessageType; }

    /**
     * 获取超时时间
     * @return tars::Int32
     */
    Tseerapi::Int32 getTarsTimeout() const { return this->timeOut; }

    /**
     * 获取参数编码后内容
     * @return const vector<tars::Char>&
     */
    const vector<Tseerapi::Char>& getTarbuffer() const { return this->buffer; }

    /**
     * 获取上下文信息
     * @return const map<std::string,std::string>&
     */
    const map<std::string, std::string>& getTarsContext() const { return this->context; }

    /**
     * 获取特殊消息的状态值
     * @return const map<std::string,std::string>&
     */
    const map<std::string, std::string>& getTarsStatus() const { return this->status; }

    /**
     * 获取调用tars的返回值
     * 
     * @retrun tars::Int32
     */
    Tseerapi::Int32 getTarsResultCode() const
    {
        map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_CODE)) == this->status.end())
        {
            return 0;
        }
        else
        {
            return atoi(it->second.c_str());
        }
    }

    /**
     * 获取调用tars的返回描述
     * 
     * @retrun string
     */
    string getTarsResultDesc() const
    {
        map<std::string, std::string>::const_iterator it;
        if((it = this->status.find(STATUS_RESULT_DESC)) == this->status.end())
        {
            return "";
        }
        else
        {
            return it->second;
        }
    }

};

#ifdef __GNUC__
#   if __GNUC__ >3 || __GNUC_MINOR__ > 3
        typedef UniAttribute<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> UniAttrPoolAlloc;
        typedef UniPacket<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> UniPacketPoolAlloc;
        typedef TarsUniPacket<BufferWriter,BufferReader, __gnu_cxx::__pool_alloc> TarsUniPacketPoolAlloc;        
#   endif
#endif




}
////////////////////////////////////////////////////////////////////////////////////////////////
#endif
