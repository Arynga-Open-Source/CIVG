/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Transport threading implementation. Manage threads and synchronization.
 * Specific transport must implement only producer, consumer and config.
 */

#ifndef THREADED_TRANSPORT_H
#define THREADED_TRANSPORT_H

# ifdef __cplusplus
extern "C" {
# endif

#include "transport.h"
#include "dispatcher.h"

/**
 * @brief Sets message event dispatcher. Dispatcher will be used to dispatch new
 *       message events to maint/init thread.
 * @param[in] dispatcher Pointer to dispatcher instance.
 */
extern void threaded_transport_setDispatcher(dispatcher_Dispatcher* dispatcher);

# ifdef __cplusplus
}
# endif

#endif
