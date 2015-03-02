/** Arygna CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * CIVG adapter that expose public CIVG functionality to external module like Demo UI.
 */

#ifndef WS_ADAPTER_H
#define WS_ADAPTER_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Initialize CIVG adapter service.
 * @return 0 if succeed, 1 otherwise.
 */
extern int adapter_init();

/**
 * @brief Initialize CIVG adapter service.
 * @return 0 if succeed, 1 otherwise.
 */
extern void adapter_cleanup();

# ifdef __cplusplus
}
# endif

#endif

