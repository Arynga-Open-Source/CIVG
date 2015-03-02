/**
 *
 *  Download Upload Messaging Manager
 *
 *  Copyright (C) 2012-2013  Open-RnD Sp. z o.o. All rights reserved.
 *
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
 *
 * @file curl-wrapper.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef __HU_CURL_WRAPPER_H__
#define __HU_CURL_WRAPPER_H__

#include <string>		//for string
#include <stdio.h>		//for FILE*
#include <curl/curl.h>	//libCurl header

#include "curl-httpheaders.h"
#include "dumm-errors.h"

namespace dumm
{
    /** Curl Private data struct */
    typedef struct __CurlData
    {
        CURL 		*curl;	/**< Curl handler */
        CURLcode	res;	/**< Curl last error code */
    } CurlData;

    /** HTTP request type */
    typedef enum
    {
        HTTP_GET,       /**< HTTP GET request */
        HTTP_POST,      /**< HTTP POST request */
        HTTP_PUT,		/**< HTTP PUT request */
    } HttpReqTypeT;

    /** Authentification method */
    typedef enum
    {
        NET_AUTH_BASIC, /**< Basic */
        NET_AUTH_NONE   /**< No authentification */
    } NetAuthTypeT;

    /** Curl callback function type */
    typedef  size_t (*curlRWFunT)(void *param, void *buffer, size_t size, size_t nitems);

    /** Curl progress callback type */
    typedef int (*curlProgressFunT)(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    /** Helper structure for Curl callback */
    typedef struct
    {
        curlRWFunT	 write_cb;	/**< Write function */
        curlRWFunT	 read_cb;	/**< Read function */
        void		*param;		/**< File descriptor */
    } NetProtoCurl;

    /** Structure for Curl progress callback */
    typedef struct
    {
        curlProgressFunT	progress_cb;    /**< Progress callback function */
        void		*param;                 /**< User data */
    } NetProgressCurl;

    void __curl_global_initialization();
    DummErrorT __curl_init(CurlData *p);
    void __curl_deinit(CurlData *p);
    DummErrorT __curl_reset(CurlData *p);
    DummErrorT __curl_sethttpheaders(CurlData *p, curlHeaders &headers);
    DummErrorT __curl_seturl(CurlData *p, const std::string &url);
    DummErrorT __curl_verbose(CurlData *p, bool enable = true);
    DummErrorT __curl_setdatasize(CurlData *p, size_t size);
    DummErrorT __curl_setnobody(CurlData *p);
    DummErrorT __curl_reqtype(CurlData *p, HttpReqTypeT type);
    DummErrorT __curl_perform(CurlData *p);
    DummErrorT __curl_setreceiver(CurlData *p, void *userp);
    DummErrorT __curl_setreceiverfd(CurlData *p, FILE *f);
    DummErrorT __curl_sethttpheaderreceiver(CurlData *p, void *userp);
    DummErrorT __curl_unsetreceiver(CurlData *p);
    DummErrorT __curl_settransmitter(CurlData *p, void *userp);
    DummErrorT __curl_settransmitterfd(CurlData *p, FILE *f);
    DummErrorT __curl_unsettransmitter(CurlData *p);
    DummErrorT __curl_unsethttpheaderreceiver(CurlData *p);
    DummErrorT __curl_setprogress(CurlData *p, void *userp);
    DummErrorT __curl_unsetprogress(CurlData *p);
    DummErrorT __curl_settransmitterfd(CurlData *p, FILE *f);
    DummErrorT __curl_setfilesize(CurlData *p, int size);
    DummErrorT __curl_setauth(CurlData *p,
                             NetAuthTypeT type,
                             const std::string &user,
                             const std::string &pass);
    DummErrorT __curl_resumefrom(CurlData *p, int position);
    DummErrorT __curl_geturl(CurlData *p, std::string &url);
    DummErrorT __curl_getHttpError(CurlData *p, int &error);
    DummErrorT __curl_getContentType(CurlData *p, std::string &type);
    DummErrorT __curl_getContentLength(CurlData *p, int &len);
    DummErrorT __curl_settimeout(CurlData *p, long  timeout);
    DummErrorT __curl_follow(CurlData *p, bool enable = true);
    DummErrorT __curl_pause(CurlData *p);
    DummErrorT __curl_unpause(CurlData *p);
    DummErrorT __curl_setupload(CurlData *p);
};

#endif /*curl_wrapper.cpp*/
