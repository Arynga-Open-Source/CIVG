/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "downloaditem_tests.h"
#include "downloaditem.h"
#include "strtools.h"

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void newAndDeleteTests() {
    downloaditem_Item* first = 0;
    // preppend and delete
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    downloaditem_Item* second = first;
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    CU_ASSERT_FATAL(downloaditem_deleteFirstItem(&first) == second);
    CU_ASSERT_FATAL(downloaditem_deleteFirstItem(&second) == 0);

    // preppend and delete, but starting from second element
    first = 0;
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    second = first;
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    CU_ASSERT_FATAL(downloaditem_deleteFirstItem(&second) == first);
    CU_ASSERT_FATAL(downloaditem_deleteFirstItem(&first) == 0);
}

static void findItemByIdTests() {
    downloaditem_Item* first = 0;
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    first->id = strtools_sprintNew("qwerty");
    downloaditem_Item* second = first;
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    CU_ASSERT_FATAL(downloaditem_newItemPreppend(&first) == first);
    CU_ASSERT_FATAL(downloaditem_findItemById(first, "qwerty") == second);
    CU_ASSERT_FATAL(downloaditem_findItemById(first, "invalid") == 0);
    while (downloaditem_deleteFirstItem(&first));
}

CU_pSuite downloaditem_tests_getSuite()
{
   /* add a suite to the registry */
   CU_pSuite suite = CU_add_suite("Test download item module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, newAndDeleteTests);
   CU_ADD_TEST(suite, findItemByIdTests);

   return suite;
}
