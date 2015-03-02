/* Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#ifndef _CS_SOUTH_UTILS_H_
#define _CS_SOUTH_UTILS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns contents of UF metadata from given UF file
 * @param uf_path[in] path ot the update file
 * @return string with metadata about given UF, has to be 
 * 		freed when no longer needed
 */
const char* cs_proto_get_uf_meta(const char* uf_path, size_t* len);

#ifdef __cplusplus
}
#endif

#endif /* _CS_SOUTH_UTILS_H_ */
