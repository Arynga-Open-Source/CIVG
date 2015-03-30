/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * This is library for querying CIVG Service
 */

#ifndef ADAPTER_H
#define ADAPTER_H

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Initialize downloader.
 * @return 0 if succeed, 1 otherwise.
 */
extern int adapter_init();

/**
 * @brief Release all acquired resources before shutdown.
 */
extern void adapter_cleanup();

# ifdef __cplusplus
}
# endif

#endif
