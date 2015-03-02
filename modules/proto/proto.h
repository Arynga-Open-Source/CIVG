/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef PROTO_H
#define PROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.pb-c.h"
#include "releasepackage.pb-c.h"
#include "southapi.pb-c.h"

typedef CarSync__Proto__Messages__BaseMessage proto_BaseMessage;
typedef CarSync__Proto__Messages__Report proto_Report;
typedef CarSync__Proto__Messages__Report__UpdateInfo proto_ReportUpdateInfo;
typedef CarSync__Proto__Messages__RPC proto_RPC;
typedef CarSync__Proto__Messages__Notification proto_Notification;
typedef CarSync__Proto__Messages__RPCConfigurationReq proto_InstalledConfig;
typedef CarSync__Proto__Messages__RPCConfigurationRsp proto_AvailableConfig;
typedef CarSync__Proto__Messages__RPCError proto_RPCError;
typedef CarSync__Proto__ReleasePackage__ReleasePackageMeta proto_ReleasePackageMeta;
typedef CarSync__Proto__ReleasePackage__UpdateDescriptor proto_UpdateDescriptor;
typedef CarSync__Proto__ReleasePackage__UpdateFileItem proto_UpdateFileItem;
typedef CarSync__Proto__ReleasePackage__UpdateFileMeta proto_UpdateFileMeta;
typedef CarSync__Proto__Common__Version proto_Version;

typedef enum {
    proto_MESSAGE_REPORT = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__MESSAGE_TYPE__MESSAGE_REPORT,
    proto_MESSAGE_CONFIG_RPC = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__MESSAGE_TYPE__MESSAGE_RPC,
    proto_MESSAGE_NOTIFICATION = CAR_SYNC__PROTO__MESSAGES__BASE_MESSAGE__MESSAGE_TYPE__MESSAGE_NOTIFICATION
} proto_MessageType;

typedef enum {
    proto_REPORT_STATUS = CAR_SYNC__PROTO__MESSAGES__REPORT__REPORT_TYPE__REPORT_STATUS,
    proto_REPORT_UPDATE = CAR_SYNC__PROTO__MESSAGES__REPORT__REPORT_TYPE__REPORT_UPDATE,
    proto_REPORT_ERROR = CAR_SYNC__PROTO__MESSAGES__REPORT__REPORT_TYPE__REPORT_ERROR
} proto_ReportType;

typedef enum {
  proto_UPDATE_PENDING = CAR_SYNC__PROTO__MESSAGES__REPORT__UPDATE_INFO__UPDATE_STATUS__UPDATE_PENDING,
  proto_UPDATE_SUCCESS = CAR_SYNC__PROTO__MESSAGES__REPORT__UPDATE_INFO__UPDATE_STATUS__UPDATE_SUCCESS,
  proto_UPDATE_FAILURE = CAR_SYNC__PROTO__MESSAGES__REPORT__UPDATE_INFO__UPDATE_STATUS__UPDATE_FAILURE
} proto_UpdateStatus;

typedef enum {
    proto_NOTIFICATION_CONF_REPORT = CAR_SYNC__PROTO__MESSAGES__NOTIFICATION__NOTIFICATION_TYPE__CONF_REPORT,
    proto_NOTIFICATION_VERSION_UPDATE = CAR_SYNC__PROTO__MESSAGES__NOTIFICATION__NOTIFICATION_TYPE__VERSION_UPDATE
} proto_NotificationType;

/**
 * @brief Compares two version structures. If some field is missng then it is
 *       considered as smaller.
 * @param[in] a First version structure.
 * @param[in] b Second version structure.
 * @return 0 if equal, -1 if a < b, 1 if a > b.
 */
extern int proto_versionCmp(const proto_Version* a, const proto_Version* b);

/**
 * @brief Unpack default UF.meta file to UpdateFileMeta structure.
 * @param[in] ufMetaDir Directory where UF.meta will be looked for.
 * @return Pointer to unpacked UpdateFileMeta.
 *        Caller of this function is responsible for releasing allocated
 *        structure when they are not needed anymore (with *__free__unpacked).
 */
extern proto_UpdateFileMeta* proto_unpackMetaFile(const char* ufMetaDir);

#ifdef __cplusplus
}
#endif

#endif
