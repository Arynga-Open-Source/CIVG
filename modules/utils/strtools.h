/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * OS specific implementation of file related functionality.
 */

#ifndef STRTOOLS_H
#define STRTOOLS_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Allocates necessary memory for new string and formats it with given
 *       format and arguments.
 * @param[in] format Format of new string.
 * @return Newly allocated string or 0 if failed. Returned value must be freed
 *        manually when no more needed.
 */
extern char* strtools_sprintNew(const char* format, ...);

# ifdef __cplusplus
}
# endif

#endif
