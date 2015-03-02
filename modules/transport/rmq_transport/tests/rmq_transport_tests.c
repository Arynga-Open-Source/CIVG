/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "rmq_transport_tests.h"
#include "rmq_config_priv.h"
#include "threaded_producer.h"
#include "threaded_consumer.h"
#include "threaded_transport.h"
#include "glib_dispatcher.h"
#include "config.h"
#include "proto.h"
#include "filetools.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <glib.h>

#define TEST_VBS_IP "10.90.0.6"
#define TEST_VBS_PORT 5672

static proto_Version _protocolVersion = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
#define TIMESTAMP_SIZE 11
static char _timestamp[TIMESTAMP_SIZE] = "\0";
static char* _vin = "1HGCM82633A004352";
#define RPC_ID_SIZE 17
static char _recentRPCId[RPC_ID_SIZE] = "INVALID\0";
static unsigned int _recentRPCIdCount = 0;

static GMainLoop* _loop = 0;
dispatcher_Dispatcher* _dispatcher;

static void* _hMsg = 0;
static int _hMsgSize = 0;
static void handler(const void* msg, int size) {
    free(_hMsg);
    _hMsgSize = size;
    if (_hMsgSize > 0) {
        _hMsg = malloc(_hMsgSize);
        memcpy(_hMsg, msg, _hMsgSize);
    } else {
        _hMsg = 0;
    }
    if(_loop && g_main_loop_is_running(_loop) == TRUE)
        g_main_loop_quit(_loop);
}

static int init()
{
    _loop = g_main_loop_new(NULL, FALSE);
    _dispatcher = glib_dispatcher_newDispatcher(g_main_context_default());
    _protocolVersion.has_major_version = 1;
    _protocolVersion.major_version = 1;
    _protocolVersion.has_minor_version = 1;
    _protocolVersion.minor_version = 0;
    _protocolVersion.has_build_version = 1;
    _protocolVersion.build_version = 0;

    config_Storage* storage = config_newStorage(RMQ_STORAGE_ID);
    if (storage) {
        config_setValue(storage, USERNAME_KEY, "guest", strlen("guest") + 1);
        config_setValue(storage, PASSWORD_KEY, "guest", strlen("guest") + 1);
        config_setValue(storage, VHOST_KEY, "/", strlen("/") + 1);
        config_setValue(storage, ADDR_KEY, TEST_VBS_IP, strlen(TEST_VBS_IP) + 1);
        config_setIValue(storage, PORT_KEY, TEST_VBS_PORT);
        config_deleteStorage(storage);
    }
    system("ssh root@" TEST_VBS_IP " 'cd /root/test_sctipts/ && ./resetDB.sh'");
    return 0;
}

static int cleanup()
{
    glib_dispatcher_deleteDispatcher(_dispatcher);
    g_main_loop_unref(_loop);
    free(_hMsg);
    return 0;
}

static int sendMsg(proto_MessageType type, void* msg, int size) {
    proto_BaseMessage baseMsg = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__INIT;
    // fill base message
    baseMsg.vin = _vin;
    baseMsg.version = &_protocolVersion;
    snprintf(_timestamp, TIMESTAMP_SIZE, "%ld", (long)time(NULL));
    baseMsg.timestamp = _timestamp;
    baseMsg.has_data_type = 1;
    baseMsg.data_type = type;
    baseMsg.has_data = 1;
    baseMsg.data.len = size;
    baseMsg.data.data = msg;
    // serialize and send
    int status = 1;
    int bufferSize = car_sync__proto__messages__base_message__get_packed_size(&baseMsg);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__base_message__pack(&baseMsg, buffer);
        status = transport_sendMsg(buffer, bufferSize);
        free(buffer);
    }
    return status;
}

static int sendReport(proto_Report* report) {
    //serialize and send
    int status = 1;
    int bufferSize = car_sync__proto__messages__report__get_packed_size(report);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__report__pack(report, buffer);
        status = sendMsg(proto_MESSAGE_REPORT, buffer, bufferSize);
        free(buffer);
    }
    return status;
}

static int sendErrorReport(char *description) {
    // fill Report
    proto_Report report = CAR_SYNC__PROTO__MESSAGES__REPORT__INIT;
    report.description = description;
    report.has_type = 1;
    report.type = proto_REPORT_ERROR;
    return sendReport(&report);
}

static int sendRPC(proto_RPC* rpc) {
    //serialize and send
    int status = 1;
    int bufferSize = car_sync__proto__messages__rpc__get_packed_size(rpc);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__rpc__pack(rpc, buffer);
        status = sendMsg(proto_MESSAGE_CONFIG_RPC, buffer, bufferSize);
        free(buffer);
    }
    return status;
}

