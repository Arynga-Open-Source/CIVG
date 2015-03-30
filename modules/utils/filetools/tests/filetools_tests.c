/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "filetools_tests.h"
#include "filetools.h"
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

static void testGetDir()
{
    char* dir = filetools_getDir("qwerty/aswd");
    CU_ASSERT_FATAL(dir != 0);
    CU_ASSERT_FATAL(strcmp(dir, "qwerty") == 0);
    free(dir);
    dir = filetools_getDir("qwerty");
    CU_ASSERT_FATAL(dir == 0);
}

static void testJoinPath()
{
    char* path = filetools_joinPath("qwerty", "aswd");
    CU_ASSERT_FATAL(path != 0);
    CU_ASSERT_FATAL(strcmp(path, "qwerty/aswd") == 0);
    free(path);
    path = filetools_joinPath("qwerty/", "aswd");
    CU_ASSERT_FATAL(path != 0);
    CU_ASSERT_FATAL(strcmp(path, "qwerty/aswd") == 0);
    free(path);
    path = filetools_joinPath("qwerty", 0);
    CU_ASSERT_FATAL(path == 0);
    path = filetools_joinPath(0, "aswd");
    CU_ASSERT_FATAL(path == 0);
    path = filetools_joinPath("", "");
    CU_ASSERT_FATAL(path == 0);
}

static void testReadAll()
{
    void* content;
    unsigned int cSize;
    CU_ASSERT_FATAL(filetools_readAll("utils/filetools/tests/no.file", &content, &cSize) != 0);
    CU_ASSERT_FATAL(filetools_readAll("utils/filetools/tests/test.file", &content, &cSize) == 0);
    CU_ASSERT_FATAL(memcmp(content, "qwerty@aswd\n", cSize) == 0);
    free(content);
}

static void testExtractTar()
{
    void* content;
    unsigned int cSize;
    CU_ASSERT_FATAL(filetools_extractTar("utils/filetools/tests/no.tar", "utils/filetools/tests/") != 0);
    CU_ASSERT_FATAL(filetools_extractTar("utils/filetools/tests/test.tar", "utils/filetools/tests/") == 0);
    CU_ASSERT_FATAL(filetools_readAll("utils/filetools/tests/a.file", &content, &cSize) == 0);
    CU_ASSERT_FATAL(memcmp(content, "acontent", cSize) == 0);
    free(content);
    CU_ASSERT_FATAL(filetools_readAll("utils/filetools/tests/b.file", &content, &cSize) == 0);
    CU_ASSERT_FATAL(memcmp(content, "bcontent", cSize) == 0);
    free(content);
}

static void testRemoveDir()
{
    CU_ASSERT_FATAL(system("rm -rf utils/filetools/tests/test.dir") == 0);
    CU_ASSERT_FATAL(system("mkdir utils/filetools/tests/test.dir") == 0);
    CU_ASSERT_FATAL(system("ls utils/filetools/tests/test.dir") == 0);
    CU_ASSERT_FATAL(system("touch utils/filetools/tests/test.dir/empty.file") == 0);
    CU_ASSERT_FATAL(system("mkdir utils/filetools/tests/test.dir/subdir") == 0);
    CU_ASSERT_FATAL(system("echo content > utils/filetools/tests/test.dir/subdir/some.file") == 0);
    CU_ASSERT_FATAL(system("echo content > utils/filetools/tests/test.dir/some.file") == 0);
    CU_ASSERT_FATAL(filetools_removeDir("utils/filetools/tests/no.dir") == 0);
    CU_ASSERT_FATAL(filetools_removeDir("utils/filetools/tests/test.dir") == 0);
    CU_ASSERT_FATAL(system("ls utils/filetools/tests/test.dir") != 0);
}

static void testMakePath()
{
    CU_ASSERT_FATAL(filetools_makePath(0) == 1);
    CU_ASSERT_FATAL(filetools_makePath("") == 1);
    CU_ASSERT_FATAL(filetools_makePath("/") == 1);
    CU_ASSERT_FATAL(system("rm -rf qwery") == 0);
    CU_ASSERT_FATAL(filetools_makePath("qwery") == 0);
    CU_ASSERT_FATAL(system("rm -rf qwery") == 0);
    CU_ASSERT_FATAL(filetools_makePath("qwery/") == 0);
    CU_ASSERT_FATAL(system("rm -rf qwery") == 0);
    CU_ASSERT_FATAL(filetools_makePath("qwery/a/b/c") == 0);
}

static void testCalculateSHA1Sum()
{
    char* checksum = NULL;

    checksum = filetools_getSHA1Checksum("some/random/unexisting/path");
    CU_ASSERT_FATAL(checksum == NULL);

    system("rm utils/filetools/tests/sha1.file");
    CU_ASSERT_FATAL(system("echo somecontentformd5checksum > utils/filetools/tests/sha1.file") == 0);
    checksum = filetools_getSHA1Checksum("utils/filetools/tests/sha1.file");
    CU_ASSERT_FATAL(system("rm utils/filetools/tests/sha1.file") == 0);
    CU_ASSERT_FATAL(checksum != NULL);
    CU_ASSERT_STRING_EQUAL_FATAL(checksum, "10da2994f28c2468095a7f612ac0f0539654ebed");
    free(checksum);
}

CU_pSuite filetools_tests_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test file module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, testGetDir);
    CU_ADD_TEST(suite, testJoinPath);
    CU_ADD_TEST(suite, testReadAll);
    CU_ADD_TEST(suite, testExtractTar);
    CU_ADD_TEST(suite, testRemoveDir);
    CU_ADD_TEST(suite, testMakePath);
    CU_ADD_TEST(suite, testCalculateSHA1Sum);

    return suite;
}
