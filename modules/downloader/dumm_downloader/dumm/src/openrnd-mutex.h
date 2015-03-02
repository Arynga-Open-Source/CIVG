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
 * @file openrnd-mutex.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */


#ifndef __OPENRND_MUTEX_H__
#define __OPENRND_MUTEX_H__

#include <pthread.h>

/**
 * @class _mutex
 * @brief Class wrapper for system mutexes.
 * 
 * @see _mutexLock
 */
class _mutex
{
    public:
        /**
         * @brief Contructor.
         *
         * In case of errors with initializing mutex exception will be
         * thrown.
         * For more details see manual for :
         *  - pthread_mutex_init
         *  - pthread_mutex_t
         */
        _mutex();

        /**
         * @brief Desctructor.
         *  Releases and then destroys mutex.
         */
        virtual ~_mutex();

        /**
         * @brief Get (lock) mutex.
         */
        void lock();

        /**
         * @brief Release (unlock) mutex.
         */
        void unlock();

        /**
         * @brief Try to get (lock) mutex.
         *
         * @retval true Mutex is locked
         * @retval false There was a problem with locking mutex (used).
         */
        bool trylock();

    private:
        /**
         * @brief Hide it, not implemented
         */
        _mutex(const _mutex&);
        /**
         * @brief Hide it, not implemented
         */
        _mutex& operator=(const _mutex&);

    private:
        pthread_mutex_t m_;     /**< Pthread mutex object */
};

/**
 * @class _mutexLock
 * @brief Helper class that is used for automatically releasing mutexes when there are not used.
 *
 * This is helper class that can be used on top of @ref _mutex class.
 * In constructor of this class, the provided _mutex object is automatically
 * locked and automatically release in desctrutor.
 *
 * @code
 *  void my_fun(_mutex &mutex)
 *  {
 *      //automatic lock
 *      _mutexLock(mutex);
 *
 *      do_sth();
 *      //automatic release
 *  }
 *
 * int main()
 * {
 *      _mutex m;
 *      my_fun(m);
 *      return 0;
 * }
 * @endcode
 *
 * @see _mutex
 */
class _mutexLock
{
    public:
        /**
         * @brief Constructor
         *
         * @param m _mutex object that will be controlled by this class.
         *
         * Automatically lock the provided mutex.
         */
        explicit _mutexLock(_mutex &m);
        ~_mutexLock();

    private:
        /**
         * @brief Hide it, not implemented
         */
        _mutexLock(const _mutexLock&);
        /**
         * @brief Hide it, not implemented
         */
        _mutexLock& operator=(const _mutexLock&);

    private:
        _mutex *m_;     /**< _mutex object that is controlling by this class*/
};


#endif
