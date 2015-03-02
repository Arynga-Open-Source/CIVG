/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "RMQ_CONSUMER"
#define LOGGER_FILE "rmq_consumer.log"

#include "rmq_consumer.h"
#include "rmq_config.h"
#include "msgqueue.h"
#include "rmq_tools.h"
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static struct timeval CONSUME_MSG_TIMEOUT = {1, 1};
static const int RECONNECT_INTERVAL = 1; // in seconds
static const int CHECK_INTERVAL = 30; // in seconds

static const transport_config_ConnectionConfig* _config;
static msgqueue_Queue* _queue = 0;
static pthread_mutex_t _queueLock;

static int _breakReceive  = 0;
static pthread_mutex_t _breakLock;

static time_t _lastCheck = 0;
static int _checkRequired = 0;
static pthread_mutex_t _checkLock;

static amqp_bytes_t _rmqQueue = {0, 0};
static pthread_mutex_t _replyQueueLock;

static void disconnect() {
    amqpDisconnect();
    if (_rmqQueue.bytes) {
        amqp_bytes_free(_rmqQueue);
        _rmqQueue.bytes = 0;
        // lock reply queue since it is not available
        // producer will have to wait for reconnect
        pthread_mutex_lock(&_replyQueueLock);
    }
}

static int checkConnection() {
    int status = 1;
    if (_rmqConnection) {
        pthread_mutex_lock(&_checkLock);
        time_t currentTime = time(NULL);
        if (_checkRequired || currentTime - _lastCheck > CHECK_INTERVAL) {
            _checkRequired = 0;
            _lastCheck = currentTime;
            // test if rmq server is still available
            // ToDo make it in smarter way
            amqp_connection_state_t testConnection = amqp_new_connection();
            if (testConnection) {
                amqp_socket_t* socket = amqp_tcp_socket_new(testConnection);
                if (socket
                    && amqp_socket_open(socket, _config->addr, _config->port) == AMQP_STATUS_OK) {
                    status = 0;
                }
                amqp_destroy_connection(testConnection);
            }
        } else {
            status = 0;
        }
        pthread_mutex_unlock(&_checkLock);
    } else if (amqpConnect(_config) == 0) {
        amqp_queue_declare_ok_t* qResult =
                amqp_queue_declare(_rmqConnection, 1, amqp_empty_bytes,
                                   0, 0, 1, 1, amqp_empty_table);
        if (amqpReplyCheck(amqp_get_rpc_reply(_rmqConnection)) != 0) {
            LOGM(LOGG_WARNING, "Failed to declare rmq queue!");
        } else {
            _rmqQueue = amqp_bytes_malloc_dup(qResult->queue);
            if (_rmqQueue.bytes) {
                amqp_queue_bind(_rmqConnection, 1, _rmqQueue,
                                amqp_cstring_bytes("NOTIFICATION"),
                                amqp_empty_bytes, amqp_empty_table);
                if (amqpReplyCheck(amqp_get_rpc_reply(_rmqConnection)) != 0) {
                    LOGM(LOGG_WARNING, "Failed to bind NOTIFICATION to queue!");
                } else {
                    amqp_basic_consume(_rmqConnection, 1, _rmqQueue, amqp_empty_bytes,
                                       0, 0, 0, amqp_empty_table);
                    if (amqpReplyCheck(amqp_get_rpc_reply(_rmqConnection)) != 0) {
                        LOGM(LOGG_WARNING, "Failed to basic consume!");
                    } else {
                        _lastCheck = time(NULL);
                        status = 0;
                    }
                }
                // unlock reply queue to be used by producer
                pthread_mutex_unlock(&_replyQueueLock);
            } else {
                LOGM(LOGG_WARNING, "Failed to malloc rmq queue!");
            }
        }
    }
    if (status != 0) {
        disconnect();
    }
    return status;
}

