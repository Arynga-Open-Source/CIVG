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

#include "obj-common.h"
#include "files.h"

//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME "DUMM-CM-OBJ"
#define LOGGER_COL "34m"
#include "dumm-logger.h"


namespace dumm
{
    CommonObj::CommonObj(int id, DB_Detail::DbImplSlot *slot)
        : fd_(0),
          id_(id),
          slot_(slot),
          m_(PTHREAD_MUTEX_INITIALIZER),
          inUsed_(false)
    {
    }

    CommonObj::~CommonObj()
    {
        if(fd_)
        {
            fflush(fd_);
            fclose(fd_);
        }
    }

    int CommonObj::id() const
    {
        return id_;
    }

    DB_Detail::DbImplSlot *CommonObj::slot() const
    {
    	return slot_;
    }

    bool CommonObj::tryLock()
    {
        return pthread_mutex_trylock(&m_) == 0;
    }

    bool CommonObj::timedLock(unsigned int sec)
    {
        struct timespec abs_time;

        clock_gettime(CLOCK_REALTIME , &abs_time);
        abs_time.tv_sec += sec;

        return pthread_mutex_timedlock (&m_, &abs_time) == 0;
    }

    void CommonObj::lock()
    {
        pthread_mutex_lock(&m_);
    }

    void CommonObj::unlock()
    {
        pthread_mutex_unlock(&m_);
    }


    bool CommonObj::used()
    {
        if(!inUsed_)
        {
            inUsed_ = true;
            return true;
        }
        return false;
    }

    bool CommonObj::unUsed()
    {
        if(inUsed_)
        {
            inUsed_ = false;
            return true;
        }
        return false;
    }

    bool CommonObj::isUsed() const
    {
        return inUsed_;
    }

    FILE* CommonObj::getFd()
    {
        return fd_;
    }
}
