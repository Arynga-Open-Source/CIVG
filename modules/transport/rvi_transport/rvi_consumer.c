/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "RVI_CONSUMER"
#define LOGGER_FILE "rvi_consumer.log"

#include <logger.h>
#include <threaded_consumer.h>
#include <carinfo.h>
#include "rvi_config.h"
#include "msgqueue.h"
#include "rvi_tools.h"
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>
#include <libsoup/soup.h>

static const int RECONNECT_INTERVAL = 1; // in seconds
static const int CHECK_INTERVAL = 30; // in seconds

static const transport_config_ConnectionConfig* _config;
static msgqueue_Queue* _queue = 0;
static pthread_mutex_t _queueLock;

static int _breakReceive  = 0;
static pthread_mutex_t _breakLock;

static int _checkRequired = 0;
static pthread_mutex_t _checkLock;

static pthread_mutex_t _replyQueueLock;

/* glib context */
static GMainContext *_ctx = NULL;

/* count of received messages during single dispatch */
static int _received = 0;

/* handler for RVI requests */
static SoupServer *_rvi_server = NULL;
/* configuration response service name */
static gchar *_configuration_service = NULL;
/* notification update service name */
static gchar *_notification_service = NULL;

#define LISTEN_PORT 12999
#define CARSYNC_LISTEN_HOST "http://localhost:12999"
#define CARSYNC_CONFIGURATION_SERVICE "carsync/configuration/response"
#define CARSYNC_CONFIGURATION_URI "/configuration/response"
#define CARSYNC_NOTIFICATION_SERVICE "carsync/notification/update"
#define CARSYNC_NOTIFICATION_URI "/notification/update"

#define ERR_BAD_REQUEST -32001
#define ERR_BAD_FORMAT  -32002

static int register_rvi_services(void);
static int setup_rvi(void);
static int setup_rvi_listener(void);

/**
 * @brief handle incoming message
 *
 * Try to decode incoming message and push it to message queue. A
 * decoded message that failed to be pushed to queue is still ok.
 *
 * @return 0 on success
 */
static int __handle_cs_msg(const char *data,
                           const char *target)
{
    LOG(LOGG_DEBUG, "got request: %s", data);

    size_t cs_msg_size = 0;
    void *cs_msg = rvi_parse_cs_message(target, data, &cs_msg_size);

    if (cs_msg == NULL)
    {
        LOGM(LOGG_ERROR, "failed to parse CS message");
        return -1;
    }

    /* got the message, now push to received queue */
    pthread_mutex_lock(&_queueLock);
    int status = msgqueue_push(_queue, cs_msg, cs_msg_size);
    pthread_mutex_unlock(&_queueLock);
    if (status == 0) {
        /* pushed to queue, bump received messages count */
        _received++;
    } else {
        free(cs_msg);
    }

    return 0;
}

/**
 * @brief format RVI service to include VIN
 * @param[in] vin            VIN
 * @param[in] service_name   base service name, ex. carsync/configuration..
 * @return allocated service name, freed with g_free()
 *         by caller or NULL
 */
static char *format_service(const char *vin, const char *service_name)
{
    /* skip leading / if present */
    if (service_name[0] == '/')
        service_name++;

    return g_strdup_printf("/%s/%s", vin, service_name);
}

static void __respond_error(SoupMessage *msg, int http_status,
                            int error_code, const char *error_message)
{
    soup_message_set_status(msg, http_status);

    char *resp = rvi_format_error(error_code, error_message);
    LOG(LOGG_DEBUG, "error response: %s", resp);
    soup_message_set_response(msg, "application/json",
                              SOUP_MEMORY_COPY,
                              resp, strlen(resp));
    free(resp);
}

static void __respond_ok(SoupMessage *msg, int http_status)
{
    soup_message_set_status(msg, http_status);

    char *resp = rvi_format_ok();
    LOG(LOGG_DEBUG, "OK response: %s", resp);
    soup_message_set_response(msg, "application/json",
                              SOUP_MEMORY_COPY,
                              resp, strlen(resp));
    free(resp);
}


static void __notification_cb(SoupServer *srv G_GNUC_UNUSED,
                              SoupMessage *msg,
                              const char *path G_GNUC_UNUSED,
                              GHashTable *query G_GNUC_UNUSED,
                              SoupClientContext *client G_GNUC_UNUSED,
                              gpointer user_data G_GNUC_UNUSED)
{
    LOG(LOGG_DEBUG, "client connected to %s", path);
    LOG(LOGG_DEBUG, "method: %s", msg->method);

    if (msg->method != SOUP_METHOD_POST)
    {
        /* not a post request */
        __respond_error(msg, SOUP_STATUS_NOT_IMPLEMENTED,
                        ERR_BAD_REQUEST, "not supported");
        return;
    }

    if (__handle_cs_msg(msg->request_body->data,
                        _notification_service) != 0)
    {
        __respond_error(msg, SOUP_STATUS_BAD_REQUEST,
                        ERR_BAD_FORMAT, "incorrect notification data");
        return;
    }
    else
    {
        LOGM(LOGG_DEBUG, "got notification message");
    }

    __respond_ok(msg, SOUP_STATUS_OK);
}


