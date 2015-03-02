/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "vbsclient_tests.h"
#include "vbsclient.h"
#include "dummy_carinfo_priv.h"
#include "test_transport/test_transport.h"
#include "config.h"
#include "strtools.h"
#include <stdlib.h>
#include <string.h>

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

static TRANSPORT_MSG_HANDLER _handler = 0;
static void tHandler(TRANSPORT_MSG_HANDLER handler) {
    _handler = handler;
}

static int init()
{
    testtransport_init(tInit);
    testtransport_cleanup(tCleanup);
    testtransport_sendMsg(tSend);
    testtransport_setMsgHandler(tHandler);
    return 0;
}

static int cleanup()
{
    return 0;
}

static void initAndCleanupTests() {
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    CU_ASSERT_FATAL(storage != 0);
    config_removeKey(storage, VIN_KEY);

    _tInitRes = 0;
    _tInitCalled = 0;
    _tCleanupCalled = 0;

    // no vin
    CU_ASSERT_FATAL(vbsclient_init() == 1);
    CU_ASSERT_FATAL(_tInitCalled == 0);
    CU_ASSERT_FATAL(_tCleanupCalled == 0);

    const char vin[] = "testVIN";
    const int size = sizeof(vin);
    CU_ASSERT_FATAL(config_setValue(storage, VIN_KEY, vin, size) == 0);
    _tInitRes = 1;

    // vin found, but transport init failed
    CU_ASSERT_FATAL(vbsclient_init() == 1);
    CU_ASSERT_FATAL(_tInitCalled == 1);
    CU_ASSERT_FATAL(_tCleanupCalled == 1);
    _tInitRes = 0;
    _handler = 0;
    // successfull init
    CU_ASSERT_FATAL(vbsclient_init() == 0);
    CU_ASSERT_FATAL(_handler != 0);
    CU_ASSERT_FATAL(_tInitCalled == 2);
    CU_ASSERT_FATAL(_tCleanupCalled == 1);

    // cleanup should call transport cleanup
    vbsclient_cleanup();
    vbsclient_cleanup();
    CU_ASSERT_FATAL(_tCleanupCalled == 2);
    config_deleteStorage(storage);
}

