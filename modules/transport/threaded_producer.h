/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * Consumer implementation cyclically send messages from internal queue.
 * Pushed messages are stored on internal synchronized queue, where wait for threaded_producer_sendNext()
 * to be called from worker thread.
 */

#ifndef THREADED_PRODUCER_H
#define THREADED_PRODUCER_H

#include "transport_config.h"
#include <stddef.h>

/**
 * @brief Initialize threaded producer. Called from main thread.
 * @param[in] config Pointer to initialized config that will be used to setup cloud connecton.
 * @return 0 if succeed, 1 otherwise.
 */
extern int threaded_producer_init(const transport_config_ConnectionConfig* config);

/**
 * @brief Release all acquired resources before shutdown. Called from main thread.
 */
extern void threaded_producer_cleanup();

/**
 * @brief Cyclically called from worker thread. Sends next message from internal
 *       queue or blocks if queue is empty until new message pushed.
 *       Use threaded_producer_pushMsg(0, 0) to force function exit
 *       (null messages are not sent but cause function exits).
 *       Called from worker thread.
 */
extern void threaded_producer_sendNext();

/**
 * @brief Push message on internal queue.
 *       Called from main thread.
 * @param[in] msg Message to be sent.
 * @param[in] size Size of message in bytes.
 * @return 0 if succeed, 1 otherwise.
 */
extern int threaded_producer_pushMsg(const void* msg, size_t size);

#endif
