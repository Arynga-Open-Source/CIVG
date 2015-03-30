/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "core_tests.h"
#include "core_priv.h"
#include "dummy_carinfo_priv.h"
#include "test_downloader/test_downloader.h"
#include "test_transport/test_transport.h"
#include "test_rpmanager/test_rpmanager.h"
#include "config.h"
#include "downloaditem.h"
#include "strtools.h"
#include "filetools.h"
#include <stdlib.h>

static int _tInitRes = 0;
static int _tInitCalled = 0;
static int tInit() {
    ++_tInitCalled;
    return _tInitRes;
}

static int _tCleanupCalled = 0;
static void tCleanup() {
    ++_tCleanupCalled;
}

static void* _lastMsg = 0;
static int _lastMsgSize = 0;
static int tSend(const void* msg, int size) {
    free(_lastMsg);
    _lastMsgSize = size;
    if (size > 0) {
        _lastMsg = malloc(_lastMsgSize);
        memcpy(_lastMsg, msg, _lastMsgSize);
    } else {
        _lastMsg = 0;
    }
    return 0;
}

static TRANSPORT_MSG_HANDLER _tHandler = 0;
static void tHandler(TRANSPORT_MSG_HANDLER handler) {
    _tHandler = handler;
}

static int _rpInitRes = 0;
static int _rpInitCalled = 0;
static int rpInit() {
    ++_rpInitCalled;
    return _rpInitRes;
}

static int _rpCleanupCalled = 0;
static void rpCleanup() {
    ++_rpCleanupCalled;
}

static rpmanager_Status _rpApplyRes = RPMANAGER_STATUS_OK;
static int _rpApplyCalled = 0;
static const char* _lastApplyUFPath = 0;
static const proto_AvailableConfig* _lastApplyConfig = 0;
static char* _rpApplyStatusMessage = 0;
static rpmanager_Status rpApply(const proto_AvailableConfig* availableConfig,
                                const char* updateFilesPath, char** statusMessage) {
    ++_rpApplyCalled;
    _lastApplyConfig = availableConfig;
    _lastApplyUFPath = updateFilesPath;
    *statusMessage = _rpApplyStatusMessage;
    return _rpApplyRes;
}

static rpmanager_Status _rpResumeRes = RPMANAGER_STATUS_OK;
static int _rpResumeCalled = 0;
static const char* _lastResumeUFPath = 0;
static const proto_AvailableConfig* _lastResumeConfig = 0;
static char* _rpResumeStatusMessage = 0;
static rpmanager_Status rpResume(const proto_AvailableConfig* availableConfig,
                                 const char* updateFilesPath, char** statusMessage) {
    ++_rpResumeCalled;
    _lastResumeConfig = availableConfig;
    _lastResumeUFPath = updateFilesPath;
    *statusMessage = strtools_sprintNew("%s", _rpResumeStatusMessage);
    return _rpResumeRes;
}

static rpmanager_Status _rpInstalledConfigRes = RPMANAGER_STATUS_OK;
static int _rpInstalledConfigCalled = 0;
static rpmanager_Status rpInstalledConfig(proto_InstalledConfig ** installedConfig) {
    ++_rpInstalledConfigCalled;
    if (_rpInstalledConfigRes == RPMANAGER_STATUS_OK) {
        proto_InstalledConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_REQ__INIT;
        config.n_rps = 2;
        CarSync__Proto__ReleasePackage__ReleasePackageMeta a = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
        a.uuid = "1234567890";
        proto_Version v = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
        v.has_major_version = 1;
        v.major_version = 3;
        v.has_minor_version = 1;
        v.minor_version = 4;
        v.has_build_version = 1;
        v.build_version = 5;
        a.version = &v;
        CarSync__Proto__ReleasePackage__ReleasePackageMeta b = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
        CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[2] = {&a, &b};
        config.rps = rps;
        int size = car_sync__proto__messages__rpcconfiguration_req__get_packed_size(&config);
        void* buff = malloc(size);
        car_sync__proto__messages__rpcconfiguration_req__pack(&config, buff);
        *installedConfig = car_sync__proto__messages__rpcconfiguration_req__unpack(NULL, size, buff);
        free(buff);
    }
    return _rpInstalledConfigRes;
}

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

