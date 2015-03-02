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
 * @file download-obj.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef DOWNLOAD_OBJ_H_
#define DOWNLOAD_OBJ_H_

#include "obj-common.h"
#include "mgr-download.h"
#include "dumm-db-common.h"

namespace dumm
{
    class DownloadObj : public CommonObj
    {
        public:
            DownloadStateT downloadPause();
            DownloadStateT downloadStart();
            DownloadStateT downloadResume();
            DownloadStateT downloadAbort();
            DownloadStateT downloadError();
            DownloadStateT downloadFinish();


            //add listeners

            DownloadStateT getState();

            /** Special flag which marks if the objects is in use */

            /**
             * @brief Return total size of file (remote file)
             */
            int getTotalSize();

            /**
             * @brief Return size of current downloaded file
             */
            int getSize();

            /**
             * @brief
             */
            bool setTotalSize(int size);

            /**
             * @brief Set the estimate time value
             *
             * @param t [in] Estimate time in seconds
             */
            void setEstTime(int t);

            /**
             * @brief Return estimate time
             *
             * @return Estimate time value in secodns
             */
            int getEstTime() const;

            /**
             * @brief Return Download time
             */
            int getDownloadTime();

            /**
             *
             */
            std::string getDownloadDescription();

            /**
             *
             */
            bool setDownloadDescription(const std::string &val);

            /**
             * @brief
             */
            virtual std::string getUrl();

            /**
             * @brief Set the HTTP error code
             *
             * @param error [in] Error code
             */
            virtual bool setHttpError(int error);

            /**
             * @brief Return last HTTP error code
             */
            virtual int getHttpError();

            /**
             *
             */
            virtual std::string getVisibility();

            /**
             *
             */
            virtual std::string getStoragePath();

            /**
             *
             */
            virtual std::string getFilename();

            /**
             *
             */
            virtual int getLifetime();

            /**
             *
             */
            virtual int getCreationTime();

            /**
             *
             */
            virtual int getProxyPort();

            /**
             *
             */
            virtual bool setProxyPort(int val);

            /**
             *
             */
            virtual std::string getProxyUrl();

            /**
             *
             */
            virtual bool setProxyUrl(const std::string &val);

            /**
             *
             */
            virtual std::string getProxyUser();

            /**
             *
             */
            virtual bool setProxyUser(const std::string &val);

            /**
             *
             */
            virtual std::string getProxyPasswd();

            /**
             *
             */
            virtual bool setProxyPasswd(const std::string &val);

            /**
             *
             */
            virtual bool getKeepOverLifecycle();

            /**
             *
             */
            virtual bool getAutoResume();

            /**
             *
             */
            virtual bool getAbortAllowed();

            /**
             *
             */
            virtual bool getPauseAllowed();

            /**
             *
             */
            virtual void dbusEmitPropChange(const char *name);

        protected:
            DownloadObj(MgrDownloadI &mgr, int id, DB_Detail::DbImplSlot *slot);
            virtual ~DownloadObj();

            friend class MgrDownload;

            /**
             * @brief Helper
             */
            DownloadStateT downloadChangeState(DownloadStateT oldState, DownloadStateT newState);

            /**
             * @brief Helper
             */
            DummErrorT handleFd(DownloadStateT state, bool create = true);

            /**
             * @Helper
             */
            DownloadStateT changeState(DownloadStateT newState);

        private:
            //prevent by using
            DownloadObj(const DownloadObj&);
            DownloadObj& operator=(const DownloadObj&);

        private:
            MgrDownloadI    &mgr_;      /**< Interface for database */
            int             estTime_;   /**< Estimate time to finish (sec)*/
    };
};

#endif /* DOWNLOAD_OBJ_H_ */

