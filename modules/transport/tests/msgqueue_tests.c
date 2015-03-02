/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "msgqueue_tests.h"
#include "msgqueue.h"
#include "strtools.h"

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void basicTest() {
    msgqueue_Queue* queue = msgqueue_newQueue();
    CU_ASSERT_FATAL(msgqueue_pop(queue, 0, 0) == 0);
    void* msg1 = strtools_sprintNew("qwerty1");
    int msg1Size = strlen(msg1) + 1;
    void* msg2 = strtools_sprintNew("qwerty2");
    int msg2Size = strlen(msg2) + 1;
    void* msg3 = strtools_sprintNew("qwerty3");
    int msg3Size = strlen(msg3) + 1;
    void* popped = 0;
    int poppedSize = 0;
    CU_ASSERT_FATAL(msgqueue_push(queue, msg1, msg1Size) == 0);
    CU_ASSERT_FATAL(msgqueue_push(queue, msg2, msg2Size) == 0);
    CU_ASSERT_FATAL(msgqueue_push(queue, msg3, msg3Size) == 0);

    CU_ASSERT_FATAL(msgqueue_pop(queue, &popped, &poppedSize) == 1);
    CU_ASSERT_FATAL(msg1Size == poppedSize);
    CU_ASSERT_FATAL(memcmp(popped, msg1, msg1Size) == 0);

    CU_ASSERT_FATAL(msgqueue_pop(queue, &popped, &poppedSize) == 1);
    CU_ASSERT_FATAL(msg2Size == poppedSize);
    CU_ASSERT_FATAL(memcmp(popped, msg2, msg2Size) == 0);

    CU_ASSERT_FATAL(msgqueue_pop(queue, &popped, &poppedSize) == 1);
    CU_ASSERT_FATAL(msg3Size == poppedSize);
    CU_ASSERT_FATAL(memcmp(popped, msg3, msg3Size) == 0);

    poppedSize = 0;
    popped = 0;
    CU_ASSERT_FATAL(msgqueue_pop(queue, &popped, &poppedSize) == 0);
    CU_ASSERT_FATAL(poppedSize == 0);
    CU_ASSERT_FATAL(popped == 0);

    CU_ASSERT_FATAL(msgqueue_push(queue, msg1, msg1Size) == 0);
    CU_ASSERT_FATAL(msgqueue_push(queue, msg2, msg2Size) == 0);
    CU_ASSERT_FATAL(msgqueue_push(queue, msg3, msg3Size) == 0);

    msgqueue_deleteQueue(queue);
    msgqueue_deleteQueue(0);
}

CU_pSuite msgqueue_tests_getSuite()
{
   /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test msgqueue module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, basicTest);

   return suite;
}
