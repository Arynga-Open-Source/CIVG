/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */
#include "config.h"
#include "rpmanager_priv.h"
#include "rpmanager_tests.h"
#include "strtools.h"
#include <stdlib.h>
#include <string.h>

#define RP_UUID_1 "1234567890"
#define RP_UUID_2 "1020304050"
#define RP_UUID_3 "9999999999"
#define RP_UUID_4 "8888888888"

#define UF_UUID_1 "111222333"
#define UF_UUID_2 "444555666"
#define UF_UUID_3 "777888999"

static config_Storage* _configStorage = 0;
static proto_InstalledConfig* _installedConfig = 0;
static proto_AvailableConfig* _availableConfig = 0;

static const char* _updateFilesPath1="rpmanager/tests/"RP_UUID_1;
static const char* _updateFilesPath2="rpmanager/tests/"RP_UUID_2;
static const char* _updateFilesPath3="rpmanager/tests/"RP_UUID_3;
static const char* _updateFilesPath4="rpmanager/tests/"RP_UUID_4;

static void deinitializeAvailableconfig()
{
    if (_availableConfig) {
        for (size_t i=0; i < _availableConfig->update->n_updates; ++i) {
            free(_availableConfig->update->updates[i]->version);
            free(_availableConfig->update->updates[i]->uuid);
            free(_availableConfig->update->updates[i]);
        }

        free(_availableConfig->update->updates);
        free(_availableConfig->update->uuid);
        free(_availableConfig->update->from_version);
        free(_availableConfig->update->to_version);
        free(_availableConfig->update);

        for (size_t i=0; i < _availableConfig->n_rps; ++i) {
            free(_availableConfig->rps[i]->version);
            free(_availableConfig->rps[i]->uuid);
            free(_availableConfig->rps[i]);
        }
        free(_availableConfig->rps);
        free(_availableConfig);
        _availableConfig = 0;
    }
}

static void initializeAvailableconfig()
{
    deinitializeAvailableconfig();

    _availableConfig=(proto_AvailableConfig*)malloc(sizeof(proto_AvailableConfig));
    car_sync__proto__messages__rpcconfiguration_rsp__init(_availableConfig);

    _availableConfig->n_rps = 3;
    _availableConfig->rps = (proto_ReleasePackageMeta**)malloc(_availableConfig->n_rps * sizeof(proto_ReleasePackageMeta*));

    for (size_t i = 0; i < _availableConfig->n_rps; ++i) {
        _availableConfig->rps[i] = (proto_ReleasePackageMeta*)malloc(sizeof(proto_ReleasePackageMeta));
        car_sync__proto__release_package__release_package_meta__init(_availableConfig->rps[i]);

        _availableConfig->rps[i]->version = (proto_Version*)malloc(sizeof(proto_Version));
        car_sync__proto__common__version__init(_availableConfig->rps[i]->version);
        _availableConfig->rps[i]->version->has_major_version = 1;
        _availableConfig->rps[i]->version->has_minor_version = 1;
        _availableConfig->rps[i]->version->has_build_version = 1;
        _availableConfig->rps[i]->version->major_version = 1;
        _availableConfig->rps[i]->version->minor_version = 2;
        _availableConfig->rps[i]->version->build_version = 4;
    }

    _availableConfig->rps[0]->uuid = strtools_sprintNew(RP_UUID_1);
    _availableConfig->rps[1]->uuid = strtools_sprintNew(RP_UUID_2);
    _availableConfig->rps[2]->uuid = strtools_sprintNew(RP_UUID_3);

    _availableConfig->update = (proto_UpdateDescriptor*)malloc(sizeof(proto_UpdateDescriptor));
    car_sync__proto__release_package__update_descriptor__init(_availableConfig->update);

    _availableConfig->update->from_version = (proto_Version*)malloc(sizeof(proto_Version));
    car_sync__proto__common__version__init(_availableConfig->update->from_version);
    _availableConfig->update->from_version->has_major_version = 1;
    _availableConfig->update->from_version->has_minor_version = 1;
    _availableConfig->update->from_version->has_build_version = 1;
    _availableConfig->update->from_version->major_version = 1;
    _availableConfig->update->from_version->minor_version = 2;
    _availableConfig->update->from_version->build_version = 3;

    _availableConfig->update->to_version = (proto_Version*)malloc(sizeof(proto_Version));
    car_sync__proto__common__version__init(_availableConfig->update->to_version);
    _availableConfig->update->to_version->has_major_version = 1;
    _availableConfig->update->to_version->has_minor_version = 1;
    _availableConfig->update->to_version->has_build_version = 1;
    _availableConfig->update->to_version->major_version = 1;
    _availableConfig->update->to_version->minor_version = 2;
    _availableConfig->update->to_version->build_version = 4;

    _availableConfig->update->uuid = strtools_sprintNew(RP_UUID_1);

    _availableConfig->update->n_updates = 3;
    _availableConfig->update->updates = (proto_UpdateFileItem**)malloc(_availableConfig->update->n_updates*sizeof(proto_UpdateFileItem*));
    for (unsigned int i = 0; i < _availableConfig->update->n_updates; ++i) {
        _availableConfig->update->updates[i] = (proto_UpdateFileItem*)malloc(sizeof(proto_UpdateFileItem));
        car_sync__proto__release_package__update_file_item__init(_availableConfig->update->updates[i]);

        _availableConfig->update->updates[i]->version = (proto_Version*)malloc(sizeof(proto_Version));
        car_sync__proto__common__version__init(_availableConfig->update->updates[i]->version);
    }

    _availableConfig->update->updates[0]->uuid = strtools_sprintNew(UF_UUID_1);
    _availableConfig->update->updates[1]->uuid = strtools_sprintNew(UF_UUID_2);
    _availableConfig->update->updates[2]->uuid = strtools_sprintNew(UF_UUID_3);
}

