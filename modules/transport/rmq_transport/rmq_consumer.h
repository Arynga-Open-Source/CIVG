/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#ifndef RMQ_CONSUMER_H
#define RMQ_CONSUMER_H

#include "threaded_consumer.h"
#include <amqp.h>

/**
 * @brief Gets consumer reply queue.
 * @return Consumer reply queue.
 */
amqp_bytes_t rmq_consumer_getReplyQueue();

/**
 * @brief Should be called whenever there is a suspicion that connection
 *       with rmq could be lost.
 */
void rmq_consumer_checkConnection();

#endif
