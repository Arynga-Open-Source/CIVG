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
 * @file openrnd-exceptions.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */


#ifndef __OPENRND_EXCEPTIONS_H__
#define __OPENRND_EXCEPTIONS_H__

#include <stdexcept>
#include <string>
#include <stdio.h>

/**
 * @class Base class for out expections.
 * @brief TBD
 *
 *
 */
template<typename T>
class exceptionBase : public T
{
    public:
        /**
         * @brief Default constructor.
         *
         * Initialize default exception, such string
         * \"Unknown exception was thrown\" will be displayed.
         */
        exceptionBase() : T("Unknown exception was thrown"){};

        /**
         * @brief Specialized constructor.
         * @param msg String which will be added to exception.
         *
         * Allows to throw exception with given string.
         */
        exceptionBase(const std::string& msg) : T(msg){};

        /**
         * @brief Specialized constructor.
         * @param msg String which will be added to exception.
         * @param file File name in which exception is thrown.
         * @param line Line in which exception is thrown.
         *
         * Allows to throw exception with given string, file and line.\n
         * Format is :
         * @code
         * info @ file : line
         * @endcode
         */
        exceptionBase(const std::string& msg,
                      const std::string& file,
                      int line) :
        T(msg + " @ " + file + " : " + number(line)){};
    private:
        /**
         * @brief  Convert number (integer) to string.
         * @param i Number which will be converted.
         *
         * Helper method that is used to convert given number
         * to string.
         */
        static const char* const number(int i)
        {
            static char buf[20] = {0};
            snprintf(buf, 20, "%d", i);
            return buf;
        };
};

/**
 * @class exception
 * @brief Specialization of exception.
 */
class runtimeException : public exceptionBase<std::runtime_error>
{
    public:
        /**
         * @copydoc exceptionBase::exceptionBase()
         */
		runtimeException();

        /**
         * @copydoc exceptionBase::exceptionBase(const std::string&)
         */
		runtimeException(const std::string& msg);

        /**
         * @copydoc exceptionBase::exceptionBase(const std::string&,const std::string&,int)
         */
		runtimeException(const std::string& msg,
                   const std::string& file,
                   int line);
};


#define RUNTIME_EXCEPTION(__d__)   runtimeException(__d__, __FILE__, __LINE__)
#define RUNTIME_EXCEPTION_FUN()    RUNTIME_EXCEPTION(__PRETTY_FUNCTION__)
#define RUNTIME_THROW(__d__)       throw RUNTIME_EXCEPTION(__d__)
#define RUNTIME_THROW_FUN()        throw RUNTIME_EXCEPTION_FUN()
#define RUNTIME_THROW_TEST(__T__)       \
    if(!(__T__)) RUNTIME_THROW(#__T__ " failed");


/**
 * @class argException
 * @brief for invalif argument

 */
class argException : public exceptionBase<std::invalid_argument>
{
    public:
        /**
         * @copydoc exceptionBase::exceptionBase()
         */
		argException();

        /**
         * @copydoc exceptionBase::exceptionBase(const std::string&)
         */
		argException(const std::string& msg);

        /**
         * @copydoc exceptionBase::exceptionBase(const std::string&,const std::string&,int)
         */
		argException(const std::string& msg,
                      const std::string& file,
                      int line);
};


#define ARG_EXCEPTION(__d__)   argException(__d__, __FILE__, __LINE__)
#define ARG_EXCEPTION_FUN()    ARG_EXCEPTION(__PRETTY_FUNCTION__)
#define ARG_THROW(__d__)       throw ARG_EXCEPTION(__d__)
#define ARG_THROW_FUN()        throw ARG_EXCEPTION_FUN()
#define ARG_THROW_TEST(__T__)       \
    if(!(__T__)) ARG_THROW("Invalid arg " #__T__);


#endif /* __OPENRND_EXCEPTIONS_H__ */
