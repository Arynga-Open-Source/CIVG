/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * RVI specific connection config strucuture.
 */

#ifndef RVI_CONFIG_H
#define RVI_CONFIG_H

#include "transport_config.h"

struct transport_config_ConnectionConfig {
    char *rvi_node; /** RVI node address */
};

#endif
