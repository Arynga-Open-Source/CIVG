/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "config_tests.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void testInt()
{
    config_Storage* storage = config_newStorage("test");
    CU_ASSERT_FATAL(storage != 0);

    int status;
    int value;
    status = config_getIValue(storage, "invalid", &value);
    CU_ASSERT_FATAL(status != 0);

    status = config_setIValue(storage, "intKey", 18679200);
    CU_ASSERT_FATAL(status == 0);
    status = config_getIValue(storage, "intKey", &value);
    CU_ASSERT_FATAL(status == 0);
    CU_ASSERT_FATAL(value == 18679200);

    status = config_removeKey(storage, "invalid");
    CU_ASSERT_FATAL(status != 0);

    status = config_removeKey(storage, "intKey");
    CU_ASSERT_FATAL(status == 0);
    status = config_getIValue(storage, "intKey", &value);
    CU_ASSERT_FATAL(status != 0);

    config_deleteStorage(storage);
}

static const int ITER_CNT = 50;
static const int THREAD_COUNT = 10;
static void * __thread_access(void *arg)
{
    config_Storage *storage = (config_Storage *)arg;
    char value[] = "thread";
    const int vSize = sizeof(value);
    int i = 0;

    long status = 0;
    for (; i < ITER_CNT; i++)
    {
        status = config_setValue(storage, "bogus_key",
                                 value, vSize);
        if (status != 0)
            break;
    }

    pthread_exit((void *) status);
    return NULL;
}

/**
 * @brief run threaded access test
 *
 * Fires THREAD_COUNT thread that do simultaneous config_setValue()
 * for ITER_CNT iterations. Each thread returns a status value, which
 * when 0 indicates that all accesses were successful. Test checks if
 * all threads were successful.
 */
static void testThread()
{
    struct th_status_s {
        pthread_t th;
        long status;
    } threads[THREAD_COUNT];

    memset(&threads, 0, sizeof(threads));

    config_Storage* storage = config_newStorage("test");
    CU_ASSERT_FATAL(storage != NULL);

    /* start all threads */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        /* by default threads are joinable */
        pthread_create(&threads[i].th, NULL, __thread_access, storage);
    }

    /* wait for them to finish */
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i].th, (void **)&threads[i].status);
    }

    /* check return status */
    int good_count = 0;
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        if (threads[i].status == 0)
        {
            good_count++;
        }
    }

    CU_ASSERT_FATAL(good_count == THREAD_COUNT);

    void *ret;
    int retSize;
    int status = 0;
    status = config_getValue(storage, "bogus_key", &ret, &retSize);
    free(ret);
    CU_ASSERT_FATAL(status == 0);

    config_deleteStorage(storage);
}

static void testBlob()
{
    config_Storage* storage = config_newStorage("test");
    CU_ASSERT_FATAL(storage != 0);

    int status;
    char value[] = "qwerty123\09876543210";
    const int vSize = sizeof(value);
    void* ret;
    int retSize;
    status = config_getValue(storage, "invalid", &ret, &retSize);
    CU_ASSERT_FATAL(status != 0);

    status = config_setValue(storage, "blobKey", value, vSize);
    CU_ASSERT_FATAL(status == 0);
    status = config_getValue(storage, "blobKey", &ret, &retSize);
    CU_ASSERT_FATAL(status == 0);
    CU_ASSERT_FATAL(retSize == vSize);
    CU_ASSERT_FATAL(memcmp(ret, value, vSize) == 0);
    free(ret);

    char value2[] = "aswd321\0123456789";
    const int vSize2 = sizeof(value2);
    status = config_setValue(storage, "blobKey", value2, vSize2);
    CU_ASSERT_FATAL(status == 0);
    status = config_getValue(storage, "blobKey", &ret, &retSize);
    CU_ASSERT_FATAL(status == 0);
    CU_ASSERT_FATAL(retSize == vSize2);
    CU_ASSERT_FATAL(memcmp(ret, value2, vSize2) == 0);
    free(ret);

    status = config_removeKey(storage, "invalid");
    CU_ASSERT_FATAL(status != 0);

    status = config_removeKey(storage, "blobKey");
    CU_ASSERT_FATAL(status == 0);
    status = config_getValue(storage, "blobKey", &ret, &retSize);
    CU_ASSERT_FATAL(status != 0);

    config_deleteStorage(storage);
}

CU_pSuite config_tests_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test config module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, testInt);
    CU_ADD_TEST(suite, testBlob);
    CU_ADD_TEST(suite, testThread);

    return suite;
}
