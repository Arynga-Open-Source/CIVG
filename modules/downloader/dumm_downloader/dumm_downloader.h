/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * This is library for parallel downloading set of files.
 */

#ifndef DUMM_DOWNLOADER_H
#define DUMM_DOWNLOADER_H

# ifdef __cplusplus
extern "C" {
# endif

#include "downloader.h"
#include <dbus/dbus-glib.h>

/**
 * @brief Sets glib/dbus context.
 * @param[in] connection DBus connection structure.
 */
extern void dumm_downloader_setupConnection(DBusGConnection *connection);

/**
 * @brief Registers marshallers used by dumm signals.
 */
void dumm_downloader_registerSignalMarshallers();

# ifdef __cplusplus
}
# endif

#endif
