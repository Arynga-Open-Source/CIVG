/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
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