static DOWNLOADER_FINISHED_HANDLER _dHandler = 0;
static void dHandler(DOWNLOADER_FINISHED_HANDLER handler) {
    _dHandler = handler;
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

static core_State _lastCivgState = CORE_STATE_IDLE;
static core_Status _lastCivgStatus = CORE_STATUS_OK;
static void civgStatusHandler(const core_State state, core_Status status) {
    _lastCivgState = state;
    _lastCivgStatus = status;
}

static config_Storage* _storage = 0;
static int init()
{
    testtransport_init(tInit);
    testtransport_cleanup(tCleanup);
    testtransport_sendMsg(tSend);
    testtransport_setMsgHandler(tHandler);
    testrpmanager_init(rpInit);
    testrpmanager_cleanup(rpCleanup);
    testrpmanager_applyRP(rpApply);
    testrpmanager_resume(rpResume);
    testrpmanager_getInstalledConfig(rpInstalledConfig);
    testdownloader_init(dInit);
    testdownloader_cleanup(dCleanup);
    testdownloader_setFinishedHandler(dHandler);
    testdownloader_downloadItem(dDownloadItem);
    testdownloader_cancel(dCancel);

    _storage = config_newStorage(CORE_STORAGE_ID);

    int status = 1;
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    if (storage) {
        const char vin[] = "testVIN";
        const int size = sizeof(vin);
        if (config_setValue(storage, VIN_KEY, vin, size) == 0) {
            status = 0;
        }
        config_deleteStorage(storage);
    }
    return status;
}

static int cleanup()
{
    config_deleteStorage(_storage);
    while (downloaditem_deleteFirstItem(&_diFirst) != 0);
    return 0;
}

static void idleSetup() {
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_IDLE) == 0);
    config_removeKey(_storage, AVAILABLE_CONFIG_KEY);
    _tInitCalled = 0;
    _tInitRes = 0;
    _dInitCalled = 0;
    _dInitRes = 0;
    _rpInitCalled = 0;
    _rpInitRes = 0;
}

static void initAndCleanupTests() {
    idleSetup();
    // transport init failed - critical, but nothing can do
    _tInitRes = 1;
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);
    CU_ASSERT_FATAL(_tInitCalled == 1);

    // rpmanager init failed - critical, send report
    free(_lastMsg);
    _lastMsg = 0;
    _tInitRes = 0;
    _rpInitRes = 1;
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);
    CU_ASSERT_FATAL(_rpInitCalled == 1);
    CU_ASSERT_FATAL(_lastMsg != 0);

    // dwonloader init failed - critical, send report
    free(_lastMsg);
    _lastMsg = 0;
    _rpInitRes = 0;
    _dInitRes = 1;
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);
    CU_ASSERT_FATAL(_dInitCalled == 1);
    CU_ASSERT_FATAL(_lastMsg != 0);

    _dInitRes = 0;
    // error in rpmanager_getInstalledConfig - enter broken state
    _tCleanupCalled = 0;
    _dCleanupCalled = 0;
    _rpCleanupCalled = 0;
    _rpInstalledConfigRes = RPMANAGER_STATUS_ERROR;
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);
    CU_ASSERT_FATAL(_tCleanupCalled == 1);
    CU_ASSERT_FATAL(_dCleanupCalled == 1);
    CU_ASSERT_FATAL(_rpCleanupCalled == 1);
    _rpInstalledConfigRes = RPMANAGER_STATUS_OK;

    _tInitCalled = 0;
    _tInitRes = 0;
    _dInitCalled = 0;
    _dInitRes = 0;
    _rpInitCalled = 0;
    _rpInitRes = 0;
    // successfule init
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_OK);
    CU_ASSERT_FATAL(_tInitCalled == 1);
    CU_ASSERT_FATAL(_dInitCalled == 1);
    CU_ASSERT_FATAL(_rpInitCalled == 1);
    // doubled initialization should fail
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_ERROR);
    _tCleanupCalled = 0;
    _dCleanupCalled = 0;
    _rpCleanupCalled = 0;
    // cleanup should call all submodules cleanup
    core_cleanup();
    core_cleanup();
    CU_ASSERT_FATAL(_tCleanupCalled == 1);
    CU_ASSERT_FATAL(_dCleanupCalled == 1);
    CU_ASSERT_FATAL(_rpCleanupCalled == 1);
}

static void resumeDownloadTests() {
    idleSetup();
    // resume download, but no available config found
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_DOWNLOAD) == 0);
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_ERROR);
    core_cleanup();

    // resume download, but invalid config in storage found
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_DOWNLOAD) == 0);
    CU_ASSERT_FATAL(config_setValue(_storage, AVAILABLE_CONFIG_KEY, "giberish", 9) == 0);
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_ERROR);
    core_cleanup();

    // succesfull download resume
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_DOWNLOAD) == 0);
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    uDescriptor.uuid = "1234567890";
    proto_Version toV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    toV.has_major_version = 1;
    toV.major_version = 7;
    toV.has_minor_version = 1;
    toV.minor_version = 8;
    toV.has_build_version = 1;
    toV.build_version = 9;
    uDescriptor.to_version = &toV;
    config.update = &uDescriptor;
    int bufSize = car_sync__proto__messages__rpcconfiguration_rsp__get_packed_size(&config);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__rpcconfiguration_rsp__pack(&config, buf);
    CU_ASSERT_FATAL(config_setValue(_storage, AVAILABLE_CONFIG_KEY, buf, bufSize) == 0);
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_OK);
    free(buf);
    core_cleanup();
}

