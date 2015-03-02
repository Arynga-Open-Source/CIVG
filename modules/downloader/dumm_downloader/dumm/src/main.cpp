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
 * @file curl-headers.h
 * @author Bartlomiej Jozwiak (bj@open-rnd.pl)
 * @date 01-04-2013
 * @brief __BRIEF_HERE_TBD__
 *
 * __DETAILS__
 */


#ifdef HAVE_CONFIG_H
#include <dumm-config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gdbus.h>

#include "dumm.h"
#include "dumm-dbus.h"
#include "dumm-dm.h"
#include "dumm-um.h"
#include "files.h"


#ifndef RCS_VER
#define RCS_VER "unknown"
#endif
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__ " " __TIME__
#endif



//***********************************************************************************
// LOGGER Settings
//***********************************************************************************
#define LOGGER_NAME "DUMM-MAIN"
#define LOGGER_COL "33m"
#include "dumm-logger.h"

using namespace dumm;

typedef struct
{
    const char  *name;
    moduleInitT init;
    void        *params;
} ModuleItemT;

/** modules registration */
static ModuleItemT _gModules[] = {

        {"UM", umInit, 0},
        {"DM", dmInit, 0},
        {NULL, NULL, NULL}
};


/**
 * @brief parse arguments
 */
DummErrorT handle_args(CtxT *ctx, int argc, const char * const argv[])
{
    int opt;
    //  do not show errors from getopt
    opterr = 0;

    while ((opt = getopt(argc, (char * const *) argv, "s")) != -1)
    {
        switch (opt)
        {
        case 's':
            ctx->busType = DBUS_BUS_SYSTEM;
            break;
        default:
            break;
        }

    }

    return DUMM_ERR_OK;
}


/**
 * @brief TBD
 */
DummErrorT lowlevel_init(CtxT *ctx)
{

    return DUMM_ERR_OK;
}

/**
 * @brief Initialization of enviroment for DUMM
 */
DummErrorT init(CtxT *ctx)
{
    DummErrorT err = DUMM_ERR_UNKNOWN;

    do
    {
        if(!ctx || !ctx->conf)
        {
            err = DUMM_ERR_ARGUMENT;
            break;
        }

        const char *p = ctx->conf->getDownloadPath();
        if(!p)
            p = "/mnt/dumm/downloads"; //TODO Change it to another value

        if(!isDir(p))
            if(makeDir(p))
            {
                err = DUMM_ERR_FS_CREATE_DIR;
                break;
            }
        err = DUMM_ERR_OK;
    }while(0);

    return err;
}

/**
 * @brief TBD
 */
DummErrorT parse_conf(CtxT *ctx)
{
    DummErrorT err = DUMM_ERR_UNKNOWN;

    do
    {
        if(!ctx)
        {
            err = DUMM_ERR_ARGUMENT;
            break;
        }

        try
        {
            //TODO: path
            ctx->conf = new DummConf();
        }
        catch(...)
        {
            err = DUMM_ERR_PARSE_CONF;
            break;
        }
        err = DUMM_ERR_OK;
    }while(0);

    return err;
}

/**
 * @brief TBD
 */
static void cbDisconnect(DBusConnection *conn, void *user_data)
{
    logger(LOG_WARN, "DBus disconnect\n");
    CtxT *ctx = (CtxT*)user_data;
    if(ctx)
    {
        g_main_loop_quit(ctx->main_loop);
    }
}

/**
 * @brief TBD
 */
DummErrorT dbus_init(CtxT *ctx)
{
    DBusError err;

    if(!ctx)
        return DUMM_ERR_ARGUMENT;

//#ifdef NEED_THREADS
    if (dbus_threads_init_default() == FALSE)
    {
        logger(LOG_ERROR, "Can't init usage of threads\n");
        return DUMM_ERR_DBUS_INIT;
    }
//#endif

    dbus_error_init(&err);

    ctx->conn = g_dbus_setup_bus(ctx->busType, DUMM_SERVICE, &err);
    if (ctx->conn == NULL)
    {
        if (dbus_error_is_set(&err) == TRUE)
        {
            logger(LOG_ERROR, "%s\n", err.message);
            dbus_error_free(&err);
        }
        else
            logger(LOG_ERROR, "Can't register with system bus\n");
        return DUMM_ERR_DBUS_INIT;
    }

    g_dbus_set_disconnect_function(ctx->conn, cbDisconnect, ctx, NULL);

    return DUMM_ERR_OK;
}

/**
 * @brief TBD
 */
void dbus_deinit(CtxT *ctx)
{
    if(!ctx)
        return;

    dbus_connection_unref(ctx->conn);
}

/**
 * @brief TBD
 */
static
void __initCtx(CtxT *ctx)
{
    if(!ctx)
        return;

    memset(ctx, 0, sizeof(CtxT));

    /** TODO set it in params or config !!! */
    ctx->busType = DBUS_BUS_SESSION;
}


/**
 * @brief Main
 */
int main(int argc, const char * const argv[])
{


    CtxT ctx;

    printf("Download Upload Messaging Manager\nversion " RCS_VER " built on " BUILD_DATE "\n\n");

#if !GLIB_CHECK_VERSION(2, 32, 0)
    if (g_thread_supported() == FALSE)
        g_thread_init(NULL);
#endif

    __initCtx(&ctx);

    /** Handle arguments */
    if(handle_args(&ctx, argc, argv) != DUMM_ERR_OK)
    {
        logger(LOG_ERROR, "Cannot parse command line args\n");
        exit(1);
    }

    if(lowlevel_init(&ctx) !=  DUMM_ERR_OK)
    {
        logger(LOG_ERROR, "Cannot perform low-level init\n");
        exit(1);
    }

    if(parse_conf(&ctx) !=  DUMM_ERR_OK)
    {
        logger(LOG_ERROR, "Cannot perform low-level init\n");
        exit(1);
    }

    if(init(&ctx) !=  DUMM_ERR_OK)
    {
        logger(LOG_ERROR, "Initialize DUMM\n");
        exit(1);
    }

    ctx.main_loop = g_main_loop_new(NULL, FALSE);

    if(dbus_init(&ctx) !=  DUMM_ERR_OK)
    {
        logger(LOG_ERROR, "Cannot init dbus\n");
        exit(1);
    }

    ModuleItemT *item = _gModules;
    while(item && item->init && item->name)
    {
        logger(LOG_INFO, "Initializating %s ...\n", item->name);
        if(item->init(&ctx, item->params) != DUMM_ERR_OK)
        {
            logger(LOG_ERROR, "Cannot init %s module !!!\n", item->name);
            exit(1);
        }

        item++;
    }

    g_main_loop_run(ctx.main_loop);

    dbus_deinit(&ctx);

    g_main_loop_unref(ctx.main_loop);

    return 0;
}
