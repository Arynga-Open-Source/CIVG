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
 * @file client-dumm-bus.h
 * @date 03-06-2013
 * @brief DBus wrapper header
 *
 * __DETAILS__
 */

#ifndef __CLIENT_DUMM_BUS_H__
#define __CLIENT_DUMM_BUS_H__
#include <dbus/dbus-glib.h>

DBusGProxy *get_dumm_proxy(DBusGConnection *conn);
DBusGProxy *get_dumm_item_props_proxy(DBusGConnection *conn, int id);
int get_dumm_item_prop(DBusGProxy *prox, const gchar *prop, GValue *val);
int get_dumm_item_prop_all(DBusGProxy *prox, GHashTable **hash);

#endif /* __CLIENT_DUMM_BUS_H__ */
