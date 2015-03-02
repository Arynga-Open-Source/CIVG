/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "test_config.h"
#include <stdlib.h>

transport_config_ConnectionConfig* transport_config_get() {
    transport_config_ConnectionConfig* conf = malloc(sizeof(transport_config_ConnectionConfig));
    if (conf) {
        conf->id = 0;
    }
    return conf;
}

void transport_config_free(transport_config_ConnectionConfig* config) {
    if (config) {
        free(config);
    }
}
