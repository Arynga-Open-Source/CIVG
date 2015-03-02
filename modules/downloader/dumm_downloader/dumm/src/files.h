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
 * @file files.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 */

#ifndef FILES_H_
#define FILES_H_

#include <string>

namespace dumm
{
    /**
     * @brief
     */
    bool isDir(const std::string &path);

    /**
     * @brief
     */
    bool isFile(const std::string &path);

    /**
     * @brief
     */
    bool isLink(const std::string &path);

    /**
     * @brief Return current file size
     *
     * @param path [in] File path
     *
     * @return file size or -1 in case of error
     */
    int fileSize(const std::string &path);

    /**
     * @brief Return current file size
     *
     * @param fd [in] File descriptor
     *
     * @return file size or -1 in case of error
     */
    int fileSize(int fd);

    /**
     * @brief
     */
    bool fileExists(const std::string &path);

    /**
     * @brief
     */
    std::string getPath(const std::string &name, const std::string &dir);

    /**
     * @brief
     */
    int makeDir(std::string path);
};
#endif /* FILES_H_ */
