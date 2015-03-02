/**
 * Copyright (C) <2013>, GENIVI Alliance, Inc.
 * Author: bj@open-rnd.pl
 *
 * This file is part of <GENIVI Download Upload Messaging Manager>.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contributor License Agreements.29
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License (MPL), v. 2.0.
 * If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 *
 * List of changes:
 * <01.04.2013>, <Bartlomiej Jozwiak>, <Initial version>
 */


/**
 * @file client-main.c
 * @date 03-06-2013
 * @brief Command line client
 *
 * __DETAILS__
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

#include <dumm-download-c-stub.h>

#include "client-dumm-bus.h"

typedef struct command_s command_t;
typedef struct dl_info_s dl_info_t;

static int setup_conn(void);
static int show_help(const char *cmd, int argc, const char *argv[]);
static int dl_get(const char *cmd, int argc, const char *argv[]);
static int dl_resume(DBusGProxy *prox, int id);
static int dl_abort(DBusGProxy *prox, int id);
static int dl_pause(DBusGProxy *prox, int id);
static int dl_single_cmd(const char *cmd, int argc, const char *argv[]);
static int dl_status(const char *cmd, int argc, const char *argv[]);
static int dl_monitor(const char *cmd, int argc, const char *argv[]);
static void free_dlinfo(struct dl_info_s *dli);
static int fill_dlinfo_for_id(DBusGProxy *proxy, int id, struct dl_info_s *dli);
static void __fill_dlinfo(gpointer key, gpointer value, gpointer data);
static int update_dlinfo_for_id(DBusGProxy *prox, int id, struct dl_info_s *dli);
static void print_dli_header(struct dl_info_s *dli, int size_info);
static void print_progress(struct dl_info_s *dli, int newline);
static int show_status_for_id(int id);

/**
 * @brief wrapper for data of a single download
 */
struct dl_info_s {
    gchar *url;                 /* url */
    gchar *path;                /* save path */
    gchar *filename;            /* save file name */
    gchar *full_path;           /* full path to file */
    int total_size;             /* total size */
    int size;                   /* downloaded size */
    int estimate_time;          /* estimated time of completion */
    gchar *state;               /* state */
};
#define DL_INFO_INIT {NULL, NULL, NULL, NULL, 0, 0, 0, NULL}
/**
 * @brief wrappef for command
 */
struct command_s
{
    const char *cmd;            /* textual command */
    int (*clbk) (const char *cmd, int argc, const char *argv[]); /* command callback */
} __commands[] = {
    {
        .cmd = "help",
        .clbk = show_help
    },
    {
        .cmd = "get",
        .clbk = dl_get
    },
    {
        .cmd = "pause",
        .clbk = dl_single_cmd
    },
    {
        .cmd = "resume",
        .clbk = dl_single_cmd
    },
    {
        .cmd = "abort",
        .clbk = dl_single_cmd
    },
    {
        .cmd = "status",
        .clbk = dl_status
    },
    {
        .cmd = "monitor",
        .clbk = dl_monitor
    },
    /* always last entry */
    {
        .cmd = NULL,
        .clbk = NULL
    }
};
/* static data */
DBusGConnection *_dbus_conn = NULL;

/**
 * @brief parse comamnd line arguments
 */
void
parse_args(int argc, const char *argv[])
{

}

/**
 * @brief main
 */
int
main(int argc, const char *argv[])
{
    dbus_g_thread_init();
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif

    parse_args(argc, argv);
    if (argc < 2)
    {
        show_help("help", argc - 1, argv + 1);
        exit(0);
    }

    g_debug("find matching command for %s", argv[1]);
    for (const command_t *cmd = __commands; cmd->cmd != NULL; cmd++)
    {
        if (g_strcmp0(argv[1], cmd->cmd) == 0)
            return cmd->clbk(cmd->cmd,
                             argc - 2,
                             argv + 2);
    }

    g_message("unknown command %s, see help", argv[1]);

    if (_dbus_conn)
        dbus_g_connection_unref(_dbus_conn);
    return 0;
}

/**
 * @brief show help info
 */
