/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * This library generate list of files to be downloaded (based on update meta)
 * and pass it to actual downloader.
 */

#ifndef DOWNLOADMGR_H
#define DOWNLOADMGR_H

#include "proto.h"

/**
 * @brief Initialize DownloadMgr module.
 * @return 0 if succeed, 1 otherwise.
 */
int downloadmgr_init();

/**
 * @brief Release all acquired resources before shutdown.
 */
void downloadmgr_cleanup();

/**
 * @brief Finished download handler function.
 * @param[in] status 0 if succeed, 1 otherwise.
 */
typedef void (*DOWNLOADMGR_FINISHED_HANDLER)(int status);
/**
 * @brief Download progress handler function.
 * @param[in] Progress as integer 0-100 .
 */
typedef void (*DOWNLOADMGR_PROGRESS_HANDLER)(unsigned int progress);
/**
 * @brief Sets finished download handler function.
 * @param[in] handler Pointer to the function that will handle finished download.
 */
void downloadmgr_setFinishedHandler(DOWNLOADMGR_FINISHED_HANDLER handler);

/**
 * @brief Sets download progress handler function.
 * @param[in] handler Pointer to the function that will handle download progress.
 */
void downloadmgr_setProgressHandler(DOWNLOADMGR_PROGRESS_HANDLER handler);

/**
 * @brief Starts downloading all files required by update. Finished handler will
 *       be called on download end (either successful or not).
 * @param[in] updateDescriptor Structure describing update to be downloaded.
 * @param[in] updateFilesPath Path to directory where all update related files
 *       should be downloaded.
 * @return 0 if succeed, 1 otherwise.
 */
int downloadmgr_downloadUFs(const proto_UpdateDescriptor *updateDescriptor,
                            const char* updateFilesPath);

/**
 * @brief Try to resume interrupted download (e.g. unexpected system reboot
 *       during download).
 * @param[in] updateDescriptor Structure describing update to be downloaded.
 * @param[in] updateFilesPath Path to directory where all update related files
 *       should be downloaded.
 * @return 0 if succeed, 1 otherwise.
 */
int downloadmgr_resume(const proto_UpdateDescriptor *updateDescriptor,
                       const char* updateFilesPath);

/**
 * @brief Requests to abort all started downloads. It is not guaranted when
 *       actual downloads will end, but finished handler won't be called for
 *       them anyway.
 * @return 0 if succeed, 1 otherwise.
 */
int downloadmgr_cancel();

#endif
