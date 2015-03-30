/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "basic_imagehandler_tests.h"
#include "imagehandler.h"
#include "strtools.h"
#include <stdlib.h>
#include <string.h>

static const char path1[]="rpmanager/basic_imagehandler/tests/test1";
static const char path2[]="rpmanager/basic_imagehandler/tests/test2";
static const char path3[]="rpmanager/basic_imagehandler/tests/test3";
static const char path4[]="rpmanager/basic_imagehandler/tests/test4";
static const char wrong_path[]="rpmanager/basic_imagehandler/tests/wrong_path";

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void testImagehandlerApply()
{
    int ret=1;
    char* msg=0;
    char* correct_msg=0;

    //use wrong type
    ret=imagehandler_apply(path4,"WRONG_TYPE",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,"Unsupported image type!");
    free(msg);
    msg=0;

    //use wrong_path
    ret=imagehandler_apply(wrong_path,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    correct_msg=strtools_sprintNew("Cannot access directory %s", wrong_path);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,correct_msg);
    free(correct_msg);
    free(msg);
    msg=0;

    //missing script
    ret=imagehandler_apply(path1,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 2");
    free(msg);
    msg=0;

    //non-executable script
    ret=imagehandler_apply(path2,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 126");
    free(msg);
    msg=0;

    //bad script
    ret=imagehandler_apply(path3,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 127");
    free(msg);
    msg=0;

    //use correct files
    ret=imagehandler_apply(path4,"RAW",&msg);
    CU_ASSERT_FATAL(ret==0);
    CU_ASSERT_FATAL(msg==0);
    free(msg);
    msg=0;

}

static void testImagehandlerRollback()
{
    int ret=1;
    char* msg=0;
    char* correct_msg=0;

    //use wrong type
    ret=imagehandler_rollback(path4,"WRONG_TYPE",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,"Unsupported image type!");
    free(msg);
    msg=0;

    //use non-existing file
    ret=imagehandler_rollback(wrong_path,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    correct_msg=strtools_sprintNew("Cannot access directory %s", wrong_path);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,correct_msg);
    free(correct_msg);
    free(msg);
    msg=0;

    //use file with missing script
    ret=imagehandler_rollback(path1,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 2");
    free(msg);
    msg=0;

    //use file with non-executable script
    ret=imagehandler_rollback(path2,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 126");
    free(msg);
    msg=0;

    //use file with bad script
    ret=imagehandler_rollback(path3,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 127");
    free(msg);
    msg=0;

    //use correct file
    ret=imagehandler_rollback(path4,"RAW",&msg);
    CU_ASSERT_FATAL(ret==0);
    CU_ASSERT_FATAL(msg==0);
    free(msg);
    msg=0;

}

static void testImagehandlerResume()
{
    int ret=1;
    char* msg=0;
    char* correct_msg=0;

    //use wrong type
    ret=imagehandler_resume(path4,"WRONG_TYPE",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,"Unsupported image type!");
    free(msg);
    msg=0;

    //use non-existing file
    ret=imagehandler_resume(wrong_path,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    correct_msg=strtools_sprintNew("Cannot access directory %s", wrong_path);
    CU_ASSERT_STRING_EQUAL_FATAL(msg,correct_msg);
    free(correct_msg);
    free(msg);
    msg=0;

    //use file with missing script
    ret=imagehandler_resume(path1,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 2");
    free(msg);
    msg=0;

    //use file with non-executable script
    ret=imagehandler_resume(path2,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 126");
    free(msg);
    msg=0;

    //use file with bad script
    ret=imagehandler_resume(path3,"RAW",&msg);
    CU_ASSERT_FATAL(ret==1);
    CU_ASSERT_STRING_EQUAL_FATAL(msg, "Script returned non-zero status! Status returned: 127");
    free(msg);
    msg=0;

    //use correct file
    ret=imagehandler_resume(path4,"RAW",&msg);
    CU_ASSERT_FATAL(ret==0);
    CU_ASSERT_FATAL(msg==0);
    free(msg);
    msg=0;

}


CU_pSuite basic_imagehandler_getSuite()
{
   /* add a suite to the registry */
   CU_pSuite suite = CU_add_suite("Test basic ImageHandler module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, testImagehandlerApply);
   CU_ADD_TEST(suite, testImagehandlerRollback);
   CU_ADD_TEST(suite, testImagehandlerResume);

   return suite;
}
