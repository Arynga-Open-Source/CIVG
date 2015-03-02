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

#include "openrnd-exceptions.h"

runtimeException::runtimeException()
    :exceptionBase<std::runtime_error>()
{};

runtimeException::runtimeException(const std::string& info)
    :exceptionBase<std::runtime_error>(info)
{};

runtimeException::runtimeException(const std::string& info,
            const std::string& file,
            int line)
    :exceptionBase<std::runtime_error>(info, file, line)
{};


argException::argException()
    :exceptionBase<std::invalid_argument>()
{};

argException::argException(const std::string& info)
    :exceptionBase<std::invalid_argument>(info)
{};

argException::argException(const std::string& info,
            const std::string& file,
            int line)
    :exceptionBase<std::invalid_argument>(info, file, line)
{};



