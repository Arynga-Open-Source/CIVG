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
 * @file common-obji.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef MGR_COMMON_I_H_
#define MGR_COMMON_I_H_

#include "dumm-db-common.h"

namespace dumm
{
    /** Interface class for handling download/upload objects requests*/
    class MgrI
    {
        public:
            /**
             * TBD
             */
            virtual void release(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual MgrStateT getState(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual bool setState(int id, DB_Detail::DbImplSlot *slot, MgrStateT state) = 0;

            /**
             * TBD
             */
            virtual std::string getStoragePath(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return String with filename or empty string in case of error
             */
            virtual std::string getFilename(int id, DB_Detail::DbImplSlot *slot) = 0;


            /**
             * @brief Return URL to file
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return String with url or empty string in case of error or missing URL
             */
            virtual std::string getUrl(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Set last HTTP error
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             * @param err [in] HTTP error code to be set
             *
             * @retval true in case of success
             * @retval false in case of error
             */
            virtual bool setHttpError(int id, DB_Detail::DbImplSlot *slot, int err) = 0;

            /**
             * @brief Get last HTTP error
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @retval Error code
             */
            virtual int getHttpError(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief return CtxT pointer
             *
             * @return CtxT pointer
             */
            virtual CtxT * getCtx() = 0;

            /**
             * @brief Return visibility of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Visibility
             */
            virtual VisibilityT getVisibility(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return description string of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Description string
             */
            virtual std::string getDescription(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Set description string of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             * @param val [in] Value to be set
             *
             * @retval true in case of success
             * @retval false in case of error
             */
            virtual bool setDescription(int id, DB_Detail::DbImplSlot *slot, const std::string &val) = 0;

            /**
             * @brief Return lifetime of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Life time of object in seconds
             */
            virtual int getLifetime(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return creation time of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Creation time of object in EPOCH
             */
            virtual int getCreationTime(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return proxy port of object
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Proxy port number
             */
            virtual int getProxyPort(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual bool setProxyPort(int id, DB_Detail::DbImplSlot *slot, int val) = 0;

            /**
             * TBD
             */
            virtual std::string getProxyUrl(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual bool setProxyUrl(int id, DB_Detail::DbImplSlot *slot, const std::string &val) = 0;

            /**
             * TBD
             */
            virtual std::string getProxyUser(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual bool setProxyUser(int id, DB_Detail::DbImplSlot *slot, const std::string &val) = 0;

            /**
             * TBD
             */
            virtual std::string getProxyPasswd(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * TBD
             */
            virtual bool setProxyPasswd(int id, DB_Detail::DbImplSlot *slot, const std::string &val) = 0;

            /**
             * @brief Return KeepOverLifecycle flag value
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Flag value (true or false)
             */
            virtual bool getKeepOverLifecycle(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return AutoResume flag value
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Flag value (true or false)
             */
            virtual bool getAutoResume(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return AbortAllowed flag value
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Flag value (true or false)
             */
            virtual bool getAbortAllowed(int id, DB_Detail::DbImplSlot *slot) = 0;

            /**
             * @brief Return PauseAllowed flag value
             *
             * @param id [in] Item ID (database ID)
             * @param slot [in] Database slot in which query will be invoked
             *
             * @return Flag value (true or false)
             */
            virtual bool getPauseAllowed(int id, DB_Detail::DbImplSlot *slot) = 0;

    };

};

#endif /* DOWNLOAD_OBJI_H_ */