static int
show_help(const char *cmd, int argc, const char *argv[])
{
    g_message("help message\n");

    printf("usage: %s <command>\n"
           "\n"
           "available commands:\n"
           "	get [-w] <url> [<storage path>]\n"
           "	status <id>\n"
           "	pause <id>\n"
           "	abort <id>\n"
           "	monitor\n", argv[0]);
    return 0;
}

/**
 * @brief request file download
 */
static int
dl_get(const char *cmd, int argc, const char *argv[])
{
    DBusGProxy *prox;
    gboolean ret;
    const char *url;
    const char *storage_path = NULL;
    guint id;
    GError *err = NULL;
    int watch = 0;
    int optind = 0;

    if (argc == 0)
    {
        g_message("usage: <url>");
        return 0;
    }

    if (g_strcmp0(argv[optind], "-w") == 0)
    {
        watch = 1;
        optind++;
    }

    url = argv[optind];
    g_debug("url: %s", url);
    g_debug("watch: %d", watch);

    optind++;

    if (optind < argc)
        storage_path = argv[optind];

    if (setup_conn())
    {
        g_warning("cannot connect to D-Bus");
        return -1;
    }

    prox = get_dumm_proxy(_dbus_conn);
    if (!prox)
    {
        g_warning("failed to obtain proxy\n");
        return -1;
    }
    g_debug("got proxy");
    ret = net_dumm_download_init(prox,
                                 url,
                                 storage_path, /* storage path */
                                 NULL, /* filename */
                                 0,    /* lifetime */
                                 FALSE, /* keep */
                                 TRUE,  /* autoresume */
                                 NULL,  /* visibility */
                                 &id,
                                 &err); /* error */
    if (ret == FALSE)
    {
        g_warning("failed to add download: %s", err->message);
        g_error_free(err);
    }
    else
    {
        g_message("download added, id: %u\n", id);
    }

    err = NULL;
    ret = net_dumm_download_start(prox, id, &err);
    if (ret == FALSE)
    {
        g_warning("failed to start download: %s", err->message);
    }
    else
    {
        if (watch)
            show_status_for_id(id);
    }
    g_object_unref(prox);

    return 0;
}

/**
 * @brief pause download or downloads
 */
static gboolean
dl_pause(DBusGProxy *prox, int id)
{
    gboolean ret = FALSE;
    GError *err = NULL;
    ret = net_dumm_download_pause(prox,
                                  id,
                                  &err); /* error */
    if (ret == FALSE)
    {
        g_warning("failed to pause download: %s", err->message);
        g_error_free(err);
    }
    else
    {
        g_message("paused");
    }
    return ret;
}

/**
 * @brief release resources dli holds
 */
static void
free_dlinfo(struct dl_info_s *dli)
{
    g_assert(dli != NULL);

    g_free(dli->url);
    g_free(dli->state);
    g_free(dli->path);
    g_free(dli->filename);
    g_free(dli->full_path);
    memset(dli, 0, sizeof(*dli));
}

/**
 * @brief given proxy fill download info by doing dbus call
 */
static int
fill_dlinfo_for_id(DBusGProxy *prox, int id, struct dl_info_s *dli)
{
    GHashTable *hash = NULL;

    g_assert(prox != NULL && dli != NULL);

    g_debug("get all properties");
    if (get_dumm_item_prop_all(prox, &hash))
        return -1;

    /* fetch all relevant data to download info first */
    g_hash_table_foreach(hash, __fill_dlinfo, dli);

    dli->full_path = g_strdup_printf("%s%s%s", dli->path,
                                     g_str_has_suffix(dli->path, "/") ? "" : "/",
                                     dli->filename);
    g_hash_table_unref(hash);

    return 0;
}

/**
 * @brief update download size and estimtes for dl info
 */
