/** Arynga CarSync(TM)
 * 2014-2015 Copyrights by Arynga Inc. All rights reserved.
 */

/**
 * Simple implementation of messages queue.
 */

#ifndef MSGQUEUE_H
#define MSGQUEUE_H

# ifdef __cplusplus
extern "C" {
# endif

#include <stddef.h>

typedef struct msgqueue_Queue msgqueue_Queue;

/**
 * @brief Creates new empty queue.
 * @return Pointer to opaque queue strucuture.
 */
extern msgqueue_Queue* msgqueue_newQueue();

/**
 * @brief Releases all resources related to queue (queue strucuture, but also
 *       messages stored inside if queue is not empty).
 * @param[in] queue Pointer to queue structure.
 */
extern void msgqueue_deleteQueue(msgqueue_Queue* queue);

/**
 * @brief Push message on given queue.
 * @param[in] queue Pointer to the queue strucuture.
 * @param[in] msg Message to be pushed.
 *       Messages queue takes over responsibility for releasing pushed message
 *       (in msgqueue_deleteQueue()) until it is popped back.
 * @param[in] size Size of message in bytes.
 * @return 0 if succeed, 1 otherwise.
 */
extern int msgqueue_push(msgqueue_Queue* queue, void* msg, size_t size);

/**
 * @brief Pop message from given queue if any. This function is non-blocking
 *       and returns 0 if no message to pop.
 * @param[in] queue Pointer to the queue strucuture.
 * @param[out] msg Pointer to popped message.
 *       Caller of this function takes over responsibility for releasing popped message.
 * @param[out] size Size of popped message in bytes.
 * @return 0 if there was no message to pop, 1 otherwise.
 */
extern int msgqueue_pop(msgqueue_Queue *queue, void** msg, size_t* size);

/**
 * @brief Checks if given queue is empty.
 * @param[in] queue Pointer to the queue strucuture.
 * @return 1 if empty, 0 otherwise.
 */
extern int msgqueue_empty(const msgqueue_Queue *queue);

# ifdef __cplusplus
}
# endif

#endif