static void resumeUpgradeTests() {
    idleSetup();
    // resume upgrade, but no available config found
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_UPGRADE) == 0);
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);

    // resume upgrade, but rpmanager_resume fails
    _rpResumeRes = RPMANAGER_STATUS_CRITICAL;
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    uDescriptor.uuid = "1234567890";
    proto_Version toV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    toV.has_major_version = 1;
    toV.major_version = 7;
    toV.has_minor_version = 1;
    toV.minor_version = 8;
    toV.has_build_version = 1;
    toV.build_version = 9;
    uDescriptor.to_version = &toV;
    config.update = &uDescriptor;
    int bufSize = car_sync__proto__messages__rpcconfiguration_rsp__get_packed_size(&config);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__rpcconfiguration_rsp__pack(&config, buf);
    CU_ASSERT_FATAL(config_setValue(_storage, AVAILABLE_CONFIG_KEY, buf, bufSize) == 0);
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_CRITICAL);

    // resume upgrade, but rpmanager_resume fails with critical
    _rpResumeRes = RPMANAGER_STATUS_ERROR;
    _rpResumeStatusMessage = "test error message";
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_ERROR);
    core_cleanup();

    // succesfull upgrade resume
    CU_ASSERT_FATAL(config_setIValue(_storage, STATE_KEY, CORE_STATE_UPGRADE) == 0);
    CU_ASSERT_FATAL(config_setValue(_storage, AVAILABLE_CONFIG_KEY, buf, bufSize) == 0);
    _rpResumeRes = RPMANAGER_STATUS_OK;
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_OK);
    free(buf);

    core_cleanup();
}

static void handleRpc(proto_BaseMessage* baseMsg, proto_RPC* rpc) {
    baseMsg->data.len = car_sync__proto__messages__rpc__get_packed_size(rpc);
    baseMsg->data.data = malloc(baseMsg->data.len);
    car_sync__proto__messages__rpc__pack(rpc, baseMsg->data.data);
    int bufSize = car_sync__proto__messages__base_message__get_packed_size(baseMsg);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__base_message__pack(baseMsg, buf);
    _tHandler(buf, bufSize);
    free(baseMsg->data.data);
    free(buf);
}

static void nothingToDownloadScenarioTests() {
    idleSetup();
    CU_ASSERT_FATAL(core_init(1) == CORE_STATUS_OK);

    proto_BaseMessage baseMsg = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__INIT;
    baseMsg.vin = "testVIN";
    baseMsg.has_data_type = 1;
    baseMsg.data_type = proto_MESSAGE_CONFIG_RPC;
    baseMsg.has_data = 1;
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    config.n_rps = 2;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta a = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta b = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[2] = {&a, &b};
    config.rps = rps;
    rpc.conf_rsp = &config;
    CU_ASSERT_FATAL(_lastMsg != 0);
    proto_BaseMessage* baseMsgReq = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    CU_ASSERT_FATAL(baseMsgReq != 0);
    CU_ASSERT_FATAL(baseMsgReq->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsgReq->data_type == (int)proto_MESSAGE_CONFIG_RPC);
    CU_ASSERT_FATAL(baseMsgReq->has_data == 1);
    proto_RPC* rpcReq = car_sync__proto__messages__rpc__unpack(NULL, baseMsgReq->data.len, baseMsgReq->data.data);
    CU_ASSERT_FATAL(rpcReq != 0);
    CU_ASSERT_FATAL(rpcReq->rpc_id != 0);
    rpc.rpc_id = strtools_sprintNew("%s", rpcReq->rpc_id);
    car_sync__proto__messages__rpc__free_unpacked(rpcReq, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsgReq, NULL);
    free(_lastMsg);
    _lastMsg = 0;
    // available config successfully received
    handleRpc(&baseMsg, &rpc);
    free(rpc.rpc_id);
    CU_ASSERT_FATAL(_lastMsg == 0);

    core_cleanup();
}

