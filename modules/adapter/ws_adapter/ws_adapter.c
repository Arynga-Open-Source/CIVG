/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#define LOGGER_MODULE "CIVG_WS_ADAPTER"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "ws_adapter.h"
#include "carinfo.h"
#include "rpmanager.h"
#include "core.h"
#include "strtools.h"
#include "cJSON.h"
#include "libwebsockets.h"
#include <sys/time.h>
#include <pthread.h>
#include "core.h"

static pthread_t _wsThread = 0;
struct libwebsocket_context *_civg_ws_context;
static int _civg_ws_continue;
static pthread_mutex_t _civg_ws_continueLock;
static pthread_mutex_t _civg_ws_percentageLock;
static int _ws_updatePercentage = 0;

static int civg_ws_checkContinue() {
    int value;
    pthread_mutex_lock(&_civg_ws_continueLock);
    value = _civg_ws_continue;
    pthread_mutex_unlock(&_civg_ws_continueLock);
    return value;
}

enum tCarSyncState {
    Broken, // CarSync cannot operate properly
    Idle, // CarSync is ready for operation
    Download, // CarSync downloads an update
    Update // CarSync performs an update
};


// Contains exit status of last operation performed by CarSync.
// Combined together with tCarSyncState gives overview of current operation.

enum tCarSyncStatus {
    Critical, // Critical error, proper operation can not be ensured
    Ok, // Operation finished properly
    Error, // Error occurred
};

static int callback_http(struct libwebsocket_context *context,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    return 0;
}

void ws_downloadProgressHandler(unsigned int progress) {
    pthread_mutex_lock(&_civg_ws_percentageLock);
    _ws_updatePercentage = progress/2;
    pthread_mutex_unlock(&_civg_ws_percentageLock);
}

