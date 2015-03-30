/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * Library that actualy apply/rollback specific image using image type and OS
 * specific method.
 */

#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

# ifdef __cplusplus
extern "C" {
# endif

#define IMAGEHANDLER_IMAGES_SUBDIR "data"

/**
 * @brief Apply specifed image on system.
 * @param[in] dataPath Absolute path to directory where image file is stored.
 * @param[in] type Type of image file.
 * @param[out] message Pointer to error message string.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return 0 if succeed, 1 otherwise.
 */
extern int imagehandler_apply(const char* dataPath, const char* type, char** message);

/**
 * @brief Rollback specifed image from system.
 * @param[in] dataPath Absolute path to directory where image file is stored.
 * @param[in] type Type of image file.
 * @param[out] message Pointer to error message string.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return 0 if succeed, 1 otherwise.
 */
extern int imagehandler_rollback(const char* dataPath, const char* type,  char** message);

/**
 * @brief Resume interrupted update for a given image.
 * @param[in] dataPath Absolute path to directory where image file is stored.
 * @param[in] type Type of image file.
 * @param[out] message Pointer to error message string.
 *       Caller of this function is responsible for releasing allocated
 *       string when it is not needed anymore.
 * @return 0 if succeed, 1 otherwise.
 */
extern int imagehandler_resume(const char* dataPath, const char* type,  char** message);

# ifdef __cplusplus
}
# endif

#endif
