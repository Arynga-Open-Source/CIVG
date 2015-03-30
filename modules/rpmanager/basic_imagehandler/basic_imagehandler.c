/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#define LOGGER_MODULE "CIVG_IMAGEHANDLER"

#include "imagehandler.h"
#include "strtools.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define IMAGEHANDLER_SCRIPT_DIR      "scripts"
#define IMAGEHANDLER_UPDATE_SCRIPT   "update"
#define IMAGEHANDLER_RECOVER_SCRIPT  "recover"
#define IMAGEHANDLER_ROLLBACK_SCRIPT "rollback"

#define RAW_IMAGE_TYPE "raw"

typedef enum{
    IMAGEHANDLER_UPDATE,
    IMAGEHANDLER_RECOVER,
    IMAGEHANDLER_ROLLBACK
} Action;

static int rawAction(const char* path, Action action, char** message)
{
    if (access(path, R_OK))
    {
        *message = strtools_sprintNew("Cannot access directory %s", path);
        LOGM(LOGG_ERROR, *message);
        return 1;
    }

    char* cmd = NULL;

    switch ( action )
    {
        case IMAGEHANDLER_UPDATE:
            cmd = strtools_sprintNew("cd %s/%s && ../%s/%s", path, IMAGEHANDLER_IMAGES_SUBDIR, IMAGEHANDLER_SCRIPT_DIR, IMAGEHANDLER_UPDATE_SCRIPT);
            break;

        case IMAGEHANDLER_RECOVER:
            cmd = strtools_sprintNew("cd %s/%s && ../%s/%s", path, IMAGEHANDLER_IMAGES_SUBDIR, IMAGEHANDLER_SCRIPT_DIR, IMAGEHANDLER_RECOVER_SCRIPT);
            break;

        case IMAGEHANDLER_ROLLBACK:
            cmd = strtools_sprintNew("cd %s/%s && ../%s/%s", path, IMAGEHANDLER_IMAGES_SUBDIR, IMAGEHANDLER_SCRIPT_DIR, IMAGEHANDLER_ROLLBACK_SCRIPT);
            break;

        default:
            *message = strtools_sprintNew("Unsupported action! Type: %d", action);
            LOGM(LOGG_ERROR, *message);
            return 1;
    }

    if (cmd == NULL)
    {
        *message = strtools_sprintNew("Failed to allocate memory in imagehandler");
        LOGM(LOGG_ERROR, *message);
        return 1;
    }

    LOG(LOGG_DEBUG, "Executing script \"%s\"", cmd);

    int status = system(cmd);

    if(status < 0)
    {
        *message = strtools_sprintNew("Failed to run command!");
        LOGM(LOGG_ERROR, *message);
        status = 1;
    } else if (status > 0)
    {
         *message = strtools_sprintNew("Script returned non-zero status! Status returned: %d", WEXITSTATUS(status));
         LOGM(LOGG_ERROR, *message);
         status = 1;
    }
    free(cmd);
    return status;
}

static int perform(const char* dataPath, const char* type, Action action, char** message)
{
    if (type != NULL)
    {
        if(!strcmp(type,RAW_IMAGE_TYPE))
        {
            LOG(LOGG_INFO, "Performing action. Type: %d", action);
            return rawAction(dataPath, action, message);
        }
        else
        {
            *message = strtools_sprintNew("Unsupported image type!");
            LOGM(LOGG_ERROR, *message);
            return 1;
        };
    }
    else
    {
        LOG(LOGG_WARNING, "Empty image type, assuming RAW by default and performing action. Type: %d", action);
        return rawAction(dataPath, action, message);
    }
}

int imagehandler_apply(const char* dataPath, const char* type,  char** message)
{
    return perform(dataPath, type, IMAGEHANDLER_UPDATE, message);
}

int imagehandler_resume(const char* dataPath, const char* type,  char** message)
{
    return perform(dataPath, type, IMAGEHANDLER_RECOVER, message);
}

int imagehandler_rollback(const char* dataPath, const char* type,  char** message)
{
    return perform(dataPath, type, IMAGEHANDLER_ROLLBACK, message);
}
