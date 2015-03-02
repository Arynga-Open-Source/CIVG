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

#include "thbc.h"

namespace dumm
{

    bool defCmpThBcIntFlag(int val)
    {
        return val == 0;
    }
    ///////////////////////////////////////////////////////
    ThBc::ThBc()
        : condCond_(PTHREAD_COND_INITIALIZER),
          condMutex_(PTHREAD_MUTEX_INITIALIZER)
    {}

    ThBc::~ThBc()
    {}

    void ThBc::lock()
    {
        pthread_mutex_lock(&condMutex_);
    }

    void ThBc::unlock()
    {
        pthread_mutex_unlock(&condMutex_);
    }

    void ThBc::wait()
    {
        while(waitCond())
            pthread_cond_wait(&condCond_, &condMutex_);
    }

    void ThBc::wait2()
    {
        lock();
        wait();
        unlock();
    }

    void ThBc::broadcast()
    {
        pthread_cond_broadcast(&condCond_);
    }

    void ThBc::broadcast2()
    {
        lock();
        broadcast();
        unlock();
    }

    ///////////////////////////////////////////////////////
    ThBcLocker::ThBcLocker(ThBc &m)
        : m_(m)
    {
        m_.lock();
    }

    ThBcLocker::~ThBcLocker()
    {
        m_.unlock();
    }

    ///////////////////////////////////////////////////////
    ThBcIntFlag::ThBcIntFlag(funCmpThBcIntFlag fun)
        : ThBc(),
          flag_(0),
          fun_(fun)
    {}

    ThBcIntFlag::~ThBcIntFlag()
    {}

    bool ThBcIntFlag::waitCond()
    {
        if(fun_)
            return fun_(flag_);
        return false;
    }

    int ThBcIntFlag::get() const
    {
        return flag_;
    }

    int ThBcIntFlag::get2()
    {
        lock();
        int ret = flag_;
        unlock();
        return ret;
    }

    void ThBcIntFlag::set(int val)
    {
        flag_ = val;
    }

    void ThBcIntFlag::set2(int val)
    {
        lock();
        flag_ = val;
        unlock();
    }

    void ThBcIntFlag::broadcast(int val)
    {
        flag_ = val;
        ThBc::broadcast();
    }

    void ThBcIntFlag::broadcast2(int val)
    {
        lock();
        flag_ = val;
        ThBc::broadcast();
        unlock();
    }

    void ThBcIntFlag::broadcastInc()
    {
        flag_++;
        ThBc::broadcast();
    }

    void ThBcIntFlag::broadcastInc2()
    {
        lock();
        flag_++;
        ThBc::broadcast();
        unlock();
    }

    ThBcIntFlag& ThBcIntFlag::operator++()
    {
        broadcastInc2();
        return *this;
    }

    ThBcIntFlag& ThBcIntFlag::operator-=(int val)
    {
        flag_ -= val;
        return *this;
    }

    ThBcIntFlag& ThBcIntFlag::operator+=(int val)
    {
        flag_ += val;
        return *this;
    }
}
