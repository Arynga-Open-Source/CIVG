/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "threaded_transport.h"
#include "threaded_consumer.h"
#include "threaded_producer.h"
#include "transport_config.h"
#include "logger.h"
#include "civg_compiler.h"
#include <pthread.h>
#include <stdlib.h>

static pthread_t _initThread = 0;
static pthread_t _consumerThread = 0;
static pthread_t _producerThread = 0;

static int _continue = 0;
static pthread_mutex_t _continueLock;

static TRANSPORT_MSG_HANDLER _handler = 0;
static dispatcher_Dispatcher* _dispatcher = 0;

static transport_config_ConnectionConfig* _connectionConfig = 0;

static int checkContinue() {
    int value;
    pthread_mutex_lock(&_continueLock);
    value = _continue;
    pthread_mutex_unlock(&_continueLock);
    return value;
}

static void* producerRun(void* tArgs) {
    while (checkContinue()) {
        // sent next message or wait for something to send
        threaded_producer_sendNext();
    }
    pthread_exit(tArgs);
}

static void* consumerRun(void* tArgs) {
    while (checkContinue()) {
        if (threaded_consumer_receiveNext()
            && _dispatcher) {
            // post new message event via dispatcher
            dispatcher_postEvent(_dispatcher, 0);
        }
    }
    pthread_exit(tArgs);
}

static void eventHandler(CIVG_UNUSED int eventId ) {
    if (!pthread_equal(_initThread, pthread_self())) {
        LOGM(LOGG_ERROR, "Critical errror! MsgEvent not dispatched correctly!");
        return;
    }

    void* msg;
    size_t size;
    while (threaded_consumer_popMsg(&msg, &size)) {
        if (_handler) {
            _handler(msg, size);
        }
        free(msg);
    }
}

extern int transport_init() {
    if (!_dispatcher) {
        LOGM(LOGG_ERROR, "Can not init threaded_transport. Setup msg event dispatcher first!");
        return 1;
    }
    if (_initThread) {
        LOGM(LOGG_ERROR, "Doubled transport initialization!");
        return 1;
    }

    pthread_attr_t attr;
    int status = 1;
    if (pthread_attr_init(&attr) == 0) {
        if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0) {
            if (pthread_mutex_init(&_continueLock, NULL) == 0) {
                _continue = 1;
                _initThread = pthread_self();

                _connectionConfig = transport_config_get();
                if (!_connectionConfig) {
                    LOGM(LOGG_ERROR, "Failed to get connection config!");
                } else if (threaded_consumer_init(_connectionConfig) != 0) {
                    LOGM(LOGG_ERROR, "Failed to init consumer resources!");
                } else if (pthread_create(&_consumerThread, &attr, consumerRun, NULL) != 0) {
                    LOGM(LOGG_ERROR, "Failed to create consumer thread!");
                } else if (threaded_producer_init(_connectionConfig) != 0) {
                    LOGM(LOGG_ERROR, "Failed to init producer resources!");
                } else if (pthread_create(&_producerThread, &attr, producerRun, NULL) != 0) {
                    LOGM(LOGG_ERROR, "Failed to create producer thread!");
                } else {
                    status = 0;
                }
                if (status != 0) {
                    transport_cleanup();
                }
            } else {
                LOGM(LOGG_ERROR, "Failed to initialize mutex!");
            }
        } else {
            LOGM(LOGG_ERROR, "Failed to set thread attributes!");
        }
        pthread_attr_destroy(&attr);
    } else {
        LOGM(LOGG_ERROR, "Failed to initialize thread attributes!");
    }
    return status;
}

extern void transport_cleanup() {
    if (_initThread) {
        pthread_mutex_lock(&_continueLock);
        _continue = 0;
        pthread_mutex_unlock(&_continueLock);

        // push empty message to make sure producer exit sendNext
        threaded_producer_pushMsg(0, 0);
        if (_producerThread) {
            pthread_join(_producerThread, NULL);
            _producerThread = 0;
        }

        threaded_consumer_break();
        if (_consumerThread) {
            pthread_join(_consumerThread, NULL);
            _consumerThread = 0;
        }

        // cleanup when all threads are stopped
        threaded_producer_cleanup();
        threaded_consumer_cleanup();

        pthread_mutex_destroy(&_continueLock);

        transport_config_free(_connectionConfig);
        _connectionConfig = 0;
        _dispatcher = 0;
        _handler = 0;

        _initThread = 0;
    }
}

extern int transport_sendMsg(const void* msg, size_t size) {
    return threaded_producer_pushMsg(msg, size);
}

extern void transport_setMsgHandler(TRANSPORT_MSG_HANDLER handler) {
    _handler = handler;
}

extern void threaded_transport_setDispatcher(dispatcher_Dispatcher *dispatcher) {
    _dispatcher = dispatcher;
    dispatcher_setEventHandler(_dispatcher, eventHandler);
}
