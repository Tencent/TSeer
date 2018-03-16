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

#ifndef __TARS_H__
#define __TARS_H__

#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>



//支持iphone
#ifdef __APPLE__
    #include "TarsType.h"
#elif defined ANDROID  // android
    #include "TarsType.h"
#else
    #include "TarsType.h"
#endif

namespace Tseerapi
{
//////////////////////////////////////////////////////////////////
    struct TarsStructBase
    {
    protected:
        TarsStructBase() {}

        ~TarsStructBase() {}
    };

    struct TarsException : public std::runtime_error
    {
        TarsException(const std::string& s) : std::runtime_error(s) {}
    };

    struct TarsEncodeException : public TarsException
    {
        TarsEncodeException(const std::string& s) : TarsException(s) {}
    };

    struct TarsDecodeException : public TarsException
    {
        TarsDecodeException(const std::string& s) : TarsException(s) {}
    };

    struct TarsDecodeMismatch : public TarsDecodeException
    {
        TarsDecodeMismatch(const std::string & s) : TarsDecodeException(s) {}
    };

    struct TarsDecodeRequireNotExist : public TarsDecodeException
    {
        TarsDecodeRequireNotExist(const std::string & s) : TarsDecodeException(s) {}
    };

    struct TarsDecodeInvalidValue : public TarsDecodeException
    {
        TarsDecodeInvalidValue(const std::string & s) : TarsDecodeException(s) {}
    };

    struct TarsNotEnoughBuff : public TarsException
    {
        TarsNotEnoughBuff(const std::string & s) : TarsException(s) {}
    };

//////////////////////////////////////////////////////////////////
    namespace
    {
/// 数据头信息的封装，包括类型和tag
        class DataHead
        {
            uint8_t _type;
            uint8_t _tag;
        public:
            enum
            {
                eChar = 0,
                eShort = 1,
                eInt32 = 2,
                eInt64 = 3,
                eFloat = 4,
                eDouble = 5,
                eString1 = 6,
                eString4 = 7,
                eMap = 8,
                eList = 9,
                eStructBegin = 10,
                eStructEnd = 11,
                eZeroTag = 12,
                eSimpleList = 13,
            };

            struct helper
            {
                uint8_t     type : 4;
                uint8_t     tag  : 4;
            }__attribute__((packed));

        public:
            DataHead() : _type(0), _tag(0) {}
            DataHead(uint8_t type, uint8_t tag) : _type(type), _tag(tag) {}

            uint8_t getTag() const      { return _tag;}
            void setTag(uint8_t t)      { _tag = t;}
            uint8_t getType() const     { return _type;}
            void setType(uint8_t t)     { _type = t;}

            /// 读取数据头信息
            template<typename InputStreamT>
            void readFrom(InputStreamT& is)
            {
                size_t n = peekFrom(is);
                is.skip(n);
            }

            /// 读取头信息，但不前移流的偏移量
            template<typename InputStreamT>
            size_t peekFrom(InputStreamT& is)
            {
                helper h;
                size_t n = sizeof(h);
                is.peekBuf(&h, sizeof(h));
                _type = h.type;
                if (h.tag == 15)
                {
                    is.peekBuf(&_tag, sizeof(_tag), sizeof(h));
                    n += sizeof(_tag);
                }
                else
                {
                    _tag = h.tag;
                }
                return n;
            }

            /// 写入数据头信息
            template<typename OutputStreamT>
            void writeTo(OutputStreamT& os)
            {
                /*
                helper h;
                h.type = _type;
                if(_tag < 15){
                    h.tag = _tag;
                    os.writeBuf(&h, sizeof(h));
                }else{
                    h.tag = 15;
                    os.writeBuf(&h, sizeof(h));
                    os.writeBuf(&_tag, sizeof(_tag));
                }
                */
                writeTo(os, _type, _tag);
            }

            /// 写入数据头信息
            template<typename OutputStreamT>
            static void writeTo(OutputStreamT& os, uint8_t type, uint8_t tag)
            {
                helper h;
                h.type = type;
                if (tag < 15)
                {
                    h.tag = tag;
                    os.writeBuf(&h, sizeof(h));
                }
                else
                {
                    h.tag = 15;
                    os.writeBuf(&h, sizeof(h));
                    os.writeBuf(&tag, sizeof(tag));
                }
            }
        };
    }


//////////////////////////////////////////////////////////////////
/// 缓冲区读取器封装
    class BufferReader
    {
        const char *        _buf;        ///< 缓冲区
        size_t              _buf_len;    ///< 缓冲区长度
        size_t              _cur;        ///< 当前位置

