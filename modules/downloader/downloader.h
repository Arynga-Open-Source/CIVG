/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This is library for parallel downloading set of files.
 */

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Initialize downloader.
 * @return 0 if succeed, 1 otherwise.
 */
extern int downloader_init();

/**
 * @brief Release all acquired resources before shutdown.
 */
extern void downloader_cleanup();

/**
 * @brief Finished download handler function.
 * @param[in] id Id of download item that has been finished.
 * @param[in] status 0 if succeed, 1 otherwise.
 */
typedef void (*DOWNLOADER_FINISHED_HANDLER)(const char* id, int status);
/**
 * @brief Download progress handler function.
 * @param[in] id Id of download item that has been finished.
 * @param[in] progress percentage as unsigned int 0-100.
 */
typedef void (*DOWNLOADER_PROGRESS_HANDLER)(const char* id, unsigned int status);
/**
 * @brief Sets finished download handler function.
 * @param[in] handler Pointer to the function that will handle finished download.
 */
extern void downloader_setFinishedHandler(DOWNLOADER_FINISHED_HANDLER handler);

/**
 * @brief Sets download progress handler function.
 * @param[in] handler Pointer to the function that will handle download progress.
 */
extern void downloader_setProgressHandler(DOWNLOADER_PROGRESS_HANDLER handler);

/**
 * @brief Starts downloading of requested item. Finished handler will be called
 *       on download end (either successful or not).
 * @param[in] url Source url where file will be downloaded from.
 * @param[in] path Absolute file path (with file name) where downloaded file should be saved.
 * @param[out] id 0 terminated string with unique id of download item.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return 0 if succeed, 1 otherwise.
 */
extern int downloader_downloadItem(const char* url, const char* path, char** id);

/**
 * @brief Requests to abort started download. It is not guaranted when
 *       actual download will end.
 * @param[in] id 0 terminated string with id of download item to be canceled.
 * @return 0 if succeed, 1 otherwise.
 */
extern int downloader_cancel(const char* id);

# ifdef __cplusplus
}
# endif

#endif