const char* handle_civg_request (char* in)
{
    core_State state;
    core_Status status;
    char *reply = NULL;

    cJSON *json_root = cJSON_Parse(in);
    if (NULL == json_root) {
        return NULL;
    }
    LOG(LOGG_DEBUG, "HMI request content: %s", in);


    cJSON *method = cJSON_GetObjectItem(json_root,"method");
    cJSON *id = cJSON_GetObjectItem(json_root,"id");
    if ( method && id ) {
        LOG(LOGG_DEBUG, "WS method %s called", cJSON_PrintUnformatted(method));
    } else {
        cJSON *json_error = cJSON_Parse(
            "{\"jsonrpc\": \"2.0\", \"error\": "
            "{\"code\": -32700, \"message\": \"Parse error.\"},"
            "\"id\"=null"
            "}");
        reply = strdup(cJSON_PrintUnformatted(json_error));
        if(json_error){
            cJSON_Delete(json_error);
        }
        return reply;
    }

    cJSON *packages = cJSON_CreateArray();
    cJSON *json_reply = cJSON_Parse("{\"jsonrpc\": \"2.0\"}");

    cJSON_AddStringToObject(json_reply,"id",cJSON_PrintUnformatted(id));
    struct timeval now;
    gettimeofday(&now,NULL);
    if ( 0 == strcmp(method->valuestring,"reset")) {
        LOGM(LOGG_DEBUG, "reset called");
        system("nohup /var/lib/civg/.demo/demoreset.sh &");
        cJSON_AddItemToObject(json_reply,
                              "result",
                              cJSON_CreateString("0"));
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"StartUpdate")) {
        /* FIXME Commented out for demo purpose
         * along with permanent autoUpgrade uption
        status = core_performUpgrade();
        pthread_mutex_lock(&_civg_ws_percentageLock);
        _ws_updatePercentage=51;
        pthread_mutex_unlock(&_civg_ws_percentageLock);
        if (CORE_STATUS_OK==status){
            cJSON_AddItemToObject(json_reply,"result",cJSON_CreateString("0"));
        } else {
            cJSON_AddItemToObject(json_reply,"result",cJSON_CreateString("1"));
        }
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"StartDownload")) {
        * end of code commented out for demo purpose
        */
        pthread_mutex_lock(&_civg_ws_percentageLock);
        _ws_updatePercentage=1;
        pthread_mutex_unlock(&_civg_ws_percentageLock);
        status = core_performDownload();
        /* FIXME workaround for sleep in core */
        sleep(1);
        if (CORE_STATUS_OK==status){
            cJSON_AddItemToObject(json_reply,"result",cJSON_CreateString("0"));
        } else {
            cJSON_AddItemToObject(json_reply,"result",cJSON_CreateString("1"));
        }
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"CheckUpdates")) {
        core_checkUpdates();
        cJSON_AddItemToObject(json_reply,"result",cJSON_CreateString("0"));
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"GetCarSyncState")) {
        core_getStatus(&state, &status);
        cJSON *state_result_object = cJSON_CreateObject();
        switch (state) {
            case CORE_STATE_IDLE:
                cJSON_AddItemToObject(state_result_object,
                                      "state",
                                      cJSON_CreateString("Idle"));
                pthread_mutex_lock(&_civg_ws_percentageLock);
                if(_ws_updatePercentage>50) {
                    _ws_updatePercentage=100;
                }
                pthread_mutex_unlock(&_civg_ws_percentageLock);

                break;
            case CORE_STATE_DOWNLOAD:
                cJSON_AddItemToObject(state_result_object,
                                      "state",
                                      cJSON_CreateString("Update"));
                        // Download is not treated as part of update, so
                        // we need to reply with Update state.
                        //                cJSON_CreateString("Download"));



                break;
            case CORE_STATE_BROKEN:
                cJSON_AddItemToObject(state_result_object,
                                      "state",
                                      cJSON_CreateString("Broken"));
                break;
            case CORE_STATE_UPGRADE:
                cJSON_AddItemToObject(state_result_object,
                                      "state",
                                      cJSON_CreateString("Update"));
                pthread_mutex_lock(&_civg_ws_percentageLock);
                // As we don't track real progress of update script, it
                // increments on every request. To prevent it from reaching
                // 100 or more, we stop on 93, usually visibly less than 100%
                // 50% is expected when download ends.
                if(_ws_updatePercentage<93 && _ws_updatePercentage>=50) {
                    _ws_updatePercentage+=1;
                }
                pthread_mutex_unlock(&_civg_ws_percentageLock);
                break;
        }

        pthread_mutex_lock(&_civg_ws_percentageLock);
        cJSON_AddItemToObject(state_result_object,
                              "progress",
                              cJSON_CreateNumber(_ws_updatePercentage) );
        pthread_mutex_unlock(&_civg_ws_percentageLock);

        cJSON_AddItemToObject(json_reply,
                              "result",
                              state_result_object);
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"GetCarSyncStatus")) {
        core_getStatus(&state, &status);
        switch (status) {
            case CORE_STATUS_OK:
                cJSON_AddItemToObject(json_reply,
                                      "result",
                                      cJSON_CreateString("Ok"));
                break;
            case CORE_STATUS_CRITICAL:
                cJSON_AddItemToObject(json_reply,
                                      "result",
                                      cJSON_CreateString("Critical"));
                break;
            case CORE_STATUS_ERROR:
                cJSON_AddItemToObject(json_reply,
                                      "result",
                                      cJSON_CreateString("Error"));
                break;
        }
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"GetRPList")) {
        proto_InstalledConfig *_ws_rp_list;
        if(RPMANAGER_STATUS_OK == rpmanager_getInstalledConfig(&_ws_rp_list)){
            CarSync__Proto__ReleasePackage__ReleasePackageMeta* rp;
            for (int i=0; i<_ws_rp_list->n_rps;i++){
                rp=*(_ws_rp_list->rps +
                     (i*sizeof(CarSync__Proto__ReleasePackage__ReleasePackageMeta)));
                cJSON *received_package = cJSON_CreateObject();
                cJSON_AddItemToObject(received_package,
                                      "uuid",
                                      cJSON_CreateString(rp->uuid));
                if(1==rp->version->major_version) {
                    cJSON_AddItemToObject(received_package,
                                          "description",
                                          cJSON_CreateString("Audio configuration version 1.0.0"));
                } else if (2==rp->version->major_version) {
                    cJSON_AddItemToObject(received_package,
                                          "description",
                                          cJSON_CreateString("Enhanced audio configuration version 2.0.0"));
                }
                cJSON *json_package_version = cJSON_CreateObject();
                cJSON_AddItemToObject(json_package_version,
                                      "major_version",
                                      cJSON_CreateNumber(rp->version->major_version));
                cJSON_AddItemToObject(json_package_version,
                                      "minor_version",
                                      cJSON_CreateNumber(rp->version->minor_version));
                cJSON_AddItemToObject(json_package_version,
                                      "build_version",
                                      cJSON_CreateNumber(rp->version->build_version));
                cJSON_AddItemToObject(received_package,
                                      "version",
                                      json_package_version);
                cJSON_AddItemToArray(packages, received_package);
            }
        }
        else
        {
            cJSON *json_error= cJSON_CreateObject();
            cJSON_AddItemToObject(json_error,
                                  "message",
                                  cJSON_CreateString("Can't get available RP List."));
            cJSON_AddItemToObject(json_error,
                                  "code",
                                  cJSON_CreateString("1000"));
            cJSON_AddItemToObject(json_reply,
                                  "error",
                                  json_error);
            LOGM(LOGG_ERROR, "Can't get RP List.");
        }
        LOGM(LOGG_DEBUG, "GetRPList called");
        cJSON_AddItemToObject(json_reply, "result",packages);
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else if ( 0 == strcmp(method->valuestring,"GetPendingUpdates")) {
        char *_ws_uuid = NULL;
        char *_ws_version = NULL;
        if (!core_getAvailableUpdates(&_ws_uuid, &_ws_version))
        {
            cJSON *received_package = cJSON_CreateObject();

            char *_ws_v_sep1=_ws_version;
            char separator = ',';
            while (_ws_v_sep1){
                if(*_ws_v_sep1==separator){
                    break;
                }
                _ws_v_sep1++;
            }
            char *_ws_v_sep2=_ws_v_sep1+1;
            while (_ws_v_sep2){
                if(*_ws_v_sep2==separator){
                    break;
                }
                _ws_v_sep2++;
            }
            char *maj_ver=strndup(_ws_version,_ws_v_sep1-_ws_version);
            char *min_ver=strndup(_ws_v_sep1+1,_ws_v_sep2-_ws_v_sep1-1);
            char *bui_ver=strdup(_ws_v_sep2+1);

            cJSON *json_package_version = cJSON_CreateObject();
            cJSON_AddItemToObject(json_package_version,
                                  "major_version",
                                  cJSON_CreateString(maj_ver));
            cJSON_AddItemToObject(json_package_version,
                                  "minor_version",
                                  cJSON_CreateString(min_ver));
            cJSON_AddItemToObject(json_package_version,
                                  "build_version",
                                  cJSON_CreateString(bui_ver));
            cJSON_AddItemToObject(received_package,
                                  "uuid",
                                  cJSON_CreateString(_ws_uuid));

            if(0==strncmp(maj_ver, "1",1)) {
                cJSON_AddItemToObject(received_package,
                                      "description",
                                      cJSON_CreateString("Audio configuration version 1.0.0"));
            } else if (0==strncmp(maj_ver, "2",1)) {
                cJSON_AddItemToObject(received_package,
                                      "description",
                                      cJSON_CreateString("Enhanced audio configuration version 2.0.0"));
            }

            cJSON_AddItemToObject(received_package,
                                  "version",
                                  json_package_version);
            cJSON_AddItemToArray(packages,
                                 received_package);

        }
        cJSON_AddItemToObject(json_reply,
                              "result",
                              packages);

        if (_ws_uuid)
            free (_ws_uuid);
        if (_ws_version)
            free (_ws_version);
        LOGM(LOGG_DEBUG, "GetPendingUpdates called");
        reply = strdup(cJSON_PrintUnformatted(json_reply));
    } else {
        LOGM(LOGG_DEBUG, "Error reply");
        cJSON *json_error = cJSON_Parse(
            "{\"jsonrpc\": \"2.0\", \"error\": "
            "{\"code\": -32601, \"message\": \"Procedure not found.\"}"
            "}");
        cJSON_AddItemToObject(json_error, "id", id);
        reply = strdup(cJSON_PrintUnformatted(json_error));
    }


    if (json_reply)
        cJSON_Delete(json_reply);
    if(json_root)
        cJSON_Delete(json_root);
    LOG(LOGG_DEBUG, "HMI reply content: %s", reply);

    return reply;
}

