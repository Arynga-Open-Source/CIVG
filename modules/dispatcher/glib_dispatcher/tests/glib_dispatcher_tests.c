/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "glib_dispatcher_tests.h"
#include "glib_dispatcher.h"
#include <pthread.h>

static GMainLoop* _loop = 0;
static pthread_t _mThread = 0;

static int _lastId = 0;
static pthread_t _hThread = 0;
void handler(int eventId) {
    _lastId = eventId;
    _hThread = pthread_self();
    if(_loop && g_main_loop_is_running(_loop) == TRUE)
        g_main_loop_quit(_loop);
}

static void* postRun(void* tArgs) {
    dispatcher_Dispatcher* d = tArgs;
    dispatcher_postEvent(d, 7);
    pthread_exit(tArgs);
}

static int init()
{
    _loop = g_main_loop_new(NULL, FALSE);
    _mThread = pthread_self();
    return 0;
}

static int cleanup()
{
    g_main_loop_unref(_loop);
    return 0;
}

static void baseTests()
{
    dispatcher_Dispatcher* d = glib_dispatcher_newDispatcher(g_main_context_default());
    dispatcher_setEventHandler(d, handler);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    static pthread_t postThread = 0;
    pthread_create(&postThread, &attr, postRun, d);
    pthread_attr_destroy(&attr);
    pthread_join(postThread, NULL);

    _lastId = 0;
    g_main_loop_run(_loop);

    CU_ASSERT_FATAL(pthread_equal(postThread, _hThread) == 0);
    CU_ASSERT_FATAL(pthread_equal(_mThread, _hThread) != 0);
    CU_ASSERT_FATAL(_lastId == 7);
    glib_dispatcher_deleteDispatcher(d);
}

CU_pSuite glib_dispatcher_tests_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test glib dispatcher module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, baseTests);

    return suite;
}
