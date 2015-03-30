/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "filetools.h"
#include "logger.h"
#include "strtools.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <openssl/sha.h>

#define READ_BLOCK_SIZE 4096

char* filetools_joinPath(const char* dir, const char* name)
{
    size_t len = 0;
    size_t dirlen = 0;
    char* path = NULL;
    int trailingSlash = 0;

    if(name==NULL || dir==NULL) {
        LOGM(LOGG_ERROR, "Can not join NULL paths!");
        return NULL;
    }

    if(strlen(name)==0 || strlen(dir)==0){
        LOGM(LOGG_ERROR, "Can not join empty paths!");
        return NULL;
    }

    dirlen = strlen(dir);
    if(dir[dirlen-1]=='/')
    {
        trailingSlash = 1;
        len = dirlen + strlen(name) + 1;
    }
    else
        len = dirlen + strlen(name) + 2;

    path = malloc(len);
    memset(path, '\0', len);
    if(path==NULL) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for joined path!");
        return NULL;
    }

    strncpy(path, dir, dirlen);
    if(trailingSlash==0)
        path[dirlen] = '/';

    strcat(path, name);

    return path;
}

char* filetools_getDir(const char* path) {
    char* lastSlash = strrchr(path, '/');
    size_t dirSize = lastSlash - path;
    char* dir = 0;
    if (lastSlash && dirSize > 0) {
        dir = malloc(dirSize + 1);
        if (dir) {
            strncpy(dir, path, dirSize);
            dir[dirSize] = '\0';
        } else {
            LOGM(LOGG_ERROR, "Can not allocate memory for dir!");
        }
    } else {
        LOGM(LOGG_DEBUG, "No dir found in given path!");
    }
    return dir;
}


#define TAR_BLOCK_SIZE 512


/** taken from tar(5) */
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];              // contains TMAGIC
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} header_posix_ustar;

extern int filetools_extractTar(const char* path, const char* targetDir) {
    char* cmd = strtools_sprintNew("tar -C \"%s\" -xf \"%s\"", targetDir, path);
    if (!cmd) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for tar command!");
        return 1;
    }
    int status = (system(cmd) != 0);
    free(cmd);
    return status;
}

extern int filetools_readAll(const char* path, void** data, size_t* size) {
    struct stat fileStat;
    if (stat(path, &fileStat) != 0) {
        LOG(LOGG_ERROR, "Failed stat on file. Path: %s", path);
        return 1;
    }
    FILE* file = fopen(path, "rb");
    if(file == NULL) {
        LOG(LOGG_ERROR, "Failed to open file. Path: %s; Reason: %s", path, strerror(errno));
        return 1;
    }
    int status = 1;
    if (fileStat.st_size > 0) {
        *data = malloc(fileStat.st_size);
        if (*data) {
            *size = fileStat.st_size;
            size_t bRead = 0;
            while (!feof(file) && !ferror(file)) {
                bRead += fread(*data, 1, *size, file);
            }
            if (bRead != *size) {
                LOG(LOGG_ERROR, "Failed to read whole file content! Path: %s", path);
                free(*data);
                *data = 0;
                *size = 0;
            } else {
                status = 0;
            }
        } else {
            LOG(LOGG_ERROR, "Failed to allocate memory for file content! Path: %s", path);
        }
    } else {
        *size = fileStat.st_size;
        *data = 0;
        status = 0;
    }
    fclose(file);
    return status;
}

extern int filetools_removeDir(const char* dirPath) {
    int status = 0;
    DIR *dir = opendir(dirPath);
    if (dir) {
        struct dirent *ent;
        char* entPath;
        while ((ent=readdir(dir)) != NULL) {
            // Skip the names "." and ".." as we don't want to recurse on them.
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0){
                entPath = filetools_joinPath(dirPath, ent->d_name);
                if (entPath) {
                    if (ent->d_type == DT_DIR) {
                        status = filetools_removeDir(entPath);
                    } else {
                        status = (unlink(entPath) != 0);
                    }
                    if(status){
                        LOG(LOGG_WARNING, "Failed to remove directory %s", entPath);
                        free(entPath);
                        break;
                    }
                    free(entPath);
                } else {
                    status = 1;
                    break;
                }
            }
        }
        closedir(dir);
        status |= rmdir(dirPath);
    } else if (errno != ENOENT) {
        LOG(LOGG_DEBUG, "Failed to open directory! DirPath: %s", dirPath);
        status = 1;
    }
    return status;
}