static int
update_dlinfo_for_id(DBusGProxy *prox, int id, struct dl_info_s *dli)
{
    GValue size_val = G_VALUE_INIT;
    GValue total_size_val = G_VALUE_INIT;
    GValue est_val = G_VALUE_INIT;
    GValue state_val = G_VALUE_INIT;

    g_assert(prox != NULL && dli != NULL);
    get_dumm_item_prop(prox, "Size", &size_val);
    get_dumm_item_prop(prox, "TotalSize", &total_size_val);
    get_dumm_item_prop(prox, "EstimateTime", &est_val);
    get_dumm_item_prop(prox, "DownloadState", &state_val);

    dli->size = g_value_get_int(&size_val);
    dli->total_size = g_value_get_int(&total_size_val);
    dli->estimate_time = g_value_get_int(&est_val);
    if (dli->state)
        g_free(dli->state);
    dli->state = g_value_dup_string(&state_val);
}

static void
__fill_dlinfo(gpointer key, gpointer value, gpointer data)
{
    GValue *val = (GValue *)value;
    gchar *keys = (gchar *)key;
    struct dl_info_s *dli = (struct dl_info_s *)data;

    g_debug("key: %s", keys);

    if (g_strcmp0(keys, "Url") == 0)
        dli->url = g_value_dup_string(val);
    else if (g_strcmp0(keys, "DownloadState") == 0)
        dli->state = g_value_dup_string(val);
    else if (g_strcmp0(keys, "Filename") == 0)
        dli->filename = g_value_dup_string(val);
    else if (g_strcmp0(keys, "TotalSize") == 0)
        dli->total_size = g_value_get_int(val);
    else if (g_strcmp0(keys, "Size") == 0)
        dli->size = g_value_get_int(val);
    else if (g_strcmp0(keys, "StoragePath") == 0)
        dli->path = g_value_dup_string(val);
    else if (g_strcmp0(keys, "EstimateTime") == 0)
        dli->estimate_time = g_value_get_int(val);
}

static void
print_dli_header(struct dl_info_s *dli, int size_info)
{
    printf("Download from URL: %s\n", dli->url);
    printf("   save to: %s\n", dli->full_path);
    printf("   state: %s\n", dli->state);
    if (size_info && dli->total_size > 0)
    {
        if (dli->size > 0)
        {
            printf("   done: %d/%d %d%%\n",
                   dli->size, dli->total_size,
                   ((long long) dli->size * 100) / dli->total_size);
        }
    }
    else
    {
        printf("   total size: %d\n", dli->total_size);
    }

}

static void
print_progress(struct dl_info_s *dli, int newline)
{
    const char *columns = getenv("COLUMNS");
    int cols = atoi(columns ? columns : "0");
    char percent_str[20] = {0};
    char speed_str[200] = {0};
    double speed_ps = 0.0;
    const char *suffix = "kB/s";
    int percent = 0;

    if (dli->total_size)
        percent = ((long long) dli->size * 100) / dli->total_size;

    if (cols == 0)
        cols = 80;

    g_snprintf(percent_str, sizeof(percent_str), "%3d %% ",
               percent);
    speed_ps = ((double) dli->total_size - dli->size) / dli->estimate_time;

    g_snprintf(speed_str, sizeof(speed_str), " %8.2lf %s",
               speed_ps, suffix);

    int bar_length = cols - strlen(percent_str) - strlen(speed_str);

    g_debug("bar length: %d", bar_length);

    int fill_length = bar_length - 2; /* account for [ ] at ends */
    int done_fill_length = 0;
    if (percent > 0)
        done_fill_length = (((long long) fill_length) * percent) / 100;
    int empty_fill_length = fill_length - done_fill_length;

    g_debug("done length: %d empty length: %d", done_fill_length, empty_fill_length);

    fprintf(stdout, "\r");
    for (int i = 0; i < cols; i++)
        fprintf(stdout, " ");
    fprintf(stdout, "\r");

    fprintf(stdout, "%s", percent_str);
    fprintf(stdout, "[");
    for (int i = 0; i < done_fill_length; i++)
    {
        if (i != (done_fill_length - 1))
            fprintf(stdout, "=");
        else
            fprintf(stdout, ">");
    }
    for (int i = 0; i < empty_fill_length; i++)
        fprintf(stdout, " ");
    fprintf(stdout, "]");
    fprintf(stdout, "%s", speed_str);
    if (newline)
        fprintf(stdout, "\n");
    else
        fflush(stdout);
}