static void deinitializeInstalledconfig()
{
    if (_installedConfig) {
        for (size_t i = 0; i < _installedConfig->n_rps; ++i) {
            free(_installedConfig->rps[i]->version);
            free(_installedConfig->rps[i]->uuid);
            free(_installedConfig->rps[i]);
        }
        free(_installedConfig->rps);
        free(_installedConfig);
        _installedConfig = 0;
    }
}

static void initializeInstalledconfig()
{
    deinitializeInstalledconfig();

    _installedConfig = (proto_InstalledConfig*)malloc(sizeof(proto_InstalledConfig));
    car_sync__proto__messages__rpcconfiguration_req__init(_installedConfig);

    _installedConfig->n_rps = 3;
    _installedConfig->rps = (proto_ReleasePackageMeta**)malloc(_installedConfig->n_rps * sizeof(proto_ReleasePackageMeta*));

    for (size_t i = 0; i < _installedConfig->n_rps; ++i) {
        _installedConfig->rps[i] = (proto_ReleasePackageMeta*)malloc(sizeof(proto_ReleasePackageMeta));
        car_sync__proto__release_package__release_package_meta__init(_installedConfig->rps[i]);

        _installedConfig->rps[i]->version = (proto_Version*)malloc(sizeof(proto_Version));
        car_sync__proto__common__version__init(_installedConfig->rps[i]->version);
        _installedConfig->rps[i]->version->has_major_version = 1;
        _installedConfig->rps[i]->version->has_minor_version = 1;
        _installedConfig->rps[i]->version->has_build_version = 1;
        _installedConfig->rps[i]->version->major_version = 1;
        _installedConfig->rps[i]->version->minor_version = 2;
        _installedConfig->rps[i]->version->build_version = 3;
    }

    _installedConfig->rps[0]->uuid = strtools_sprintNew(RP_UUID_1);
    _installedConfig->rps[1]->uuid = strtools_sprintNew(RP_UUID_2);
    _installedConfig->rps[2]->uuid = strtools_sprintNew(RP_UUID_3);
}