    public:

        BufferReader() : _buf(NULL),_buf_len(0),_cur(0) {}

        void reset() { _cur = 0;}

        /// 读取缓存
        void readBuf(void * buf, size_t len)
        {
            if(len <= _buf_len && (_cur + len) <= _buf_len)
            {
                peekBuf(buf, len);
                _cur += len;
            }
            else
            {
                char s[64];
                snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
                throw TarsDecodeException(s);
            }
        }

        /// 读取缓存，但不改变偏移量
        void peekBuf(void * buf, size_t len, size_t offset = 0)
        {
            if (_cur + offset + len > _buf_len)
            {
                char s[64];
                snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
                throw TarsDecodeException(s);
            }
            ::memcpy(buf, _buf + _cur + offset, len);
        }

        /// 跳过len个字节
        void skip(size_t len)
        {
            if(len <= _buf_len && (_cur + len) <= _buf_len)
            {
                _cur += len;
            }
            else
            {
                char s[64];
                snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
                throw TarsDecodeException(s);
            }
        }

        /// 设置缓存
        void setBuffer(const char * buf, size_t len)
        {
            _buf = buf;
            _buf_len = len;
            _cur = 0;
        }

        /// 设置缓存
        template<typename Alloc>
        void setBuffer(const std::vector<char,Alloc> &buf)
        {
            _buf = &buf[0];
            _buf_len = buf.size();
            _cur = 0;
        }

        /**
         * 判断是否已经到BUF的末尾
         */
        bool hasEnd()
        {
            return _cur >= _buf_len;
        }
        size_t tellp() const
        {
            return _cur;
        }
        const char* base() const
        {
            return _buf;
        }
        size_t size() const
        {
            return _buf_len;
        }
    };

//当tars文件中含有指针型类型的数据用MapBufferReader读取
//在读数据时利用MapBufferReader提前分配的内存 减少运行过程中频繁内存分配
//结构中定义byte指针类型，指针用*来定义，如下：
//byte *m;
//指针类型使用时需要MapBufferReader提前设定预分配内存块setMapBuffer()，
//指针需要内存时通过偏移指向预分配内存块，减少解码过程中的内存申请

    class MapBufferReader : public BufferReader
    {

    public:
        MapBufferReader() : _buf(NULL),_buf_len(0),_cur(0) {}

        void reset() { _cur = 0; BufferReader::reset();}

        char* cur()
        {
            if (_buf == NULL)
            {
                char s[64];
                snprintf(s, sizeof(s), "MapBufferReader's buff not set,_buf = null");
                throw TarsDecodeException(s);
            }
            return _buf+_cur;
        }

        size_t left(){return _buf_len-_cur;}

        /// 跳过len个字节
        void mapBufferSkip(size_t len)
        {
            if (_cur + len > _buf_len)
            {
                char s[64];
                snprintf(s, sizeof(s), "MapBufferReader's buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
                throw TarsDecodeException(s);
            }
            _cur += len;
        }

        /// 设置缓存
        void setMapBuffer(char * buf, size_t len)
        {
            _buf = buf;
            _buf_len = len;
            _cur = 0;
        }

        /// 设置缓存
        template<typename Alloc>
        void setMapBuffer(std::vector<char,Alloc> &buf)
        {
            _buf = &buf[0];
            _buf_len = buf.size();
            _cur = 0;
        }
    public:
        char *              _buf;        ///< 缓冲区
        size_t              _buf_len;    ///< 缓冲区长度
        size_t              _cur;        ///< 当前位置
    };

//////////////////////////////////////////////////////////////////
/// 缓冲区写入器封装
    class BufferWriter
    {
        char *  _buf;
        size_t  _len;
        size_t  _buf_len;

    public:
        BufferWriter(const BufferWriter & bw)
        {
            _buf = NULL;
            _len = 0;
            _buf_len = 0;

            writeBuf(bw._buf, bw._len);
            _len = bw._len;
            //_buf_len    = bw._buf_len;
        }

