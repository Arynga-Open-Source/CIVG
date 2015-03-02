/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

/**
 * RVI specific connection config strucuture.
 */

#ifndef RVI_TOOLS_H
#define RVI_TOOLS_H

#include <stddef.h>

/**
 * @brief register RVI service
 * @param[in] node    RVI node
 * @param[in] path    service path, ex. /carsync/updates
 * @param[in] target  target for handling service messages
 * @return 0 on success
 */
int rvi_register_service(const char *node, const char *path,
                         const char *target);

/**
 * @brief send CS message to RVI node
 * @param[in] node     RVI node address
 * @param[in] target   target service address
 * @param[in] msg      CS message
 * @param[in] msg_size size of CS message
 *
 * Send CS message to given target serializing it to JSON. In this
 * case, the binary PB message is base64 encoded.
 *
 * @return 0 on success
 */
int rvi_send_cs_message(const char *node , const char *target,
                        void *msg, size_t msg_size);

/**
 * @brief parse CS message received from RVI node
 * @param[in]  node     RVI node address
 * @param[in]  body     RVI request body
 * @param[out] msg_size size of allocated message
 *
 * Parse CS message that was received from RVI node. The message is
 * expected to have been serialized to JSON and baase64
 * encoded. Returns pointer to newly allocated binary message.
 *
 * @return pointer to CS message or NULL
 */
void *rvi_parse_cs_message(const char *target, const char *body,
                           size_t *msg_size);

/**
 * @brief allocate a string with formatted error reponse
 * @param[in] code   error code
 * @param[in] msg    descriptive error message
 * @return pointer to allocated message, freed by caller
 */
char *rvi_format_error(int code, const char *msg);

/**
 * @brief allocate a string with formatted OK response
 * @return pointer to allocated message, freed by caller
 */
char *rvi_format_ok(void);

#endif
