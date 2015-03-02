/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "RMQ_PRODUCER"
#define LOGGER_FILE "rmq_producer.log"

#include "threaded_producer.h"
#include "rmq_consumer.h"
#include "rmq_config.h"
#include "msgqueue.h"
#include "proto.h"
#include "rmq_tools.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static const transport_config_ConnectionConfig* _config = 0;
static msgqueue_Queue* _queue = 0;
static pthread_mutex_t _queueLock;

static pthread_mutex_t _emptyLock;

int threaded_producer_init(const transport_config_ConnectionConfig* config) {
    if (_config) {
        LOGM(LOGG_ERROR, "Doubled rmq producer initialization!");
        return 1;
    }

    if (pthread_mutex_init(&_queueLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init producer queue lock!");
        return 1;
    }
    if (pthread_mutex_init(&_emptyLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init producer empty lock!");
        pthread_mutex_destroy(&_queueLock);
        return 1;
    }
    pthread_mutex_lock(&_emptyLock);
    _config = config;
    _queue = msgqueue_newQueue();
    if (!_queue) {
        LOGM(LOGG_ERROR, "Failed to create producer queue!");
        threaded_producer_cleanup();
        return 1;
    }
    return 0;
}

void threaded_producer_cleanup() {
    if (_config) {
        amqpDisconnect();
        if (msgqueue_empty(_queue)) {
            pthread_mutex_unlock(&_emptyLock);
        }
        pthread_mutex_destroy(&_emptyLock);
        pthread_mutex_destroy(&_queueLock);
        msgqueue_deleteQueue(_queue);
        _config = 0;
    }
}

static int publishMessage(void* msg, size_t msgSize) {
    if (!_rmqConnection) {
        LOGM(LOGG_WARNING, "Can not publish if not connected yet!");
        return 1;
    }

    // fill amqp message properties based on message type
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, msgSize, msg);
    if (!baseMsg) {
        LOGM(LOGG_WARNING, "Invalid message to publish - can not unpack BaseMessage.");
        return 1;
    }
    int status = 1;
    const char* routingKey = 0;
    amqp_basic_properties_t props;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 2;
    if (baseMsg->has_data_type) {
        switch (baseMsg->data_type) {
        case proto_MESSAGE_REPORT:
            routingKey = "REPORT";
            props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
            break;
        case proto_MESSAGE_CONFIG_RPC:
            // get reply queue from consumer (where rpc reply will be send to)
            props.reply_to = rmq_consumer_getReplyQueue();
            if (props.reply_to.bytes) {
                routingKey = "RPC";
                props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_REPLY_TO_FLAG;
            } else {
                LOGM(LOGG_WARNING, "Failed to get reply queue - can not send RPC!");
            }
            break;
        default:
            LOG(LOGG_WARNING, "Unsupported message to publish. Type: %d", baseMsg->data_type);
            break;
        }
    } else {
        LOGM(LOGG_WARNING, "Invalid message to publish - missing data type.");
    }
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    // if successfully got props then routingKey is not empty and we can publish message
    if (routingKey) {
        amqp_bytes_t data;
        data.bytes = msg;
        data.len = msgSize;
        // TODO: FiXMe amqp_basic_publish returns OK even if thre is no connection and message is not delivered!?
        amqp_status_enum amqpStatus = amqp_basic_publish(_rmqConnection, 1, amqp_empty_bytes, amqp_cstring_bytes(routingKey), 0, 0, &props, data);
        if (amqpStatus == AMQP_STATUS_OK) {
            status = 0;
        } else {
            LOG(LOGG_WARNING, "Failed to publish message! AMQP Status: %d", amqpStatus);
        }
        if (props._flags & AMQP_BASIC_REPLY_TO_FLAG) {
            // remember to release memory allocated for reply_to
            amqp_bytes_free(props.reply_to);
            props.reply_to.bytes = 0;
        }
    }
    return status;
}

void threaded_producer_sendNext() {
    // wait if queue is empty until something is pushed
    pthread_mutex_lock(&_emptyLock);

    pthread_mutex_lock(&_queueLock);
    void* msg;
    size_t msgSize;
    if (msgqueue_pop(_queue, &msg, &msgSize)) {
        // unlock emptyLock if still something left
        if (!msgqueue_empty(_queue)) {
            pthread_mutex_unlock(&_emptyLock);
        }
        // unlock queue
        pthread_mutex_unlock(&_queueLock);
        // and continue processing

        int status = 1;

        if (amqpConnect(_config) == 0) {
            if (publishMessage(msg, msgSize) == 0) {
                status = 0;
                free(msg);
            }
            amqpDisconnect();
        } else {
            // notify consumer about connection error
            rmq_consumer_checkConnection();
        }

        if (status != 0) {
            // push message back on queue if failed to send
            pthread_mutex_lock(&_queueLock);
            int wasEmpty = msgqueue_empty(_queue);
            if (msgqueue_push(_queue, msg, msgSize) == 0) {
                if (wasEmpty) {
                    // unlock emptyLock if queue was empty - it is not anymore
                    pthread_mutex_unlock(&_emptyLock);
                }
            } else {
                LOGM(LOGG_ERROR, "Failed to push not sent message back on queue - message lost!");
                free (msg);
            }
            pthread_mutex_unlock(&_queueLock);
        }
    } else {
        // if there is nothing to pop (queue is empty) then do not unlock emptyLock - block on next call
        pthread_mutex_unlock(&_queueLock);
    }
}

int threaded_producer_pushMsg(const void* msg, size_t size) {
    pthread_mutex_lock(&_queueLock);
    // unlock empty lock if queue was empty - it is not anymore
    if (msgqueue_empty(_queue)) {
        pthread_mutex_unlock(&_emptyLock);
    }
    int status = 1;
    if (msg && size > 0) {
        // create message safe copy
        void* msgCopy = malloc(size);
        if (msgCopy) {
            memcpy(msgCopy, msg, size);
            if (msgqueue_push(_queue, msgCopy, size) == 0) {
                status = 0;
            } else {
                free(msgCopy);
            }
        } else {
            LOGM(LOGG_ERROR, "Failed to allocate memory for message to be pushed on queue!");
        }
    }
    pthread_mutex_unlock(&_queueLock);
    return status;
}
