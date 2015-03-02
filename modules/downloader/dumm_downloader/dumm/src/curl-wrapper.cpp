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

#include <stdarg.h>

#include "openrnd-exceptions.h"
#include "curl-wrapper.h"


//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME		"CURL WRAPPER"
#define LOGGER_COL		"30m"
#include "dumm-logger.h"

namespace dumm
{
    DummErrorT __curl_Err2Carsync(CURLcode err)
    {
        //TODO: delete this !!
        if(err != CURLE_OK)
            logger(LOG_ERROR, ">>> CURL ERR: %s (%d)\n", curl_easy_strerror(err), err);

        switch(err)
        {
            case CURLE_OK:
                return DUMM_ERR_OK;
            /**
             * TODO: Map more errors here !!!!
             */

            case CURLE_REMOTE_FILE_NOT_FOUND:
                return DUMM_ERR_DM_REMOTE_FILE_NOT_FOUND;

            case CURLE_OUT_OF_MEMORY:
                return DUMM_ERR_OUT_OF_MEMORY;

            case CURLE_ABORTED_BY_CALLBACK:
                return DUMM_ERR_NET_ABORTED;

            case CURLE_RANGE_ERROR:
                return DUMM_ERR_NET_RANGE_NOT_SUPPORTED;

            default:
                return DUMM_ERR_FAIL;
        }
    }

    static
    void __curl_once_fun()
    {
        CURLcode res;
        res = curl_global_init(CURL_GLOBAL_ALL);

        if(res != CURLE_OK)
        {
            //raise an exception here
            runtimeException("Cannot initialize CURL (global init)");
        }
    }


    void __curl_global_initialization()
    {
         pthread_once_t once_init_flag= PTHREAD_ONCE_INIT;

        if(pthread_once(&once_init_flag, __curl_once_fun))
        {
            runtimeException("Cannot initialize CURL");
        }
    }


    #define __CURL_CHECKINIT(__CURL__)				\
        if(!(__CURL__))								\
            return DUMM_ERR_ARGUMENT;				\
        if(!(__CURL__)->curl)						\
            return DUMM_ERR_NOT_INIT


    DummErrorT __curl_init(CurlData *p)
    {
        if(!p)
            return DUMM_ERR_ARGUMENT;

        p->res = CURLE_OK;

        p->curl = curl_easy_init();

        if(!p->curl)
        {
            p->res = CURLE_FAILED_INIT;		//I am not sure if we can use this error code here !!!
            return DUMM_ERR_NOT_INIT;
        }

        return __curl_Err2Carsync(p->res);
    }


    void __curl_deinit(CurlData *p)
    {
        if(!p) return;
        if(p->curl)
            curl_easy_cleanup(p->curl);
        p->curl = 0;
        p->res = CURLE_OK;
    }


    DummErrorT __curl_reset(CurlData *p)
    {
        __CURL_CHECKINIT(p);
        p->res = CURLE_OK;
        curl_easy_reset(p->curl);

        return DUMM_ERR_OK;
    }

    #define __CURL_SETOPT(__CURL__, __OPT__, __VAL__)		\
        (__CURL__)->res = curl_easy_setopt((__CURL__)->curl, (__OPT__), (__VAL__))

    #define __CURL_ERR(__CURL__)							\
            __curl_Err2Carsync((__CURL__)->res)

    #define __CURL_GETINFO(__CURL__, __INFO__, __VAL__)		\
        (__CURL__)->res = curl_easy_getinfo((__CURL__)->curl, (__INFO__), (__VAL__))


    #define __CURL_ISERROR(__CURL__)		\
        ((__CURL__)->res != CURLE_OK)

