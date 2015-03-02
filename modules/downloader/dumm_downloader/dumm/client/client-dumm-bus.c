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
 * @file client-dumm-bus.c
 * @date 03-06-2013
 * @brief DBus wrapper
 *
 * __DETAILS__
 */
#include <dbus/dbus-glib.h>

/**
 * @brief get proxy for DUMM
 */
DBusGProxy *
get_dumm_proxy(DBusGConnection *conn)
{
    DBusGProxy *prox;
    prox = dbus_g_proxy_new_for_name(conn,
                                     "net.dumm",
                                     "/",
                                     "net.dumm.download");
    if (prox == NULL)
    {
        g_return_val_if_fail(prox != NULL, NULL);
    }

    return prox;
}

/**
 * @brief get proxy for DMObject items
 */
DBusGProxy *
get_dumm_item_props_proxy(DBusGConnection *conn, int id)
{
    char *path;
    DBusGProxy *prox;

    path = g_strdup_printf("/DMObject/%d", id);
    g_return_val_if_fail(path, NULL);

    prox = dbus_g_proxy_new_for_name(conn,
                                     "net.dumm",
                                     path,
                                     DBUS_INTERFACE_PROPERTIES);
    if (prox == NULL)
    {
        g_error("failed to obtain proxy for DM Object\n");
    }

    g_free(path);

    return prox;
}

/**
 * @brief get signle property for object
 */
int
get_dumm_item_prop(DBusGProxy *proxy, const gchar *prop, GValue *val)
{
    GError *error = NULL;
    g_assert(proxy != NULL && prop != NULL);

    if (!dbus_g_proxy_call (proxy, "Get", &error,
                           G_TYPE_STRING, "net.dumm.download",
                           G_TYPE_STRING, prop,
                           G_TYPE_INVALID,
                           G_TYPE_VALUE, val,
                           G_TYPE_INVALID))
    {
        g_debug ("Failed to get active connection Connection property: %s",
                 error->message);
        g_error_free(error);
        return -1;
    }
    return 0;
}

/**
 * @brief get all properties of given object
 */
int
get_dumm_item_prop_all(DBusGProxy *proxy, GHashTable **hash)
{
    GError *error = NULL;
    g_assert(proxy != NULL);

    g_debug("proxy call");
    if (!dbus_g_proxy_call (proxy, "GetAll", &error,
                            G_TYPE_STRING, "net.dumm.download",
                            G_TYPE_INVALID,
                            dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), hash,
                            G_TYPE_INVALID))
    {
        g_debug("failed to get all properties: %s", error->message);
        g_error_free(error);
        return -1;
    }
    return 0;
}

