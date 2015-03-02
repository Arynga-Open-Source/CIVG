/**
 * Copyright (C) <2013>, GENIVI Alliance, Inc.
 * Author: bj@open-rnd.pl
 *
 * This file is part of <GENIVI Download Upload Messaging Manager>.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contributor License Agreements.29
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 *
 * List of changes:
 * <01.04.2013>, <Bartlomiej Jozwiak>, <Initial version> 
 */


/**
 * @file curl-headers.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef __HU_CURL_HEADERS_H__
#define __HU_CURL_HEADERS_H__

#include <vector>
#include <string>
#include <curl/curl.h>

/**
 * @brief Helper class for handling issues related to CURL HTTP headers
 */
namespace dumm
{
    class curlHeaders
    {
        public:
            curlHeaders();
            curlHeaders(const std::vector<std::string>& headers);
            ~curlHeaders();

            void reset();
            bool append(const std::string &s);
            bool append(const std::vector<std::string> &headers);
            bool append(const char *s);

            operator struct curl_slist*();
            curlHeaders& operator=(const std::string &s);
            curlHeaders& operator=(const std::vector<std::string> &headers);
            curlHeaders& operator=(const char *s);
            curlHeaders& operator+=(const std::string &s);
            curlHeaders& operator+=(const std::vector<std::string> &headers);
            curlHeaders& operator+=(const char *s);

        private:
            curlHeaders(const curlHeaders&);
            curlHeaders& operator= (const curlHeaders&);
            curlHeaders& operator+= (const curlHeaders&);

        private:
            struct curl_slist *list_;			/**< CURL header list */
    };
};


#endif /*__HU_CURL_HEADERS_H__*/
