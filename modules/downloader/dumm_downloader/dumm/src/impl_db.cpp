/**
 *
 *  Download Upload Messaging Manager
 *
 *  Copyright (C) 2012-2013  Open-RnD Sp. z o.o. All rights reserved.
 * @verbatim
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
 * @endverbatim

 * @file curl-headers.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 * @brief __BRIEF_HERE_TBD__
 *
 * __DETAILS__
 */

#ifdef HAVE_CONFIG_H
#include <dumm-config.h>
#endif
#include <cassert>
#include "impl_db.h"

#include "openrnd-exceptions.h"
#include "files.h"
#include "dumm-logger.h"


using namespace std;

namespace dumm
{
    namespace DB_Detail
    {

/**
 * @brief Helper class for initializaing SQLite
 */
class SqliteInit
{
    public:
        static bool get();
        ~SqliteInit();
        static void release();

    private:
        SqliteInit();

    private:
        static SqliteInit *hnd_;
        static int cnt_;
};

SqliteInit * SqliteInit::hnd_ = 0;
int SqliteInit::cnt_ = 0;

bool SqliteInit::get()
{
    if(!hnd_)
    {
        hnd_ = new SqliteInit();
        if(sqlite3_initialize() == SQLITE_OK)
            cnt_++;
        else
        {
            delete hnd_;
            hnd_ = 0;
        }
    }
    else
        cnt_++;

    return hnd_ != 0;
}

void SqliteInit::release()
{
    if(cnt_ == 0)
        return;

    if(cnt_ > 1)
    {
        cnt_ --;
        return;
    }

    sqlite3_shutdown();
    cnt_ = 0;
}

SqliteInit::SqliteInit()
{}

SqliteInit::~SqliteInit()
{}

struct SqliteSlot : public DbImplSlot
{
    SqliteSlot()
        : stmt(NULL)
        {
        }

    ~SqliteSlot()
        {
            stmt = NULL;
        }

    void finalize()
        {
            if (stmt)
            {
                sqlite3_finalize(stmt);
                stmt = NULL;
            }
        }

    static SqliteSlot *fromDbSlot(DbImplSlot *slot)
        {
            SqliteSlot *ss = dynamic_cast<SqliteSlot*>(slot);
            assert(ss);
            return ss;
        }

    bool valid ()
        {
            return stmt != NULL;
        }

