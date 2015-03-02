/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Library that handle south API communication.
 */

#ifndef VBSCLIENT_H
#define VBSCLIENT_H

#include "proto.h"

/**
 * @brief Initialize VBSClient module.
 * @return 0 if succeed, 1 otherwise.
 */
int vbsclient_init();

/**
 * @brief Release all acquired resources before shutdown.
 */
void vbsclient_cleanup();

/**
 * @brief Send update status report to VBS.
 * @param[in] status Update status.
 * @param[in] availableConfig Applied config.
 * @param[in] description Report description.
 * @return 0 if succeed, 1 otherwise.
 */
int vbsclient_sendUpdateReport(proto_UpdateStatus status,
                               proto_AvailableConfig* availableConfig,
                               char *description);

/**
 * @brief Send error report to VBS.
 * @param[in] description Error description.
 * @return 0 if succeed, 1 otherwise.
 */
int vbsclient_sendErrorReport(char *description);

/**
 * @brief Checks available configuration for given vehicle and installed config.
 * @param[in] installedConfig Structure describing installed config.
 * @return 0 if succeed, 1 otherwise.
 */
int vbsclient_checkConfig(proto_InstalledConfig *installedConfig);

/**
 * @brief Available config handler function.
 * @param[in] availableConfig Pointer to structure describing available config.
 *       Pointed strucutre is deleted after function return, so it must be cloned
 *       for further use (if needed).
 * @return 0 if succeed, 1 otherwise.
 */
typedef void (*VBSCLIENT_AVAILABLE_CONFIG_HANDLER)(proto_AvailableConfig* availableConfig);
/**
 * @brief Sets available config handler function.
 * @param[in] handler Pointer to the function that will handle available config.
 */
void vbsclient_setAvailableConfigHandler(VBSCLIENT_AVAILABLE_CONFIG_HANDLER handler);

/**
 * @brief Update notification handler function.
 */
typedef void (*VBSCLIENT_UPDATE_NOTIFICATION_HANDLER)();
/**
 * @brief Sets update notification handler function.
 * @param[in] handler Pointer to the function that will handle update notification.
 */
void vbsclient_setUpdateNotificationHandler(VBSCLIENT_UPDATE_NOTIFICATION_HANDLER handler);
#endif
