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

#include <dumm-config.h>

#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "mgr-download.h"
#include "obj-download.h"
#include "dumm-db-dm.h"
#include "openrnd-exceptions.h"
#include "files.h"
#include "dumm-dbus.h"

#ifdef QNX_BUILD
#include <libgen.h>
#endif

using namespace std;

//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME "DUMM-DM-MGR"
#define LOGGER_COL "34m"
#include "dumm-logger.h"

namespace dumm
{
    MgrDownload::MgrDownload(CtxT *ctx)
        : objs_(),
          db_(0),
          ctx_(ctx),
          m_()
    {
        if(!ctx_)
            ARG_THROW_FUN();

        try
        {
            db_ = new DbDM();

            //TODO: delete all items from DB which are not valid (error, done etc)

            //read elements form database
            if(db_->queryAllOrderByCreation())
            {
                /**
                 * here we have the temp queues.
                 *
                 */
                vector<DownloadObj*> objs_INIT;
                vector<DownloadObj*> objs_ACTIVE;
                vector<DownloadObj*> objs_PAUSED;
                vector<DownloadObj*> objs_INTERRUPTED;
                vector<DownloadObj*> objs_WAITING;


                while(db_->getRowNext())
                {
                    DownloadObj           *obj  = 0;
                    vector<DownloadObj*>  *ptrObjs;
                    DownloadStateT         s;
                    DB_Detail::DbImplSlot *slot = NULL;
                    int                    id   = 0;

                    s = StrToDownloadState(db_->getRowState());

                    logger(LOG_INFO, "NEXT ROW: %s\n", db_->getRowState().c_str());
                    /**
                     * TODO: Here we should make some more computation to check if particular
                     * file can be downloaded after reset, if timelife it good etc.
                     * Currently we do this simple.
                     */
                    switch(s)
                    {
                        case STATE_WAITING: //so we put element to queue, but we haven't got time for processingit
                                            //should we check the life time ?
                            ptrObjs = &objs_WAITING;
                            break;

                        case STATE_ACTIVE: //well application should have been terminated
                            ptrObjs = &objs_ACTIVE;
                            break;
                        case STATE_PAUSED:
                            ptrObjs = &objs_PAUSED;
                            break;

                        case STATE_INTERRUPTED:
                            ptrObjs = &objs_INTERRUPTED;
                            break;

                        case STATE_INIT:
                            ptrObjs = &objs_INIT;
                            break;

                        default:
                        {
                            //TODO: delete all old items, becuase we restarted
                            int id = db_->getRowId();
                            db_->delItem(id);
                            s = STATE_MAX;
                        }
                    }

                    if(s >= STATE_MAX)
                        continue; //get next row

                    //create object
                    try
                    {
                        slot = db_->slotNew();
                        id = db_->getRowId();
                        logger(LOG_INFO, "NEXT ROW: id: %d, slot: %p\n", id, slot);

                        obj = new DownloadObj(*this, id, slot);
                    }
                    catch(...)
                    {
                        //currently we go to next row, and skipp such element.
                        //TODO: Should we mark it as invalid ?

                        if(slot >= 0)
                            db_->slotRelease(slot);
                        continue;
                    }

                    //add object to end to proper temp queue
                    if (obj->getAutoResume()) {
                        ptrObjs->push_back(obj);
                    } else {
                        delete obj;
                    }
                }

                vector<DownloadObj*> *tab_fixup[] = {
                        &objs_ACTIVE,
                        &objs_PAUSED,
                        &objs_INTERRUPTED,
                };

                //fixup the initial state. Becuase we are runnig from beginning there is no
                //active objects
                for(unsigned int i = 0; i < sizeof(tab_fixup) / sizeof(tab_fixup[0]); i++)
                    for(unsigned int j = 0; j < tab_fixup[i]->size(); j++)
                    {
                        (*tab_fixup[i])[j]->changeState(STATE_WAITING);
                    }


                vector<DownloadObj*> *tab[] = {
                        &objs_ACTIVE,
                        &objs_PAUSED,
                        &objs_INTERRUPTED,
                        &objs_WAITING,
                        &objs_INIT
                };

                for(unsigned int i = 0; i < sizeof(tab) / sizeof(tab[0]); i++)
                    for(unsigned int j = 0; j < tab[i]->size(); j++)
                    {
                        objs_.push_back((*tab[i])[j]);
                    }
            }
        }
        catch(...)
        {
            for(unsigned int i = 0; i < objs_.size(); i++)
                delete objs_[i];

            if(db_)
                delete db_;
        }
    }

