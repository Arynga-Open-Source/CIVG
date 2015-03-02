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
 * @file thbc.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef THBC_H_
#define THBC_H_

#include <pthread.h>

namespace dumm
{
    /**
     * @brief Thread broadcast class
     */
    class ThBc
    {
        public:
            ThBc();
            virtual ~ThBc();

            void lock();
            void unlock();
            void wait();
            void wait2();
            virtual bool waitCond() = 0;
            void broadcast();
            void broadcast2();

        private:
            //we do not want these methods
            ThBc(const ThBc&);
            ThBc& operator=(const ThBc&);

        private:
            pthread_cond_t  condCond_;
            pthread_mutex_t condMutex_;
    };

    /**
     * @brief Helper class for autolocker
     */
    class ThBcLocker
    {
        public:
            explicit ThBcLocker(ThBc &m);
            ~ThBcLocker();

        private:
            ThBcLocker(const ThBcLocker&);
            ThBcLocker& operator=(const ThBcLocker&);

        private:
            ThBc &m_;

    };

    /** Helper function for condition checking */
    typedef bool (*funCmpThBcIntFlag)(int val);

    bool defCmpThBcIntFlag(int val);

    /** */
    class ThBcIntFlag : public ThBc
    {
        public:
            ThBcIntFlag(funCmpThBcIntFlag fun = defCmpThBcIntFlag);
            ~ThBcIntFlag();

            bool waitCond();

            int get() const;
            int get2();
            void set(int val);
            void set2(int val);

            void broadcast(int val);
            void broadcast2(int val);
            void broadcastInc();
            void broadcastInc2();

            ThBcIntFlag& operator++();

            ThBcIntFlag& operator-=(int val);
            ThBcIntFlag& operator+=(int val);
        private:
            int flag_;
            funCmpThBcIntFlag fun_;
    };
};
#endif /* THBC_H_ */
