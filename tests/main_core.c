/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include <CUnit/Basic.h>
#include "core_tests.h"
#include "downloaditem_tests.h"
#include "downloadmgr_tests.h"
#include "vbsclient_tests.h"

int main()
{
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (!downloaditem_tests_getSuite()
            || !downloadmgr_tests_getSuite()
            || !vbsclient_tests_getSuite()
            || !core_tests_getSuite())
    {
       CU_cleanup_registry();
       return CU_get_error();
    }

    /* Run all tests using the CUnit Automated interface (results printed to stdout) */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int failed = CU_get_number_of_tests_failed();
    CU_cleanup_registry();
    return failed;
}