    DummErrorT __curl_sethttpheaders(CurlData *p, curlHeaders &headers)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_HTTPHEADER, (struct curl_slist*)headers);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_seturl(CurlData *p, const std::string &url)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_URL, url.c_str());

        return __CURL_ERR(p);
    }


    DummErrorT __curl_verbose(CurlData *p, bool enable)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_VERBOSE, (enable?1L:0L));

        return __CURL_ERR(p);
    }


    DummErrorT __curl_setdatasize(CurlData *p, size_t size)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_POSTFIELDSIZE, size);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_settimeout(CurlData *p, long  timeout)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_TIMEOUT, timeout);
            if(__CURL_ISERROR(p))
                break;

            /**
             * So I set heresome defualt values
             */

            __CURL_SETOPT(p, CURLOPT_CONNECTTIMEOUT, 5L);
            if(__CURL_ISERROR(p))
                break;

            __CURL_SETOPT(p, CURLOPT_LOW_SPEED_LIMIT, 1);
            if(__CURL_ISERROR(p))
                break;


        }while(0);


        return __CURL_ERR(p);
    }



    DummErrorT __curl_setnobody(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_NOBODY, 1L);

        return __CURL_ERR(p);
    }




    DummErrorT __curl_reqtype(CurlData *p, HttpReqTypeT type)
    {
        __CURL_CHECKINIT(p);

        switch(type)
        {
            case HTTP_POST:
                __CURL_SETOPT(p, CURLOPT_POST, 1L);
                break;

            case HTTP_GET:
                __CURL_SETOPT(p, CURLOPT_HTTPGET, 1L);
                break;

            case HTTP_PUT:
                __CURL_SETOPT(p, CURLOPT_PUT, 1L);
                break;
        }

        return __CURL_ERR(p);
    }


    DummErrorT __curl_perform(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        p->res = curl_easy_perform(p->curl);

        return __CURL_ERR(p);
    }


    static
    size_t __curl_write_cb(void *buffer, size_t size, size_t nitems, void *userp)
    {
        NetProtoCurl *p = (NetProtoCurl *)userp;

        if(p && p->write_cb)
            return p->write_cb(p->param, buffer, size, nitems);
        return 0;
    }


    static
    size_t __curl_read_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        NetProtoCurl *p = (NetProtoCurl *)userdata;

        if(p && p->read_cb)
            return p->read_cb(p->param, ptr, size, nmemb);
        return 0;
    }

    /*

    size_t __curl_read_header_cb(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        NetProtoCurl *p = (NetProtoCurl *)userdata;

        return p->read_header_cb(ptr, size, nmemb);
    }
    */

    static
    int __curl_progress_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        NetProgressCurl *p = (NetProgressCurl *) clientp;
        if(p && p->progress_cb)
            return p->progress_cb(p->param, dltotal, dlnow, ultotal, ulnow);

        return 0;
    }


    DummErrorT __curl_setreceiver(CurlData *p, void *userp)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_WRITEDATA, userp);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_WRITEFUNCTION, __curl_write_cb);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }

    DummErrorT __curl_sethttpheaderreceiver(CurlData *p, void *userp)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_WRITEHEADER, userp);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_HEADERFUNCTION, __curl_write_cb);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_setreceiverfd(CurlData *p, FILE *f)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_WRITEDATA, f);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_unsetreceiver(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_WRITEDATA, 0);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_WRITEFUNCTION, 0);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }

    DummErrorT __curl_unsethttpheaderreceiver(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_WRITEHEADER, 0);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_HEADERFUNCTION, 0);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }



    DummErrorT __curl_settransmitter(CurlData *p, void *userp)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_READDATA, userp);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_READFUNCTION, __curl_read_cb);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_unsettransmitter(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_READDATA, 0);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_READFUNCTION, 0);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }

    DummErrorT __curl_settransmitterfd(CurlData *p, FILE *f)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_READDATA, f);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_setprogress(CurlData *p, void *userp)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_NOPROGRESS, 0L);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_PROGRESSDATA, userp);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_PROGRESSFUNCTION, __curl_progress_cb);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_unsetprogress(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_NOPROGRESS, 1L);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_PROGRESSDATA, 0);
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_PROGRESSFUNCTION, 0);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);
    }



    DummErrorT __curl_setfilesize(CurlData *p, int size)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_INFILESIZE_LARGE, size);

        return __CURL_ERR(p);
    }




    DummErrorT __curl_setauth(CurlData *p,
                             NetAuthTypeT type,
                             const std::string &user,
                             const std::string &pass)
    {
        long int param = 0;

        __CURL_CHECKINIT(p);

        switch(type)
        {
            case NET_AUTH_BASIC:
                param = CURLAUTH_BASIC;
                __CURL_SETOPT(p, CURLOPT_HTTPAUTH, param);
                if(__CURL_ISERROR(p))
                    break;
                __CURL_SETOPT(p, CURLOPT_USERNAME, user.c_str());
                if(__CURL_ISERROR(p))
                    break;
                __CURL_SETOPT(p, CURLOPT_PASSWORD, pass.c_str());
                if(__CURL_ISERROR(p))
                    break;
                break;

            default:
                p->res = CURLE_OK;
        }

        return __CURL_ERR(p);
    }


    /*

    DummErrorT __curl_setheaderreceiver(CurlData *p, void *userp)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_WRITEHEADER , userp);
        __CURL_SETOPT(p, CURLOPT_HEADERFUNCTION, __curl_read_header_cb);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_unsetheaderreceiver(CurlData *p)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_WRITEHEADER , 0);
        __CURL_SETOPT(p, CURLOPT_HEADERFUNCTION, 0);

        return __CURL_ERR(p);
    }
    */


    DummErrorT __curl_resumefrom(CurlData *p, int position)
    {
        __CURL_CHECKINIT(p);

        __CURL_SETOPT(p, CURLOPT_RESUME_FROM, position);

        return __CURL_ERR(p);
    }




    DummErrorT __curl_geturl(CurlData *p, std::string &url)
    {
        __CURL_CHECKINIT(p);

        char *u = NULL;
        __CURL_GETINFO(p, CURLINFO_EFFECTIVE_URL, &u);

        if(p->res == CURLE_OK)
        {
            url.clear();
            url.append(u);
        }

        return __CURL_ERR(p);
    }


    DummErrorT __curl_getHttpError(CurlData *p, int &error)
    {
        __CURL_CHECKINIT(p);

        __CURL_GETINFO(p, CURLINFO_RESPONSE_CODE, &error);

        return __CURL_ERR(p);
    }


    DummErrorT __curl_getContentType(CurlData *p, std::string &type)
    {
        __CURL_CHECKINIT(p);

        char *s = NULL;
        __CURL_GETINFO(p, CURLINFO_CONTENT_TYPE, &s);

        if(p->res == CURLE_OK)
        {
            type.clear();
            type.append(s);
        }

        return __CURL_ERR(p);
    }



    DummErrorT __curl_getContentLength(CurlData *p, int &len)
    {
        __CURL_CHECKINIT(p);

        double val = 0.0;
        __CURL_GETINFO(p, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &val);

        if(p->res == CURLE_OK)
        {
            len = int(val);
        }

        return __CURL_ERR(p);
    }


    DummErrorT __curl_follow(CurlData *p, bool enable)
    {
        __CURL_CHECKINIT(p);

        do
        {
            __CURL_SETOPT(p, CURLOPT_MAXREDIRS, 5); //max redirects
            if(__CURL_ISERROR(p))
                break;
            __CURL_SETOPT(p, CURLOPT_FOLLOWLOCATION, (int)enable);
            if(__CURL_ISERROR(p))
                break;
        }while(0);

        return __CURL_ERR(p);

    }

    DummErrorT __curl_pause(CurlData *p)
    {
        __CURL_CHECKINIT(p);
        p->res = curl_easy_pause(p->curl, CURLPAUSE_ALL);
        return __CURL_ERR(p);
    }

    DummErrorT __curl_unpause(CurlData *p)
    {
        __CURL_CHECKINIT(p);
        p->res = curl_easy_pause(p->curl, CURLPAUSE_CONT);
        return __CURL_ERR(p);
    }

    DummErrorT __curl_setupload(CurlData *p)
    {
        __CURL_CHECKINIT(p);
        __CURL_SETOPT(p, CURLOPT_UPLOAD, 1L);
        return __CURL_ERR(p);

    }

#undef __CURL_CHECKINIT
#undef __CURL_SETOPT
#undef __CURL_GETINFO
#undef __CURL_ERR
}
