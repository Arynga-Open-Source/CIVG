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
 * @file common-obj.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef COMMON_OBJ_H_
#define COMMON_OBJ_H_

#include <pthread.h>
#include <stdio.h>
#include <string>
#include "dumm-db-common.h"

namespace dumm
{
    class CommonObj
    {
        public:
    		/**
    		 *
    		 */
            int id() const;

            /**
             *
             */
           DB_Detail::DbImplSlot *slot() const;

    		/**
    		 *
    		 */
            bool tryLock();

    		/**
    		 *
    		 */
            bool timedLock(unsigned int sec);

    		/**
    		 *
    		 */
            void lock();

    		/**
    		 *
    		 */
            void unlock();

            /** Special flag which marks if the objects is in use */

            /**
             * @brief Mark DM object as used
             */
            bool used();

            /**
             * @brief Mark DM object as unused
             */
            bool unUsed();

            /**
             * @brief Check if DM object is in used.
             *
             * @retval true - in used
             * @retval false - not used.
             */
            bool isUsed() const;

            /**
             * @brief Get file descriptor
             */
            FILE* getFd();

            /**
             * @brief
             */
            virtual std::string getUrl() = 0;

            /**
             * @brief Set the HTTP error code
             *
             * @param error [in] Error code
             */
            virtual bool setHttpError(int error) = 0;

            /**
             * @brief Return last HTTP error code
             */
            virtual int getHttpError() = 0;

            /**
             *
             */
            virtual std::string getVisibility() = 0;

            /**
             *
             */
            virtual std::string getStoragePath() = 0;

            /**
             *
             */
            virtual std::string getFilename() = 0;

            /**
             *
             */
            virtual int getLifetime() = 0;

            /**
             *
             */
            virtual int getCreationTime() = 0;

            /**
             *
             */
            virtual int getProxyPort() = 0;

            /**
             *
             */
            virtual bool setProxyPort(int val) = 0;

            /**
             *
             */
            virtual std::string getProxyUrl() = 0;

            /**
             *
             */
            virtual bool setProxyUrl(const std::string &val) = 0;

            /**
             *
             */
            virtual std::string getProxyUser() = 0;

            /**
             *
             */
            virtual bool setProxyUser(const std::string &val) = 0;

            /**
             *
             */
            virtual std::string getProxyPasswd() = 0;

            /**
             *
             */
            virtual bool setProxyPasswd(const std::string &val) = 0;

            /**
             *
             */
            virtual bool getKeepOverLifecycle() = 0;

            /**
             *
             */
            virtual bool getAutoResume() = 0;

            /**
             *
             */
            virtual bool getAbortAllowed() = 0;

            /**
             *
             */
            virtual bool getPauseAllowed() = 0;

            /**
             *
             */
            virtual void dbusEmitPropChange(const char *name) = 0;

        protected:
            CommonObj(int id, DB_Detail::DbImplSlot *slot);
            virtual ~CommonObj();

        private:
            //prevent by using
            CommonObj(const CommonObj&);
            CommonObj& operator=(const CommonObj&);

        protected:
            FILE           *fd_;        /**< File handler */

        private:
            int                    id_;   /**< DM Object Id - unique*/
            DB_Detail::DbImplSlot *slot_; /**< Special slot for DB */
            pthread_mutex_t        m_;    /**< mutex */
            bool                   inUsed_;
    };
};

#endif /* COMMON_OBJ_H_ */