static int
show_status_for_id(int id)
{
    DBusGProxy *prox;
    struct dl_info_s dli = DL_INFO_INIT;

    setup_conn();
    prox = get_dumm_item_props_proxy(_dbus_conn, id);
    if (!prox)
    {
        g_warning("failed to obtain proxy to download object");
        return -1;
    }

    if (fill_dlinfo_for_id(prox, id, &dli))
    {
        printf("information on download of id %d is unavailable\n",
               id);
        goto cleanup;
    }

    print_dli_header(&dli, 0);
    if (strcmp(dli.state, "ACTIVE") == 0 &&
        dli.total_size >= 0)
    {
        while (1)
        {
            print_progress(&dli, 0);
            if (g_strcmp0(dli.state, "FINISHED") == 0)
                break;
            sleep(1);
            update_dlinfo_for_id(prox, id, &dli);
        }
    }
cleanup:
    free_dlinfo(&dli);

    g_object_unref(prox);
    return 0;
}

/**
 * @brief show download status
 */
static int
dl_status(const char *cmd, int argc, const char *argv[])
{
    guint id;

    if (argc < 1)
        return -1;

    id = strtoul(argv[0], NULL, 0);

    show_status_for_id(id);
}

/**
 * @brief show download monitor
 */
static int
dl_monitor(const char *cmd, int argc, const char *argv[])
{

}

/**
 * @bref wrapper for resume, abort, pause commands
 */
static int
dl_single_cmd(const char *cmd, int argc, const char *argv[])
{
    DBusGProxy *prox;
    gboolean ret;
    guint id = 0;
    GError *err = NULL;

    if (argc == 0)
    {
        g_message("usage: %s <id>", cmd);
        return 0;
    }

    /* command may take ID */
    id = strtoul(argv[0], NULL, 0);

    if (setup_conn())
    {
        g_warning("cannot connect to D-Bus");
        return -1;
    }

    prox = get_dumm_proxy(_dbus_conn);
    if (!prox)
    {
        g_warning("failed to obtain proxy\n");
    }
    g_debug("got proxy");

    if (g_strcmp0(cmd, "abort") == 0)
        dl_abort(prox, id);
    else if (g_strcmp0(cmd, "pause") == 0)
        dl_pause(prox, id);
    else if (g_strcmp0(cmd, "resume") == 0)
        dl_resume(prox, id);

    g_object_unref(prox);
    return 0;

}

/**
 * @brief resume paused download
 */
static gboolean
dl_resume(DBusGProxy *prox, int id)
{
    gboolean ret = FALSE;
    GError *err = NULL;
    ret = net_dumm_download_resume(prox,
                                   id,
                                   &err); /* error */
    if (ret == FALSE)
    {
        g_warning("failed to resume download: %s", err->message);
        g_error_free(err);
    }
    else
    {
        g_message("resumed");
    }
    return ret;
}

/**
 * @brief abort download
 */
static gboolean
dl_abort(DBusGProxy *prox, int id)
{
    gboolean ret = FALSE;
    GError *err = NULL;
    ret = net_dumm_download_abort(prox,
                                  id,
                                  &err); /* error */
    if (ret == FALSE)
    {
        g_warning("failed to abort download: %s", err->message);
        g_error_free(err);
        goto out;
    }
    else
    {
        g_message("aborted");
    }

    /* call finalize to do the cleanup */
    net_dumm_download_finalize(prox, id, &err);

out:
    return ret;
}

/**
 * @brief setup D-Bus connection
 */
static int
setup_conn(void)
{
    GError *err = NULL;
    DBusGConnection *conn = NULL;

    conn = dbus_g_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn)
    {
        if (err)
        {
            g_warning("cannot obtain D-Bus conncetion: %s", err->message);
            g_error_free(err);
        }
        return -1;
    }

    g_debug("connection to D-Bus obtained");
    _dbus_conn = conn;
    return 0;
}

