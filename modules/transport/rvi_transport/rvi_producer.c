/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#define LOGGER_MODULE "RVI_PRODUCER"
#define LOGGER_FILE "rvi_producer.log"

#include <logger.h>
#include "threaded_producer.h"
#include "rvi_config.h"
#include "msgqueue.h"
#include "proto.h"
#include "rvi_tools.h"
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
        if (msgqueue_empty(_queue)) {
            pthread_mutex_unlock(&_emptyLock);
        }
        pthread_mutex_destroy(&_emptyLock);
        pthread_mutex_destroy(&_queueLock);
        msgqueue_deleteQueue(_queue);
        _config = 0;
    }
}

static int publishMessage(void* msg, int msgSize) {
    // find target URI based on message type
    proto_BaseMessage* baseMsg = car_sync__proto__messages__base_message__unpack(NULL, msgSize, msg);
    if (!baseMsg) {
        LOGM(LOGG_WARNING, "Invalid message to publish - can not unpack BaseMessage.");
        return 1;
    }
    int status = 1;

    const char *target = NULL;

    if (baseMsg->has_data_type) {
        switch (baseMsg->data_type) {
        case proto_MESSAGE_REPORT:
            target = "jlr.com/backend/carsync/report";
            break;
        case proto_MESSAGE_CONFIG_RPC:
            target = "jlr.com/backend/carsync/configuration/request";
            break;
        default:
            LOG(LOGG_WARNING, "Unsupported message to publish. Type: %d", baseMsg->data_type);
            break;
        }
    } else {
        LOGM(LOGG_WARNING, "Invalid message to publish - missing data type.");
    }
    car_sync__proto__messages__base_message__free_unpacked(baseMsg, NULL);

    LOG(LOGG_DEBUG, "publish to: %s", target);
    LOG(LOGG_DEBUG, "node: %s", _config->rvi_node);

    if (target) {
        int ret = rvi_send_cs_message(_config->rvi_node, target,
                                      msg, msgSize);
        if (ret == 0) {
            status = 0;
        } else {
            LOG(LOGG_WARNING, "Failed to publish message! RVI Status: %d", ret);
        }
    }
    return status;
}

void threaded_producer_sendNext() {
    // wait if queue is empty until something is pushed
    pthread_mutex_lock(&_emptyLock);

    pthread_mutex_lock(&_queueLock);
    void* msg;
    int msgSize;
    if (msgqueue_pop(_queue, &msg, &msgSize)) {
        // unlock emptyLock if still something left
        if (!msgqueue_empty(_queue)) {
            pthread_mutex_unlock(&_emptyLock);
        }
        // unlock queue
        pthread_mutex_unlock(&_queueLock);
        // and continue processing

        int status = 1;

        if (publishMessage(msg, msgSize) == 0) {
            status = 0;
            free(msg);
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
