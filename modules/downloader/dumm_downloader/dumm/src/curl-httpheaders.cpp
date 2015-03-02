/**
 *
 *  Download Upload Messaging Manager
 *
 *  Copyright (C) 2012-2013  Open-RnD Sp. z o.o. All rights reserved.
 * @verbatim
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License version 3
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * @endverbatim

 * @file curl-headers.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 * @brief __BRIEF_HERE_TBD__
 *
 * __DETAILS__
 */

#include "curl-httpheaders.h"

namespace dumm
{
    curlHeaders::curlHeaders()
        : list_(0)
    {
    }

    curlHeaders::~curlHeaders()
    {
        reset();
    }

    void curlHeaders::reset()
    {
        if(list_)
            curl_slist_free_all(list_);
        list_ = 0;
    }

    bool curlHeaders::append(const std::string &s)
    {
        if(!s.empty())
        {
            return append(s.c_str());
        }
        return true;
    }

    bool curlHeaders::append(const std::vector<std::string> &headers)
    {
        for(unsigned int i = 0; i < headers.size(); i++)
        {
            if(!append(headers[i]))
                return false;
        }
        return true;
    }

    bool curlHeaders::append(const char *s)
    {
        if(s)
        {
            struct curl_slist *chunk = 0;
            chunk = curl_slist_append(list_, s);
            if(!chunk)
                return false;
            list_ = chunk;
        }
        return true;
    }

    curlHeaders::operator struct curl_slist*()
    {
        return list_;
    }

    curlHeaders& curlHeaders::operator=(const std::string &s)
    {
        reset();
        append(s);
        return *this;
    }

    curlHeaders& curlHeaders::operator=(const std::vector<std::string> &headers)
    {
        reset();
        append(headers);
        return *this;
    }

    curlHeaders& curlHeaders::operator=(const char *s)
    {
        reset();
        append(s);
        return *this;
    }

    curlHeaders& curlHeaders::operator+=(const std::string &s)
    {
        append(s);
        return *this;
    }

    curlHeaders& curlHeaders::operator+=(const std::vector<std::string> &headers)
    {
        append(headers);
        return *this;
    }

    curlHeaders& curlHeaders::operator+=(const char *s)
    {
        append(s);
        return *this;
    }
}