static void __configuration_cb(SoupServer *srv G_GNUC_UNUSED,
                               SoupMessage *msg,
                               const char *path G_GNUC_UNUSED,
                               GHashTable *query G_GNUC_UNUSED,
                               SoupClientContext *client G_GNUC_UNUSED,
                               gpointer user_data G_GNUC_UNUSED)
{
    LOG(LOGG_DEBUG, "client connected to %s", path);
    LOG(LOGG_DEBUG, "method: %s", msg->method);

    if (msg->method != SOUP_METHOD_POST)
    {
        /* not a post request */
        __respond_error(msg, SOUP_STATUS_NOT_IMPLEMENTED,
                        ERR_BAD_REQUEST, "not supported");
        return;
    }

    if (__handle_cs_msg(msg->request_body->data,
                        _configuration_service) != 0)
    {
        LOG(LOGG_ERROR, "incorrect request data: %s",
            msg->request_body->data);
        __respond_error(msg, SOUP_STATUS_BAD_REQUEST,
                        ERR_BAD_FORMAT, "incorrect configuration data");
        return;
    }
    else
    {
        LOGM(LOGG_DEBUG, "got configuration response message");
    }

    __respond_ok(msg, SOUP_STATUS_OK);
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
        goto cleanup_error;
    }

    if (setup_rvi() != 0)
    {
        LOGM(LOGG_ERROR, "RVI setup failed");
        goto cleanup_error;
    }

    return 0;

cleanup_error:
    threaded_consumer_cleanup();
    return 1;
}

/**
 * @brief setup HTTP listener for RVI service callbacks
 * @return 0 on success
 */
static int setup_rvi_listener(void)
{
    /* prep main context */
    _ctx = g_main_context_new();

    LOG(LOGG_INFO, "setting up RVI listener at: %s",
        CARSYNC_LISTEN_HOST);

    /* setup a server with custom context, context will be poked
     * directly from the code */
    _rvi_server = soup_server_new(SOUP_SERVER_PORT,
                                  LISTEN_PORT,
                                  SOUP_SERVER_SERVER_HEADER,
                                  "CarSync RVI Service",
                                  SOUP_SERVER_ASYNC_CONTEXT,
                                  _ctx,
                                  NULL);
    if (_rvi_server == NULL)
    {
        LOGM(LOGG_ERROR, "failed to create and bind RVI service handler");
        g_main_context_unref(_ctx);
        _ctx = NULL;
        return -1;
    }

    /* register URI handlers */
    soup_server_add_handler(_rvi_server, CARSYNC_CONFIGURATION_URI,
                            __configuration_cb, NULL, NULL);
    soup_server_add_handler(_rvi_server, CARSYNC_NOTIFICATION_URI,
                            __notification_cb, NULL, NULL);

    /* enable server */
    soup_server_run_async(_rvi_server);

    return 0;
}

/**
 * @brief register carsync services with local RVI node
 * @return 0 on success
 */
static int register_rvi_services(void)
{
    int ret = 0;
    int res = 0;
    /* register services with RVI */
    res = rvi_register_service(_config->rvi_node,
                               _configuration_service,
                               CARSYNC_LISTEN_HOST CARSYNC_CONFIGURATION_URI);
    if (res != 0)
    {
        LOGM(LOGG_ERROR, "failed to register configuration service");
        return -1;
    }

    if (ret == 0)
    {
        /* continue registering only if configuration got
         * registered */
        res = rvi_register_service(_config->rvi_node,
                                   _notification_service,
                                   CARSYNC_LISTEN_HOST CARSYNC_NOTIFICATION_URI);
        if (res != 0)
        {
            LOGM(LOGG_ERROR, "failed to register notification service");
            /* RVI services cannot be deregistered? */
            ret = -1;
        }
    }

    return ret;
}

/**
 * @brief setup RVI connection
 * @return 0 on success
 */
static int setup_rvi(void)
{
    LOGM(LOGG_INFO, "setting up RVI 0.2.1 transport");

    char *vin = NULL;
    if (carinfo_getVin(&vin))
    {
        LOGM(LOGG_ERROR,
             "failed to obtain VIN for RVI service names");
        return -1;
    }

    _configuration_service = format_service(vin,
                                            CARSYNC_CONFIGURATION_SERVICE);
    _notification_service = format_service(vin,
                                           CARSYNC_NOTIFICATION_SERVICE);
    /* VIN no longer needed */
    free(vin);

    if (setup_rvi_listener() != 0)
    {
        LOGM(LOGG_ERROR, "failed to setup RVI service listener");
        return -1;
    }

#define BOGUS_SLEEP 10
#define RETRY_COUNT  12
   int retry = 0;
   int register_success = 0;
   while (register_success == 0 && retry < RETRY_COUNT)
   {
       if (register_rvi_services() != 0)
       {
           LOG(LOGG_ERROR,
               "failed to register RVI services with local node, retry in %us",
               BOGUS_SLEEP);
           sleep(BOGUS_SLEEP);
           retry++;
       }
       else
       {
           LOG(LOGG_INFO, "registration complete after %u attempts",
               retry + 1);
           register_success = 1;
           break;
       }
   }

   if (register_success == 0)
   {
       LOG(LOGG_ERROR, "RVI registration failed after %u attempts",
           RETRY_COUNT);
       return -2;
   }

    LOGM(LOGG_DEBUG, "RVI setup complete");
    return 0;
}

void threaded_consumer_cleanup() {
    if (_config) {
        if (_rvi_server)
        {
            soup_server_quit(_rvi_server);
            g_object_unref(_rvi_server);
            _rvi_server = NULL;
        }
        if (_ctx)
        {
            g_main_context_unref(_ctx);
            _ctx = NULL;
        }
        g_clear_pointer(&_configuration_service, g_free);
        g_clear_pointer(&_notification_service, g_free);

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

    while (!checkBreak()) {
        _received = 0;
        gboolean ret = g_main_context_iteration(_ctx, TRUE);
        if (ret == TRUE)
        {
            /* events were dispatched, anything received by the server? */
            received = _received;
            /* queue push happenend in server callbacks */
        }
        if (received) {
            break;
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

