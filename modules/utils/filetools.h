/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * OS specific implementation of file related functionality.
 */

#ifndef FILETOOLS_H
#define FILETOOLS_H

# ifdef __cplusplus
extern "C" {
# endif

#include <stddef.h>

/**
 * @brief Creates path from path to the directory and file name
 *       Returned value must be freed manually.
 * @param[in] dir directory path
 * @param[in] name file name
 * @retval NULL on error
 * @retval c-string containing path on success
 */
extern char* filetools_joinPath(const char* dir, const char* name);

/**
 * @brief Gets directory from file path.
 *       Returned value must be freed manually.
 * @param[in] path File path.
 * @return
 */
extern char* filetools_getDir(const char* path);

/**
 * @brief Extracts tar file in specified place.
 * @param[in] path Path to the tar file.
 * @param[in] targetDir Path to target directory.
 * @return 0 if succeed, 1 otherwise.
 */
extern int filetools_extractTar(const char* path, const char* targetDir);

/**
 * @brief Returns content of file.
 * @param[in] path Path to the file.
 * @param[out] data Pointer to newly allocated array with file content.
 *       Caller of this function is responsible for releasing allocated
 *       memory when it is not needed anymore.
 * @param[out] size Size of returned array.
 * @return 0 if succeed, 1 otherwise.
 */
extern int filetools_readAll(const char* path, void** data, size_t *size);

/**
 * @brief Removes whole directory with its content.
 * @param[in] dirPath Path to existing directory.
 * @return 0 if succeed, 1 otherwise.
 */
extern int filetools_removeDir(const char* dirPath);

/**
 * @brief Creates directory with all parents.
 * @param[in] dirPath Path to directory which will be created.
 * @return 0 if succeed, 1 otherwise.
 */
extern int filetools_makePath(const char* dirPath);

/**
 * @brief Calculates SHA1 checksum for a file
 * @param[in] filePath Path to the file.
 * @return Calculated checksum, or NULL on error.
 */
extern char* filetools_getSHA1Checksum(const char* filePath);

# ifdef __cplusplus
}
# endif

#endif
