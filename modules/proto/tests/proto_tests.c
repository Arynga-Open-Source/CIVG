/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "proto_tests.h"
#include "proto.h"
#include "filetools.h"
#include <stdio.h>
#include <stdlib.h>

static int init()
{
    return 0;
}

static int cleanup()
{
    return 0;
}

static void testUfFormat() {
    unsigned int size = 0;
    char* data = 0;
    filetools_readAll("proto/tests/UF.meta", (void**)&data, &size);

    CU_ASSERT_FATAL(data != NULL);

	FILE* f = NULL;
    f=fopen("proto/tests/data.uf", "wb");
    CU_ASSERT_FATAL(f != NULL);

    CU_ASSERT_FATAL(fwrite(data, sizeof(char), size, f) == (size_t)size);
    fclose(f);

    proto_UpdateFileMeta* uf;

	// Unpack the message using protobuf-c.
    uf = car_sync__proto__release_package__update_file_meta__unpack(NULL, size, (uint8_t*)data);
    CU_ASSERT_FATAL(uf != NULL);

    CU_ASSERT_FATAL(uf->uuid != NULL);
    CU_ASSERT_FATAL(strlen(uf->uuid) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->uuid, "40399461-60b1-4ad7-a6b3-ab168bb70b0e" );

    CU_ASSERT_FATAL(uf->url != NULL);
    CU_ASSERT_FATAL(strlen(uf->url) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->url, "http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.2.0/ivi4-demo2.patch");

    CU_ASSERT_FATAL(uf->version != NULL);
    CU_ASSERT_FATAL(uf->version->major_version == 1);
    CU_ASSERT_FATAL(uf->version->minor_version == 2);
    CU_ASSERT_FATAL(uf->version->build_version == 0);

    CU_ASSERT_FATAL(uf->checksum != NULL);
    CU_ASSERT_FATAL(strlen(uf->checksum) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->checksum, "5f010b68f5dcff8ddf966d4d678b163b7fa6d090");

    free(data);
    car_sync__proto__release_package__update_file_meta__free_unpacked(uf, NULL);
}

static void testVersionCmp()
{
    CU_ASSERT_FATAL(proto_versionCmp(0, 0) == 0);
    proto_Version a = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    proto_Version b = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    CU_ASSERT_FATAL(proto_versionCmp(&a, &b) == 0);

    a.has_major_version = 1;
    a.major_version = 1;
    CU_ASSERT_FATAL(proto_versionCmp(&a, &b) == 1);
    CU_ASSERT_FATAL(proto_versionCmp(&b, &a) == -1);

    b.has_major_version = 1;
    b.major_version = 2;
    CU_ASSERT_FATAL(proto_versionCmp(&a, &b) == -1);

    a.major_version = 2;
    b.has_minor_version = 1;
    b.minor_version = 2;
    a.has_minor_version = 1;
    a.minor_version = 2;
    b.has_build_version = 1;
    b.build_version = 2;
    a.has_build_version = 1;
    a.build_version = 2;
    CU_ASSERT_FATAL(proto_versionCmp(&a, &b) == 0);
}

static void testUnpackMetaFile()
{
    proto_UpdateFileMeta* uf = proto_unpackMetaFile("invalidPath");
    CU_ASSERT_FATAL(uf == NULL);

    uf = proto_unpackMetaFile("proto/tests/");
    CU_ASSERT_FATAL(uf != NULL);
    CU_ASSERT_FATAL(uf->uuid != NULL);
    CU_ASSERT_FATAL(strlen(uf->uuid) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->uuid, "40399461-60b1-4ad7-a6b3-ab168bb70b0e" );

    CU_ASSERT_FATAL(uf->url != NULL);
    CU_ASSERT_FATAL(strlen(uf->url) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->url, "http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.2.0/ivi4-demo2.patch");

    CU_ASSERT_FATAL(uf->version != NULL);
    CU_ASSERT_FATAL(uf->version->major_version == 1);
    CU_ASSERT_FATAL(uf->version->minor_version == 2);
    CU_ASSERT_FATAL(uf->version->build_version == 0);

    CU_ASSERT_FATAL(uf->checksum != NULL);
    CU_ASSERT_FATAL(strlen(uf->checksum) !=0);
    CU_ASSERT_STRING_EQUAL_FATAL(uf->checksum, "5f010b68f5dcff8ddf966d4d678b163b7fa6d090");
    car_sync__proto__release_package__update_file_meta__free_unpacked(uf, NULL);
}
CU_pSuite proto_tests_getSuite()
{
   /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test proto module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, testUfFormat);
   CU_ADD_TEST(suite, testVersionCmp);
   CU_ADD_TEST(suite, testUnpackMetaFile);

   return suite;
}