        BufferWriter& operator=(const BufferWriter& buf)
        {
            writeBuf(buf._buf,buf._len);
            _len = buf._len;
            //_buf_len = buf._buf_len;
            return *this;
        }

        BufferWriter()
        : _buf(NULL)
        , _len(0)
        , _buf_len(0)
        {}
        ~BufferWriter()
        {
            delete[] _buf;
        }
        void reserve(size_t len)
        {
            if (_buf_len < len)
            {
                len *= 2;
                if(len<128)
                    len=128;
                char * p = new char[len];
                memcpy(p, _buf, _len);
                delete[] _buf;
                _buf = p;
                _buf_len = len;
            }
        }
        void reset() { _len = 0;}

        void writeBuf(const void * buf, size_t len)
        {
            reserve(_len + len);
            memcpy(_buf + _len, buf, len);
            _len += len;
        }

        //const std::vector<char> &getByteBuffer() const      { return _buf; }
        std::vector<char> getByteBuffer() const      { return std::vector<char>(_buf, _buf + _len);}
        const char * getBuffer() const                      { return _buf;}//{ return &_buf[0]; }
        size_t getLength() const                            { return _len;}    //{ return _buf.size(); }
        //void swap(std::vector<char>& v)                     { _buf.swap(v); }
        void swap(std::vector<char>& v)
        {
            v.assign(_buf, _buf + _len);
        }
        void swap(BufferWriter& buf)
        {
            std::swap(_buf, buf._buf);
            std::swap(_buf_len, buf._buf_len);
            std::swap(_len, buf._len);
        }
    };

///////////////////////////////////////////////////////////////////////////////////////////////////
/// 预先设定缓存的封装器

    class BufferWriterBuff
    {
        char *  _buf;
        size_t  _len;
        size_t  _buf_len;

        BufferWriterBuff(const BufferWriterBuff&);
    public:

        BufferWriterBuff& operator=(const BufferWriterBuff& buf)
        {
            writeBuf(buf._buf, buf._len);
            _len = buf._len;
            _buf_len = buf._buf_len;
            return *this;
        }

        BufferWriterBuff()
        : _buf(NULL)
        , _len(0)
        , _buf_len(0)
        {}
        ~BufferWriterBuff()
        {

        }

        void setBuffer(char * buffer, size_t size_buff)
        {
            _buf = buffer;
            _len = 0;
            _buf_len = size_buff;
        }

        /*
        void reserve(size_t len)
        {
            if(_buf_len < len)
            {

            }
        }
        */
        void reset() { _len = 0;}

        void writeBuf(const void * buf, size_t len)
        {
            if (_buf_len < _len + len)
            {
                throw TarsNotEnoughBuff("not enough buffer");
            }

            memcpy(_buf + _len, buf, len);
            _len += len;
        }

