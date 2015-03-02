/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include <CUnit/Basic.h>
#include "config_tests.h"
#include "strtools_tests.h"
#include "filetools_tests.h"
#include "proto_tests.h"
#include "rpmanager_tests.h"
#include "basic_imagehandler_tests.h"
#include "dumm_downloader_tests.h"
#include "rmq_transport_tests.h"
#include "glib_dispatcher_tests.h"

int main()
{
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (!config_tests_getSuite()
            || !dumm_downloader_tests_getSuite()
            || !rmq_transport_tests_getSuite()
            || !basic_imagehandler_getSuite()
            || !rpmanager_getSuite()
            || !glib_dispatcher_tests_getSuite()
            || !strtools_tests_getSuite()
            || !filetools_tests_getSuite()
            || !proto_tests_getSuite() )
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