static int callback_dumb_increment(struct libwebsocket_context *context,
                                   struct libwebsocket *wsi,
                                   enum libwebsocket_callback_reasons reason,
                                   void *user, void *in, size_t len)
{
    LOG(LOGG_DEBUG, "DI callback called: reason id = %d\n", reason);
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
            LOGM(LOGG_INFO, "HMI connection established");
            break;
        case LWS_CALLBACK_RECEIVE: {
            LOGM(LOGG_DEBUG, "HMI request received");
            char *reply;

            reply=(char*)handle_civg_request(in);//, &reply);
            if( 0 == reply ) {
                return 0;
            }

            /* start off with 1k */
            const size_t BUF_SIZE = 1024;
            char *padded_buf = (char*) malloc(BUF_SIZE);
            /* trimming available size to account for libwebsocket pre
             * & post padding */
            size_t avail_len = BUF_SIZE;
            /* pre padding */
            avail_len -= LWS_SEND_BUFFER_PRE_PADDING;
            /* post padding */
            avail_len -= LWS_SEND_BUFFER_POST_PADDING;
            /* make headroom */
            char *buf = padded_buf + LWS_SEND_BUFFER_PRE_PADDING;
            size_t filled = snprintf(buf, avail_len,
                                     /* "Content-Type: application/json\r\n" */
                                     /* "Content-Length: %u\r\n" */
                                     /* "\r\n\r\n" */
                                     "%s",
                                     /* (unsigned int)reply_len, */
                                     reply);

            libwebsocket_write(wsi, (unsigned char *)buf, filled,
                               LWS_WRITE_TEXT);
            free(padded_buf);
            free(reply);
            break;
        }
        default:
            break;
    }

    return 0;
}


