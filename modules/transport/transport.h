/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * This library wrapps cloud communication transport layer.
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

# ifdef __cplusplus
extern "C" {
# endif

#include <stddef.h>

/**
 * @brief Initialize transport layer.
 * @return 0 if succeed, 1 otherwise.
 */
extern int transport_init();

/**
 * @brief Release all acquired resources before shutdown.
 */
extern void transport_cleanup();

/**
 * @brief Sends message to the cloud.
 * @param[in] msg Message to be sent.
 * @param[in] size Size of message in bytes.
 * @return 0 if succeed, 1 otherwise.
 */
extern int transport_sendMsg(const void* msg, size_t size);

/**
 * @brief Received message handler function.
 * @param[in] msg Message to be handled.
 * @param[in] size Size of message in bytes.
 */
typedef void (*TRANSPORT_MSG_HANDLER)(const void* msg, size_t size);
/**
 * @brief Sets message handler function.
 * @param[in] handler Pointer to the function that will handle received messages.
 */
extern void transport_setMsgHandler(TRANSPORT_MSG_HANDLER handler);

# ifdef __cplusplus
}
# endif

#endif
