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
 * @file download-obji.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef MGR_DOWNLOAD_I_H_
#define MGR_DOWNLOAD_I_H_

#include "mgr-commoni.h"

namespace dumm
{
    /** Interface class for handling download objects requests*/
    class MgrDownloadI : public MgrI
    {
        public:

            /**
             * @brief Return total size of file (remote)
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Number of bytes or -1 in case of error
             */
            virtual int getTotalSize(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Set total size of file
             *
             * The size is retrieved from remote server.
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             * @param size [in] Total size (retrieved from server) to be stored
             *
             * @retval true in case of success
             * @retval false in case of error
             */
            virtual bool setTotalSize(int id, DB_Detail::DbImplSlot *slot, int size) = 0;

            /**
             * @brief Set download time.
             *
             * The time is taken automatically from current system time.
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @retval true in case of success
             * @retval false in case of error
             */
            virtual bool setDownloadTime(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return last download time
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Epoch when download has been started
             */
            virtual int getDownloadTime(int id, DB_Detail::DbImplSlot *slot) = 0;
    };

};

#endif /* DOWNLOAD_OBJI_H_ */
