/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
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
