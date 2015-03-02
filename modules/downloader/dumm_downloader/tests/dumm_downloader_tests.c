/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "dumm_downloader_tests.h"
#include "dumm_downloader.h"
#include "strtools.h"
#include <string.h>
#include <stdlib.h>

static GMainLoop* _loop = 0;

static char* _lastId = 0;
static int _lastStatus = 0;
static void finishedHandler(const char* id, int status) {
    free(_lastId);
    _lastId = strtools_sprintNew("%s", id);
    _lastStatus = status;
    if(_loop && g_main_loop_is_running(_loop) == TRUE)
        g_main_loop_quit(_loop);
}

static int init()
{
    g_type_init();
    dumm_downloader_registerSignalMarshallers();
    _loop = g_main_loop_new(NULL, FALSE);
    return 0;
}

static int cleanup()
{
    free(_lastId);
    g_main_loop_unref(_loop);
    return 0;
}

static void baseTests()
{
    DBusGConnection *connection;
    GError *error = NULL;
    connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    CU_ASSERT_FATAL(connection != NULL);
    dumm_downloader_setupConnection(connection);
    CU_ASSERT_FATAL(downloader_init() == 0);
    CU_ASSERT_FATAL(downloader_init() != 0);
    downloader_setFinishedHandler(finishedHandler);

    CU_ASSERT_FATAL(downloader_cancel("testId") != 0);
    char* id;
    free(_lastId);
    _lastId = 0;
    _lastStatus = -1;
    CU_ASSERT_FATAL(downloader_downloadItem("fakeURL", DOWNLOAD_DIR "/file.name", &id) == 0);
    g_main_loop_run(_loop);
    CU_ASSERT_FATAL(_lastStatus == 1);
    CU_ASSERT_FATAL(strcmp(_lastId, id) == 0);
    free(id);

    free(_lastId);
    _lastId = 0;
    _lastStatus = -1;
    CU_ASSERT_FATAL(downloader_downloadItem("file:///mnt/dumm/dumm.conf", DOWNLOAD_DIR "/dumm.conf", &id) == 0);
    g_main_loop_run(_loop);
    CU_ASSERT_FATAL(_lastStatus == 0);
    CU_ASSERT_FATAL(strcmp(_lastId, id) == 0);
    free(id);

    downloader_cleanup();
}

CU_pSuite dumm_downloader_tests_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test dumm_downloader module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, baseTests);

    return suite;
}
