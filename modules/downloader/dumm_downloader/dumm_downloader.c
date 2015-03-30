/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "dumm_downloader.h"
#include "logger.h"
#include "strtools.h"
#include "filetools.h"
#include "dumm_dbus_client.h"
#include "dumm_object_marshal.h"
#include "civg_compiler.h"
#include <stdlib.h>
#include <string.h>

static DOWNLOADER_FINISHED_HANDLER _finishedHandler = 0;
static DOWNLOADER_PROGRESS_HANDLER _progressHandler = 0;

struct DUMM_downloaditem_Item {
    char* id;
    struct DUMM_downloaditem_Item* next;
    unsigned int progress;
};
typedef struct DUMM_downloaditem_Item DUMM_downloaditem_Item;

static const char* DUMM_STATE_CHANGED_SIGNAL = "dmObjectStateChange";
static const char* DUMM_PROGRESS_SIGNAL = "dmObjectProgress";
static const char* DUMM_SERVICE = "net.dumm";
static const char* DUMM_OBJECT = "/";
static const char* DUMM_INTERFACE = "net.dumm.download";
static DBusGConnection* _connection = 0;
static DBusGProxy* _busProxy = 0;
static DBusGProxy* _signalProxy = 0;
static DUMM_downloaditem_Item* _firstItem = 0;

static void addItemToList(DUMM_downloaditem_Item** first, char* id)
{
    DUMM_downloaditem_Item* item=(DUMM_downloaditem_Item*)malloc(sizeof(DUMM_downloaditem_Item));
    if (item) {
        item->next = 0;
        item->id=(char*)malloc(strlen(id)+1);

        if (item->id)
        {
            strcpy(item->id, id);
            item->next = *first;
            *first = item;
        }
        else {
           LOGM(LOGG_ERROR, "Failed to allocate memory for DUMM download item id!");
        }
    }
    else {
       LOGM(LOGG_ERROR, "Failed to allocate memory for DUMM download item!");
    }
}

static void removeItemFromList(DUMM_downloaditem_Item** first, char* id)
{
    DUMM_downloaditem_Item* item = *first;
    DUMM_downloaditem_Item* prev = 0;
    while (item) {
        if (item->id && strcmp(item->id, id) == 0) {
            break;
        }
        prev = item;
        item = item->next;
    }

    if (item) {
        free(item->id);

        if (prev) {
            prev->next=item->next;
        }
        else {
            *first=item->next;
        }

        free(item);
        item = 0;
    }
}

static void nameOwnerChangedHandler(CIVG_UNUSED DBusGProxy *proxy,
                                    const gchar *name,
                                    CIVG_UNUSED const gchar *old_owner,
                                    const gchar *new_owner,
                                    CIVG_UNUSED gpointer userData)
{
    if ((g_strcmp0(name, DUMM_SERVICE)==0) && (g_strcmp0(new_owner,"")==0)) {
        LOGM(LOGG_ERROR, "Dumm service disappeared!");

        DOWNLOADER_FINISHED_HANDLER finishedHandler = _finishedHandler;

        downloader_cleanup();

        while(_firstItem) {
            if(finishedHandler) {
                finishedHandler(_firstItem->id, 1);
            }
            removeItemFromList(&_firstItem, _firstItem->id);
        }
    }
}

static int finalizeDownload(unsigned int dummId) {
    GError* error = NULL;
    if (!net_dumm_download_finalize(_busProxy, dummId, &error)) {
        if (error)
        {
            LOG(LOGG_WARNING, "Failed to finalize download! Id: %u; ErrorCode: %d; "
                "ErrorMessage: %s", dummId, error->code, error->message);
            g_error_free(error);
        } else {
            LOG(LOGG_WARNING, "Failed to finalize download! Id: %u", dummId);
        }
        return 1;
    }
    return 0;
}

static void dummObjectChangedHandler(CIVG_UNUSED DBusGProxy *proxy,
                                     guint dummId, const gchar *state,
                                     CIVG_UNUSED gpointer userData) {
    if (_finishedHandler
        && (strcmp(state, "FINISHED") == 0 || strcmp(state, "ERROR") == 0)) {
        int status = finalizeDownload(dummId);
        char* id = 0;
        id = strtools_sprintNew("%u", dummId);
        if (id) {
            status |= (strcmp(state, "ERROR") == 0);
            _finishedHandler(id, status);
            removeItemFromList(&_firstItem, id);
            if(id)
                free(id);
        } else {
            LOG(LOGG_ERROR, "Failed to handle download StatusChanged - invalid id. Id: %u; State: %s", dummId, state);
        }
    }
}

static void dummObjectProgressHandler(CIVG_UNUSED DBusGProxy *proxy,
                                      guint dummId,
                                      const guint percentage,
                                      CIVG_UNUSED gpointer userData) {
    if (_progressHandler) {
        char* id = 0;
        id = strtools_sprintNew("%u", dummId);
        if (id) {
            _progressHandler(id, percentage);
            free(id);
        } else {
            LOG(LOGG_ERROR, "Failed update progress - invalid id. Id: %u", dummId);
        }
    }
}