static void saveRPManagerState(rpmanager_Status status, int last_installed_uf_nb)
{
    CU_ASSERT_FATAL(config_setIValue(_configStorage, RPMANAGER_STATE_KEY, status) == 0);

    if (last_installed_uf_nb < 0) {
        config_removeKey(_configStorage, LAST_INSTALLED_UF_NB_KEY);
    } else {
        CU_ASSERT_FATAL(config_setIValue(_configStorage, LAST_INSTALLED_UF_NB_KEY, last_installed_uf_nb) == 0);
    }

    int bufferSize = car_sync__proto__messages__rpcconfiguration_req__get_packed_size(_installedConfig);
    void* buffer = malloc(bufferSize);
    car_sync__proto__messages__rpcconfiguration_req__pack(_installedConfig, buffer);
    CU_ASSERT_FATAL(config_setValue(_configStorage, INSTALLED_CONFIG_KEY, buffer, bufferSize) == 0);
    free(buffer);
}

static int init()
{
    _configStorage = config_newStorage(RPMANAGER_STORAGE_ID);
    return (_configStorage == 0);
}

static int cleanup()
{
    deinitializeAvailableconfig();
    deinitializeInstalledconfig();
    config_deleteStorage(_configStorage);
    _configStorage = 0;
    return 0;
}

static void testRpmanagerGetinstalledconfig()
{
    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    deinitializeInstalledconfig();

    CU_ASSERT_FATAL(rpmanager_init() == 0);

    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_1) == 0);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[1]->uuid,RP_UUID_2) == 0);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[2]->uuid,RP_UUID_3) == 0);

    for (size_t i = 0; i < _installedConfig->n_rps; ++i) {
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->has_major_version == 1);
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->has_minor_version == 1);
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->has_build_version == 1);
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->major_version == 1);
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->minor_version == 2);
        CU_ASSERT_FATAL(_installedConfig->rps[i]->version->build_version == 3);
    }

    rpmanager_cleanup();
}

static void testRpmanagerConsistencyError()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_PENDING, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "Cannot run update with inconsistent state! Try resuming previous update first.") == 0);
    free(statusMessage);
    statusMessage=NULL;

    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);

    CU_ASSERT_FATAL(rpmanager_resume(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "There is nothing to resume, previous update was either successful or completely rolled back.") == 0);
    free(statusMessage);
    statusMessage=NULL;

    rpmanager_cleanup();
}

static void testRpmanagerVersionMismatch()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    //change to wrong version
    _installedConfig->rps[0]->version->build_version=6;
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "Installed package version and update \"from\" version do not match!") == 0);
    free(statusMessage);
    statusMessage=NULL;

    rpmanager_cleanup();
}

static void testRpmanagerNoUpdates()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    //change nb of updates to 0
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    _availableConfig->update->n_updates = 0;
    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "No updates were found in available config.") == 0);
    free(statusMessage);
    statusMessage=NULL;
    //change back
    _availableConfig->update->n_updates=3;

    rpmanager_cleanup();
}

static void testRpmanagerUpdate()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_OK);
    CU_ASSERT_FATAL(statusMessage == 0);

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_1) == 0);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 4);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    rpmanager_cleanup();
}

static void testRpmanagerResumeUpdate()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_PENDING, 0);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_resume(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_OK);
    CU_ASSERT_FATAL(statusMessage == 0);

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_1) == 0);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 4);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    rpmanager_cleanup();
}

static void testRpmanagerResumeRollback()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_ROLLBACK_PENDING, 1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_resume(_availableConfig, _updateFilesPath1, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "Failed to install release package, but rollback was successful.") == 0);
    free(statusMessage);
    statusMessage=NULL;

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_1) == 0);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 3);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    rpmanager_cleanup();
}

static void testRpmanagerBadUpdateScript()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    strcpy(_availableConfig->update->uuid, RP_UUID_2);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath2, &statusMessage) == RPMANAGER_STATUS_ERROR);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "Failed to install release package, but rollback was successful.") == 0);
    free(statusMessage);
    statusMessage=NULL;

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_1) == 0);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 3);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    rpmanager_cleanup();
}