static void upgradeWithDownloadScenarioTests() {
    idleSetup();
    CU_ASSERT_FATAL(core_init(0) == CORE_STATUS_OK);
    core_setStatusHandler(civgStatusHandler);

    while (downloaditem_deleteFirstItem(&_diFirst) != 0);
    proto_BaseMessage baseMsg = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__INIT;
    baseMsg.vin = "testVIN";
    baseMsg.has_data_type = 1;
    baseMsg.data_type = proto_MESSAGE_CONFIG_RPC;
    baseMsg.has_data = 1;
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    config.n_rps = 2;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta a = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;

    CarSync__Proto__ReleasePackage__ReleasePackageMeta b = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[2] = {&a, &b};
    config.rps = rps;
    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    uDescriptor.uuid = "1234567890";
    proto_Version toV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    toV.has_major_version = 1;
    toV.major_version = 7;
    toV.has_minor_version = 1;
    toV.minor_version = 8;
    toV.has_build_version = 1;
    toV.build_version = 9;
    uDescriptor.to_version = &toV;
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
    config.update = &uDescriptor;
    rpc.conf_rsp = &config;
    proto_BaseMessage* baseMsgReq = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    CU_ASSERT_FATAL(baseMsgReq != 0);
    CU_ASSERT_FATAL(baseMsgReq->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsgReq->data_type == (int)proto_MESSAGE_CONFIG_RPC);
    CU_ASSERT_FATAL(baseMsgReq->has_data == 1);
    proto_RPC* rpcReq = car_sync__proto__messages__rpc__unpack(NULL, baseMsgReq->data.len, baseMsgReq->data.data);
    CU_ASSERT_FATAL(rpcReq != 0);
    CU_ASSERT_FATAL(rpcReq->rpc_id != 0);
    rpc.rpc_id = strtools_sprintNew("%s", rpcReq->rpc_id);
    car_sync__proto__messages__rpc__free_unpacked(rpcReq, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsgReq, NULL);
    _lastCivgState = CORE_STATE_BROKEN;
    _lastCivgStatus = CORE_STATUS_CRITICAL;

    // invalid/not matching config received
    handleRpc(&baseMsg, &rpc);
    CU_ASSERT_FATAL(_lastCivgState == CORE_STATE_IDLE);
    CU_ASSERT_FATAL(_lastCivgStatus == CORE_STATUS_ERROR);

    proto_Version fromV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    fromV.has_major_version = 1;
    fromV.major_version = 3;
    fromV.has_minor_version = 1;
    fromV.minor_version = 4;
    fromV.has_build_version = 1;
    fromV.build_version = 5;
    uDescriptor.from_version = &fromV;
    // available config successfully received
    handleRpc(&baseMsg, &rpc);

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
    _dHandler("0URL", 0);
    _dHandler("1URL", 0);
    // duplicated config during download
    handleRpc(&baseMsg, &rpc);
    // data downloaded
    _dHandler("http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch", 0);
    _dHandler("http://10.90.0.6/packages/40399461-60b1-4ad7-a6b3-ab168bb70b0e/1.0.0/ivi4-demo.patch", 0);
    // waiting for upgrade trigger
    CU_ASSERT_FATAL(_lastCivgState == CORE_STATE_IDLE);
    CU_ASSERT_FATAL(_lastCivgStatus == CORE_STATUS_OK);
    free(_lastMsg);
    _lastMsg = 0;
    core_performUpgrade();
    // should receive config check ater succesfull upgrade
    CU_ASSERT_FATAL(_lastCivgState == CORE_STATE_IDLE);
    CU_ASSERT_FATAL(_lastCivgStatus == CORE_STATUS_OK);
    baseMsgReq = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    CU_ASSERT_FATAL(baseMsgReq != 0);
    CU_ASSERT_FATAL(baseMsgReq->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsgReq->data_type == (int)proto_MESSAGE_CONFIG_RPC);
    CU_ASSERT_FATAL(baseMsgReq->has_data == 1);
    rpcReq = car_sync__proto__messages__rpc__unpack(NULL, baseMsgReq->data.len, baseMsgReq->data.data);
    CU_ASSERT_FATAL(rpcReq != 0);
    CU_ASSERT_FATAL(rpcReq->conf_req != 0);
    CU_ASSERT_FATAL(rpcReq->conf_req->n_rps == 2);
    car_sync__proto__messages__rpc__free_unpacked(rpcReq, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsgReq, NULL);

    free(rpc.rpc_id);
    for (unsigned int i = 0; i < uDescriptor.n_updates; ++i) {
        free(uDescriptor.updates[i]);
    }
    free(uDescriptor.updates);
    core_cleanup();
}

CU_pSuite core_tests_getSuite()
{
   /* add a suite to the registry */
   CU_pSuite suite = CU_add_suite("Test CIVG core", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, initAndCleanupTests);
   CU_ADD_TEST(suite, resumeDownloadTests);
   CU_ADD_TEST(suite, resumeUpgradeTests);
   CU_ADD_TEST(suite, nothingToDownloadScenarioTests);
   CU_ADD_TEST(suite, upgradeWithDownloadScenarioTests);

   return suite;
}
