/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * GLib and pipes based dispatcher implementation.
 */

#ifndef GLIB_DISPATCHER_H
#define GLIB_DISPATCHER_H

#include "dispatcher.h"
#include <glib.h>

# ifdef __cplusplus
extern "C" {
# endif

/**
 * @brief Creates new dispatcher for given context.
 * @param[in] ctx Context pointer to which events will be dispatched.
 * @return Pointer to opaque dispatcher strucuture.
 */
extern dispatcher_Dispatcher* glib_dispatcher_newDispatcher(GMainContext* ctx);

/**
 * @brief Releases all associated resources.
 * @param[in] dispatcher Pointer to dispatcher structure.
 */
extern void glib_dispatcher_deleteDispatcher(dispatcher_Dispatcher* dispatcher);

# ifdef __cplusplus
}
# endif

#endif