extern int downloader_init() {
    if (!_connection) {
        LOGM(LOGG_ERROR, "Can not init dumm_downloader. Setup DBus connection first!");
        return 1;
    }
    if (_busProxy) {
        LOGM(LOGG_ERROR, "Doubled Downloader initialization!");
        return 1;
    }
    _busProxy = dbus_g_proxy_new_for_name(_connection, DUMM_SERVICE, DUMM_OBJECT, DUMM_INTERFACE);
    if (_busProxy) {
        dbus_g_proxy_add_signal(_busProxy, DUMM_STATE_CHANGED_SIGNAL,
                                G_TYPE_UINT,
                                G_TYPE_STRING,
                                G_TYPE_INVALID);

        dbus_g_proxy_connect_signal(_busProxy, DUMM_STATE_CHANGED_SIGNAL,
                                    G_CALLBACK(dummObjectChangedHandler), NULL, NULL);

        dbus_g_proxy_add_signal(_busProxy, DUMM_PROGRESS_SIGNAL,
                                G_TYPE_UINT,
                                G_TYPE_UINT,
                                G_TYPE_INVALID);

        dbus_g_proxy_connect_signal(_busProxy, DUMM_PROGRESS_SIGNAL,
                                    G_CALLBACK(dummObjectProgressHandler), NULL, NULL);



        _signalProxy = dbus_g_proxy_new_for_name(_connection, "org.freedesktop.DBus",
                                                 "/org/freedesktop/DBus",
                                                 "org.freedesktop.DBus");
        if (_signalProxy) {
            dbus_g_proxy_add_signal(_signalProxy, "NameOwnerChanged", G_TYPE_STRING,
                                    G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);
            dbus_g_proxy_connect_signal(_signalProxy, "NameOwnerChanged",
                                        G_CALLBACK(nameOwnerChangedHandler), NULL, NULL);

        } else {
            LOGM(LOGG_ERROR, "Failed to get proxy for org.freedesktop.DBus!");
            downloader_cleanup();
            return 1;
        }

    } else {
        LOGM(LOGG_ERROR, "Failed to get DBus proxy for DUMM!");
        return 1;
    }
    return 0;
}

extern void downloader_cleanup() {
    if (_busProxy) {
        _finishedHandler = 0;
        dbus_g_proxy_disconnect_signal(_busProxy, DUMM_STATE_CHANGED_SIGNAL,
                                       G_CALLBACK(dummObjectChangedHandler), NULL);
        dbus_g_proxy_disconnect_signal(_busProxy, DUMM_PROGRESS_SIGNAL,
                                       G_CALLBACK(dummObjectProgressHandler), NULL);

         g_object_unref(_busProxy);
         _busProxy = 0;

         dbus_g_proxy_disconnect_signal(_signalProxy, "NameOwnerChanged",
                                        G_CALLBACK(nameOwnerChangedHandler), NULL);
         g_object_unref(_signalProxy);
         _signalProxy = 0;
    }
}

extern void downloader_setFinishedHandler(DOWNLOADER_FINISHED_HANDLER handler) {
    _finishedHandler = handler;
}

extern void downloader_setProgressHandler(DOWNLOADER_PROGRESS_HANDLER handler) {
    _progressHandler = handler;
}

extern int downloader_downloadItem(const char* url, const char* path, char** id) {
    if (!_busProxy) {
        LOGM(LOGG_ERROR, "Failed to start download, DUMM downloader not initialized!");
        return 1;
    }
    char* fName = strrchr(path, '/');
    if (!fName || strlen(++fName) == 0) {
        return 1;
    }
    char* dir = filetools_getDir(path);
    if (!dir) {
        return 1;
    }
    filetools_makePath(dir);
    int status = 1;
    unsigned int dummId;
    GError* error = NULL;
    if (net_dumm_download_init(_busProxy, url, dir, fName, 0, 0, 0, "", &dummId, &error)){
        if (net_dumm_download_start(_busProxy, dummId, &error)) {
            *id = strtools_sprintNew("%u", dummId);
            if (id) {
                status = 0;
                addItemToList(&_firstItem, *id);
            }
        } else {
            if (error)
            {
                LOG(LOGG_WARNING, "Failed to start download! Url: %s; Id: %u; "
                    "ErrorCode: %d; ErrorMessage: %s", url, dummId,
                    error->code, error->message);
                g_error_free(error);
            } else {
                LOG(LOGG_WARNING, "Failed to start download! Url: %s; Id: %u", url, dummId);
            }
            finalizeDownload(dummId);
        }
    } else {
        if (error)
        {
            LOG(LOGG_WARNING, "Failed to init download! Url: %s; ErrorCode: %d; "
                "ErrorMessage: %s", url, error->code, error->message);
            g_error_free(error);
        } else {
            LOG(LOGG_WARNING, "Failed to init download! Url: %s", url);
        }
    }
    free(dir);
    return status;
}

extern int downloader_cancel(const char* id) {
    if (!_busProxy) {
        LOGM(LOGG_ERROR, "Failed to abort download, DUMM downloader not initialized!");
        return 1;
    }
    unsigned long int dummId = strtoul(id, 0, 0);
    GError* error = NULL;
    if (!net_dumm_download_abort(_busProxy, dummId, &error)) {
        if (error)
        {
            LOG(LOGG_WARNING, "Failed to abort download! Id: %s; ErrorCode: %d; "
                "ErrorMessage: %s", id, error->code, error->message);
            g_error_free(error);
        } else {
            LOG(LOGG_WARNING, "Failed to abort download! Id: %s", id);
        }
        return 1;
    }
    return finalizeDownload(dummId);
}

void dumm_downloader_setupConnection(DBusGConnection* connection)
{
    _connection = connection;
}

void dumm_downloader_registerSignalMarshallers() {
    dbus_g_object_register_marshaller(dumm_object_marshal_NONE__UINT_STRING,
                                      G_TYPE_NONE,
                                      G_TYPE_UINT,
                                      G_TYPE_STRING,
                                      G_TYPE_INVALID);
    dbus_g_object_register_marshaller(dumm_object_marshal_NONE__UINT_STRING,
                                      G_TYPE_NONE,
                                      G_TYPE_UINT,
                                      G_TYPE_UINT,
                                      G_TYPE_INVALID);
}
