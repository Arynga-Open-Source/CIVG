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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "files.h"

using namespace std;


//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME "DUMM-FILES"
#define LOGGER_COL "35m"
#include "dumm-logger.h"


#if __WIN__
#define DIR_SEPARATOR   '\\'
#else
#define DIR_SEPARATOR   '/'
#endif

namespace dumm
{
    static
    bool checkFileType(const std::string &path, int type)
    {
            struct stat sb;

            if(path.empty())
                    return false;

            if(stat(path.c_str(), &sb) == -1)
                    return false;

            return (sb.st_mode & S_IFMT) == type;
    }

    bool isDir(const std::string &path)
    {
            return checkFileType(path, S_IFDIR);
    }

    bool isFile(const std::string &path)
    {
            return checkFileType(path, S_IFREG);
    }

    bool isLink(const std::string &path)
    {
            struct stat sb;

            if(path.empty())
                    return false;

            if(stat(path.c_str(), &sb) == -1)
                    return false;

            return S_ISLNK(sb.st_mode);
    }

    bool fileExists(const std::string &path)
    {
            struct stat sb;

            if(path.empty())
                    return false;

            if(stat(path.c_str(), &sb) == -1)
                    return false;
            return true;
    }

    int fileSize(const std::string &path)
    {
            struct stat sb;

            if(path.empty())
                    return -1;

            if(stat(path.c_str(), &sb) == -1)
                    return -1;

            return sb.st_size;
    }

    int fileSize(int fd)
    {
        struct stat sb;

        if(fd < 0)
                return -1;

        if(fstat(fd, &sb) == -1)
                return -1;

        return sb.st_size;
    }

    std::string getPath(const std::string &name, const std::string &dir)
    {
            string path = dir;
            if(!path.empty())
            {
                    if( path[path.size() - 1] != DIR_SEPARATOR)
                            path += DIR_SEPARATOR;
            }
            path += name;

            return path;
    }


    int makeDir(std::string path)
    {
            struct stat st = {0};

            if(path.empty())
                    return -1;

            if(stat(path.c_str(), &st) != 0)
            {
                    if(mkdir(path.c_str(), S_IRWXU) != 0)
                    {
                            switch(errno)
                            {
                                    case EEXIST:
                                            //nothing to do
                                            break;

                                    case ENOENT:
                                            //no parent folder
                                            if( makeDir( path.substr(0, path.find_last_of(DIR_SEPARATOR)) ) != 0 )
                                                    return mkdir(path.c_str(), S_IRWXU);

                                    default:
                                            return -1;
                            }
                    }
            }
            else if(!S_ISDIR(st.st_mode))
            {
                    //not dir
                    return -1;
            }
            //so the path exists
            return 0;
    }
} //namespace dumm