int filetools_makePath(const char *dirPath)
{
    if (!dirPath || strlen(dirPath) == 0) {
        LOGM(LOGG_ERROR, "Can not make empty path!");
        return 1;
    }
    if (strlen(dirPath) == 1 && dirPath[0] == '/') {
        LOGM(LOGG_ERROR, "Can not make root path!");
        return 1;
    }
    int status = 1;
    size_t origSize = strlen(dirPath);
    if (dirPath[origSize - 1] != '/') {
        // omit last slash
        ++origSize;
    }
    char* path = malloc(origSize);
    if (path) {
        memcpy(path, dirPath, origSize);
        // set terminating null
        path[origSize - 1] = '\0';

                //truncate not existing dirs
        char* lastSlash = strrchr(path, '/');
        while (strlen(path) > 0
               && access(path, W_OK) != 0
               && lastSlash) {
            *lastSlash = '\0';
            lastSlash = strrchr(path, '/');
        }
        // make not existing parent dirs
        while (origSize - 1 > strlen(path)) {
            if (strlen(path) > 0
                && access(path, W_OK) != 0
                && mkdir(path, S_IRWXU)) {
                status = -1;
                break;
            }
            path[strlen(path)] = '/';
        }
        // make final dir
        // if it exists, remove the directory
        LOG(LOGG_DEBUG, "Checking if directory exists: %s", path);
        DIR* dir = opendir(path);
        if (dir)
        {
            LOG(LOGG_DEBUG, "Directory exists: %s. Removing!", path);
            /* Directory exists. */
            closedir(dir);
            int removed = filetools_removeDir(path);
            if(removed != 0)
            {
                LOGM(LOGG_DEBUG,"Failed to clean download dir!");
                status=1;
            }
        }
        else if (ENOENT == errno)
        {
            LOG(LOGG_DEBUG, "Directory %s not exists. Creating.", path);
        }
        // creating directory
        if (status != -1 && mkdir(path, S_IRWXU) == 0) {
            status = 0;
        } else {
            status = 1;
            LOG(LOGG_DEBUG, "Failed to create path! Path: %s", path);
        }
        free(path);
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for path string!");
    }
    return status;
}
extern char* filetools_getSHA1Checksum(const char *filePath){
    if(!filePath || strlen(filePath)==0){
        LOGM(LOGG_ERROR, "File path not provided!");
        return NULL;
    }

    FILE *inFile = fopen (filePath, "rb");
    if (!inFile) {
        LOG(LOGG_ERROR, "%s can't be opened to calculate checksum.", filePath);
        return NULL;
    }

    unsigned char shaSum[SHA_DIGEST_LENGTH];
    SHA_CTX shaContext;
    int bytes = 0;
    unsigned char data[4096];
    char* shaSumString = NULL;

    // allocate memory for string
    shaSumString = malloc(sizeof(char)*2*SHA_DIGEST_LENGTH+1);
    if(!shaSumString){
        LOGM(LOGG_ERROR, "Cannot allocate memory for checksum string!");
        fclose(inFile);
        return NULL;
    }

    SHA1_Init(&shaContext);
    while((bytes = fread (data, 1, READ_BLOCK_SIZE, inFile)) != 0)
    {
        SHA1_Update(&shaContext, data, bytes);
    }
    SHA1_Final(shaSum, &shaContext);

    for(int i=0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(shaSumString+2*i, "%02x", shaSum[i]);
    }

    // add trailing null character
    shaSumString[SHA_DIGEST_LENGTH*2] = '\0';

    fclose(inFile);
    return shaSumString;
}