static void reportTests() {
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    CU_ASSERT_FATAL(storage != 0);
    const char vin[] = "testVIN";
    const int size = sizeof(vin);
    CU_ASSERT_FATAL(config_setValue(storage, VIN_KEY, vin, size) == 0);
    _tInitRes = 0;
    CU_ASSERT_FATAL(vbsclient_init() == 0);

    // successfull error report
    free(_lastMsg);
    _lastMsg = 0;
    CU_ASSERT_FATAL(vbsclient_sendErrorReport("test error description") == 0);
    CU_ASSERT_FATAL(_lastMsg != 0);
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    // check base message
    CU_ASSERT_FATAL(baseMsg != 0);
    CU_ASSERT_FATAL(baseMsg->vin != 0);
    CU_ASSERT_FATAL(strcmp(baseMsg->vin, "testVIN") == 0);
    CU_ASSERT_FATAL(baseMsg->version != 0);
    CU_ASSERT_FATAL(baseMsg->version->has_major_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_minor_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_build_version == 1);
    CU_ASSERT_FATAL(baseMsg->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsg->data_type == (int)proto_MESSAGE_REPORT);
    CU_ASSERT_FATAL(baseMsg->has_data == 1);
    proto_Report* report = car_sync__proto__messages__report__unpack(NULL, baseMsg->data.len, baseMsg->data.data);
    // check report fields
    CU_ASSERT_FATAL(report != 0);
    CU_ASSERT_FATAL(report->has_type == 1);
    CU_ASSERT_FATAL(report->type == (int)proto_REPORT_ERROR);
    CU_ASSERT_FATAL(report->description != 0);
    CU_ASSERT_FATAL(strcmp(report->description, "test error description") == 0);
    car_sync__proto__messages__report__free_unpacked(report, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    proto_UpdateDescriptor uDescriptor = CAR_SYNC__PROTO__RELEASE_PACKAGE__UPDATE_DESCRIPTOR__INIT;
    uDescriptor.uuid = "1234567890";
    proto_Version fromV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    fromV.has_major_version = 1;
    fromV.major_version = 12;
    uDescriptor.from_version = &fromV;
    proto_Version toV = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    uDescriptor.to_version = &toV;
    toV.has_major_version = 1;
    toV.major_version = 13;
    toV.has_minor_version = 1;
    toV.minor_version = 3;
    toV.has_build_version = 1;
    toV.build_version = 7;
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    config.update = &uDescriptor;
    free(_lastMsg);
    _lastMsg = 0;
    // successfull update report
    CU_ASSERT_FATAL(vbsclient_sendUpdateReport(proto_UPDATE_SUCCESS, &config, "test update description") == 0);
    CU_ASSERT_FATAL(_lastMsg != 0);
    baseMsg = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    // check base message
    CU_ASSERT_FATAL(baseMsg != 0);
    CU_ASSERT_FATAL(baseMsg->vin != 0);
    CU_ASSERT_FATAL(strcmp(baseMsg->vin, "testVIN") == 0);
    CU_ASSERT_FATAL(baseMsg->version != 0);
    CU_ASSERT_FATAL(baseMsg->version->has_major_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_minor_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_build_version == 1);
    CU_ASSERT_FATAL(baseMsg->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsg->data_type == (int)proto_MESSAGE_REPORT);
    CU_ASSERT_FATAL(baseMsg->has_data == 1);
    report = car_sync__proto__messages__report__unpack(NULL, baseMsg->data.len, baseMsg->data.data);
    // check report fields
    CU_ASSERT_FATAL(report != 0);
    CU_ASSERT_FATAL(report->has_type == 1);
    CU_ASSERT_FATAL(report->type == (int)proto_REPORT_UPDATE);
    CU_ASSERT_FATAL(report->description != 0);
    CU_ASSERT_FATAL(strcmp(report->description, "test update description") == 0);
    CU_ASSERT_FATAL(report->update_info != 0);
    CU_ASSERT_FATAL(report->update_info->from_version != 0);
    CU_ASSERT_FATAL(proto_versionCmp(report->update_info->from_version, &fromV) == 0);
    CU_ASSERT_FATAL(report->update_info->to_version != 0);
    CU_ASSERT_FATAL(proto_versionCmp(report->update_info->to_version, &toV) == 0);
    CU_ASSERT_FATAL(report->update_info->has_status == 1);
    CU_ASSERT_FATAL(report->update_info->status == (int)proto_UPDATE_SUCCESS);
    CU_ASSERT_FATAL(report->update_info->uuid != 0);
    CU_ASSERT_FATAL(strcmp(report->update_info->uuid, uDescriptor.uuid) == 0);
    car_sync__proto__messages__report__free_unpacked(report, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    vbsclient_cleanup();
    config_deleteStorage(storage);
    free(_lastMsg);
    _lastMsg = 0;
}

static void checkConfigTests() {
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    CU_ASSERT_FATAL(storage != 0);
    const char vin[] = "testVIN";
    const int size = sizeof(vin);
    CU_ASSERT_FATAL(config_setValue(storage, VIN_KEY, vin, size) == 0);
    _tInitRes = 0;
    CU_ASSERT_FATAL(vbsclient_init() == 0);

    proto_InstalledConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_REQ__INIT;
    config.n_rps = 2;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta a = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    a.uuid = "1234567890";
    CarSync__Proto__ReleasePackage__ReleasePackageMeta b = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    b.uuid = "0987654321";
    CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[2] = {&a, &b};
    config.rps = rps;

    // successfull check config
    CU_ASSERT_FATAL(vbsclient_checkConfig(&config) == 0);
    CU_ASSERT_FATAL(_lastMsg != 0);
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, _lastMsgSize, _lastMsg);
    // check base message
    CU_ASSERT_FATAL(baseMsg != 0);
    CU_ASSERT_FATAL(baseMsg->vin != 0);
    CU_ASSERT_FATAL(strcmp(baseMsg->vin, "testVIN") == 0);
    CU_ASSERT_FATAL(baseMsg->version != 0);
    CU_ASSERT_FATAL(baseMsg->version->has_major_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_minor_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_build_version == 1);
    CU_ASSERT_FATAL(baseMsg->has_data_type == 1);
    CU_ASSERT_FATAL(baseMsg->data_type == (int)proto_MESSAGE_CONFIG_RPC);
    CU_ASSERT_FATAL(baseMsg->has_data == 1);
    proto_RPC* rpc = car_sync__proto__messages__rpc__unpack(NULL, baseMsg->data.len, baseMsg->data.data);
    // check rpc content
    CU_ASSERT_FATAL(rpc != 0);
    CU_ASSERT_FATAL(rpc->rpc_id != 0);
    CU_ASSERT_FATAL(rpc->conf_rsp == 0);
    CU_ASSERT_FATAL(rpc->error == 0);
    CU_ASSERT_FATAL(rpc->conf_req != 0);
    CU_ASSERT_FATAL(rpc->conf_req->n_rps == 2);
    CU_ASSERT_FATAL(rpc->conf_req->rps[0]->uuid != 0);
    CU_ASSERT_FATAL(strcmp(rpc->conf_req->rps[0]->uuid, "1234567890") == 0);
    CU_ASSERT_FATAL(rpc->conf_req->rps[1]->uuid != 0);
    CU_ASSERT_FATAL(strcmp(rpc->conf_req->rps[1]->uuid, "0987654321") == 0);
    car_sync__proto__messages__rpc__free_unpacked(rpc, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    vbsclient_cleanup();
    config_deleteStorage(storage);
}

static int _notifCalled = 0;
static void vbsNotifHandler() {
    ++_notifCalled;
}

static void handleNotif(proto_BaseMessage* baseMsg, proto_Notification* notif) {
    baseMsg->data.len = car_sync__proto__messages__notification__get_packed_size(notif);
    baseMsg->data.data = malloc(baseMsg->data.len);
    car_sync__proto__messages__notification__pack(notif, baseMsg->data.data);
    int bufSize = car_sync__proto__messages__base_message__get_packed_size(baseMsg);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__base_message__pack(baseMsg, buf);
    _handler(buf, bufSize);
    free(baseMsg->data.data);
    free(buf);
}

static void updateHandlerTests() {
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    CU_ASSERT_FATAL(storage != 0);
    const char vin[] = "testVIN";
    const int size = sizeof(vin);
    CU_ASSERT_FATAL(config_setValue(storage, VIN_KEY, vin, size) == 0);
    _tInitRes = 0;
    CU_ASSERT_FATAL(vbsclient_init() == 0);
    vbsclient_setUpdateNotificationHandler(vbsNotifHandler);
    CU_ASSERT_FATAL(_handler != 0);
    _notifCalled = 0;
    // empty message received by transport
    _handler(0, 0);
    CU_ASSERT_FATAL(_notifCalled == 0);

    // empty message with VIN, but nothing else
    proto_BaseMessage baseMsg = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__INIT;
    baseMsg.vin = "testVIN";
    int bufSize = car_sync__proto__messages__base_message__get_packed_size(&baseMsg);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__base_message__pack(&baseMsg, buf);
    _handler(buf, bufSize);
    CU_ASSERT_FATAL(_notifCalled == 0);
    free(buf);

    // invalid data type
    baseMsg.has_data_type = 1;
    baseMsg.data_type = proto_MESSAGE_REPORT;
    bufSize = car_sync__proto__messages__base_message__get_packed_size(&baseMsg);
    buf = malloc(bufSize);
    car_sync__proto__messages__base_message__pack(&baseMsg, buf);
    _handler(buf, bufSize);
    CU_ASSERT_FATAL(_notifCalled == 0);
    free(buf);

    // added data type ok, but invalid data
    proto_Notification notif = CAR_SYNC__PROTO__MESSAGES__NOTIFICATION__INIT;
    notif.has_type = 1;
    notif.type = proto_NOTIFICATION_CONF_REPORT;
    baseMsg.has_data = 1;
    handleNotif(&baseMsg, &notif);
    CU_ASSERT_FATAL(_notifCalled == 0);

    // unsupported notification
    baseMsg.data_type = proto_MESSAGE_NOTIFICATION;
    notif.type = -1;
    handleNotif(&baseMsg, &notif);
    CU_ASSERT_FATAL(_notifCalled == 0);

    // missing notification type
    notif.has_type = 0;
    notif.type = proto_NOTIFICATION_VERSION_UPDATE;
    handleNotif(&baseMsg, &notif);
    CU_ASSERT_FATAL(_notifCalled == 0);

    // valid update notification
    notif.has_type = 1;
    notif.type = proto_NOTIFICATION_VERSION_UPDATE;
    handleNotif(&baseMsg, &notif);
    CU_ASSERT_FATAL(_notifCalled == 1);

    vbsclient_cleanup();
    config_deleteStorage(storage);
}

static int _configCalled = 0;
static int _lastNRps = 0;
static void vbsConfigHandler(proto_AvailableConfig* availableConfig) {
    ++_configCalled;
    if (availableConfig) {
        _lastNRps = availableConfig->n_rps;
    }
}

static void handleRpc(proto_BaseMessage* baseMsg, proto_RPC* rpc) {
    baseMsg->data.len = car_sync__proto__messages__rpc__get_packed_size(rpc);
    baseMsg->data.data = malloc(baseMsg->data.len);
    car_sync__proto__messages__rpc__pack(rpc, baseMsg->data.data);
    int bufSize = car_sync__proto__messages__base_message__get_packed_size(baseMsg);
    void* buf = malloc(bufSize);
    car_sync__proto__messages__base_message__pack(baseMsg, buf);
    _handler(buf, bufSize);
    free(baseMsg->data.data);
    free(buf);
}

static void configHandlerTests() {
    config_Storage* storage = config_newStorage(DUMMY_CARINFO_STORAGE_ID);
    CU_ASSERT_FATAL(storage != 0);
    const char vin[] = "testVIN";
    const int size = sizeof(vin);
    CU_ASSERT_FATAL(config_setValue(storage, VIN_KEY, vin, size) == 0);
    _tInitRes = 0;
    CU_ASSERT_FATAL(vbsclient_init() == 0);
    vbsclient_setAvailableConfigHandler(vbsConfigHandler);
    CU_ASSERT_FATAL(_handler != 0);
    _configCalled = 0;
    _handler(0, 0);
    CU_ASSERT_FATAL(_configCalled == 0);

    // missing data
    proto_BaseMessage baseMsg = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__INIT;
    baseMsg.vin = "testVIN";
    baseMsg.has_data_type = 1;
    baseMsg.data_type = proto_MESSAGE_CONFIG_RPC;
    baseMsg.has_data = 1;
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    handleRpc(&baseMsg, &rpc);
    CU_ASSERT_FATAL(_configCalled == 0);

    // missing rpc_id
    proto_AvailableConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_RSP__INIT;
    config.n_rps = 2;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta a = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta b = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[2] = {&a, &b};
    config.rps = rps;
    rpc.conf_rsp = &config;
    handleRpc(&baseMsg, &rpc);
    CU_ASSERT_FATAL(_configCalled == 0);

    // obsolete rpc_id
    rpc.rpc_id = "obsoleteId";
    handleRpc(&baseMsg, &rpc);
    CU_ASSERT_FATAL(_configCalled == 0);

    // send config request and extract valid rpc_id
    proto_InstalledConfig installedConfig = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_REQ__INIT;
    CU_ASSERT_FATAL(vbsclient_checkConfig(&installedConfig) == 0);
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

    // available config successfully received
    _lastNRps = 0;
    handleRpc(&baseMsg, &rpc);
    CU_ASSERT_FATAL(_configCalled == 1);
    CU_ASSERT_FATAL(_lastNRps == 2);
    free(rpc.rpc_id);

    vbsclient_cleanup();
    config_deleteStorage(storage);
}

CU_pSuite vbsclient_tests_getSuite()
{
   /* add a suite to the registry */
   CU_pSuite suite = CU_add_suite("Test VBS client module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, initAndCleanupTests);
   CU_ADD_TEST(suite, reportTests);
   CU_ADD_TEST(suite, checkConfigTests);
   CU_ADD_TEST(suite, updateHandlerTests);
   CU_ADD_TEST(suite, configHandlerTests);

   return suite;
}
