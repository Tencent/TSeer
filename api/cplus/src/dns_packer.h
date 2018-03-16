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

#ifndef __TSEER_API_DNS_PACKER_H__
#define __TSEER_API_DNS_PACKER_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <endian.h>
#include <netinet/in.h>
#include <vector>

namespace Tseerapi {

const short DNS_ERR_BAD_DOMAIN    = 32767;
const short DNS_ERR_BAD_BUFFER    = 32766;
const short DNS_ERR_NULL_CNT      = 32765;
const short DNS_ERR_BAD_ID        = 32764;
const short DNS_ERR_UNPACK        = 32763;
const short DNS_ERR_TIMEOUT       = 32762;

/// @attention 要特别注意字节序
struct DnsHeader
{
    uint16_t id; // bits: 0-15; Identification
    union        // bits: 16-31; Flag
    {
        struct // 这个结构体的定义是为了方便操作具体的bit fields
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            uint8_t rd : 1;     // bit:  23; Recursion Desired
            uint8_t tc : 1;     // bit:  22; Truncated
            uint8_t aa : 1;     // bit:  21; Authoritative Answer
            uint8_t opcode : 4; // bits: 17,18,19,20; Operation Code
            uint8_t qr : 1;     // bit:  16; Query/Response

            uint8_t rcode : 4;  // bits: 28,29,30,31; Return Code
            uint8_t cd : 1;     // bit:  27; Checking Disabled
            uint8_t ad : 1;     // bit:  26; Authenticated Data
            uint8_t z : 1;      // bit:  25; Zero, Reserved
            uint8_t ra : 1;     // bit:  24; Recursion Available
#else
            uint8_t qr : 1;     // bit:  16; Query/Response
            uint8_t opcode : 4; // bits: 17,18,19,20; Operation Code
            uint8_t aa : 1;     // bit:  21; Authoritative Answer
            uint8_t tc : 1;     // bit:  22; Truncated
            uint8_t rd : 1;     // bit:  23; Recursion Desired

            uint8_t ra : 1;     // bit:  24; Recursion Available
            uint8_t z : 1;      // bit:  25; Zero, Reserved
            uint8_t ad : 1;     // bit:  26; Authenticated Data
            uint8_t cd : 1;     // bit:  27; Checking Disabled
            uint8_t rcode : 4;  // bits: 28,29,30,31; Return Code
#endif
        };
        uint16_t flag; // 如果要同时操作多个bit fields，可以把值打包好一次性赋值给flag
    };
    uint16_t qdcount; // bits: 32-47; Total Questions; qd means Question Domain?
    uint16_t ancount; // bits: 48-63; Total Answer RRs; an means ANswer
    uint16_t nscount; // bits: 64-79; Total Authority RRs; ns means Name Server
    uint16_t arcount; // bits: 80-95; Total Additional RRs; ar means Additional Record

    void Clear() { memset(this, 0, sizeof(*this)); };

    void Show() // for debugging
    {
        printf("%7s 0x%04x(%u)\n", "id", id, id);
        printf("%7s 0x%04x(%u)\n", "flag", flag, flag);
        printf("%7s 0x%04x(%u)\n", "qr", qr, qr);
        printf("%7s 0x%04x(%u)\n", "opcode", opcode, opcode);
        printf("%7s 0x%04x(%u)\n", "aa", aa, aa);
        printf("%7s 0x%04x(%u)\n", "tc", tc, tc);
        printf("%7s 0x%04x(%u)\n", "rd", rd, rd);
        printf("%7s 0x%04x(%u)\n", "ra", ra, ra);
        printf("%7s 0x%04x(%u)\n", "z", z, z);
        printf("%7s 0x%04x(%u)\n", "ad", ad, ad);
        printf("%7s 0x%04x(%u)\n", "cd", cd, cd);
        printf("%7s 0x%04x(%u)\n", "rcode", rcode, rcode);
        printf("%7s 0x%04x(%u)\n", "qdcount", qdcount, qdcount);
        printf("%7s 0x%04x(%u)\n", "ancount", ancount, ancount);
        printf("%7s 0x%04x(%u)\n", "nscount", nscount, nscount);
        printf("%7s 0x%04x(%u)\n", "arcount", arcount, arcount);
    }
} __attribute__((packed));

/// @brief ipv4请求包编码
/// @param[out] buf 编码后的数据
/// @param[in,out] buflen 输入：buf长度，输出：编码后的数据长度
/// @param[in] domain 要解析的域名
/// @param[in] domainlen 要解析的域名长度
/// @param[in] id dns header里面的Identification字段
/// @return 0: 成功，非0：失败
int IPv4ReqPack(
        char* buf, uint32_t& buflen, const char* domain, uint32_t domainlen, uint16_t id = 0);

/// @brief ipv4响应包解码
/// @param[out] ips 获取到的ipv4地址列表(big endian)
/// @param[in] buf 待解码的数据
/// @param[in] buflen 待解码的数据长度
/// @param[in] id dns header里面的Identification字段, 如果id不为0的话跟请求包里面的id必须匹配
/// @return 0: 成功，非0：失败
int IPv4RspUnpack(std::vector<uint32_t>& ips, const char* buf, uint32_t buflen, uint16_t id = 0);

}

#endif

