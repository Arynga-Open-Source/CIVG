/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "test_downloader.h"
#include "strtools.h"

static int (*_initFunc)() = 0;
static void (*_cleanupFunc)() = 0;
static void (*_setFinishedHandlerFunc)(DOWNLOADER_FINISHED_HANDLER) = 0;
static int (*_downloadItemFunc)(const char *, const char *, char **) = 0;
static int (*_cancelFunc)(const char *) = 0;

int downloader_init() {
    if (_initFunc) {
        return _initFunc();
    }
    return 0;
}

void downloader_cleanup() {
    if (_cleanupFunc) {
        _cleanupFunc();
    }
}

void downloader_setFinishedHandler(DOWNLOADER_FINISHED_HANDLER handler) {
    if (_setFinishedHandlerFunc) {
        _setFinishedHandlerFunc(handler);
    }
}

extern int downloader_downloadItem(const char* url, const char* path, char** id) {
    if (_downloadItemFunc) {
        return _downloadItemFunc(url, path, id);
    }
    return 0;
}

extern int downloader_cancel(const char* id) {
    if (_cancelFunc) {
        return _cancelFunc(id);
    }
    return 0;
}


void testdownloader_init(int (*func)())
{
    _initFunc = func;
}


void testdownloader_cleanup(void (*func)())
{
    _cleanupFunc = func;
}


void testdownloader_setFinishedHandler(void (*func)(DOWNLOADER_FINISHED_HANDLER))
{
    _setFinishedHandlerFunc = func;
}


void testdownloader_downloadItem(int (*func)(const char *, const char *, char **))
{
    _downloadItemFunc = func;
}


void testdownloader_cancel(int (*func)(const char *))
{
    _cancelFunc = func;
}
