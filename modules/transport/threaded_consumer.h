/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Consumer implementation cyclically receives messages and maintains connection.
 * Received messages should be stored on internal and synchronized queue.
 * They will be popped from main thread (after event dispatched) with threaded_consumer_popMsg().
 */

#ifndef THREADED_CONSUMER_H
#define THREADED_CONSUMER_H

#include "transport_config.h"
#include <stddef.h>

/**
 * @brief Initialize threaded consumer. Called from main thread.
 * @param[in] config Pointer to initialized config that will be used to setup cloud connecton.
 * @return 0 if succeed, 1 otherwise.
 */
extern int threaded_consumer_init(const transport_config_ConnectionConfig* config);

/**
 * @brief Release all acquired resources before shutdown. Called from main thread.
 */
extern void threaded_consumer_cleanup();

/**
 * @brief Cyclically called from worker thread. Implementation should take care
 *       of connection maintenance and all required setup. It should process incomming
 *       messages or wait for next one (until new come or threaded_consumer_break() called).
 * @return Number of messages received. Non zero value will trigger new message event
 *        that will be dispatched to main thread and trigger threaded_consumer_popMsg().
 */
extern unsigned int threaded_consumer_receiveNext();

/**
 * @brief Cause exit from threaded_consumer_receiveNext() as soon as possible.
 */
extern void threaded_consumer_break();

/**
 * @brief Pops msg from internal received messages queue.
 *       Called from main thread.
 * @param[out] msg Pointer to newly allocated byte array with message.
 *       Caller of this function is responsible for releasing allocated
 *       memory when it is not needed anymore.
 * @param[out] size Size of returned message (in bytes).
 * @return 0 if there was no message to pop, 1 otherwise.
 */
extern int threaded_consumer_popMsg(void** msg, size_t* size);

#endif
