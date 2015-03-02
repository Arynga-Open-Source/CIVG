/**
 *
 *  Download Upload Messaging Manager
 *
 *  Copyright (C) 2012-2013  Open-RnD Sp. z o.o. All rights reserved.
 *
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
 *
 * @file impl_db.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef __impl_db_h__
#define __impl_db_h__

#include <string>
#include <list>
#include <sqlite3.h>

#include "openrnd-mutex.h"

namespace dumm
{
    namespace DB_Detail
    {

        struct DbImplSlot
        {
            virtual ~DbImplSlot() {}
            // empty class
        };

        class DbImpl
        {
            public:
                enum ErrorT
                {
                    OK = 0,
                    DONE,
                    ROW,
                    ERROR
                };

            #define DB_FLAG_RDONLY      (0x01)
            #define DB_FLAG_RDWR        (0x01 << 1)
            #define DB_FLAG_CREATE      (0x01 << 2)

            public:
                DbImpl(const std::string &filename, const std::string &dir = "", unsigned int flags = 0);
                virtual ~DbImpl();

                /**
                 * @brief Transaction begin
                 */
                bool transBegin();

                /**
                 * @brief Transaction commit/end
                 */
                bool transEnd();

                /**
                 * @brief Transaction cancel/roolback
                 */
                bool transCancel();

                /**
                 * @brief Create new slot for handling SQL queries
                 */
                DbImplSlot *stmtNewSlot();

                /**
                 * @brief Delete slot for handling SQL queries
                 *
                 * @param slot [in] Slot ID
                 */
                void stmtReleaseSlot(DbImplSlot *slot);

                /**
                 * @brief Check if slot is valid
                 *
                 * @param slot [in] Slot ID
                 */
                bool stmtIsValidSlot(DbImplSlot *slot);


                bool prepare(const std::string &query, DbImplSlot *slot);
                void finalize(DbImplSlot *slot);
                ErrorT step(DbImplSlot *slot);
                ErrorT simple(const std::string &query);

                int colAsInt(DbImplSlot *slot, int col);
                std::string colAsStr(DbImplSlot *slot, int col);
                double colAsDouble(DbImplSlot *slot, int col);

                operator _mutex&();

                /**
                 * @brief Return last inserted id
                 *
                 * @param id [out] Id value
                 *
                 * @retval true on success
                 * @retval false on fail
                 */
                bool getLastInsId(int &id);


            private:
                DbImpl(const DbImpl&);
                DbImpl& operator=(const DbImpl&);



            private:
                sqlite3 *db_;
                _mutex   m_;
                std::string dir_;
                std::string filename_;
                typedef std::list<DbImplSlot*> slots_list;
                slots_list slots_;
                unsigned int flags_;
        };
    };
};

#endif /*__impl_db_h__*/
