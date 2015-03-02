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
 * @brief __BRIEF_HERE_TBD__
 *
 * __DETAILS__
 */


#include <errno.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>


#include "mgr-upload.h"
#include "obj-upload.h"
#include "files.h"

#include "dumm-dbus.h"
#include "gdbus.h"


//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME "DUMM-UM-OBJ"
#define LOGGER_COL "34m"
#include "dumm-logger.h"


namespace dumm
{
    static
    gboolean upgetterProps(const GDBusPropertyTable *property,
                         DBusMessageIter *iter, void *data)
    {
        UploadObj *obj = (UploadObj*)data;
        gboolean ret = TRUE;

        if(property || data)
        {
            obj->lock();
            logger(LOG_DEBUG, "property->name: %s\n", property->name);
            do
            {
                if(strcmp(property->type, "s") == 0)
                {
                    char *s = 0;

                    if(strcmp(property->name, "Url") == 0)
                        s = strdup(obj->getUrl().c_str());
                    else if(strcmp(property->name, "Visibility") == 0)
                        s = strdup(obj->getVisibility().c_str());
                    else if(strcmp(property->name, "UploadState") == 0)
                        s = strdup(UploadStateToStr(obj->getState()));
                    else if(strcmp(property->name, "UploadDescription") == 0)
                        s = strdup(obj->getUploadDescription().c_str());
                    else if(strcmp(property->name, "StoragePath") == 0)
                        s = strdup(obj->getStoragePath().c_str());
                    else if(strcmp(property->name, "Filename") == 0)
                        s = strdup(obj->getFilename().c_str());
                    else if(strcmp(property->name, "ProxyUrl") == 0)
                        s = strdup(obj->getProxyUrl().c_str());
                    else if(strcmp(property->name, "ProxyUser") == 0)
                        s = strdup(obj->getProxyUser().c_str());
                    else if(strcmp(property->name, "ProxyPasswd") == 0)
                        s = strdup(obj->getProxyPasswd().c_str());
                    else
                        s = strdup("Not supported now");

                    dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &s);
                    if(s)
                        free(s);
                }
                else if(strcmp(property->type, "i") == 0)
                {
                    int val = 0;
                    if(strcmp(property->name, "Lifetime") == 0)
                        val = obj->getLifetime();
                    else if(strcmp(property->name, "CreationTime") == 0)
                        val = obj->getCreationTime();
                    else if(strcmp(property->name, "UploadTime") == 0)
                        val = obj->getUploadTime();
                    else if(strcmp(property->name, "EstimateTime") == 0)
                        val = obj->getEstTime();
                    else if(strcmp(property->name, "Size") == 0)
                        val = obj->getSize();
                    else if(strcmp(property->name, "UploadSize") == 0)
                        val = obj->getUploadSize();
                    else if(strcmp(property->name, "LastHTTPResponseCode") == 0)
                        val = obj->getHttpError();
                    else if(strcmp(property->name, "ProxyPort") == 0)
                        val = obj->getProxyPort();
                    else
                    {
                        ret = FALSE;
                        break;
                    }

                    dbus_message_iter_append_basic (iter, DBUS_TYPE_INT32, &val);
                }
                else if(strcmp(property->type, "b") == 0)
                {
                    gboolean val = TRUE;
                    if(strcmp(property->name, "KeepOverLifecycle") == 0)
                        val = obj->getKeepOverLifecycle();
                    else if(strcmp(property->name, "AutoResume") == 0)
                        val = obj->getAutoResume();
                    else if(strcmp(property->name, "AbortAllowed") == 0)
                        val = obj->getAbortAllowed();
                    else if(strcmp(property->name, "PauseAllowed") == 0)
                        val = obj->getPauseAllowed();
                    else
                    {
                        ret = FALSE;
                        break;
                    }

                    dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &val);
                }
                else if(strcmp(property->type, "as") == 0)
                {
                    DBusMessageIter arrIter;
                    const char * ss = "DM_header: Not supported\n\r";

                    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "s", &arrIter);
                    dbus_message_iter_append_basic (&arrIter, DBUS_TYPE_STRING, &ss);
                    dbus_message_iter_append_basic (&arrIter, DBUS_TYPE_STRING, &ss);
                    dbus_message_iter_append_basic (&arrIter, DBUS_TYPE_STRING, &ss);
                    dbus_message_iter_close_container(iter, &arrIter);
                }
                else
                    ret = FALSE;
            }while(0);
            obj->unlock();
        }
        else
            ret = FALSE;

        return ret;
    }

    void upsetterProps(const GDBusPropertyTable *property,
                     DBusMessageIter *value, GDBusPendingPropertySet id,
                     void *data)
    {
        logger(LOG_DEBUG, "setterProps %p, %p, %p\n", property, value, data);

        bool ret = false;
        UploadObj *obj = (UploadObj*)data;

        if(obj)
        {
            ret = true;
            switch(dbus_message_iter_get_arg_type(value))
            {
                case DBUS_TYPE_STRING:
                {
                    const char *s;
                    dbus_message_iter_get_basic(value, &s);
                    logger(LOG_INFO, "\tsetterProps: %s\n", s);

                    if(strcmp(property->name, "UploadDescription") == 0)
                        ret = obj->setUploadDescription(s);
                    else if(strcmp(property->name, "ProxyUrl") == 0)
                        ret = obj->setProxyUrl(s);
                    else if(strcmp(property->name, "ProxyUser") == 0)
                        ret = obj->setProxyUser(s);
                    else if(strcmp(property->name, "ProxyPasswd") == 0)
                        ret = obj->setProxyPasswd(s);
                    else
                        ret = false;
                    break;
                }

                case DBUS_TYPE_INT32:
                {
                    int val = 0;
                    dbus_message_iter_get_basic(value, &val);

                    if(strcmp(property->name, "ProxyPort") == 0)
                        ret = obj->setProxyPort(val);
                    else
                        ret = false;
                    break;
                }

                case DBUS_TYPE_BOOLEAN:
                {
                    break;
                }

                case DBUS_TYPE_ARRAY:
                default:
                    ret = false;
            }
        }

        if(ret)
        {
            obj->dbusEmitPropChange(property->name);
            g_dbus_pending_property_success(id);
        }
        else
            dummDbusPropError(id, DUMM_ERR_ARGUMENT);
    }

    static const GDBusPropertyTable _gUM_Prop[] = {
            {"Lifetime", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"CreationTime", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"UploadTime", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"EstimateTime", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"KeepOverLifecycle", "b", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"AutoResume", "b", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"Visibility", "s", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"UploadState", "s", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"Size", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"UploadSize", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"UploadDescription", "s", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {"AbortAllowed", "b", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"PauseAllowed", "b", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"Url", "s", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"StoragePath", "s", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"Filename", "s", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"LastHTTPResponseCode", "i", upgetterProps, NULL, NULL, (GDBusPropertyFlags)0},
            {"HTTPHeaders", "as", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {"ProxyUrl", "s", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {"ProxyPort", "i", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {"ProxyUser", "s", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {"ProxyPasswd", "s", upgetterProps, upsetterProps, NULL, (GDBusPropertyFlags)0},
            {}
    };

    static
    const char* __getObjName(int id)
    {
        static char tmp[30] = {0};
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "/UMObject/%d", id);
        return tmp;
    }

    static
    bool __addObjToDbus(CtxT *ctx, UploadObj *obj)
    {
        logger(LOG_INFO, "%p, %p, %p\n", ctx, ctx->conn, obj);
        if(!ctx || !ctx->conn || !obj)
            return false;
        return g_dbus_register_interface(ctx->conn, __getObjName(obj->id()), DUMM_UM_OBJS, NULL, NULL, _gUM_Prop, obj, NULL) == TRUE;
    }

    static
    void __delObjFromDbus(CtxT *ctx, UploadObj *obj)
    {
        if(!ctx || !ctx->conn || !obj)
            return;
        g_dbus_unregister_interface(ctx->conn, __getObjName(obj->id()), DUMM_UM_OBJS);
    }



    UploadObj::UploadObj(MgrUploadI &mgr, int id, DB_Detail::DbImplSlot *slot)
        : CommonObj(id, slot),
          mgr_(mgr),
          estTime_(0)
    {
        if(mgr_.getCtx())
        {
            //TODO: Check result ?
            __addObjToDbus(mgr_.getCtx(), this);
        }
    }

    UploadObj::~UploadObj()
    {
        __delObjFromDbus(mgr_.getCtx(), this);

        mgr_.release(id(), slot());
    }


    UploadStateT UploadObj::getState()
    {
        return mgr_.getState(id(), slot());
    }

    int UploadObj::getUploadSize()
    {
        return mgr_.getUploadSize(id(), slot());
    }

    bool UploadObj::setUploadSize(int size)
    {
        return mgr_.setUploadSize(id(), slot(), size);
    }

    int UploadObj::getSize()
    {
        int ret = -1;

        if(fd_)
        {
            ret = fileSize(fileno(fd_));
        }
        else if(getState() == STATE_FINISHED)
        {
            ret = getUploadSize();
        }

        return ret;
    }

    std::string UploadObj::getUrl()
    {
        return mgr_.getUrl(id(), slot());
    }

    bool UploadObj::setHttpError(int error)
    {
        return mgr_.setHttpError(id(), slot(), error);
    }

    int UploadObj::getHttpError()
    {
        return mgr_.getHttpError(id(), slot());
    }

    int UploadObj::getUploadTime()
    {
        return mgr_.getUploadTime(id(), slot());
    }

    void UploadObj::setEstTime(int t)
    {
        estTime_ = t;
    }

    int UploadObj::getEstTime() const
    {
        return estTime_;
    }

    std::string UploadObj::getVisibility()
    {
        std::string ret;

        ret = VisibilityToStr(mgr_.getVisibility(id(), slot()));

        return ret;
    }

    std::string UploadObj::getUploadDescription()
    {
        return mgr_.getDescription(id(), slot());
    }

    bool UploadObj::setUploadDescription(const std::string &val)
    {
        return mgr_.setDescription(id(), slot(), val);
    }

    std::string UploadObj::getStoragePath()
    {
        return mgr_.getStoragePath(id(), slot());
    }

    std::string UploadObj::getFilename()
    {
        return mgr_.getFilename(id(), slot());
    }

    int UploadObj::getLifetime()
    {
        return mgr_.getLifetime(id(), slot());
    }

    int UploadObj::getCreationTime()
    {
        return mgr_.getCreationTime(id(), slot());
    }

    int UploadObj::getProxyPort()
    {
        return mgr_.getProxyPort(id(), slot());
    }

    bool UploadObj::setProxyPort(int val)
    {
        return mgr_.setProxyPort(id(), slot(), val);
    }

    std::string UploadObj::getProxyUrl()
    {
        return mgr_.getProxyUrl(id(), slot());
    }

    bool UploadObj::setProxyUrl(const std::string &val)
    {
        return mgr_.setProxyUrl(id(), slot(), val);
    }

    std::string UploadObj::getProxyUser()
    {
        return mgr_.getProxyUser(id(), slot());
    }

    bool UploadObj::setProxyUser(const std::string &val)
    {
        return mgr_.setProxyUser(id(), slot(), val);
    }

    std::string UploadObj::getProxyPasswd()
    {
        return mgr_.getProxyPasswd(id(), slot());
    }

    bool UploadObj::setProxyPasswd(const std::string &val)
    {
        return mgr_.setProxyPasswd(id(), slot(), val);
    }

    bool UploadObj::getKeepOverLifecycle()
    {
        return mgr_.getKeepOverLifecycle(id(), slot());
    }

    bool UploadObj::getAutoResume()
    {
        return mgr_.getAutoResume(id(), slot());
    }

    bool UploadObj::getAbortAllowed()
    {
        return mgr_.getAbortAllowed(id(), slot());
    }

    bool UploadObj::getPauseAllowed()
    {
        return mgr_.getPauseAllowed(id(), slot());
    }


    void UploadObj::dbusEmitPropChange(const char *name)
    {
        if(!name)
            return;

        g_dbus_emit_property_changed(mgr_.getCtx()->conn, __getObjName(id()), DUMM_UM_OBJS, name);
    }
    /**
     * States management
     *
     * STATE_INIT -> STATE_WAITING
     * STATE_INIT -> STATE_ERROR : maybe it is impossible to create file or something ?
     *
     * STATE_WAITING -> STATE_ACTIVE
     * STATE_WAITING -> STATE_ABORTED
     * STATE_WAITING -> STATE_ERROR : for example we were not able to run downlaoding
     *
     * STATE_ACTIVE -> STATE_INTERRUPTED : some external inttr comes
     * STATE_ACTIVE -> STATE_PAUSED : user pause
     * STATE_ACTIVE -> STATE_ABORTED : user
     * STATE_ACTIVE -> STATE_ERROR : some error occurred,
     * STATE_ACTIVE -> STATE_FINISHED : done
     *
     * STATE_INTERRUPTED -> STATE_ACTIVE ??
     * STATE_INTERRUPTED -> STATE_ERROR ??
     *
     * STATE_PAUSED -> STATE_ACTIVE
     * STATE_PAUSED -> STATE_ABORTED
     * STATE_PAUSED -> STATE_ERROR ??
     *
     * STATE_ABORTED : end state
     *
     * STATE_ERROR : end state
     *
     * STATE_FINISHED : end state
     *
     * STATE_MAX : so there is a wrong change or internal error
     *
     */
    static
    UploadStateT __stateChangeCheck(UploadStateT oldState, UploadStateT newState)
    {
        UploadStateT res = STATE_MAX;

        //logger(LOG_INFO, "%s (%s, %s)\n", __PRETTY_FUNCTION__, UploadStateToStr(oldState), UploadStateToStr(newState));
        switch(oldState)
        {
            case STATE_INIT:
                switch(newState)
                {
                    case STATE_INIT:    //do nothing
                    case STATE_WAITING:
                    case STATE_ERROR:
                    case STATE_ABORTED:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_WAITING:
                switch(newState)
                {
                    case STATE_WAITING:
                    case STATE_ACTIVE:
                    case STATE_ABORTED:
                    case STATE_ERROR:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_ACTIVE:
                switch(newState)
                {
                    case STATE_WAITING:
                    case STATE_ACTIVE:
                    case STATE_ABORTED:
                    case STATE_ERROR:
                    case STATE_INTERRUPTED:
                    case STATE_FINISHED:
                    case STATE_PAUSED:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_INTERRUPTED:
                switch(newState)
                {
                    case STATE_INTERRUPTED:
                    case STATE_ACTIVE:
                    case STATE_ERROR:
                    case STATE_ABORTED:
                    case STATE_WAITING:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_PAUSED:
                switch(newState)
                {
                    case STATE_ACTIVE:
                    case STATE_ABORTED:
                    case STATE_ERROR:
                    case STATE_PAUSED:
                    case STATE_WAITING:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_ABORTED:
                switch(newState)
                {
                    case STATE_ABORTED:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_ERROR:
                switch(newState)
                {
                    case STATE_ERROR:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            case STATE_FINISHED:
                switch(newState)
                {
                    case STATE_FINISHED:
                        res = newState;
                        break;
                    default:
                        res = STATE_MAX;
                }
                break;

            default:
                res = STATE_MAX;
        }

        //logger(LOG_INFO, "\tret state: %s\n", UploadStateToStr(res));
        return res;
    }

    UploadStateT UploadObj::uploadChangeState(UploadStateT oldState, UploadStateT newState)
    {
        UploadStateT ret = STATE_MAX;
        do
        {
            ret = __stateChangeCheck(oldState, newState);
            if(ret >= STATE_MAX)
                break;

            if(!mgr_.setState(id(), slot(), ret))
            {
                ret = STATE_MAX;
                break;
            }
        }while(0);

        return ret;
    }

    DummErrorT UploadObj::handleFd(UploadStateT state, bool create)
    {
        //logger(LOG_INFO, "%s (%s)\n", __PRETTY_FUNCTION__, UploadStateTOBoStr(state));
        DummErrorT err = DUMM_ERR_FAIL;

        switch(state)
        {
            case STATE_ACTIVE:
            case STATE_PAUSED:
                if(!fd_)
                {
                    std::string dir = mgr_.getStoragePath(id(), slot());
                    std::string file = mgr_.getFilename(id(), slot());

                    logger(LOG_INFO, "File name : %s\n", getPath(file, dir).c_str());

                    if(!isFile(getPath(file, dir)))
                    {
                    	logger(LOG_ERROR, "Cannot find file : %s \n", getPath(file, dir).c_str());
                    	err = DUMM_ERR_UM_FILE_NOT_FOUND;
                    	break;
                    }

                    fd_ = fopen(getPath(file, dir).c_str(), "rb");
                    if(!fd_)
                    {
                        logger(LOG_ERROR, "Cannot open file\n");
                        err = DUMM_ERR_FS_FILE_OPEN;
                        break;
                    }
                    /*
                    long curr = ftell(fd_);
                    rewind(fd_);
                    if(flock(fileno(fd_),  LOCK_EX | LOCK_NB) < 0)
                    {
                        logger(LOG_ERROR, "File seems to be locked\n");
                        fclose(fd_);
                        fd_ = 0;
                        err = DUMM_ERR_FS_FILE_IS_LOCKED;
                        break;
                    }
                    fseek(fd_, curr, SEEK_SET);
                    */
                    if(getUploadSize() > 0)
                    {
                    	if(getUploadSize() < fileSize(fileno(fd_)))
                    	{
                    		fseek(fd_, getUploadSize(), SEEK_SET);
                    	}
                    	else
                    	{
                    		//sth is wrong ???
                    		setUploadSize(0);
                    	}
                    }
                }
                return DUMM_ERR_OK;

            default:
                //here we have to close the fd
                if(fd_)
                {
#if 0
                    if(flock(fileno(fd_), LOCK_UN) < 0)
                    {
                        logger(LOG_ERROR, "Cannot unlock file\n");
                    }
                    fflush(fd_);
#endif
                    fclose(fd_);
                    fd_ = 0;
                }
                return DUMM_ERR_OK;
        }
        return err;
    }

    UploadStateT UploadObj::changeState(UploadStateT newState)
    {
        UploadStateT oldState =  getState();
        UploadStateT s = uploadChangeState(oldState, newState);

        if( s != STATE_MAX)
        {
            DummErrorT err = handleFd(s, oldState == STATE_WAITING);
            if(err != DUMM_ERR_OK)
            {
                s = STATE_ERROR;
                if(!mgr_.setState(id(), slot(), STATE_ERROR))
                    s = STATE_MAX;

                //TODO: store the error code
            }
        }

        if(s != STATE_MAX)
        {
            unsigned int objId = id();
            const char *objState = UploadStateToStr(s);

            g_dbus_emit_signal(mgr_.getCtx()->conn,
                               DUMM_UM_PATH,
                               DUMM_UM_SERVICE,
                               "umObjectStateChange",
                               DBUS_TYPE_UINT32, &objId,
                               DBUS_TYPE_STRING, &objState,
                               DBUS_TYPE_INVALID);

        }

        return s;
    }

    UploadStateT UploadObj::uploadPause()
    {
        return changeState(STATE_PAUSED);
    }

    UploadStateT UploadObj::uploadStart()
    {
        if(getState() == STATE_INIT)
            return changeState(STATE_WAITING);

        return changeState(STATE_ACTIVE);
    }

    UploadStateT UploadObj::uploadResume()
    {
        return changeState(STATE_ACTIVE);
    }


    UploadStateT UploadObj::uploadAbort()
    {
        return changeState(STATE_ABORTED);
    }

    UploadStateT UploadObj::uploadError()
    {
        return changeState(STATE_ERROR);
    }

    UploadStateT UploadObj::uploadFinish()
    {
        return changeState(STATE_FINISHED);
    }


}