        std::vector<char> getByteBuffer() const      { return std::vector<char>(_buf, _buf + _len);}
        const char * getBuffer() const               { return _buf;}
        size_t getLength() const                     { return _len;}
        void swap(std::vector<char>& v)
        {
            v.assign(_buf, _buf + _len);
        }
        void swap(BufferWriterBuff& buf)
        {
            std::swap(_buf, buf._buf);
            std::swap(_buf_len, buf._buf_len);
            std::swap(_len, buf._len);
        }
    };

//////////////////////////////////////////////////////////////////
    template<typename ReaderT = BufferReader>
    class TarsInputStream : public ReaderT
    {
    public:
        //tars自动伸缩使用的接口，其它不会使用
        bool getTagType(uint8_t tag, uint8_t &type)
        {
            try
            {
                DataHead h;
                while (!ReaderT::hasEnd())
                {
                    size_t len = h.peekFrom(*this);
                    if (tag <= h.getTag() || h.getType() == DataHead::eStructEnd)
                    {
                        if(h.getType() == DataHead::eStructEnd)
                        {
                            return false;
                        }
                        else if(tag == h.getTag())
                        {
                            type = h.getType();
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    this->skip(len);
                    skipField(h.getType());
                }
            }
            catch (TarsDecodeException& e)
            {
            }
            return false;
        }

        //tars自动伸缩使用的接口，其它不会使用
        int getFieldValue(uint8_t type, int64_t &value)
        {
            int flag = -1;

            value = 0;

            switch (type)
            {
            case DataHead::eChar:
                {                    
                    Char c = 0;
                    read(c, 0);

                    value = c;

                    flag = 0;
                }
                break;
            case DataHead::eShort:
                {                    
                    Short s = 0;
                    read(s, 0);

                    value = s;

                    flag = 0;
                }
                break;
            case DataHead::eInt32:
                {
                    Int32 i = 0;
                    read(i, 0);

                    value = i;

                    flag = 0;
                }
                break;
            case DataHead::eInt64:
                {
                    Int64 i = 0;
                    read(i, 0);

                    value = i;

                    flag = 0;
                }
                break;
            case DataHead::eFloat:
                {
                    flag = -1;
                }
                break;
            case DataHead::eDouble:
                {
                    flag = -1;
                }
                break;
            case DataHead::eString1:
                {
                    DataHead h;
                    h.readFrom(*this);

                    uint8_t len = readByType<uint8_t>();
                    value = len;

                    flag = 1;
                }
                break;
            case DataHead::eString4:
                {
                    DataHead h;
                    h.readFrom(*this);
                    
                    uint32_t len = ntohl(readByType<uint32_t>());
                    value = len;

                    flag = 1;
                }
                break;
            case DataHead::eMap:
                {
                    DataHead h;
                    h.readFrom(*this);
                    
                    Int32 size = 0;
                    read(size, 0);

                    value = size;

                    flag = 2;
                }
                break;
            case DataHead::eList:
                {
                    DataHead h;
                    h.readFrom(*this);
                    
                    Int32 size = 0;
                    read(size, 0);

                    value = size;

                    flag = 3;
                }
                break;
            case DataHead::eSimpleList:
                {
                    DataHead h;
                    h.readFrom(*this);

                    DataHead hh;
                    hh.readFrom(*this);

                    UInt32 size = 0;
                    read(size, 0);

                    value = size;

                    flag = 4;
                }
                break;
            case DataHead::eStructBegin:
                {
                    flag = -1;
                }
                break;
            case DataHead::eStructEnd:
                {
                    flag = -1;
                }
                break;
            case DataHead::eZeroTag:
                {
                    flag = 0;
                }
                break;
            default:
                {
                    char s[64];
                    snprintf(s, sizeof(s), "getFieldValue with invalid type, type value:%d.", type);
                    throw TarsDecodeMismatch(s);
                }
            }
            return flag;
        }

        /// 跳到指定标签的元素前
        bool skipToTag(uint8_t tag)
        {
            try
            {
                DataHead h;
                while (!ReaderT::hasEnd())
                {
                    size_t len = h.peekFrom(*this);
                    if (tag <= h.getTag() || h.getType() == DataHead::eStructEnd)
                        return h.getType() == DataHead::eStructEnd?false:(tag == h.getTag());
                    this->skip(len);
                    skipField(h.getType());
                }
            }
            catch (TarsDecodeException& e)
            {
            }
            return false;
        }

        /// 跳到当前结构的结束
        void skipToStructEnd()
        {
            DataHead h;
            do
            {
                h.readFrom(*this);
                skipField(h.getType());
            }while (h.getType() != DataHead::eStructEnd);
        }

        /// 跳过一个字段
        void skipField()
        {
            DataHead h;
            h.readFrom(*this);
            skipField(h.getType());
        }

        /// 跳过一个字段，不包含头信息
        void skipField(uint8_t type)
        {
            switch (type)
            {
            case DataHead::eChar:
                this->skip(sizeof(Char));
                break;
            case DataHead::eShort:
                this->skip(sizeof(Short));
                break;
            case DataHead::eInt32:
                this->skip(sizeof(Int32));
                break;
            case DataHead::eInt64:
                this->skip(sizeof(Int64));
                break;
            case DataHead::eFloat:
                this->skip(sizeof(Float));
                break;
            case DataHead::eDouble:
                this->skip(sizeof(Double));
                break;
            case DataHead::eString1:
                {
                    size_t len = readByType<uint8_t>();
                    this->skip(len);
                }
                break;
            case DataHead::eString4:
                {
                    size_t len = ntohl(readByType<uint32_t>());
                    this->skip(len);
                }
                break;
            case DataHead::eMap:
                {
                    Int32 size = 0;
                    read(size, 0);
                    for (Int32 i = 0; i < size * 2; ++i)
                        skipField();
                }
                break;
            case DataHead::eList:
                {
                    Int32 size = 0;
                    read(size, 0);
                    for (Int32 i = 0; i < size; ++i)
                        skipField();
                }
                break;
            case DataHead::eSimpleList:
                {
                    DataHead h;
                    h.readFrom(*this);
                    if (h.getType() != DataHead::eChar)
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "skipField with invalid type, type value: %d, %d.", type, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                    Int32 size = 0;
                    read(size, 0);
                    this->skip(size);
                }
                break;
            case DataHead::eStructBegin:
                skipToStructEnd();
                break;
            case DataHead::eStructEnd:
            case DataHead::eZeroTag:
                break;
            default:
                {
                    char s[64];
                    snprintf(s, sizeof(s), "skipField with invalid type, type value:%d.", type);
                    throw TarsDecodeMismatch(s);
                }
            }
        }

        /// 读取一个指定类型的数据（基本类型）
        template<typename T>
        inline T readByType()
        {
            T n;
            this->readBuf(&n, sizeof(n));
            return n;
        }
        void readUnknown(std::string & sUnkown, uint8_t tag)
        {

            size_t start = ReaderT::tellp();
            size_t last  = start;
            try
            {
                uint8_t lasttag = tag;
                DataHead h;
                while (!ReaderT::hasEnd())
                {
                    size_t len = h.peekFrom(*this);
                    if ( h.getTag() <=lasttag)
                    {
                        break;
                    }
                    lasttag = h.getTag();
                    this->skip(len);
                    skipField(h.getType());
                    last = ReaderT::tellp(); //记录下最后一次正常到达的位置
                }
            }
            catch (...) //
            {
                throw;
            }
            std::string s(ReaderT::base() +start, last - start);
            sUnkown = s;
            return ;

        }
        friend class XmlProxyCallback;

        void read(Bool& b, uint8_t tag, bool isRequire = true)
        {
            Char c = b;
            read(c, tag, isRequire);
            b = c ? true : false;
        }

        void read(Char& c, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    c = 0;
                    break;
                case DataHead::eChar:
                    this->readBuf(&c, sizeof(c));
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Char' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }

                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d.", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(UInt8& n, uint8_t tag, bool isRequire = true)
        {
            Short i = (Short)n;
            read(i,tag,isRequire);
            n = (UInt8)i;
        }

        void read(Short& n, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    n = 0;
                    break;
                case DataHead::eChar:
                    n = readByType<Char>();
                    break;
                case DataHead::eShort:
                    this->readBuf(&n, sizeof(n));
                    n = ntohs(n);
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Short' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(UInt16& n, uint8_t tag, bool isRequire = true)
        {
            Int32 i = (Int32)n;
            read(i,tag,isRequire);
            n = (UInt16)i;
        }

        void read(Int32& n, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    n = 0;
                    break;
                case DataHead::eChar:
                    n = readByType<Char>();
                    break;
                case DataHead::eShort:
                    n = (Short) ntohs(readByType<Short>());
                    break;
                case DataHead::eInt32:
                    this->readBuf(&n, sizeof(n));
                    n = ntohl(n);
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Int32' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(UInt32& n, uint8_t tag, bool isRequire = true)
        {
            Int64 i = (Int64)n;
            read(i,tag,isRequire);
            n = (UInt32)i;
        }

        void read(Int64& n, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    n = 0;
                    break;
                case DataHead::eChar:
                    n = readByType<Char>();
                    break;
                case DataHead::eShort:
                    n = (Short) ntohs(readByType<Short>());
                    break;
                case DataHead::eInt32:
                    n = (Int32) ntohl(readByType<Int32>());
                    break;
                case DataHead::eInt64:
                    this->readBuf(&n, sizeof(n));
                    n = tars_ntohll(n);
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Int64' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }

                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(Float& n, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    n = 0;
                    break;
                case DataHead::eFloat:
                    this->readBuf(&n, sizeof(n));
                    n = tars_ntohf(n);
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Float' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(Double& n, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eZeroTag:
                    n = 0;
                    break;
                case DataHead::eFloat:
                    n = readByType<Float>();
                    n = tars_ntohf(n);
                    break;
                case DataHead::eDouble:
                    this->readBuf(&n, sizeof(n));
                    n = tars_ntohd(n);
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'Double' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(std::string& s, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eString1:
                    {
                        size_t len = readByType<uint8_t>();
                        char ss[256];
                        //s.resize(len);
                        //this->readBuf((void *)s.c_str(), len);
                        this->readBuf(ss, len);
                        s.assign(ss, ss + len);
                    }
                    break;
                case DataHead::eString4:
                    {
                        uint32_t len = ntohl(readByType<uint32_t>());
                        if (len > TARS_MAX_STRING_LENGTH)
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid string size, tag: %d, size: %d", tag, len);
                            throw TarsDecodeInvalidValue(s);
                        }
                        //char *ss = new char[len];
                        //s.resize(len);
                        //this->readBuf((void *)s.c_str(), len);
                        char *ss = new char[len];
                        try
                        {
                            this->readBuf(ss, len);
                            s.assign(ss, ss + len);
                        }
                        catch (...)
                        {
                            delete[] ss;
                            throw;
                        }
                        delete[] ss;
                    }
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'string' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        void read(char *buf, const UInt32 bufLen, UInt32 & readLen, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eSimpleList:
                    {
                        DataHead hh;
                        hh.readFrom(*this);
                        if (hh.getType() != DataHead::eChar)
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d", tag, h.getType(), hh.getType());
                            throw TarsDecodeMismatch(s);
                        }
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > bufLen)
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, h.getType(), hh.getType(), size);
                            throw TarsDecodeInvalidValue(s);
                        }
                        this->readBuf(buf, size);
                        readLen = size;
                    }
                    break;

                default:
                    {
                        char s[128];
                        snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[128];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }


        template<typename K, typename V, typename Cmp, typename Alloc>
        void read(std::map<K, V, Cmp, Alloc>& m, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eMap:
                    {
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > this->size())
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid map, tag: %d, size: %d", tag, size);
                            throw TarsDecodeInvalidValue(s);
                        }
                        m.clear();

                        for (UInt32 i = 0; i < size; ++i)
                        {
                            std::pair<K, V> pr;
                            read(pr.first, 0);
                            read(pr.second, 1);
                            m.insert(pr);
                        }
                    }
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        template<typename Alloc>
        void read(std::vector<Char, Alloc>& v, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eSimpleList:
                    {
                        DataHead hh;
                        hh.readFrom(*this);
                        if (hh.getType() != DataHead::eChar)
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d", tag, h.getType(), hh.getType());
                            throw TarsDecodeMismatch(s);
                        }
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > this->size())
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, h.getType(), hh.getType(), size);
                            throw TarsDecodeInvalidValue(s);
                        }

                        v.reserve(size);
                        v.resize(size);

                        this->readBuf(&v[0], size);
                    }
                    break;
                case DataHead::eList:
                    {
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > this->size())
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, h.getType(), size);
                            throw TarsDecodeInvalidValue(s);
                        }
                        v.reserve(size);
                        v.resize(size);
                        for (UInt32 i = 0; i < size; ++i)
                            read(v[i], 0);
                    }
                    break;
                default:
                    {
                        char s[128];
                        snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[128];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        template<typename T, typename Alloc>
        void read(std::vector<T, Alloc>& v, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eList:
                    {
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > this->size())
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, h.getType(), size);
                            throw TarsDecodeInvalidValue(s);
                        }
                        v.reserve(size);
                        v.resize(size);
                        for (UInt32 i = 0; i < size; ++i)
                            read(v[i], 0);
                    }
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'vector' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        /// 读取结构数组
        template<typename T>
        void read(T* v, const UInt32 len, UInt32 & readLen, uint8_t tag, bool isRequire = true)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                switch (h.getType())
                {
                case DataHead::eList:
                    {
                        UInt32 size = 0;
                        read(size, 0);
                        if (size > this->size())
                        {
                            char s[128];
                            snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, h.getType(), size);
                            throw TarsDecodeInvalidValue(s);
                        }
                        for (UInt32 i = 0; i < size; ++i)
                            read(v[i], 0);
                        readLen = size;
                    }
                    break;
                default:
                    {
                        char s[64];
                        snprintf(s, sizeof(s), "read 'vector struct' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                        throw TarsDecodeMismatch(s);
                    }
                }
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }

        template<typename T>
        void read(T& v, uint8_t tag, bool isRequire = true, typename tars::disable_if<tars::is_convertible<T*, TarsStructBase*>, void ***>::type dummy = 0)
        {
            Int32 n = 0;
            read(n, tag, isRequire);
            v = (T) n;
        }

        /// 读取结构
        template<typename T>
        void read(T& v, uint8_t tag, bool isRequire = true, typename tars::enable_if<tars::is_convertible<T*, TarsStructBase*>, void ***>::type dummy = 0)
        {
            if (skipToTag(tag))
            {
                DataHead h;
                h.readFrom(*this);
                if (h.getType() != DataHead::eStructBegin)
                {
                    char s[64];
                    snprintf(s, sizeof(s), "read 'struct' type mismatch, tag: %d, get type: %d.", tag, h.getType());
                    throw TarsDecodeMismatch(s);
                }
                v.readFrom(*this);
                skipToStructEnd();
            }
            else if (isRequire)
            {
                char s[64];
                snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
                throw TarsDecodeRequireNotExist(s);
            }
        }
    };

