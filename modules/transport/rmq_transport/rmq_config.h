/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * RMQ specific connection config strucuture.
 */

#ifndef RMQ_CONFIG_H
#define RMQ_CONFIG_H

#include "transport_config.h"

struct transport_config_ConnectionConfig {
    char* username; /** RabbitMQ username for login */
    char* password; /** RabbitMQ password for login */
    char* vhost; /** Virtual host */
    char* addr; /** RabbitMQ server address */
    int port; /** RabbitMQ server port number */
};

#endif
