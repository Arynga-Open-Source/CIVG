/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Transport connection config strucuture and associated functions.
 */

#ifndef TRANSPORT_CONFIG_H
#define TRANSPORT_CONFIG_H

/**
 * @brief Opaque structure that identify transport connection config.
 */
typedef struct transport_config_ConnectionConfig transport_config_ConnectionConfig;

/**
 * @brief Allocates connection config and fill with values from permanent sotrage.
 * @return Pointer to newly allocaded connection config, or 0 if failed.
 *        Caller of this function is responsible for releasing allocated
 *        structure when it is not needed anymore (with transport_config_free()).
 */
transport_config_ConnectionConfig* transport_config_get();

/**
 * @brief Release all resources associated with given config.
 * @param[in] config Pointer to connection config.
 */
void transport_config_free(transport_config_ConnectionConfig* config);

#endif
