/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "strtools_tests.h"
#include "strtools.h"
#include <string.h>
#include <stdlib.h>

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void testSprintNew()
{
    char* msg = strtools_sprintNew("abc%d", 12);
    CU_ASSERT_FATAL(msg != 0);
    CU_ASSERT_FATAL(strcmp(msg, "abc12") == 0);
    free(msg);
}

CU_pSuite strtools_tests_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test strtools module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, testSprintNew);

    return suite;
}
