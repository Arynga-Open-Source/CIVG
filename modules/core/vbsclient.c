/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "vbsclient.h"
#include "transport.h"
#include "carinfo.h"
#include "logger.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static VBSCLIENT_AVAILABLE_CONFIG_HANDLER _availableConfigHandler = 0;
static VBSCLIENT_UPDATE_NOTIFICATION_HANDLER _updateNotificationHandler = 0;

static char* _vin = 0;
static proto_Version _protocolVersion = CAR_SYNC__PROTO__COMMON__VERSION__INIT;
#define TIMESTAMP_SIZE 11
static char _timestamp[TIMESTAMP_SIZE] = "\0";
#define RPC_ID_SIZE 17
static char _recentRPCId[RPC_ID_SIZE] = "INVALID\0";
static unsigned int _recentRPCIdCount = 0;

static int sendMsg(proto_MessageType type, void* msg, size_t size) {
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
    LOG(LOGG_DEBUG, "Sending BaseMessage. Timestamp: %s", baseMsg.timestamp);

    // serialize and send
    int status = 1;
    size_t bufferSize = car_sync__proto__messages__base_message__get_packed_size(&baseMsg);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__base_message__pack(&baseMsg, buffer);
        status = transport_sendMsg(buffer, bufferSize);
        free(buffer);
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for packed BaseMsg");
    }
    return status;
}

static int sendReport(proto_Report* report) {
    //serialize and send
    int status = 1;
    size_t bufferSize = car_sync__proto__messages__report__get_packed_size(report);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__report__pack(report, buffer);
        status = sendMsg(proto_MESSAGE_REPORT, buffer, bufferSize);
        free(buffer);
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for packed Report");
    }
    return status;
}

static int sendRPC(proto_RPC* rpc) {
    //serialize and send
    int status = 1;
    size_t bufferSize = car_sync__proto__messages__rpc__get_packed_size(rpc);
    void* buffer = malloc(bufferSize);
    if (buffer) {
        car_sync__proto__messages__rpc__pack(rpc, buffer);
        status = sendMsg(proto_MESSAGE_CONFIG_RPC, buffer, bufferSize);
        free(buffer);
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for packed RPC");
    }
    return status;
}

static void sendRPCError(char* description) {
    // fill ConfigRPC
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    proto_RPCError error = CAR_SYNC__PROTO__MESSAGES__RPCERROR__INIT;
    error.reason = description;
    rpc.error = &error;
    LOG(LOGG_DEBUG, "Sending RPC Error. Reason: %s", error.reason);

    sendRPC(&rpc);
}

static void handleNotification(const void* msg, size_t size) {
    proto_Notification* notif = car_sync__proto__messages__notification__unpack(NULL, size, msg);
    if (!notif) {
        LOGM(LOGG_WARNING, "Received invalid notification - can not unpack Notification.");
        return;
    }

    if (notif->has_type) {
        switch (notif->type) {
        case proto_NOTIFICATION_VERSION_UPDATE:
        case proto_NOTIFICATION_CONF_REPORT:
            if (_updateNotificationHandler) {
                _updateNotificationHandler();
            }
            break;
        default:
            LOG(LOGG_WARNING, "Received unsupported notification. Type: %d", notif->type);
            break;
        }
    } else {
        LOGM(LOGG_WARNING, "Received invalid notification - missing notification type.");
    }
    car_sync__proto__messages__notification__free_unpacked(notif, NULL);
}

static void handleConfigRPC(const void* msg, size_t size) {
    proto_RPC* rpc = car_sync__proto__messages__rpc__unpack(NULL, size, msg);
    if (!rpc) {
        LOGM(LOGG_WARNING, "Received invalid RPC - can not unpack RPC.");
        sendRPCError("Unpack RPC message failed!");
        return;
    }

    if (rpc->conf_rsp) {
        if (rpc->rpc_id) {
            if (strcmp(rpc->rpc_id, _recentRPCId) == 0) {
                if (_availableConfigHandler) {
                    _availableConfigHandler(rpc->conf_rsp);
                }
            } else {
                LOGM(LOGG_DEBUG, "Ignored obsolete RPC response.");
            }
        } else {
            LOGM(LOGG_WARNING, "Received invalid RPC response - missing RPC id.");
            sendRPCError("Missing RPC id.!");
        }
    } else if (rpc->error) {
        LOG(LOGG_WARNING, "Received RPC error. Code: %s; Reason: %s", rpc->error->code, rpc->error->reason);
    } else {
        LOGM(LOGG_WARNING, "Received invalid RPC - RPCConfigurationRsp or RPCError expected.");
        sendRPCError("Invalid RPC - RPCConfigurationRsp or RPCError expected, but not found!");
    }
    car_sync__proto__messages__rpc__free_unpacked(rpc, NULL);
}

