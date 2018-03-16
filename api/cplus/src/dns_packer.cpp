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

#include "dns_packer.h"

namespace Tseerapi {

int IPv4ReqPack(char* buf, uint32_t& buflen, const char* domain, uint32_t domainlen, uint16_t id)
{
    // check domain
    if (NULL == domain or domainlen < 3U) // at least 3 bytes, such as "a.b"
        return DNS_ERR_BAD_DOMAIN;
    if ('.' == domain[domainlen - 1]) // remove the last '.', otherwise dns server do not response
        if (--domainlen < 3U)
            return DNS_ERR_BAD_DOMAIN;

    // check buf
    uint32_t datalen = sizeof(DnsHeader) + 1 + domainlen + 1 + 2 + 2;
    if (NULL == buf or buflen < datalen)
        return DNS_ERR_BAD_BUFFER;
    buflen = datalen;

    // fill domain of question
    unsigned char ch = 0, cnt = 0;
    char* ptr = buf + sizeof(DnsHeader) + domainlen;
    for (int i = domainlen - 1; i >= 0; --i)
    {
        if ('.' == (ch = domain[i]))
        {
            *(ptr--) = cnt;
            cnt = 0;
        }
        else
        {
            *(ptr--) = ch;
            ++cnt;
        }
    }
    *ptr = cnt; // the first count
    ptr = buf + sizeof(DnsHeader) + domainlen + 1;
    *(ptr++) = 0; // the last count

    // fill type of question(big endian)
    *(ptr++) = 0;
    *(ptr++) = 1; // type 1 means ipv4

    // fill class of question(big endian)
    *(ptr++) = 0;
    *(ptr++) = 1; // class 1 means internet

    // fill dns header
    DnsHeader* header = (DnsHeader*)buf;
    header->Clear();
    if (id != 0) header->id = htons(id);
    header->rd = 1; // Recursion Desired
    header->qdcount = htons(1);
    return 0;
}

int IPv4RspUnpack(std::vector<uint32_t>& ips, const char* buf, uint32_t buflen, uint16_t id)
{
    ips.clear();

    // check buf
    if (NULL == buf or buflen < sizeof(DnsHeader))
        return DNS_ERR_BAD_BUFFER;

    const DnsHeader* header = (DnsHeader*)buf;
    uint32_t pos = sizeof(DnsHeader);
    uint16_t qdcount = ntohs(header->qdcount);
    uint16_t ancount = ntohs(header->ancount);
    unsigned char ch = 0;

    // check return code
    if (header->rcode != 0)
        return header->rcode;
    // check answer count
    if (0 == qdcount or 0 == ancount)
        return DNS_ERR_NULL_CNT;
    // check Identification
    if (id != 0 and ntohs(header->id) != id)
        return DNS_ERR_BAD_ID;

    ips.reserve(ancount);

#define DNS_SKIP_DOMAIN()  \
    while (pos < buflen) \
    { \
        if (0 == (ch = buf[pos])) /* the last count found */ \
        { \
            pos += 1; \
            break; \
        } \
        else if ((ch & 0xc0) != 0) /* pointer found */ \
        { \
            pos += 2; \
            break; \
        } \
        else \
            pos += ch + 1; /* skip count + 1 chars */ \
    }

    // skip Questions
    for (uint16_t i = 0; i < qdcount; ++i, pos += 4) // 4 == len of{type(2) + class(2)}
        DNS_SKIP_DOMAIN()

    // parse Answer RRs
    for (uint16_t i = 0; i < ancount; ++i)
    {
        DNS_SKIP_DOMAIN();

        if (pos + 10 > buflen) // 10 == len of{type(2) + class(2) + ttl(4) + rdlength(2)}
            return DNS_ERR_UNPACK;

        uint16_t rdlength = ((uint16_t)buf[pos + 8] << 8) | buf[pos + 9]; // big endian
        if (pos + 10 + rdlength > buflen)
            return DNS_ERR_UNPACK;

        if ((0 == buf[pos]) and (1 == buf[pos + 1])) // type(big endian) 1 means ipv4
        {
            if (rdlength != 4) // ipv4 should be 4 bytes
                return DNS_ERR_UNPACK;
            ips.push_back(*(uint32_t*)(buf + pos + 10));
        }

        pos += 10 + rdlength;
    }

    return 0;
}

}