//////////////////////////////////////////////////////////////////
    template<typename WriterT = BufferWriter>
    class TarsOutputStream : public WriterT
    {
    public:
        void writeUnknown(const std::string& s)
        {
            this->writeBuf(s.data(), s.size());
        }
        void writeUnknownV2(const std::string& s)
        {
            DataHead::writeTo(*this, DataHead::eStructBegin, 0);
            this->writeBuf(s.data(), s.size());
            DataHead::writeTo(*this, DataHead::eStructEnd, 0);
        }
        void write(Bool b, uint8_t tag)
        {
            write((Char) b, tag);
        }

        void write(Char n, uint8_t tag)
        {
            /*
            DataHead h(DataHead::eChar, tag);
            if(n == 0){
                h.setType(DataHead::eZeroTag);
                h.writeTo(*this);
            }else{
                h.writeTo(*this);
                this->writeBuf(&n, sizeof(n));
            }
            */
            if (n == 0)
            {
                DataHead::writeTo(*this, DataHead::eZeroTag, tag);
            }
            else
            {
                DataHead::writeTo(*this, DataHead::eChar, tag);
                this->writeBuf(&n, sizeof(n));
            }
        }

        void write(UInt8 n, uint8_t tag)
        {
            write((Short) n, tag);
        }

        void write(Short n, uint8_t tag)
        {
            //if(n >= CHAR_MIN && n <= CHAR_MAX){
            if (n >= (-128) && n <= 127)
            {
                write((Char) n, tag);
            }
            else
            {
                /*
                DataHead h(DataHead::eShort, tag);
                h.writeTo(*this);
                n = htons(n);
                this->writeBuf(&n, sizeof(n));
                */
                DataHead::writeTo(*this, DataHead::eShort, tag);
                n = htons(n);
                this->writeBuf(&n, sizeof(n));
            }
        }

        void write(UInt16 n, uint8_t tag)
        {
            write((Int32) n, tag);
        }

        void write(Int32 n, uint8_t tag)
        {
            //if(n >= SHRT_MIN && n <= SHRT_MAX){
            if (n >= (-32768) && n <= 32767)
            {
                write((Short) n, tag);
            }
            else
            {
                //DataHead h(DataHead::eInt32, tag);
                //h.writeTo(*this);
                DataHead::writeTo(*this, DataHead::eInt32, tag);
                n = htonl(n);
                this->writeBuf(&n, sizeof(n));
            }
        }

        void write(UInt32 n, uint8_t tag)
        {
            write((Int64) n, tag);
        }

        void write(Int64 n, uint8_t tag)
        {
            //if(n >= INT_MIN && n <= INT_MAX){
            if (n >= (-2147483647-1) && n <= 2147483647)
            {
                write((Int32) n, tag);
            }
            else
            {
                //DataHead h(DataHead::eInt64, tag);
                //h.writeTo(*this);
                DataHead::writeTo(*this, DataHead::eInt64, tag);
                n = tars_htonll(n);
                this->writeBuf(&n, sizeof(n));
            }
        }

        void write(Float n, uint8_t tag)
        {
            //DataHead h(DataHead::eFloat, tag);
            //h.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eFloat, tag);
            n = tars_htonf(n);
            this->writeBuf(&n, sizeof(n));
        }

        void write(Double n, uint8_t tag)
        {
            //DataHead h(DataHead::eDouble, tag);
            //h.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eDouble, tag);
            n = tars_htond(n);
            this->writeBuf(&n, sizeof(n));
        }

        void write(const std::string& s, uint8_t tag)
        {
            if (s.size() > 255)
            {
                if (s.size() > TARS_MAX_STRING_LENGTH)
                {
                    char ss[128];
                    snprintf(ss, sizeof(ss), "invalid string size, tag: %d, size: %u", tag, (uint32_t)s.size());
                    throw TarsDecodeInvalidValue(ss);
                }
                DataHead::writeTo(*this, DataHead::eString4, tag);
                uint32_t n = htonl(s.size());
                this->writeBuf(&n, sizeof(n));
                this->writeBuf(s.data(), s.size());
            }
            else
            {
                DataHead::writeTo(*this, DataHead::eString1, tag);
                uint8_t n = s.size();
                this->writeBuf(&n, sizeof(n));
                this->writeBuf(s.data(), s.size());
            }
        }

        void write(const char *buf, const UInt32 len, uint8_t tag)
        {
            DataHead::writeTo(*this, DataHead::eSimpleList, tag);
            DataHead::writeTo(*this, DataHead::eChar, 0);
            write(len, 0);
            this->writeBuf(buf, len);
        }

        template<typename K, typename V, typename Cmp, typename Alloc>
        void write(const std::map<K, V, Cmp, Alloc>& m, uint8_t tag)
        {
            //DataHead h(DataHead::eMap, tag);
            //h.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eMap, tag);
            Int32 n = m.size();
            write(n, 0);
            typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;
            for (IT i = m.begin(); i != m.end(); ++i)
            {
                write(i->first, 0);
                write(i->second, 1);
            }
        }

        template<typename T, typename Alloc>
        void write(const std::vector<T, Alloc>& v, uint8_t tag)
        {
            //DataHead h(DataHead::eList, tag);
            //h.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eList, tag);
            Int32 n = v.size();
            write(n, 0);
            typedef typename std::vector<T, Alloc>::const_iterator IT;
            for (IT i = v.begin(); i != v.end(); ++i)
                write(*i, 0);
        }

        template<typename T>
        void write(const T *v, const UInt32 len, uint8_t tag)
        {
            DataHead::writeTo(*this, DataHead::eList, tag);
            write(len, 0);
            for (Int32 i = 0; i < (Int32)len; ++i)
            {
                write(v[i], 0);
            }
        }

        template<typename Alloc>
        void write(const std::vector<Char, Alloc>& v, uint8_t tag)
        {
            //DataHead h(DataHead::eSimpleList, tag);
            //h.writeTo(*this);
            //DataHead hh(DataHead::eChar, 0);
            //hh.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eSimpleList, tag);
            DataHead::writeTo(*this, DataHead::eChar, 0);
            Int32 n = v.size();
            write(n, 0);
            this->writeBuf(&v[0], v.size());
        }

        template<typename T>
        void write(const T& v, uint8_t tag, typename tars::disable_if<tars::is_convertible<T*, TarsStructBase*>, void ***>::type dummy = 0)
        {
            write((Int32) v, tag);
        }

        template<typename T>
        void write(const T& v, uint8_t tag, typename tars::enable_if<tars::is_convertible<T*, TarsStructBase*>, void ***>::type dummy = 0)
        {
            //DataHead h(DataHead::eStructBegin, tag);
            //h.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eStructBegin, tag);
            v.writeTo(*this);
            DataHead::writeTo(*this, DataHead::eStructEnd, 0);
            /*
            h.setType(DataHead::eStructEnd);
            h.setTag(0);
            h.writeTo(*this);
            */
        }
    };
////////////////////////////////////////////////////////////////////////////////////////////////////
}

//支持iphone
#ifdef __APPLE__
    #include "TarsDisplayer.h"
#else
    #include "TarsDisplayer.h"
#endif

#endif
