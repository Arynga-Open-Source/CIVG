/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * This is test downloader which mocks downloader bahaviour.
 */

#ifndef TEST_DOWNLOADER_H
#define TEST_DOWNLOADER_H

#include "downloader.h"

extern void testdownloader_init(int (*func)());

extern void testdownloader_cleanup(void (*func)());

extern void testdownloader_setFinishedHandler(void (*func)(DOWNLOADER_FINISHED_HANDLER));

extern void testdownloader_downloadItem(int (*func)(const char* url, const char* path, char** id));

extern void testdownloader_cancel(int (*func)(const char* id));

#endif