static void messageHandler(const void* msg, size_t size) {
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, size, msg);
    if (!baseMsg) {
        LOGM(LOGG_WARNING, "Received invalid message - can not unpack BaseMessage.");
        return;
    }

    int isValid = 0;
    if (!baseMsg->has_data_type) {
        LOGM(LOGG_WARNING, "Received invalid message - missing data type.");
    } else if (!baseMsg->has_data) {
        LOGM(LOGG_WARNING, "Received invalid message - missing data.");
    } else {
        isValid = 1;
    }

    if (isValid) {
        switch (baseMsg->data_type) {
        case proto_MESSAGE_NOTIFICATION:
            handleNotification(baseMsg->data.data, baseMsg->data.len);
            break;
        case proto_MESSAGE_CONFIG_RPC:
            if (!baseMsg->vin || strcmp(baseMsg->vin, _vin) != 0) {
                LOGM(LOGG_WARNING, "Received invalid RPC message - VIN does not match.");
            } else {
                handleConfigRPC(baseMsg->data.data, baseMsg->data.len);
            }
            break;
        default:
            LOG(LOGG_WARNING, "Received unsupported message. Type: %d", baseMsg->data_type);
            break;
        }
    }
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);
}

int vbsclient_sendUpdateReport(proto_UpdateStatus status,
                               proto_AvailableConfig *availableConfig,
                               char *description) {
    // fill Report
    proto_Report report = CAR_SYNC__PROTO__MESSAGES__REPORT__INIT;
    report.description = description;
    report.has_type = 1;
    report.type = proto_REPORT_UPDATE;
    snprintf(_timestamp, TIMESTAMP_SIZE, "%ld", (long)time(NULL));
    report.timestamp=_timestamp;
    report.vin=_vin;
    // add update info
    proto_ReportUpdateInfo info = CAR_SYNC__PROTO__MESSAGES__REPORT__UPDATE_INFO__INIT;
    info.has_status = 1;
    info.status = status;
    if (availableConfig && availableConfig->update) {
        info.uuid = availableConfig->update->uuid;
        info.from_version = availableConfig->update->from_version;
        info.to_version = availableConfig->update->to_version;
    }
    report.update_info = &info;
    LOG(LOGG_DEBUG, "Sending update report. Status: %d; Description: %s", info.status, report.description);

    return sendReport(&report);
}

int vbsclient_sendErrorReport(char *description) {
    // fill Report
    proto_Report report = CAR_SYNC__PROTO__MESSAGES__REPORT__INIT;
    report.description = description;
    report.has_type = 1;
    report.type = proto_REPORT_ERROR;
    LOG(LOGG_DEBUG, "Sending error report. Description: %s", report.description);

    return sendReport(&report);
}

int vbsclient_checkConfig(proto_InstalledConfig* installedConfig) {
    // fill ConfigRPC
    proto_RPC rpc = CAR_SYNC__PROTO__MESSAGES__RPC__INIT;
    // append _recentRPCIdCount to timestamp to ensure its uniqueness
    snprintf(_recentRPCId, RPC_ID_SIZE, "%lx%x", (unsigned long)time(NULL), _recentRPCIdCount++);
    rpc.rpc_id = _recentRPCId;
    rpc.conf_req = installedConfig;
    LOG(LOGG_DEBUG, "Sending RPC Config request. RPC Id: %s", rpc.rpc_id);

    return sendRPC(&rpc);
}

void vbsclient_setAvailableConfigHandler(VBSCLIENT_AVAILABLE_CONFIG_HANDLER handler) {
    _availableConfigHandler = handler;
}

void vbsclient_setUpdateNotificationHandler(VBSCLIENT_UPDATE_NOTIFICATION_HANDLER handler) {
    _updateNotificationHandler = handler;
}

int vbsclient_init() {
    if (_vin) {
        LOGM(LOGG_ERROR, "Doubled VBSClient initialization!");
        return 1;
    }
    if (carinfo_getVin(&_vin) != 0
        || !_vin
        || strlen(_vin) == 0) {
        LOGM(LOGG_ERROR, "Failed to get VIN!");
        return 1;
    }
    _protocolVersion.has_major_version = 1;
    _protocolVersion.major_version = 1;
    _protocolVersion.has_minor_version = 1;
    _protocolVersion.minor_version = 0;
    _protocolVersion.has_build_version = 1;
    _protocolVersion.build_version = 0;
    if (transport_init() != 0) {
        LOGM(LOGG_ERROR, "Failed to initialize Transport layer!");
        vbsclient_cleanup();
        return 1;
    }
    transport_setMsgHandler(messageHandler);
    LOG(LOGG_INFO, "VBS Client initialized. Protocol Version: %d.%d.%d; VIN: %s",
        _protocolVersion.major_version, _protocolVersion.minor_version, _protocolVersion.build_version,
        _vin);
    return 0;
}


void vbsclient_cleanup() {
    if (_vin) {
        free(_vin);
        _vin = 0;
        _availableConfigHandler = 0;
        _updateNotificationHandler = 0;
        transport_cleanup();
    }
}