static void testRpmanagerBadRollbackScript()
{
    char* statusMessage = 0;

    initializeInstalledconfig();
    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);
    initializeAvailableconfig();

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    strcpy(_availableConfig->update->uuid, RP_UUID_3);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath3, &statusMessage) == RPMANAGER_STATUS_CRITICAL);
    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_FATAL(strcmp(statusMessage, "Failed to apply image! UUID: 777888999 *** Error message from imagehandler: Script returned non-zero status! Status returned: 2 Failed to rollback image! UUID: 111222333 *** Error message from imagehandler: Script returned non-zero status! Status returned: 2") == 0);
    free(statusMessage);
    statusMessage=NULL;

    rpmanager_cleanup();
}


static void generateRpWithOneUF(const char* rp, const char* uf)
{
    initializeInstalledconfig();
    initializeAvailableconfig();

    free(_installedConfig->rps[0]->uuid);
    _installedConfig->rps[0]->uuid = strtools_sprintNew(rp);
    free(_availableConfig->rps[0]->uuid);
    _availableConfig->rps[0]->uuid = strtools_sprintNew(rp);

    free(_availableConfig->update->uuid);
    _availableConfig->update->uuid = strtools_sprintNew(rp);
    free(_availableConfig->update->updates[0]->uuid);
    _availableConfig->update->updates[0]->uuid = strtools_sprintNew(uf);
}

static void testRpmanagerNoImageData()
{
    char* statusMessage = 0;

    deinitializeInstalledconfig();
    deinitializeAvailableconfig();

    // installed config - no image data available
    generateRpWithOneUF(RP_UUID_4, UF_UUID_1);

    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath4, &statusMessage) == RPMANAGER_STATUS_ERROR);

    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_STRING_EQUAL_FATAL(statusMessage, "Failed to install release package, but rollback was successful.");
    free(statusMessage);

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_4) == 0);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 3);

    rpmanager_cleanup();
}

static void testRpmanagerWrongChecksum()
{
    char* statusMessage = 0;

    deinitializeInstalledconfig();
    deinitializeAvailableconfig();

    // installed config - image with wrong checksum
    generateRpWithOneUF(RP_UUID_4, UF_UUID_2);

    saveRPManagerState(RPMANAGER_STATE_UPDATE_SUCCESS, -1);

    //initialize RPManager
    CU_ASSERT_FATAL(rpmanager_init() == 0);

    //perform update
    CU_ASSERT_FATAL(rpmanager_applyRP(_availableConfig, _updateFilesPath4, &statusMessage) == RPMANAGER_STATUS_ERROR);

    CU_ASSERT_FATAL(statusMessage != 0);
    CU_ASSERT_STRING_EQUAL_FATAL(statusMessage, "Failed to install release package, but rollback was successful.");
    free(statusMessage);

    deinitializeInstalledconfig();
    CU_ASSERT_FATAL(rpmanager_getInstalledConfig(&_installedConfig) == RPMANAGER_STATUS_OK);

    CU_ASSERT_FATAL(_installedConfig->n_rps == 3);
    CU_ASSERT_FATAL(strcmp(_installedConfig->rps[0]->uuid,RP_UUID_4) == 0);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_minor_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->has_build_version == 1);

    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->major_version == 1);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->minor_version == 2);
    CU_ASSERT_FATAL(_installedConfig->rps[0]->version->build_version == 3);

    rpmanager_cleanup();
}

CU_pSuite rpmanager_getSuite()
{
    /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test RPManager module", init, cleanup);
    if (!suite) {
        return 0;
    }

    CU_ADD_TEST(suite, testRpmanagerGetinstalledconfig);
    CU_ADD_TEST(suite, testRpmanagerConsistencyError);
    CU_ADD_TEST(suite, testRpmanagerVersionMismatch);
    CU_ADD_TEST(suite, testRpmanagerNoUpdates);
    CU_ADD_TEST(suite, testRpmanagerUpdate);
    CU_ADD_TEST(suite, testRpmanagerResumeUpdate);
    CU_ADD_TEST(suite, testRpmanagerResumeRollback);
    CU_ADD_TEST(suite, testRpmanagerBadUpdateScript);
    CU_ADD_TEST(suite, testRpmanagerBadRollbackScript);
    CU_ADD_TEST(suite, testRpmanagerNoImageData);
    CU_ADD_TEST(suite, testRpmanagerWrongChecksum);


   return suite;
}
