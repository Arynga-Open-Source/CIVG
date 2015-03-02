/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "glib_dispatcher.h"
#include "logger.h"
#include "civg_compiler.h"
#include <glib-unix.h>

struct dispatcher_Dispatcher {
    int pipefd[2];
    GIOChannel* channel;
    GSource* source;
    DISPATCHER_EVENT_HANDLER handler;
};

static gboolean sourceClbk(CIVG_UNUSED GIOChannel *source,
                           CIVG_UNUSED GIOCondition condition,
                           gpointer data) {
    dispatcher_Dispatcher* dispatcher = (dispatcher_Dispatcher*) data;
    int eventId;
    if (read(dispatcher->pipefd[0], &eventId, sizeof(eventId)) == sizeof(eventId)) {
        dispatcher->handler(eventId);
    } else {
        LOGM(LOGG_ERROR, "Read from pipe failed!");
    }
    return TRUE;
}

dispatcher_Dispatcher *glib_dispatcher_newDispatcher(GMainContext *ctx) {
    dispatcher_Dispatcher* dispatcher = malloc(sizeof(dispatcher_Dispatcher));
    if (dispatcher) {
        dispatcher->pipefd[0] = -1;
        dispatcher->pipefd[1] = -1;
        dispatcher->channel = 0;
        dispatcher->source = 0;
        dispatcher->handler = 0;

        int status = 1;
        if (pipe2(dispatcher->pipefd, O_NONBLOCK) != 0) {
            LOGM(LOGG_ERROR, "Failed to open dispatcher pipe!");
        } else if ((dispatcher->channel = g_io_channel_unix_new(dispatcher->pipefd[0])) == 0) {
            LOGM(LOGG_ERROR, "Failed to create channel for pipe!");
        } else if ((dispatcher->source = g_io_create_watch(dispatcher->channel, (GIOCondition) (G_IO_IN | G_IO_ERR))) == 0) {
            LOGM(LOGG_ERROR, "Failed to create channel watch (source)!");
        } else {
            g_source_set_callback(dispatcher->source, (GSourceFunc)sourceClbk, dispatcher , NULL);
            if (g_source_attach(dispatcher->source, ctx) > 0) {
                status = 0;
            } else {
                LOGM(LOGG_ERROR, "Failed to attach source to given context!");
            }
        }
        if (status != 0) {
            glib_dispatcher_deleteDispatcher(dispatcher);
            dispatcher = NULL;
        }
    } else {
        LOGM(LOGG_ERROR, "Failed to allocate memory for dispatcher!");
    }
    return dispatcher;
}

void glib_dispatcher_deleteDispatcher(dispatcher_Dispatcher *dispatcher) {
    if (dispatcher) {
        if (dispatcher->source) {
            g_source_destroy(dispatcher->source);
        }
        if (dispatcher->channel) {
            g_io_channel_unref(dispatcher->channel);
        }
        if (dispatcher->pipefd[0] != -1){
            close(dispatcher->pipefd[0]);
        }
        if (dispatcher->pipefd[1] != -1) {
            close(dispatcher->pipefd[1]);
        }
        free(dispatcher);
    }
}

extern void dispatcher_setEventHandler(dispatcher_Dispatcher* dispatcher, DISPATCHER_EVENT_HANDLER handler) {
    dispatcher->handler = handler;
}

extern void dispatcher_postEvent(dispatcher_Dispatcher* dispatcher, int eventId) {
    if (write(dispatcher->pipefd[1], &eventId, sizeof(eventId)) != sizeof(eventId)) {
        LOG(LOGG_ERROR, "Write to pipe failed! Pipe: %d, EventId: %d", dispatcher->pipefd[1], eventId);
    }
}
