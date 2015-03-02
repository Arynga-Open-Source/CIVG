/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "threaded_transport_tests.h"
#include "threaded_transport.h"
#include "glib_dispatcher.h"
#include "test_config.h"
#include "test_consumer.h"
#include "test_producer.h"
#include "strtools.h"
#include <pthread.h>
#include <stdlib.h>
#include <glib.h>

static pthread_t _mThread = 0;
static GMainLoop* _loop = 0;
dispatcher_Dispatcher* _dispatcher;

static int _cInitRes = 0;
static int _cInitCalled = 0;
static int cInit() {
    ++_cInitCalled;
    return _cInitRes;
}

static int _cCleanupCalled = 0;
static void cCleanup() {
    ++_cCleanupCalled;
}

static int _cRecCalled = 0;
static int _cRecRes = 0;
static int _cRecBlock = 0;
static pthread_mutex_t _cRecLock;
static pthread_mutex_t _cRecUnLock;
static pthread_t _cThread = 0;
static unsigned int cReceiveNext() {
    _cThread = pthread_self();
    ++_cRecCalled;
    if (_cRecBlock) {
        pthread_mutex_unlock(&_cRecUnLock);
        pthread_mutex_lock(&_cRecLock);
    }
    return _cRecRes;
}

static int _cBreakCalled = 0;
static void cBreak() {
    ++_cBreakCalled;
}

static int _cPopCalled = 0;
static int _cPopRes = 0;
static int cPopMsg(void** msg, int* size) {
    ++_cPopCalled;
    int result = _cPopRes;
    if (_cPopRes > 0) {
        *msg = strtools_sprintNew("test message");
        *size = strlen(*msg) + 1;
        --_cPopRes;
    }
    if(_loop && g_main_loop_is_running(_loop) == TRUE)
        g_main_loop_quit(_loop);
    return result;
}

static int _pInitRes = 0;
static int _pInitCalled = 0;
static int pInit() {
    ++_pInitCalled;
    return _pInitRes;
}

static int _pCleanupCalled = 0;
static void pCleanup() {
    ++_pCleanupCalled;
}

static int _pSendCalled = 0;
static int _pSendBlock = 0;
static pthread_mutex_t _pSendLock;
static pthread_mutex_t _pSendUnLock;
static pthread_t _pThread = 0;
static void pSendNext() {
    _pThread = pthread_self();
    ++_pSendCalled;
    if (_pSendBlock) {
        pthread_mutex_unlock(&_pSendUnLock);
        pthread_mutex_lock(&_pSendLock);
    }
}

static const void* _lastMsg = 0;
static int _lastMsgSize = 0;
static int _pPushRes = 0;
static int _pPushCalled = 0;
static int pPushMsg(const void* msg, int size) {
    _lastMsg = msg;
    _lastMsgSize = size;
    ++_pPushCalled;
    return _pPushRes;
}

static int _handlerCalled = 0;
static void* _hMsg = 0;
static int _hMsgSize = 0;
static void handler(const void* msg, int size) {
    ++_handlerCalled;
    free(_hMsg);
    _hMsgSize = size;
    if (_hMsgSize > 0) {
        _hMsg = malloc(_hMsgSize);
        memcpy(_hMsg, msg, _hMsgSize);
    } else {
        _hMsg = 0;
    }
    if(_loop && g_main_loop_is_running(_loop) == TRUE)
        g_main_loop_quit(_loop);
}

static int init()
{
    _loop = g_main_loop_new(NULL, FALSE);
    _dispatcher = glib_dispatcher_newDispatcher(g_main_context_default());
    _mThread = pthread_self();
    pthread_mutex_init(&_cRecLock, NULL);
    pthread_mutex_init(&_cRecUnLock, NULL);
    pthread_mutex_init(&_pSendLock, NULL);
    pthread_mutex_init(&_pSendUnLock, NULL);
    test_consumer_init(cInit);
    test_consumer_cleanup(cCleanup);
    test_consumer_receiveNext(cReceiveNext);
    test_consumer_break(cBreak);
    test_consumer_popMsg(cPopMsg);
    test_producer_init(pInit);
    test_producer_cleanup(pCleanup);
    test_producer_sendNext(pSendNext);
    test_producer_pushMsg(pPushMsg);
    return 0;
}

static int cleanup()
{
    glib_dispatcher_deleteDispatcher(_dispatcher);
    g_main_loop_unref(_loop);
    pthread_mutex_destroy(&_pSendLock);
    pthread_mutex_destroy(&_pSendUnLock);
    pthread_mutex_destroy(&_cRecLock);
    pthread_mutex_destroy(&_cRecUnLock);
    free(_hMsg);
    return 0;
}

