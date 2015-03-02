/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

#include "core.h"
#include "dumm_downloader.h"
#include "glib_dispatcher.h"
#include "threaded_transport.h"
#include "adapter.h"
#include "logger.h"
#include "build_version.h"
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

// Options
static int autoUpgrade = 0;
static int autoDownload = 0;
static int sessionBus = 0;
static int printVersion = 0;
static int debug = 0;

static gboolean parseOpts(int argc, char** argv)
{
    GOptionContext *optctx = NULL;
    GOptionEntry opts[] = {
        {"auto-upgrade", 'a',0, G_OPTION_ARG_NONE, &autoUpgrade, "Performs upgrades automatically - does not wait for performUpgrade call.", 0},
        {"auto-download", 'A',0, G_OPTION_ARG_NONE, &autoDownload, "Performs downloads automatically - does not wait for performDownload call.", 0},
        {"session-bus", 's',0, G_OPTION_ARG_NONE, &sessionBus, "Run dbus on session bus.", 0},
        {"debug", 'd',0, G_OPTION_ARG_NONE, &debug, "Enable debugging output.", 0},
        {"version", 'v',0, G_OPTION_ARG_NONE, &printVersion, "Print version and exit.", 0},
        {0, 0, 0, 0, 0, 0, 0}
    };
    optctx = g_option_context_new("- CarSync In Vehicle Gateway service");
    g_option_context_add_main_entries(optctx, opts, NULL);
    gboolean status = g_option_context_parse(optctx, &argc, &argv, NULL);
    g_option_context_free(optctx);
    return status;
}

int main(int argc, char *argv[])
{
#if GLIB_MINOR_VERSION < 36
    g_type_init();
#endif
    if (!parseOpts(argc, argv)) {
        printf("Cannot parse command line arguments! Use -h, --help to see available options.\n");
        return 1;
    }

    if (printVersion) {
        printf("CarSync In Vehicle Gateway service 2.0.%s.\n", BUILD_VERSION);
        return 0;
    }

    if (debug)
        logger_setLevel(LOGG_DEBUG);

    int status = 1;
    GMainLoop* main_loop = g_main_loop_new (NULL, FALSE);
    if (main_loop) {
        DBusGConnection *connection;
        GError *error = NULL;
        connection = dbus_g_bus_get(sessionBus ? DBUS_BUS_SESSION : DBUS_BUS_SYSTEM, &error);
        if (connection != NULL)
        {
            dispatcher_Dispatcher* dispatcher = glib_dispatcher_newDispatcher(g_main_context_default());
            if (dispatcher) {
                dumm_downloader_registerSignalMarshallers();
                dumm_downloader_setupConnection(connection);
                threaded_transport_setDispatcher(dispatcher);
                if (adapter_init() == 0) {
                    core_Status cStatus = core_init(autoUpgrade, autoDownload);
                    if (cStatus == CORE_STATUS_OK) {
                        g_main_loop_run(main_loop);
                        g_main_loop_unref(main_loop);
                        status = 0;
                        core_cleanup();
                    } else {
                        LOG(LOGG_ERROR, "Failed to start civg service! Status: %d", cStatus);
                    }
                    adapter_cleanup();
                } else {
                    LOGM(LOGG_ERROR, "Failed to initialize WebSocket Adapter!");
                }
                glib_dispatcher_deleteDispatcher(dispatcher);
            } else {
                LOGM(LOGG_ERROR, "Failed to create dispatcher!");
            }
        } else {
            LOG(LOGG_ERROR, "Failed to open connection to bus! Message: %s", error->message);
            g_error_free (error);
        }
    } else {
        LOGM(LOGG_ERROR, "Failed to create main loop!");
    }

    return status;
}