    sqlite3_stmt *stmt;
};

//////////////////////////

DbImpl::DbImpl(const std::string &filename, const std::string &dir, unsigned int flags)
    : db_(0),
      m_(),
      dir_(dir),
      filename_(filename),
      flags_(flags)
{
    if(!SqliteInit::get())
        RUNTIME_THROW_FUN();

    try
    {
        ARG_THROW_TEST((!filename_.empty()));
        if(dir_.empty())
            //dir_.assign("/usr/share/dumm");
            dir_.assign("/mnt/dumm");

        //create folder
        if(!isDir(dir_))
            makeDir(dir_);

        if(flags_ == 0)
        {
            if(sqlite3_open(getPath(filename_, dir_).c_str(), &db_) != SQLITE_OK)
                RUNTIME_THROW_FUN();
        }
        else
        {
            int f = 0;

            if(flags_ & DB_FLAG_RDONLY)
                f |= SQLITE_OPEN_READONLY;
            if(flags_ & DB_FLAG_RDWR)
                f |= SQLITE_OPEN_READWRITE;
            if(flags_ & DB_FLAG_CREATE)
                f |= SQLITE_OPEN_CREATE;

            if(sqlite3_open_v2(getPath(filename_, dir_).c_str(), &db_, f, NULL ) != SQLITE_OK)
                RUNTIME_THROW_FUN();
        }
    }
    catch(...)
    {
        SqliteInit::release();
        throw;
    }
}

DbImpl::~DbImpl()
{
    slots_list::iterator sit;

    for (sit = slots_.begin(); sit != slots_.end(); sit++)
    {
        finalize(*sit);
        delete *sit;
    }
    slots_.clear();

    sqlite3_close(db_);
    SqliteInit::release();
}


DbImpl::operator _mutex&()
{
    return m_;
}

bool DbImpl::transBegin()
{
    return sqlite3_exec(db_, "BEGIN", 0, 0, 0) == SQLITE_OK;
}

bool DbImpl::transEnd()
{
    return sqlite3_exec(db_, "COMMIT", 0, 0, 0) == SQLITE_OK;
}

bool DbImpl::transCancel()
{
    return sqlite3_exec(db_, "ROLLBACK", 0, 0, 0) == SQLITE_OK;
}

DbImplSlot *DbImpl::stmtNewSlot()
{
    SqliteSlot *ss = new SqliteSlot();
    slots_.push_back(ss);

    return ss;
}

void DbImpl::stmtReleaseSlot(DbImplSlot *slot)
{

    slots_list::iterator sit;
    for (sit = slots_.begin(); sit != slots_.end(); sit++)
    {
        if (*sit == slot)
        {
            slots_.erase(sit);
            break;
        }
    }

    finalize(slot);
    delete slot;
}

bool DbImpl::stmtIsValidSlot(DbImplSlot *slot)
{
    SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

    if (!ss)
        return false;
    return true;
}

bool DbImpl::prepare(const std::string &query, DbImplSlot *slot)
{
    bool ret = false;

    do
    {
        if(query.empty())
            break;

        SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

        if (!ss)
            break;

        if (ss->stmt)
        {
            logger(LOG_ERROR, "statement already allocated");
            break;
        }

        sqlite3_stmt *stmt = NULL;
        // printf("SQL: %s\n", query.c_str());
        if(sqlite3_prepare(db_, query.c_str(), query.size(), &stmt, NULL) != SQLITE_OK)
            break;

        ss->stmt = stmt;

        ret = true;
    }while(0);

    return ret;
}

void DbImpl::finalize(DbImplSlot *slot)
{
    SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

    assert(ss);
    if (ss)
        ss->finalize();
}

DbImpl::ErrorT DbImpl::step(DbImplSlot *slot)
{
    ErrorT err = ERROR;

    do
    {
        SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

        if (!ss || ss->valid() == false)
            break;

        int rc = sqlite3_step(ss->stmt);
        //printf("RC: %d\n", rc);
        if(rc == SQLITE_ROW)
            err = ROW;
        else if(rc == SQLITE_DONE)
            err = DONE;
    }while(0);

    return err;
}

DbImpl::ErrorT DbImpl::simple(const std::string &query)
{
    ErrorT err = ERROR;
    DbImplSlot *slot = NULL;
    do
    {
        if(query.empty())
            break;

        slot = stmtNewSlot();
        if(!prepare(query, slot))
            break;
        err = step(slot);
    }while(0);

    if(slot)
        stmtReleaseSlot(slot);

    return err;
}

int DbImpl::colAsInt(DbImplSlot *slot, int col)
{
    SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

    if (!ss || ss->valid() == false)
        return 0;

    //TODO: should we check sqlite3_column_count()
    return sqlite3_column_int(ss->stmt, col);
}

std::string DbImpl::colAsStr(DbImplSlot *slot, int col)
{
    SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

    if (!ss && ss->valid() == false)
        return "";

    const char * p = (const char*)sqlite3_column_text(ss->stmt, col);
    string r;
    if(p) r.assign(p);
    return r;
}

double DbImpl::colAsDouble(DbImplSlot *slot, int col)
{
    SqliteSlot *ss = SqliteSlot::fromDbSlot(slot);

    if (!ss && ss->valid() == false)
        return 0;

    return sqlite3_column_double(ss->stmt, col);
}

bool DbImpl::getLastInsId(int &id)
{
    bool ret = false;
    DbImplSlot *slot = NULL;

    do
    {
        slot = stmtNewSlot();
        string q("SELECT last_insert_rowid();");
        if(!prepare(q, slot))
            break;

        if(step(slot) == DB_Detail::DbImpl::ERROR)
            break;

        id = colAsInt(slot, 0);

        ret = true;
    }while(0);

    if(slot)
        stmtReleaseSlot(slot);

    return ret;
}


    } /*namespace DB_Detail*/
} /*namespace CarSync*/

