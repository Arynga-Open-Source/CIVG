/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "downloadmgr_tests.h"
#include "downloadmgr_priv.h"
#include "downloaditem.h"
#include "test_downloader/test_downloader.h"
#include "strtools.h"
#include "filetools.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>

static int _dInitRes = 0;
static int _dInitCalled = 0;
static int dInit() {
    ++_dInitCalled;
    return _dInitRes;
}

static int _dCleanupCalled = 0;
static void dCleanup() {
    ++_dCleanupCalled;
}

static DOWNLOADER_FINISHED_HANDLER _handler = 0;
static void dHandler(DOWNLOADER_FINISHED_HANDLER handler) {
    _handler = handler;
}

static downloaditem_Item* _diFirst = 0;
static int dDownloadItem(const char* url, const char* path, char** id) {
    *id = strtools_sprintNew("%s", url);
    downloaditem_newItemPreppend(&_diFirst);
    _diFirst->id = strtools_sprintNew("%s", url);
    _diFirst->path = strtools_sprintNew("%s", path);
    return 0;
}

static downloaditem_Item* _diCanceledFirst = 0;
static int dCancel(const char* id) {
    downloaditem_newItemPreppend(&_diCanceledFirst);
    _diCanceledFirst->id = strtools_sprintNew("%s", id);
    return 0;
}

static int _mgrFinishedStatus = 0;
static void mgrFinished(int status) {
    _mgrFinishedStatus = status;
}

static config_Storage* _configStorage = 0;

static int init()
{
    _configStorage = config_newStorage(DOWNLOADMGR_STORAGE_ID);
    testdownloader_init(dInit);
    testdownloader_cleanup(dCleanup);
    testdownloader_setFinishedHandler(dHandler);
    testdownloader_downloadItem(dDownloadItem);
    testdownloader_cancel(dCancel);
    return 0;
}

static int cleanup()
{
    config_deleteStorage(_configStorage);
    _configStorage = 0;
    while (downloaditem_deleteFirstItem(&_diFirst) != 0);
    return 0;
}

static void initAndCleanupTests() {
    // downloader init failed
    _dInitRes = 1;
    _dInitCalled = 0;
    _dCleanupCalled = 0;
    CU_ASSERT_FATAL(downloadmgr_init() == 1);
    CU_ASSERT_FATAL(_dInitCalled == 1);
    CU_ASSERT_FATAL(_dCleanupCalled == 1);

    // init succeed so msgHandler should be set
    _dInitRes = 0;
    _handler = 0;
    CU_ASSERT_FATAL(downloadmgr_init() == 0);
    CU_ASSERT_FATAL(_handler != 0);
    CU_ASSERT_FATAL(_dInitCalled == 2);
    CU_ASSERT_FATAL(_dCleanupCalled == 1);

    // downloader cleanup should be called
    downloadmgr_cleanup();
    downloadmgr_cleanup();
    CU_ASSERT_FATAL(_dCleanupCalled == 2);
}

static void downloadTests() {
    _dInitRes = 0;
    CU_ASSERT_FATAL(downloadmgr_init() == 0);

    CU_ASSERT_FATAL(filetools_removeDir("core/tests/work_dir") == 0);
    downloadmgr_setFinishedHandler(mgrFinished);
    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    // nothind to download but no error
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);
    // invalid path
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "") > 0);

    uDescriptor.n_updates = 2;
    uDescriptor.updates = malloc(sizeof(proto_UpdateFileItem*) * uDescriptor.n_updates);
    uDescriptor.updates[0] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[0]);
    uDescriptor.updates[0]->uuid = "1234567890";
    uDescriptor.updates[0]->url = "0URL";
    uDescriptor.updates[1] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[1]);
    uDescriptor.updates[1]->uuid = "0987654321";
    uDescriptor.updates[1]->url = "1URL";

    _mgrFinishedStatus = -1;
    // download some UFs
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);
    // doubled download should fail
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") > 0);
    // any UF download failed means whole download failed
    _handler("0URL", 1);
    CU_ASSERT_FATAL(_mgrFinishedStatus > 0);

    _mgrFinishedStatus = -1;
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);
    // meta download succeed, but no .uf found means download failed
    _handler("0URL", 0);
    CU_ASSERT_FATAL(_mgrFinishedStatus > 0);

    while (downloaditem_deleteFirstItem(&_diCanceledFirst) != 0);
    _mgrFinishedStatus = -1;
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);
    // cancel doesn't trigger download finished, but pending downloads should be aborted
    CU_ASSERT_FATAL(downloadmgr_cancel() == 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diCanceledFirst, "0URL") != 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diCanceledFirst, "1URL") != 0);
    CU_ASSERT_FATAL(_mgrFinishedStatus == -1);
    while (downloaditem_deleteFirstItem(&_diCanceledFirst) != 0);

    while (downloaditem_deleteFirstItem(&_diFirst) != 0);
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);
    // simulate downloaded .uf
    downloaditem_Item* it = _diFirst;
    while (it) {
        char* dir = filetools_getDir(it->path);
        char* cmd = strtools_sprintNew("mkdir -p \"%s\" && cp core/tests/test.uf \"%s\"", dir, it->path);
        CU_ASSERT_FATAL(system(cmd) == 0);
        it = it->next;
        free(cmd);
        free(dir);
    }
    // meta downloaded
    _handler("0URL", 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "0URL") != 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch") != 0);
    CU_ASSERT_FATAL(_mgrFinishedStatus == -1);
    // meta downloaded
    _handler("1URL", 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "1URL") != 0);
    CU_ASSERT_FATAL(_mgrFinishedStatus == -1);
    // data downloaded
    _handler("http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch", 0);
    CU_ASSERT_FATAL(_mgrFinishedStatus == -1);
    // data downloaded
    _handler("http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch", 0);
    // download succeed
    CU_ASSERT_FATAL(_mgrFinishedStatus == 0);
    //start next download
    CU_ASSERT_FATAL(downloadmgr_downloadUFs(&uDescriptor, "core/tests/work_dir") == 0);

    while (downloaditem_deleteFirstItem(&_diFirst) != 0);

    for (unsigned int i = 0; i < uDescriptor.n_updates; ++i) {
        free(uDescriptor.updates[i]);
    }
    free(uDescriptor.updates);
    downloadmgr_cleanup();
}