int threaded_consumer_init(const transport_config_ConnectionConfig* config) {
    if (_config) {
        LOGM(LOGG_ERROR, "Doubled rmq producer initialization!");
        return 1;
    }

    if (pthread_mutex_init(&_queueLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init consumer queue lock!");
        return 1;
    }
    if (pthread_mutex_init(&_breakLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init consumer break lock!");
        pthread_mutex_destroy(&_queueLock);
        return 1;
    }
    if (pthread_mutex_init(&_replyQueueLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init reply queue lock!");
        pthread_mutex_destroy(&_breakLock);
        pthread_mutex_destroy(&_queueLock);
        return 1;
    }
    if (pthread_mutex_init(&_checkLock, NULL) != 0) {
        LOGM(LOGG_ERROR, "Failed to init reply queue lock!");
        pthread_mutex_destroy(&_replyQueueLock);
        pthread_mutex_destroy(&_breakLock);
        pthread_mutex_destroy(&_queueLock);
        return 1;
    }
    _config = config;
    _breakReceive = 0;
    _checkRequired = 0;
    pthread_mutex_lock(&_replyQueueLock);
    _queue = msgqueue_newQueue();
    if (!_queue) {
        LOGM(LOGG_ERROR, "Failed to create consumer queue!");
        threaded_consumer_cleanup();
        return 1;
    }
    return 0;
}

void threaded_consumer_cleanup() {
    if (_config) {
        disconnect();
        pthread_mutex_destroy(&_checkLock);
        pthread_mutex_unlock(&_replyQueueLock);
        pthread_mutex_destroy(&_replyQueueLock);
        pthread_mutex_destroy(&_breakLock);
        pthread_mutex_destroy(&_queueLock);
        msgqueue_deleteQueue(_queue);
        _config = 0;
    }
}

static int checkBreak() {
    int value;
    pthread_mutex_lock(&_breakLock);
    value = _breakReceive;
    pthread_mutex_unlock(&_breakLock);
    return value;
}

unsigned int threaded_consumer_receiveNext() {
    int received = 0;
    amqp_envelope_t envelope;
    int cResult;
    while (!checkBreak()) {
        if (checkConnection() == 0) {
            amqp_maybe_release_buffers(_rmqConnection);
            cResult = amqpReplyCheck(amqp_consume_message(_rmqConnection,
                                                          &envelope, &CONSUME_MSG_TIMEOUT, 0));
            if (cResult > 0) {
                LOGM(LOGG_WARNING, "Failed to consume message!");
                rmq_consumer_checkConnection();
            } else if (cResult == 0) {
                if (envelope.message.body.bytes && envelope.message.body.len > 0) {
                    // create message safe copy
                    void* msgCopy = malloc(envelope.message.body.len);
                    if (msgCopy) {
                        memcpy(msgCopy, envelope.message.body.bytes, envelope.message.body.len);
                        pthread_mutex_lock(&_queueLock);
                        int status = msgqueue_push(_queue, msgCopy, envelope.message.body.len);
                        pthread_mutex_unlock(&_queueLock);
                        if (status == 0) {
                            ++received;
                        } else {
                            free(msgCopy);
                        }
                    } else {
                        LOGM(LOGG_ERROR, "Failed to allocate memory for message "
                             "to be pushed on queue!");
                    }
                }
                amqp_destroy_envelope(&envelope);
                if (received) {
                    break;
                }
            }
        } else {
            sleep(RECONNECT_INTERVAL);
        }
    }
    return received;
}

void threaded_consumer_break() {
    pthread_mutex_lock(&_breakLock);
    _breakReceive = 1;
    pthread_mutex_unlock(&_breakLock);
}

int threaded_consumer_popMsg(void** msg, size_t* size) {
    pthread_mutex_lock(&_queueLock);
    int status = msgqueue_pop(_queue, msg, size);
    pthread_mutex_unlock(&_queueLock);
    return status;
}

amqp_bytes_t rmq_consumer_getReplyQueue()
{
    amqp_bytes_t queue = {0, 0};
    pthread_mutex_lock(&_replyQueueLock);
    queue = amqp_bytes_malloc_dup(_rmqQueue);
    pthread_mutex_unlock(&_replyQueueLock);
     return queue;
}

void rmq_consumer_checkConnection()
{
    pthread_mutex_lock(&_checkLock);
    _checkRequired = 1;
    pthread_mutex_unlock(&_checkLock);
}