    MgrDownload::~MgrDownload()
    {
        for(unsigned int i = 0; i < objs_.size(); i++)
        {
            if(objs_[i])
                delete objs_[i];
            objs_[i] = 0;
        }

        delete db_;
    }

    DownloadObj* MgrDownload::createObj(const char *url,
            const char *storage,
            const char *filename,
            int lifetime,
            bool keep_over_lifecycle,
            bool autoresume,
            VisibilityT visibility,
            DummErrorT &err)
    {
        int id = -1;
        DB_Detail::DbImplSlot *slot = NULL;
        DownloadObj *obj  = 0;
        bool startTrans = false;

        _mutexLock lock(m_);

        do
        {
            err = DUMM_ERR_UNKNOWN;

            //TODO: check elements in the queue

            if(!db_->transBegin())
            {
                err = DUMM_ERR_DB_INTERNAL;
                break;
            }


            startTrans = true;

            const char *dir = storage;
            if(!dir || strlen(dir) < 1)
            {
                //get default dir
                dir = ctx_->conf->getDownloadPath();
            }

            //create the unique file if we do not have
            //the filename from user
            string fm;

            if(filename && strlen(filename) > 1)
                fm = filename;
            else
            {
                //TODO: it may happen that between tempnam() and fopen()
                //somebody else creates file
                char *tname = tempnam(dir, "dumm");
                if(!tname)
                {
                    err = DUMM_ERR_FS_CREATE_FILE;
                    break;
                }
                fm = basename(tname);
                free(tname);

                FILE *f = fopen(getPath(fm, dir).c_str(), "a+");
                if(!f)
                {
                    err = DUMM_ERR_FS_CREATE_FILE;
                    break;
                }
                fclose(f);
            }

            if(!db_->insert(url,
                        dir,
                        fm.c_str(),
                        lifetime,
                        keep_over_lifecycle,
                        autoresume,
                        VisibilityToStr(visibility==VIS_MAX?VIS_USER:visibility),
                        DownloadStateToStr(STATE_INIT),
                        id))
            {
                logger(LOG_ERROR, "Cannot insert new item\n");
                err = DUMM_ERR_DB_INTERNAL;
                break;
            }
            logger(LOG_INFO, "ID: %d\n", id);

            slot = db_->slotNew();

            try
            {
                obj = new DownloadObj(*this, id, slot);
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_DM_PATH,
                                   DUMM_DM_SERVICE,
                                   "dmObjectAdded",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
            }
            catch(...)
            {
                err = DUMM_ERR_DM_CANNOT_CREATE_OBJ;
                logger(LOG_ERROR, "Cannot create DM object\n");
                obj = 0;
                break;
            }
#if 0
            //the object is in INIT state, so we should now move it to waiting
            if(!obj->downloadChangeState(STATE_INIT, STATE_WAITING))
            {
                err = DUMM_ERR_DM_CANNOT_CREATE_OBJ;
                logger(LOG_ERROR, "Cannot change state to waiting\n");
                delete obj;
                obj = 0;
                break;
            }
#endif
            //DUMM_DM_OBJS


            db_->transEnd();
            objs_.push_back(obj);

            err = DUMM_ERR_OK;
        }while(0);

        if(!obj)
        {
            if(startTrans)
                db_->transCancel();

            db_->slotRelease(slot);
        }

        return obj;
    }

    void MgrDownload::releaseObj(DownloadObj *obj)
    {
        _mutexLock lock(m_);

        for(unsigned int i = 0; i < objs_.size(); i++)
        {
            if(objs_[i] == obj)
            {
                unsigned int id = obj->id();
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_DM_PATH,
                                   DUMM_DM_SERVICE,
                                   "dmObjectFinalize",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
                delete objs_[i];
                objs_.erase(objs_.begin() + i);
                break;
            }
        }
    }

    void MgrDownload::releaseObj(int id)
    {
        _mutexLock lock(m_);

        for(unsigned int i = 0; i < objs_.size(); i++)
        {
            if(objs_[i]->id() == id)
            {
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_DM_PATH,
                                   DUMM_DM_SERVICE,
                                   "dmObjectFinalize",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
                delete objs_[i];
                objs_.erase(objs_.begin() + i);
                break;
            }
        }
    }

    DownloadObj* MgrDownload::getObj(int id)
    {
        for(unsigned int i = 0; i < objs_.size(); i++)
            if(objs_[i]->id() == id)
            {
                return objs_[i];
            }
        return 0;
    }

