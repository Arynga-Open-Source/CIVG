/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "rvi_config_priv.h"
#include "config.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

transport_config_ConnectionConfig* transport_config_get() {
    transport_config_ConnectionConfig* conf = malloc(sizeof(transport_config_ConnectionConfig));
    if (conf) {
        memset(conf, 0, sizeof(transport_config_ConnectionConfig));
        config_Storage* storage = config_newStorage(RVI_STORAGE_ID);
        if (storage) {
            int size;
            int status = 1;
            if (config_getValue(storage, RVI_NODE_ADDR_KEY, (void**)&conf->rvi_node, &size) != 0
                || strlen(conf->rvi_node) == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RVI version from permanent storage!");
            } else {
                status = 0;
            }
            if (status != 0) {
                transport_config_free(conf);
                conf = 0;
            }
            config_deleteStorage(storage);
        }
    }
    return conf;
}

void transport_config_free(transport_config_ConnectionConfig* config) {
    if (config) {
        free(config->rvi_node);
        free(config);
    }
}
