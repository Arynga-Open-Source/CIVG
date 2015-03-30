/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * This is core library responsible for coordinating other parts, listening VBS,
 * triggering downloads and actual updates, and finally sending reports back to VBS.
 */

#ifndef CORE_H
#define CORE_H

# ifdef __cplusplus
extern "C" {
# endif

typedef enum {
    CORE_STATE_BROKEN = -1,
    CORE_STATE_IDLE,
    CORE_STATE_DOWNLOAD,
    CORE_STATE_UPGRADE
} core_State;

typedef enum {
    CORE_STATUS_CRITICAL = -1, /** Critical error occured - can not ensure proper application behaviour. Manual fix needed. */
    CORE_STATUS_OK, /** Last operation succeed. */
    CORE_STATUS_ERROR /** Last operation failed. */
} core_Status;

/**
 * @brief Initialize CIVG service. Try to resume interrupted action at first.
 * @param[in] autoUpgrade Indicate if upgrade procedure should be started
 *       immediately after new RP is downloaded (0 != autoUpgrade) or it should
 *       wait for startUpgrade call (0 == autoUpgrade).
 * @param[in] autoDownload Indicate if download procedure should be started
 *       immediately after new RP is available (0 != autoDownload) or it should
 *       wait for startDownload call (0 == autoDownload).
 * @return Status of the operation.
 */
extern core_Status core_init(int autoUpgrade, int autoDownload);

/**
 * @brief Release all acquired resources before shutdown.
 */
extern void core_cleanup();

/**
 * @brief Perform upgrade procedure if new RP is downloaded.
 * @return Status of the operation.
 */
extern core_Status core_performUpgrade();

/**
 * @brief Perform download procedure if new RP is available.
 * @return Status of the operation.
 */
extern core_Status core_performDownload();

/**
 * @brief Function that handles CIVG state/status change.
 * @param[in] state Current CIVG state.
 * @param[in] status Status of last finished operation.
 */
typedef void (*CORE_STATUS_HANDLER)(const core_State state, const core_Status status);
extern void core_setStatusHandler(CORE_STATUS_HANDLER handler);

/**
 * @brief Get info about status and state.
 */
extern void core_getStatus(core_State* state, core_Status* status);

/**
 * @brief Trigger update check
 */
extern void core_checkUpdates(void);

/**
 * @brief Function gets uuid and version both as char*.
 * @param[in] uuid
 * @param[in] version Contains major, minor and build version in one char*.
 * @return 0 if success, 1 if fail
 */
extern int core_getAvailableUpdates(char** uuid, char** version);


# ifdef __cplusplus
}
# endif

#endif