    DownloadObj* MgrDownload::getNextObj()
    {
        DownloadObj *obj = 0;

        _mutexLock lock(m_);

        for(unsigned int i = 0; i < objs_.size(); i++)
        {
            //if(objs_[i]->tryLock()) //TODO: Should we use timed here ?
            objs_[i]->lock();
            {
                if(objs_[i]->getState() == STATE_WAITING && !objs_[i]->isUsed())
                {
                    obj = objs_[i];
                    obj->used();
                    break;
                }

                //let's reissue an iterrupted task
                if(objs_[i]->getState() == STATE_INTERRUPTED && !objs_[i]->isUsed())
                {
                    obj = objs_[i];
                    obj->used();
                    break;
                }
                objs_[i]->unlock();
            }
        }

        return obj;
    }

    int MgrDownload::getObjsSize()
    {
        return objs_.size();
    }

    void MgrDownload::release(int id, DB_Detail::DbImplSlot *slot)
    {
        if(getObj(id))
            db_->slotRelease(slot);
    }

    /**
     *
     * TODO: We can remove slot parameter
     */
    DownloadStateT MgrDownload::getState(int id, DB_Detail::DbImplSlot *slot)
    {
        DownloadStateT state = STATE_MAX;

        do
        {
            if(!db_->slotIsValid(slot))
                break;

            string s;
            if(!db_->getDownloadState(id, s))
                break;

            state = StrToDownloadState(s.c_str(), true);
        }while(0);

        return state;
    }

    bool MgrDownload::setState(int id, DB_Detail::DbImplSlot *slot, DownloadStateT state)
    {
        bool ret = false;

        do
        {
            if(state >= STATE_MAX)
                break;

            if(!db_->slotIsValid(slot))
                break;

            ret = db_->setDownloadState(id, DownloadStateToStr(state));
        }while(0);

        return ret;
    }

    std::string MgrDownload::getStoragePath(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getStoragePath(id, ret);
        return ret;
    }

    std::string MgrDownload::getFilename(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getFilename(id, ret);
        return ret;
    }

    int MgrDownload::getTotalSize(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = -1;
        db_->getTotalSize(id, ret);
        return ret;
    }

    bool MgrDownload::setTotalSize(int id, DB_Detail::DbImplSlot *slot, int size)
    {
        return db_->setTotalSize(id, size);
    }

    std::string MgrDownload::getUrl(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getUrl(id, ret);
        return ret;
    }

    bool MgrDownload::setHttpError(int id, DB_Detail::DbImplSlot *slot, int err)
    {
        return db_->setHttpError(id, err);
    }

    int MgrDownload::getHttpError(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getHttpError(id, ret);
        return ret;
    }

    bool MgrDownload::setDownloadTime(int id, DB_Detail::DbImplSlot *slot)
    {
        return db_->setDownloadTime(id);
    }

    int MgrDownload::getDownloadTime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = -1;
        db_->getDownloadTime(id, ret);
        return ret;
    }

    VisibilityT MgrDownload::getVisibility(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string s;
        VisibilityT ret = VIS_USER;
        db_->getVisibility(id, s);
        if(!s.empty())
            ret = StrToVisibility(s.c_str());
        return ret;
    }

    std::string MgrDownload::getDescription(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getDownloadDescription(id, ret);
        return ret;
    }

    bool MgrDownload::setDescription(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setDownloadDescription(id, val);
    }

    int MgrDownload::getLifetime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getLifeTime(id, ret);
        return ret;
    }

    int MgrDownload::getCreationTime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getCreationTime(id, ret);
        return ret;
    }

    int MgrDownload::getProxyPort(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getProxyPort(id, ret);
        return ret;
    }

    bool MgrDownload::setProxyPort(int id, DB_Detail::DbImplSlot *slot, int val)
    {
        return db_->setProxyPort(id, val);
    }

    std::string MgrDownload::getProxyUrl(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyUrl(id, ret);
        return ret;
    }

    bool MgrDownload::setProxyUrl(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyUrl(id, val);
    }

    std::string MgrDownload::getProxyUser(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyUser(id, ret);
        return ret;
    }

    bool MgrDownload::setProxyUser(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyUser(id, val);
    }

    std::string MgrDownload::getProxyPasswd(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyPasswd(id, ret);
        return ret;
    }

    bool MgrDownload::setProxyPasswd(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyPasswd(id, val);
    }

    bool MgrDownload::getKeepOverLifecycle(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getKeepOverLifecycle(id, ret);
        return ret;
    }

    bool MgrDownload::getAutoResume(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getAutoResume(id, ret);
        return ret;
    }

    bool MgrDownload::getAbortAllowed(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getAbortAllowed(id, ret);
        return ret;
    }

    bool MgrDownload::getPauseAllowed(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getPauseAllowed(id, ret);
        return ret;
    }

}
