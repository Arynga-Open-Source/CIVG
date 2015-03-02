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

#include "mgr-upload.h"
#include "obj-upload.h"
#include "dumm-db-um.h"
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
#define LOGGER_NAME "DUMM-UM-MGR"
#define LOGGER_COL "34m"
#include "dumm-logger.h"

namespace dumm
{
    MgrUpload::MgrUpload(CtxT *ctx)
        : objs_(),
          db_(0),
          ctx_(ctx),
          m_()
    {
        if(!ctx_)
            ARG_THROW_FUN();

        try
        {
            db_ = new DbUM();

            //TODO: delete all items from DB which are not valid (error, done etc)

            //read elements form database
            if(db_->queryAllOrderByCreation())
            {
                /**
                 * here we have the temp queues.
                 *
                 */
                vector<UploadObj*> objs_INIT;
                vector<UploadObj*> objs_ACTIVE;
                vector<UploadObj*> objs_PAUSED;
                vector<UploadObj*> objs_INTERRUPTED;
                vector<UploadObj*> objs_WAITING;


                while(db_->getRowNext())
                {
                    UploadObj             *obj  = 0;
                    vector<UploadObj*>    *ptrObjs;
                    UploadStateT           s;
                    DB_Detail::DbImplSlot *slot = NULL;
                    int                    id   = 0;

                    s = StrToUploadState(db_->getRowState());

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
                        logger(LOG_INFO, "NEXT ROW: id: %d, slot: %d\n", id, slot);

                        obj = new UploadObj(*this, id, slot);
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
                    ptrObjs->push_back(obj);
                }

                vector<UploadObj*> *tab_fixup[] = {
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


                vector<UploadObj*> *tab[] = {
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

            throw;
        }
    }

    MgrUpload::~MgrUpload()
    {
        for(unsigned int i = 0; i < objs_.size(); i++)
        {
            if(objs_[i])
                delete objs_[i];
            objs_[i] = 0;
        }

        delete db_;
    }

    UploadObj* MgrUpload::createObj(const char *url,
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
        UploadObj *obj  = 0;
        bool startTrans = false;

        _mutexLock lock(m_);
        printf("%s : %d\n", __FUNCTION__, __LINE__);
        do
        {
            err = DUMM_ERR_UNKNOWN;

            //TODO: check elements in the queue

            printf("%s : %d\n", __FUNCTION__, __LINE__);
            printf("%p\n", db_);
            if(!db_->transBegin())
            {
                err = DUMM_ERR_DB_INTERNAL;
                break;
            }
            printf("%s : %d\n", __FUNCTION__, __LINE__);

            startTrans = true;

            printf("%s : %d\n", __FUNCTION__, __LINE__ );
            string path = getPath(filename, storage);
            if(path.empty())
            {
            	err = DUMM_ERR_ARGUMENT;
            	break;
            }
            printf("%s : %d (%s)\n", __FUNCTION__, __LINE__, path.c_str());
            if(!isFile(path))
            {
            	err = DUMM_ERR_UM_FILE_NOT_FOUND;
            	break;
            }
            printf("%s : %d\n", __FUNCTION__, __LINE__);
            if(!db_->insert(url,
                        storage,
                        filename,
                        lifetime,
                        keep_over_lifecycle,
                        autoresume,
                        VisibilityToStr(visibility==VIS_MAX?VIS_USER:visibility),
                        UploadStateToStr(STATE_INIT),
                        id))
            {
                logger(LOG_ERROR, "Cannot insert new item\n");
                err = DUMM_ERR_DB_INTERNAL;
                break;
            }
            logger(LOG_INFO, "ID: %d\n", id);
            printf("%s : %d\n", __FUNCTION__, __LINE__);
            slot = db_->slotNew();

            try
            {
                obj = new UploadObj(*this, id, slot);
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_UM_PATH,
                                   DUMM_UM_SERVICE,
                                   "umObjectAdded",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
            }
            catch(...)
            {
                err = DUMM_ERR_UM_CANNOT_CREATE_OBJ;
                logger(LOG_ERROR, "Cannot create DM object\n");
                obj = 0;
                break;
            }

            db_->transEnd();
            objs_.push_back(obj);

            err = DUMM_ERR_OK;
        }while(0);
        printf("%s : %d\n", __FUNCTION__, __LINE__);
        if(!obj)
        {
        	printf("%s : %d\n", __FUNCTION__, __LINE__);
            if(startTrans)
                db_->transCancel();

            db_->slotRelease(slot);
        }

        return obj;
    }

    void MgrUpload::releaseObj(UploadObj *obj)
    {
        _mutexLock lock(m_);

        for(unsigned int i = 0; i < objs_.size(); i++)
            if(objs_[i] == obj)
            {
                unsigned int id = obj->id();
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_UM_PATH,
                                   DUMM_UM_SERVICE,
                                   "umObjectFinalize",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
                delete objs_[i];
                objs_.erase(objs_.begin() + i);
            }
    }

    void MgrUpload::releaseObj(int id)
    {
        _mutexLock lock(m_);

        for(unsigned int i = 0; i < objs_.size(); i++)
            if(objs_[i]->id() == id)
            {
                g_dbus_emit_signal(ctx_->conn,
                                   DUMM_UM_PATH,
                                   DUMM_UM_SERVICE,
                                   "umObjectFinalize",
                                   DBUS_TYPE_UINT32, &id,
                                   DBUS_TYPE_INVALID);
                delete objs_[i];
                objs_.erase(objs_.begin() + i);
            }
    }

    UploadObj* MgrUpload::getObj(int id)
    {
        for(unsigned int i = 0; i < objs_.size(); i++)
            if(objs_[i]->id() == id)
            {
                return objs_[i];
            }
        return 0;
    }

    UploadObj* MgrUpload::getNextObj()
    {
        UploadObj *obj = 0;

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

                //let's reissue anu iterrupted task
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


    int MgrUpload::getObjsSize()
    {
        return objs_.size();
    }

    void MgrUpload::release(int id, DB_Detail::DbImplSlot *slot)
    {
        if(getObj(id))
            db_->slotRelease(slot);
    }

    /**
     *
     * TODO: We can remove slot parameter
     */
    UploadStateT MgrUpload::getState(int id, DB_Detail::DbImplSlot *slot)
    {
        UploadStateT state = STATE_MAX;

        do
        {
            if(!db_->slotIsValid(slot))
                break;

            string s;
            if(!db_->getUploadState(id, s))
                break;

            state = StrToUploadState(s.c_str(), true);
        }while(0);

        return state;
    }

    bool MgrUpload::setState(int id, DB_Detail::DbImplSlot *slot, UploadStateT state)
    {
        bool ret = false;

        do
        {
            if(state >= STATE_MAX)
                break;

            if(!db_->slotIsValid(slot))
                break;

            ret = db_->setUploadState(id, UploadStateToStr(state));
        }while(0);

        return ret;
    }

    std::string MgrUpload::getStoragePath(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getStoragePath(id, ret);
        return ret;
    }

    std::string MgrUpload::getFilename(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getFilename(id, ret);
        return ret;
    }

    int MgrUpload::getUploadSize(int id, DB_Detail::DbImplSlot *slot)
    {
    	int ret = -1;
    	db_->getUploadSize(id, ret);
    	return ret;
    }

    bool MgrUpload::setUploadSize(int id, DB_Detail::DbImplSlot *slot, int size)
    {
    	return db_->setUploadSize(id, size);
    }

    std::string MgrUpload::getUrl(int id, DB_Detail::DbImplSlot *slot)
    {
        string ret;
        db_->getUrl(id, ret);
        return ret;
    }

    bool MgrUpload::setHttpError(int id, DB_Detail::DbImplSlot *slot, int err)
    {
        return db_->setHttpError(id, err);
    }

    int MgrUpload::getHttpError(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getHttpError(id, ret);
        return ret;
    }

    bool MgrUpload::setUploadTime(int id, DB_Detail::DbImplSlot *slot)
    {
        return db_->setUploadTime(id);
    }

    int MgrUpload::getUploadTime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = -1;
        db_->getUploadTime(id, ret);
        return ret;
    }

    VisibilityT MgrUpload::getVisibility(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string s;
        VisibilityT ret = VIS_USER;
        db_->getVisibility(id, s);
        if(!s.empty())
            ret = StrToVisibility(s.c_str());
        return ret;
    }

    std::string MgrUpload::getDescription(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getUploadDescription(id, ret);
        return ret;
    }

    bool MgrUpload::setDescription(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setUploadDescription(id, val);
    }

    int MgrUpload::getLifetime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getLifeTime(id, ret);
        return ret;
    }

    int MgrUpload::getCreationTime(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getCreationTime(id, ret);
        return ret;
    }

    int MgrUpload::getProxyPort(int id, DB_Detail::DbImplSlot *slot)
    {
        int ret = 0;
        db_->getProxyPort(id, ret);
        return ret;
    }

    bool MgrUpload::setProxyPort(int id, DB_Detail::DbImplSlot *slot, int val)
    {
        return db_->setProxyPort(id, val);
    }

    std::string MgrUpload::getProxyUrl(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyUrl(id, ret);
        return ret;
    }

    bool MgrUpload::setProxyUrl(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyUrl(id, val);
    }

    std::string MgrUpload::getProxyUser(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyUser(id, ret);
        return ret;
    }

    bool MgrUpload::setProxyUser(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyUser(id, val);
    }

    std::string MgrUpload::getProxyPasswd(int id, DB_Detail::DbImplSlot *slot)
    {
        std::string ret;
        db_->getProxyPasswd(id, ret);
        return ret;
    }

    bool MgrUpload::setProxyPasswd(int id, DB_Detail::DbImplSlot *slot, const std::string &val)
    {
        return db_->setProxyPasswd(id, val);
    }

    bool MgrUpload::getKeepOverLifecycle(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getKeepOverLifecycle(id, ret);
        return ret;
    }

    bool MgrUpload::getAutoResume(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getAutoResume(id, ret);
        return ret;
    }

    bool MgrUpload::getAbortAllowed(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getAbortAllowed(id, ret);
        return ret;
    }

    bool MgrUpload::getPauseAllowed(int id, DB_Detail::DbImplSlot *slot)
    {
        bool ret = false;
        db_->getPauseAllowed(id, ret);
        return ret;
    }

}
