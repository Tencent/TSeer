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

#include "EndPoint.h"

#include "global.h"
using Tseer::RouterNodeInfo;
using std::istringstream;
using std::ostringstream;
using std::string;

namespace Tseerapi
{

EndPoint::EndPoint() {}

EndPoint::EndPoint(bool isTcp, const string& host, int port, int timeout, 
    const string &set, int weight)
{
    _istcp = isTcp;
    _host = host;
    _port = port;
    _timeout = timeout;
    _set = set;

    if (weight == -1)
    {
        weight = 100;
    }
    _weight = (weight > 100 ? 100 : weight);
}

EndPoint::EndPoint(const EndPoint &l)
{
    _istcp = l._istcp;
    _host = l._host;
    _port = l._port;
    _timeout = l._timeout;
    _set = l._set;
    _weight = l._weight;
}

EndPoint& EndPoint::operator = (const EndPoint& l)
{
    if (this != &l)
    {
        _istcp = l._istcp;
        _host = l._host;
        _port = l._port;
        _timeout = l._timeout;
        _set = l._set;
        _weight = l._weight;
    }
    return *this;
}

bool EndPoint::operator == (const EndPoint& l)
{
    return (_istcp == l._istcp && _host == l._host && _port == l._port && _timeout == l._timeout && 
        _set == l._set && _weight == l._weight);
}

int EndPoint::parse(const std::string &str, std::string &errMsg)
{
    _weight = -1;
    const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    beg = str.find_first_not_of(delim, end);
    if (beg == string::npos)
    {
        ostringstream os;
        os << FILE_FUN << "parse error:" << str;
        errMsg = os.str();
        return -1;
    }

    end = str.find_first_of(delim, beg);
    if (end == string::npos)
    {
        end = str.length();
    }

    string desc = str.substr(beg, end - beg);
    if (desc == "tcp")
    {
        _istcp = true;
    }
    else if (desc == "udp")
    {
        _istcp = false;
    }
    else
    {
        ostringstream os;
        os << FILE_FUN << "parse tcp or udp error : " << str;
        errMsg = os.str();
        return -1;
    }

    desc = str.substr(end);
    end = 0;
    while (true)
    {
        beg = desc.find_first_not_of(delim, end);
        if (beg == string::npos)
        {
            break;
        }

        end = desc.find_first_of(delim, beg);
        if (end == string::npos)
        {
            end = desc.length();
        }

        string option = desc.substr(beg, end - beg);
        if (option.length() != 2 || option[0] != '-')
        {
            ostringstream os;
            os << FILE_FUN << "parse error: " << str;
            errMsg = os.str();
            return -1;
        }

        string argument;
        string::size_type argumentBeg = desc.find_first_not_of(delim, end);
        if (argumentBeg != string::npos && desc[argumentBeg] != '-')
        {
            beg = argumentBeg;
            end = desc.find_first_of(delim, beg);
            if (end == string::npos)
            {
                end = desc.length();
            }
            argument = desc.substr(beg, end - beg);
        }

        switch (option[1])
        {
        case 'h':
        {
            if (argument.empty())
            {
                ostringstream os;
                os << FILE_FUN << "parse -h error : " << str;
                errMsg = os.str();
                return -1;
            }
            const_cast<string&>(_host) = argument;
            break;
        }
        case 'p':
        {
            istringstream p(argument);
            if (!(p >> const_cast<int&>(_port)) || !p.eof() || _port < 0 || _port > 65535)
            {
                ostringstream os;
                os << FILE_FUN << "parse -p error : " << str;
                errMsg = os.str();
                return -1;
            }
            break;
        }
        case 't':
        {
            istringstream t(argument);
            if (!(t >> const_cast<int&>(_timeout)) || !t.eof())
            {
                ostringstream os;
                os << FILE_FUN << "parse -t error : " << str;
                errMsg = os.str();
                return -1;
            }
            break;
        }
        case 'w':
        {
            istringstream t(argument);
            if (!(t >> const_cast<int&>(_weight)) || !t.eof())
            {
                ostringstream os;
                os << FILE_FUN << "parse -w error : " << str;
                errMsg = os.str();
                return -1;
            }
            break;
        }
        case 's':
        {
            if (argument.empty())
            {
                ostringstream os;
                os << FILE_FUN << "parse -s error: " << str;
                errMsg = os.str();
                return -1;
            }
            const_cast<string&>(_set) = argument;
            break;
        }
        default:
        {
        }
        }// end of switch
    }

    if (_weight == -1)
    {
        _weight = 100;
    }
    _weight = (_weight > 100 ? 100 : _weight);

    if (_host.empty())
    {
        ostringstream os;
        os << FILE_FUN << "host must not be empty: " << str;
        errMsg = os.str();
        return -1;
    }
    else if (_host == "*")
    {
        const_cast<string&>(_host) = "0.0.0.0";
    }
    return 0;
}

std::string EndPoint::toString() const
{
    std::ostringstream os;
    os << (_istcp ? "tcp" : "udp") << " -h " << _host << " -p " << _port << " -t " << _timeout;
    if (!_set.empty()) os << " -s " << _set;
    if (_weight != -1) os << " -w " << _weight;
    return os.str();
}

RouterNodeInfo EndPoint::toRouterNodeInfo()
{
    RouterNodeInfo result;
    result.ip = _host;
    result.isTcp = _istcp;
    result.port = _port;
    result.setname = _set;
    result.weight = _weight;
    return result;
}

}
