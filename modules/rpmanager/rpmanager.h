/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Library that manage and apply/rollback release packages.
 */

#ifndef RPMANAGER_H
#define RPMANAGER_H

# ifdef __cplusplus
extern "C" {
# endif

#include "proto.h"

typedef enum {
    RPMANAGER_STATUS_CRITICAL = -1, /** Critical error occured - can not ensure proper application behaviour. Manual fix needed. */
    RPMANAGER_STATUS_OK, /** Last operation succeed. */
    RPMANAGER_STATUS_ERROR /** Last operation failed. */
} rpmanager_Status;

/**
 * @brief Apply given update and update vehicle assigned RPs database.
 * @param[in] availableConfig Structure describing available config to be applied.
 * @param[in] updateFilesPath Path to directory where all update related files
 *       are stored. May be 0 if there is no update (only assigned RPs change).
 * @param[out] statusMessage Pointer to status message string.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return Status of the operation.
 */
extern rpmanager_Status rpmanager_applyRP(const proto_AvailableConfig* availableConfig,
                             const char* updateFilesPath, char** statusMessage);

/**
 * @brief Try to resume interrupted update (e.g. unexpected system reboot
 *       during update).
 * @param[in] availableConfig Structure describing available config to be applied.
 * @param[in] updateFilesPath Path to directory where all update related files
 *       are stored. May be 0 if there is no update (only assigned RPs change).
 * @param[out] statusMessage Pointer to status message string.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return Status of the operation.
 */
extern rpmanager_Status rpmanager_resume(const proto_AvailableConfig* availableConfig,
                            const char* updateFilesPath, char** statusMessage);

/**
 * @brief Gets installed configuration.
 * @param[out] installedConfig Structure describing installed config.
 *       Caller of this function is responsible for releasing allocated
 *       structure when they are not needed anymore (with *__free__unpacked).
 * @return Status of the operation.
 */
extern rpmanager_Status rpmanager_getInstalledConfig(proto_InstalledConfig** installedConfig);


/**
 * @brief Initializes RPManager storage.
 * @return Status of the operation.
 */
extern int rpmanager_init();

/**
 * @brief Cleans up RPManager storage.
 * @return Status of the operation.
 */
extern void rpmanager_cleanup();


# ifdef __cplusplus
}
# endif

#endif
