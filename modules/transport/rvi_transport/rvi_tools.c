/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "RVI_TOOLS"
#define LOGGER_FILE "rvi_tools.log"

#ifndef RVI_TOOLS_H
#define RVI_TOOLS_H

#include "rvi_config.h"
#include "logger.h"
#include <libsoup/soup.h>
#include <cJSON.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static const char *_register_text = "{ \"jsonrpc\": \"2.0\","
    "\"method\": \"register_service\", "
    "\"id\": \"1\", "
    "\"params\": { "
    " \"service\": \"%s\", "
    " \"network_address\": \"%s\" "
    " } "
    " } ";

static const char *_message_text = "{ \"jsonrpc\": \"2.0\","
    "\"method\": \"message\", "
    "\"id\": \"1\", "
    "\"params\": {"
    "  \"service_name\": \"%s\", "
    "  \"parameters\": [ "
    "  { \"b64message\": \"%s\" }"
    "  ],"
    " \"calling_service\": \"carsync\", "
    " \"timeout\": %llu"
    " } "
    " } ";

static const char *_ok_response_body = "{ \"jsonrpc\": \"2.0\","
    "\"result\": { \"status\": 0 }, "
    "\"id\": \"1\""
    " } ";

static const char *_error_response_body = "{ \"jsonrpc\": \"2.0\","
    "\"id\": \"1\","
    "\"error\": {"
    " \"code\": %d, "
    " \"message\": \"%s\" "
    " } "
    " }";

static int rvi_send_message(const char *node, void *data,
                            size_t data_size)
{
    int res = 0;
    SoupMessage *msg;
    SoupSession *session;

    /* LOG(LOGG_DEBUG, "register with RVI node"); */

#ifdef SOUP_CHECK_VERSION
    #if SOUP_CHECK_VERSION(2, 42, 0)
        session = soup_session_new();
    #else
        session = soup_session_sync_new();
    #endif
#else
    session = soup_session_sync_new();
#endif
    /* doing RVI JSON-RPC call, always POST */
    msg = soup_message_new(SOUP_METHOD_POST, node);
    /* disable redirects, not expecting them anyway */
    soup_message_set_flags(msg, SOUP_MESSAGE_NO_REDIRECT);

    soup_message_set_request(msg, "application/javascript",
                             SOUP_MEMORY_STATIC, /* don't free' */
                             data,
                             data_size);
    /* LOG(LOGG_DEBUG, "send request"); */
    soup_session_send_message(session, msg);

    if (msg->status_code != SOUP_STATUS_OK)
    {
        res = -1;
    }
    LOG(LOGG_DEBUG, "status code: %d", msg->status_code);
    if (SOUP_STATUS_IS_TRANSPORT_ERROR(msg->status_code))
    {
        LOG(LOGG_ERROR, "request to %s failed, reason: %s",
            node, msg->reason_phrase);
    }

    if (msg->response_body->data)
    {
        LOG(LOGG_DEBUG, "body: %s", msg->response_body->data);
    }

    g_object_unref(msg);
    g_object_unref(session);

    return res;
}

int rvi_register_service(const char *node, const char *path,
                         const char *target)
{
    LOG(LOGG_INFO, "registering service %s", path);
    LOG(LOGG_INFO, "     node: %s", node);
    LOG(LOGG_INFO, "   target: %s", target);

    gchar *register_request = g_strdup_printf(_register_text, path,
                                              target);

    int ret = rvi_send_message(node, register_request,
                               strlen(register_request));

    g_free(register_request);

    return ret;
}

int rvi_send_cs_message(const char *node , const char *target,
                               void *msg, size_t msg_size)
{
    /* first base64 CS message */
    gchar *enc = g_base64_encode(msg, msg_size);

    guint64 now = g_get_real_time() / 1000000;
    /* timeout is now in UTC + 10 minutes */
    guint64 timeout = now + 10 * 60;
    gchar *req = g_strdup_printf(_message_text, target, enc,
                                 timeout);

    LOG(LOGG_DEBUG, "request: %s", req);

    int ret = rvi_send_message(node, req, strlen(req));

    LOG(LOGG_DEBUG, "sent: %d", ret);

    g_free(req);
    g_free(enc);

    return ret;
}

void *rvi_parse_cs_message(const char *target, const char *body,
                           size_t *msg_size)
{
    void *msg = NULL;
    cJSON *root = cJSON_Parse(body);

    if (root == NULL)
    {
        LOGM(LOGG_ERROR, "JSON error: failed to parse JSON");
        return NULL;
    }

    cJSON *method_params = cJSON_GetObjectItem(root, "params");
    if ((method_params == NULL) || (method_params->type != cJSON_Object))
    {
        LOGM(LOGG_ERROR, "JSON error: method params not found or not an object");
        goto cleanup;
    }

    cJSON *jtarget = cJSON_GetObjectItem(method_params, "service_name");
    cJSON *jparams = cJSON_GetObjectItem(method_params, "parameters");

    if ((jtarget == NULL) || (jparams == NULL))
    {
        LOGM(LOGG_ERROR, "JSON error: failed to find target or parameters in request");
        goto cleanup;
    }

    /* "target": "/carsync/something" */
    if (jtarget->type != cJSON_String)
    {
        LOGM(LOGG_ERROR, "JSON error: target is not a string");
        goto cleanup;
    }

    /* target must match */
    if (g_strcmp0(jtarget->valuestring, target) != 0)
    {
        LOG(LOGG_ERROR, "JSON error: target mismatch, got %s, expected %s",
            jtarget->valuestring, target);
        goto cleanup;
    }

    /* "parameters": [
     *     { "b64message": "base64-encoded-CS-message" }
     *  ]
     */
    if ((jparams->type != cJSON_Array) || (cJSON_GetArraySize(jparams) != 1))
    {
        LOGM(LOGG_ERROR, "JSON error: parameters not array or of incorrect size");
        goto cleanup;
    }

    cJSON *b64msg_obj = cJSON_GetArrayItem(jparams, 0);
    cJSON *b64msg = cJSON_GetObjectItem(b64msg_obj, "b64message");
    if ((b64msg == NULL) || (b64msg->type != cJSON_String))
    {
        LOGM(LOGG_ERROR, "JSON error: b64message not found or not string");
        goto cleanup;
    }

    if (strlen(b64msg->valuestring) == 0)
    {
        LOGM(LOGG_ERROR, "JSON error: b64message is an empty string");
        goto cleanup;
    }

    size_t cs_msg_len = 0;
    guchar *cs_msg = g_base64_decode(b64msg->valuestring, &cs_msg_len);

    /* if we have a valid message, repack to chunk allocated with C
     * library allocator */
    if (cs_msg && cs_msg_len)
    {
        msg = malloc(cs_msg_len);
        memcpy(msg, cs_msg, cs_msg_len);
        /* and release glib allocated mem */
        g_free(cs_msg);

        if (msg_size)
            *msg_size = cs_msg_len;
    }

cleanup:
    /* what about other nodes? */
    cJSON_Delete(root);

    return msg;
}

char *rvi_format_error(int error_code, const char *msg)
{
    /* allocator trampoline */
    gchar *gmsg = g_strdup_printf(_error_response_body,
                                  error_code, msg);
    char *ret = strdup(gmsg);
    g_free(gmsg);

    return ret;
}

char *rvi_format_ok(void)
{
    /* allocator trampoline */
    char *ret = strdup(_ok_response_body);
    return ret;
}

#endif