static void initTest() {
    _cInitRes = 0;
    _cInitCalled = 0;
    _cCleanupCalled = 0;
    _cBreakCalled = 0;
    _cRecBlock = 0;
    _pSendBlock = 0;

    _pInitRes = 0;
    _pInitCalled = 0;
    _pCleanupCalled = 0;
    _pPushCalled = 0;
    _lastMsgSize = 1;

    threaded_transport_setDispatcher(_dispatcher);
    CU_ASSERT_FATAL(transport_init() == 0);
    CU_ASSERT_FATAL(transport_init() == 1);
    CU_ASSERT_FATAL(_cInitCalled == 1);
    CU_ASSERT_FATAL(_pInitCalled == 1);
    transport_cleanup();
    CU_ASSERT_FATAL(_cBreakCalled == 1);
    CU_ASSERT_FATAL(_cCleanupCalled == 1);
    CU_ASSERT_FATAL(_pCleanupCalled == 1);
    CU_ASSERT_FATAL(_pPushCalled == 1);
    CU_ASSERT_FATAL(_lastMsgSize == 0);

    CU_ASSERT_FATAL(transport_init() == 1);
    CU_ASSERT_FATAL(_cInitCalled == 1);
    CU_ASSERT_FATAL(_pInitCalled == 1);

    threaded_transport_setDispatcher(_dispatcher);
    _cInitRes = 1;
    CU_ASSERT_FATAL(transport_init() == 1);
    CU_ASSERT_FATAL(_cInitCalled == 2);
    CU_ASSERT_FATAL(_cCleanupCalled == 2);
    CU_ASSERT_FATAL(_pInitCalled == 1);
    CU_ASSERT_FATAL(_pCleanupCalled == 2);
    threaded_transport_setDispatcher(_dispatcher);
    _cInitRes = 0;
    _pInitRes = 1;
    CU_ASSERT_FATAL(transport_init() == 1);
    CU_ASSERT_FATAL(_cInitCalled == 3);
    CU_ASSERT_FATAL(_cCleanupCalled == 3);
    CU_ASSERT_FATAL(_pInitCalled == 2);
    CU_ASSERT_FATAL(_pCleanupCalled == 3);
}

static void threadsTest() {
    _cInitRes = 0;
    _cRecBlock = 1;
    _cRecCalled = 0;
    _cRecRes = 1;
    _pSendBlock = 1;
    _pSendCalled = 0;
    _pInitRes = 0;
    threaded_transport_setDispatcher(_dispatcher);
    transport_setMsgHandler(handler);
    pthread_mutex_lock(&_cRecLock);
    pthread_mutex_lock(&_cRecUnLock);
    pthread_mutex_lock(&_pSendLock);
    pthread_mutex_lock(&_pSendUnLock);
    CU_ASSERT_FATAL(transport_init() == 0);

    pthread_mutex_lock(&_cRecUnLock);
    pthread_mutex_lock(&_pSendUnLock);
    CU_ASSERT_FATAL(_cRecCalled == 1);
    CU_ASSERT_FATAL(_pSendCalled == 1);

    pthread_mutex_unlock(&_cRecLock);
    pthread_mutex_unlock(&_pSendLock);
    pthread_mutex_lock(&_cRecUnLock);
    pthread_mutex_lock(&_pSendUnLock);
    CU_ASSERT_FATAL(_cRecCalled == 2);
    CU_ASSERT_FATAL(_pSendCalled == 2);

    CU_ASSERT_FATAL(pthread_equal(_mThread, _cThread) == 0);
    CU_ASSERT_FATAL(pthread_equal(_mThread, _pThread) == 0);
    CU_ASSERT_FATAL(pthread_equal(_pThread, _cThread) == 0);

    _cRecBlock = 0;
    _pSendBlock = 0;
    pthread_mutex_unlock(&_cRecLock);
    pthread_mutex_unlock(&_pSendLock);

    transport_cleanup();
}

static void dispatchedAndSendTest() {
    _pPushCalled = 0;
    _pPushRes = 1;
    int size = strlen("msg test") + 1;
    CU_ASSERT_FATAL(transport_sendMsg("msg test", size) == _pPushRes);
    CU_ASSERT_FATAL(_pPushCalled == 1);
    _pPushRes = 0;
    CU_ASSERT_FATAL(transport_sendMsg("msg test", size) == _pPushRes);
    CU_ASSERT_FATAL(_pPushCalled == 2);
    CU_ASSERT_FATAL(_lastMsgSize == 9);
    CU_ASSERT_FATAL(memcmp(_lastMsg, "msg test", size) == 0);

    _cInitRes = 0;
    _cRecBlock = 0;
    _pSendBlock = 0;
    _pInitRes = 0;
    threaded_transport_setDispatcher(_dispatcher);
    transport_setMsgHandler(handler);
    CU_ASSERT_FATAL(transport_init() == 0);

    _cPopCalled = 0;
    _cPopRes = 0;
    _handlerCalled = 0;
    dispatcher_postEvent(_dispatcher, 0);
    g_main_loop_run(_loop);
    CU_ASSERT_FATAL(_cPopCalled == 1);
    CU_ASSERT_FATAL(_handlerCalled == 0);

    _cPopRes = 3;
    dispatcher_postEvent(_dispatcher, 0);
    g_main_loop_run(_loop);
    CU_ASSERT_FATAL(_cPopRes == 0);
    CU_ASSERT_FATAL(_cPopCalled == 5);
    CU_ASSERT_FATAL(_handlerCalled == 3);
    CU_ASSERT_FATAL(memcmp(_hMsg, "test message", strlen("test message") + 1) == 0);

    transport_cleanup();
}

CU_pSuite threaded_transport_tests_getSuite()
{
   /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test threaded_transport module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, initTest);
   CU_ADD_TEST(suite, threadsTest);
   CU_ADD_TEST(suite, dispatchedAndSendTest);

   return suite;
}