static struct libwebsocket_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",   // name
        callback_http, // callback
        0              // per_session_data_size
    },
    {
        "dumb-increment-protocol", // protocol name - very important!
        callback_dumb_increment,   // callback
        0                          // we don't use any per session data

    },
    {
        NULL, NULL, 0   /* End of list */
    }
};


static void* adapter_run(void* tArgs)
{
    // server url will be http://localhost:9000
    int port = 9000;
    const char *interface = NULL;

    // we don't use ssl
    const char *cert_path = NULL;
    const char *key_path = NULL;
    // no special options
    int opts = 0;

    // create libwebsocket context representing this server
    _civg_ws_context = libwebsocket_create_context(port, interface, protocols,
                                          libwebsocket_internal_extensions,
                                          cert_path, key_path, -1, -1, opts, NULL);

    if (_civg_ws_context == NULL) {
        LOGM(LOGG_ERROR, "libwebsocket init failed");
        return -1;
    }

    LOGM(LOGG_INFO, "starting server...");

    while (1==civg_ws_checkContinue()) {
        libwebsocket_service(_civg_ws_context, 50);
        // libwebsocket_service will process all waiting events with their
        // callback functions and then wait 50 ms.
        // (this is a single threaded webserver and this will keep our server
        // from generating load while there are not requests to process)
    }


    pthread_exit(tArgs);
}

extern int adapter_init()
{
    if (_wsThread) {
        LOGM(LOGG_ERROR, "Doubled adapter initialization!");
        return 1;
    }
    downloadmgr_setProgressHandler(ws_downloadProgressHandler);
    pthread_attr_t attr;
    int status = 1;
    if (pthread_attr_init(&attr) == 0) {
        if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0) {
            if (pthread_mutex_init(&_civg_ws_continueLock, NULL) == 0) {
                if(pthread_mutex_init(&_civg_ws_percentageLock, NULL) == 0 ) {
                    _civg_ws_continue = 1;
                    if (pthread_create(&_wsThread, &attr, adapter_run, NULL) != 0) {
                        LOGM(LOGG_ERROR, "Failed to create producer thread!");
                    } else {
                        status = 0;
                    }
                    if (status != 0) {
                        adapter_cleanup();
                    }
                }
                else {
                    LOGM(LOGG_ERROR, "Failed to initialize mutex!");
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



void adapter_cleanup(){
    pthread_mutex_lock(&_civg_ws_continueLock);
    _civg_ws_continue = 0;
    pthread_mutex_unlock(&_civg_ws_continueLock);
    if (_wsThread) {
        pthread_join(_wsThread, NULL);
        _wsThread = 0;
    }
    pthread_mutex_destroy(&_civg_ws_continueLock);
    libwebsocket_context_destroy(_civg_ws_context);
}
