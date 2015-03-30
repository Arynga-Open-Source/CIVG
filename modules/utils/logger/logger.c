/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "logger.h"
#include "filetools.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

static logger_LogLevel _loggerLevel = LOGG_INFO;

#define MSG_BUFFER_SIZE 1024

extern void logger_setLevel(logger_LogLevel level) {
    _loggerLevel = level;
}

extern void logger_log(const char* fileName, const char* module, logger_LogLevel level, const char* format, ...) {

    if (level > _loggerLevel)
        return;

    // get timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);

    char _msgBuffer[MSG_BUFFER_SIZE];

    // open file
    if (LOGS_DIR[strlen(LOGS_DIR) - 1] != '/') {
        snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%s/%s", LOGS_DIR, fileName);
    } else {
        snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%s%s", LOGS_DIR, fileName);
    }
    FILE* fHandle = fopen(_msgBuffer, "a");

    // format prefix
    int i;
    switch(level) {
    case LOGG_ERROR:
        i = snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%lu.%lu %s [ERR] ", tv.tv_sec, tv.tv_usec, module);
        break;
    case LOGG_WARNING:
        i = snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%lu.%lu %s [WARN] ", tv.tv_sec, tv.tv_usec, module);
        break;
    case LOGG_INFO:
        i = snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%lu.%lu %s [INFO] ", tv.tv_sec, tv.tv_usec, module);
        break;
    default:
        i = snprintf(_msgBuffer, MSG_BUFFER_SIZE, "%lu.%lu %s [DEBUG] ", tv.tv_sec, tv.tv_usec, module);
        break;
    }

    //format log message
    va_list args;
    va_start(args, format);
    vsnprintf(_msgBuffer + i, MSG_BUFFER_SIZE - i, format, args);
    va_end(args);

    // write to standard error
    fprintf(stderr, "%s\n", _msgBuffer);

    // write to file
    if(fHandle)
    {
        fprintf(fHandle, "%s\n", _msgBuffer);
        fclose(fHandle);
    }
}