static int checkConfig(proto_InstalledConfig* installedConfig) {
    // fill ConfigRPC
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    // append _recentRPCIdCount to timestamp to ensure its uniqueness
    snprintf(_recentRPCId, RPC_ID_SIZE, "%lx%x", (unsigned long)time(NULL), _recentRPCIdCount++);
    rpc.rpc_id = _recentRPCId;
    rpc.conf_req = installedConfig;
    return sendRPC(&rpc);
}

static void reportTest() {
    transport_config_ConnectionConfig* cfg = transport_config_get();
    CU_ASSERT_FATAL(cfg != 0);
    CU_ASSERT_FATAL(threaded_producer_init(cfg) == 0);

    void* fContent;
    unsigned int fSize;
    system("curl http://" TEST_VBS_IP ":8098/buckets/reports/keys?keys=true|grep error_1HGCM82633A004352 > cmd.out");
    CU_ASSERT_FATAL(filetools_readAll("cmd.out", &fContent, &fSize) == 0);
    CU_ASSERT_FATAL(fSize == 0);
    CU_ASSERT_FATAL(sendErrorReport("test error report") == 0);
    threaded_producer_sendNext();
    system("curl http://" TEST_VBS_IP ":8098/buckets/reports/keys?keys=true|grep error_1HGCM82633A004352 > cmd.out");
    CU_ASSERT_FATAL(filetools_readAll("cmd.out", &fContent, &fSize) == 0);
    CU_ASSERT_FATAL(fSize != 0);
    free(fContent);

    threaded_producer_cleanup();
    transport_config_free(cfg);
}

static void notificationTest() {
    threaded_transport_setDispatcher(_dispatcher);
    transport_setMsgHandler(handler);
    CU_ASSERT_FATAL(transport_init() == 0);

    system("ssh root@" TEST_VBS_IP " 'cd /root/test_sctipts/ && ./sendNotif.sh'");
    g_main_loop_run(_loop);

    CU_ASSERT_FATAL(_hMsg != 0);
    CU_ASSERT_FATAL(_hMsgSize != 0);
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, _hMsgSize, _hMsg);
    CU_ASSERT_FATAL(baseMsg != 0);
    CU_ASSERT_FATAL(baseMsg->has_data == 1);
    proto_Notification* notif = car_sync__proto__messages__notification__unpack(NULL, baseMsg->data.len, baseMsg->data.data);
    CU_ASSERT_FATAL(notif != 0);
    CU_ASSERT_FATAL(notif->has_type == 1);
    CU_ASSERT_FATAL(notif->type == (int)proto_NOTIFICATION_CONF_REPORT);
    car_sync__proto__messages__notification__free_unpacked(notif, NULL);
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    transport_cleanup();
}

static void rpcTest() {
    threaded_transport_setDispatcher(_dispatcher);
    transport_setMsgHandler(handler);
    CU_ASSERT_FATAL(transport_init() == 0);

    proto_InstalledConfig config = CAR_SYNC__PROTO__MESSAGES__RPCCONFIGURATION_REQ__INIT;
    config.n_rps = 1;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta rp = CAR_SYNC__PROTO__RELEASE_PACKAGE__RELEASE_PACKAGE_META__INIT;
    rp.uuid = "e53e4c73-9203-48df-8224-c5f02ead37d3";
    CarSync__Proto__Common__Version v = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
    v.has_build_version = v.has_minor_version = v.has_major_version = 1;
    v.build_version = v.minor_version = v.major_version = 0;
    rp.version = &v;
    CarSync__Proto__ReleasePackage__ReleasePackageMeta* rps[1] = {&rp};
    config.rps = rps;
    checkConfig(&config);

    g_main_loop_run(_loop);

    CU_ASSERT_FATAL(_hMsg != 0);
    CU_ASSERT_FATAL(_hMsgSize != 0);
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, _hMsgSize, _hMsg);
    CU_ASSERT_FATAL(baseMsg != 0);
    CU_ASSERT_FATAL(baseMsg->vin != 0);
    CU_ASSERT_FATAL(strcmp(baseMsg->vin, _vin) == 0);
    CU_ASSERT_FATAL(baseMsg->version != 0);
    CU_ASSERT_FATAL(baseMsg->version->has_major_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_minor_version == 1);
    CU_ASSERT_FATAL(baseMsg->version->has_build_version == 1);
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    transport_cleanup();
}

CU_pSuite rmq_transport_tests_getSuite()
{
   /* add a suite to the registry */
    CU_pSuite suite = CU_add_suite("Test rmq_transport module", init, cleanup);
   if (!suite) {
       return 0;
   }

   CU_ADD_TEST(suite, reportTest);
   CU_ADD_TEST(suite, notificationTest); //ToDo Find way to force VBS to send notification and update test
   CU_ADD_TEST(suite, rpcTest);

   return suite;
}
