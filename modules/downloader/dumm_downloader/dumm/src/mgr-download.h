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
 * @file download-mgr.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef MGR_DOWNLOAD_H_
#define MGR_DOWNLOAD_H_

#include "openrnd-mutex.h"
#include "dumm-dm.h"
#include "mgr-downloadi.h"


namespace dumm
{
    class DownloadObj;
}

namespace dumm
{
    class MgrDownload : public MgrDownloadI
    {
        public:
    		MgrDownload(CtxT *ctx);
            virtual ~MgrDownload();

            /**
             * @brief
             */
            DownloadObj* createObj(const char *url,
                                   const char *storage,
                                   const char *filename,
                                   int lifetime,
                                   bool keep_over_lifecycle,
                                   bool autoresume,
                                   VisibilityT visibility,
                                   DummErrorT &err);

            /**
             * @brief
             */
            void releaseObj(DownloadObj*);

            /**
             * @brief
             */
            void releaseObj(int id);

            /**
             * @brief
             */
            DownloadObj* getObj(int id);

            /**
             * RPovide object to thread
             */
            DownloadObj* getNextObj();

            int getObjsSize();


        /** MgrDownloadI implementation*/
        public:
            void release(int id, DB_Detail::DbImplSlot *slot);
            DownloadStateT getState(int id, DB_Detail::DbImplSlot *slot);
            bool setState(int id, DB_Detail::DbImplSlot *slot, DownloadStateT state);
            std::string getStoragePath(int id, DB_Detail::DbImplSlot *slot);
            std::string getFilename(int id, DB_Detail::DbImplSlot *slot);
            int getTotalSize(int id, DB_Detail::DbImplSlot *slot);
            bool setTotalSize(int id, DB_Detail::DbImplSlot *slot, int size);
            std::string getUrl(int id, DB_Detail::DbImplSlot *slot);
            bool setHttpError(int id, DB_Detail::DbImplSlot *slot, int err);
            int getHttpError(int id, DB_Detail::DbImplSlot *slot);
            bool setDownloadTime(int id, DB_Detail::DbImplSlot *slot);
            int getDownloadTime(int id, DB_Detail::DbImplSlot *slot);
            CtxT * getCtx()
                { return ctx_; };
            VisibilityT getVisibility(int id, DB_Detail::DbImplSlot *slot);
            std::string getDescription(int id, DB_Detail::DbImplSlot *slot);
            bool setDescription(int id, DB_Detail::DbImplSlot *slot, const std::string &val);
            int getLifetime(int id, DB_Detail::DbImplSlot *slot);
            int getCreationTime(int id, DB_Detail::DbImplSlot *slot);
            int getProxyPort(int id, DB_Detail::DbImplSlot *slot);
            bool setProxyPort(int id, DB_Detail::DbImplSlot *slot, int val);
            std::string getProxyUrl(int id, DB_Detail::DbImplSlot *slot);
            bool setProxyUrl(int id, DB_Detail::DbImplSlot *slot, const std::string &val);
            std::string getProxyUser(int id, DB_Detail::DbImplSlot *slot);
            bool setProxyUser(int id, DB_Detail::DbImplSlot *slot, const std::string &val);
            std::string getProxyPasswd(int id, DB_Detail::DbImplSlot *slot);
            bool setProxyPasswd(int id, DB_Detail::DbImplSlot *slot, const std::string &val);
            bool getKeepOverLifecycle(int id, DB_Detail::DbImplSlot *slot);
            bool getAutoResume(int id, DB_Detail::DbImplSlot *slot);
            bool getAbortAllowed(int id, DB_Detail::DbImplSlot *slot);
            bool getPauseAllowed(int id, DB_Detail::DbImplSlot *slot);

        private:
            MgrDownload(const MgrDownload&);
            MgrDownload& operator=(const MgrDownload&);

        private:
            std::vector<DownloadObj*> objs_;    /**< TODO: should we use map? */
            DbDM *db_;
            CtxT *ctx_;
            _mutex m_;     /** mutex */
    };
}


#endif /* DOWNLOAD_MGR_H_ */
