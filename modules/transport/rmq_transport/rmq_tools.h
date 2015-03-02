/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * RMQ specific connection config strucuture.
 */

#ifndef RMQ_TOOLS_H
#define RMQ_TOOLS_H

#include "rmq_config.h"
#include "logger.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>

static amqp_connection_state_t _rmqConnection = 0;

/**
 * @brief Checks rmq reply.
 * @param reply Reply to be checked.
 * @return 0 if no error, -1 on timeout, 1 other errors.
 */
static int amqpReplyCheck(amqp_rpc_reply_t reply) {
    switch (reply.reply_type) {
    case AMQP_RESPONSE_NORMAL:
        return 0;
    case AMQP_RESPONSE_NONE:
        LOGM(LOGG_WARNING, "Missing RPC reply type!");
        break;
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        if (reply.library_error != AMQP_STATUS_TIMEOUT) {
            LOG(LOGG_WARNING, "%s", amqp_error_string2(reply.library_error));
        } else {
            return -1;
        }
        break;
    case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch (reply.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
            amqp_connection_close_t *m = (amqp_connection_close_t *) reply.reply.decoded;
            LOG(LOGG_WARNING, "Server connection error %d", m->reply_code);
            break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
            amqp_channel_close_t *m = (amqp_channel_close_t *) reply.reply.decoded;
            LOG(LOGG_WARNING, "Server channel error %d", m->reply_code);
            break;
        }
        default:
            LOG(LOGG_WARNING, "Unknown server error, method id 0x%08X", reply.reply.id);
            break;
        }
    }
    return 1;
}

static void amqpDisconnect() {
    if (_rmqConnection) {
        amqp_destroy_connection(_rmqConnection);
        _rmqConnection = 0;
    }
}

static int amqpConnect(const transport_config_ConnectionConfig* config) {
    if (_rmqConnection) {
        return 0;
    }

    int status = 1;
    _rmqConnection = amqp_new_connection();
    if (_rmqConnection) {
        amqp_status_enum amqpStatus;
        amqp_socket_t* socket = amqp_tcp_socket_new(_rmqConnection);
        if (!socket) {
            LOGM(LOGG_WARNING, "Failed to create new TCP socket!");
        } else if ((amqpStatus = amqp_socket_open(socket, config->addr, config->port)) != AMQP_STATUS_OK) {
            LOG(LOGG_WARNING, "Failed to open TCP socet! Addr: %s, Port: %d, AMQP Status %d", config->addr, config->port, amqpStatus);
        } else if (amqpReplyCheck(amqp_login(_rmqConnection, config->vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, config->username, config->password)) != 0) {
            LOG(LOGG_WARNING, "Failed to login! User: %s, VHost: %s", config->username, config->vhost);
        } else {
            amqp_channel_open(_rmqConnection, 1);
            if (amqpReplyCheck(amqp_get_rpc_reply(_rmqConnection)) != 0) {
                LOGM(LOGG_WARNING, "Failed to open channel!");
            } else {
                status = 0;
            }
        }
    } else {
        LOGM(LOGG_WARNING, "Failed to create new connection!");
    }
    if (status != 0) {
        amqpDisconnect();
    }
    return status;
}

#endif
