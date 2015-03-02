/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "rmq_config_priv.h"
#include "config.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

transport_config_ConnectionConfig* transport_config_get() {
    transport_config_ConnectionConfig* conf = malloc(sizeof(transport_config_ConnectionConfig));
    if (conf) {
        memset(conf, 0, sizeof(transport_config_ConnectionConfig));
        config_Storage* storage = config_newStorage(RMQ_STORAGE_ID);
        if (storage) {
            int size;
            int status = 1;
            if (config_getValue(storage, USERNAME_KEY, (void**)&conf->username, &size) != 0
                || strlen(conf->username) == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RMQ username from permanent storage!");
            } else if (config_getValue(storage, PASSWORD_KEY, (void**)&conf->password, &size) != 0
                       || strlen(conf->password) == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RMQ password from permanent storage!");
            } else if (config_getValue(storage, VHOST_KEY, (void**)&conf->vhost, &size) != 0
                       || strlen(conf->vhost) == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RMQ vhost from permanent storage!");
            } else if (config_getValue(storage, ADDR_KEY, (void**)&conf->addr, &size) != 0
                       || strlen(conf->addr) == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RMQ address from permanent storage!");
            } else if (config_getIValue(storage, PORT_KEY, &conf->port) != 0
                       || conf->port == 0) {
                LOGM(LOGG_ERROR, "Cannot get valid RMQ port from permanent storage!");
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
        free(config->username);
        free(config->password);
        free(config->vhost);
        free(config->addr);
        free(config);
    }
}