static void markDownloaded(const char* path) {
    int diSize = 0;
    config_getIValue(_configStorage, DI_SIZE_KEY, &diSize);
    snprintf(itemKey, ITEM_KEY_SIZE, DI_PATH_KEY_FORMAT, diSize);
    config_setValue(_configStorage, itemKey, path, strlen(path) + 1);
    ++diSize;
    config_setIValue(_configStorage, DI_SIZE_KEY, diSize);
}

static void resumeTests() {
    _dInitRes = 0;
    CU_ASSERT_FATAL(downloadmgr_init() == 0);

    CU_ASSERT_FATAL(filetools_removeDir("core/tests/work_dir") == 0);
    downloadmgr_setFinishedHandler(mgrFinished);
    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    CU_ASSERT_FATAL(downloadmgr_resume(&uDescriptor, "core/tests/work_dir") == 0);

    uDescriptor.n_updates = 4;
    uDescriptor.updates = malloc(sizeof(proto_UpdateFileItem*) * uDescriptor.n_updates);
    uDescriptor.updates[0] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[0]);
    uDescriptor.updates[0]->uuid = "1234567890";
    uDescriptor.updates[0]->url = "0URL";
    uDescriptor.updates[1] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[1]);
    uDescriptor.updates[1]->uuid = "0987654321";
    uDescriptor.updates[1]->url = "1URL";
    uDescriptor.updates[2] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[2]);
    uDescriptor.updates[2]->uuid = "zxcvbnm";
    uDescriptor.updates[2]->url = "2URL";
    uDescriptor.updates[3] = malloc(sizeof(proto_UpdateFileItem));
    car_sync__proto__release_package__update_file_item__init(uDescriptor.updates[3]);
    uDescriptor.updates[3]->uuid = "mnbvcxz";
    uDescriptor.updates[3]->url = "3URL";

    _mgrFinishedStatus = -1;
    CU_ASSERT_FATAL(downloadmgr_resume(&uDescriptor, "core/tests/work_dir") == 0);
    // doubled resume should return error
    CU_ASSERT_FATAL(downloadmgr_resume(&uDescriptor, "core/tests/work_dir") > 0);
    _handler("0URL", 1);
    CU_ASSERT_FATAL(_mgrFinishedStatus > 0);

    while (downloaditem_deleteFirstItem(&_diFirst) != 0);
    _mgrFinishedStatus = -1;
    // meta 0 downloaded
    markDownloaded("core/tests/work_dir/1234567890/1234567890.uf");
    CU_ASSERT_FATAL(system("mkdir -p core/tests/work_dir/1234567890/ && cp core/tests/test.uf core/tests/work_dir/1234567890/1234567890.uf") == 0);
    // data 0 downloaded as well
    markDownloaded("core/tests/work_dir/1234567890/data/ivi4-demo.patch");
    // meta 1 downloaded, but data not
    markDownloaded("core/tests/work_dir/0987654321/0987654321.uf");
    // meta 2 and data 2 not downloaded
    // finally meta 3 marked as downloaded, but no .uf - means error
    markDownloaded("core/tests/work_dir/mnbvcxz/mnbvcxz.uf");
    CU_ASSERT_FATAL(system("mkdir -p core/tests/work_dir/0987654321/ && cp core/tests/test.uf core/tests/work_dir/0987654321/0987654321.uf") == 0);
    CU_ASSERT_FATAL(downloadmgr_resume(&uDescriptor, "core/tests/work_dir") > 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "0URL") == 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "1URL") == 0);
    downloaditem_Item* it = _diFirst;
    int count = 0;
    while (it) {
        if (strcmp(it->id, "http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch") == 0) {
            ++count;
        }
        it = it->next;
    }
    CU_ASSERT_FATAL(count == 1);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "2URL") != 0);
    CU_ASSERT_FATAL(downloaditem_findItemById(_diFirst, "3URL") == 0);
    while (downloaditem_deleteFirstItem(&_diFirst) != 0);

    for (unsigned int i = 0; i < uDescriptor.n_updates; ++i) {
        free(uDescriptor.updates[i]);
    }
    free(uDescriptor.updates);
    downloadmgr_cleanup();
}

CU_pSuite downloadmgr_tests_getSuite()
{
   /* add a suite to the registry */
   CU_pSuite suite = CU_add_suite("Test download manager module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, initAndCleanupTests);
   CU_ADD_TEST(suite, downloadTests);
   CU_ADD_TEST(suite, resumeTests);

   return suite;
}
