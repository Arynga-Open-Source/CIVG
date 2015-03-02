/** Arynga CarSync(TM)
 * 2014 Copyrights by Arynga Inc. All rights reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 */

#include "msgqueue.h"
#include "logger.h"
#include <stdlib.h>

struct Message{
    void* msg;
    size_t size;
    struct Message* prev;
};
typedef struct Message Message;

struct msgqueue_Queue {
    Message* front;
    Message* back;
};

msgqueue_Queue* msgqueue_newQueue() {
    msgqueue_Queue* queue = malloc(sizeof(msgqueue_Queue));
    if (queue) {
        queue->front = 0;
        queue->back = 0;
    }
    return queue;
}

void msgqueue_deleteQueue(msgqueue_Queue *queue) {
    if (queue) {
        Message* message = queue->front;
        while (message) {
            queue->front = message->prev;
            free(message->msg);
            free(message);
            message = queue->front;
        }
        free(queue);
    }
}

int msgqueue_push(msgqueue_Queue *queue, void *msg, size_t size) {
    Message* message = malloc(sizeof(Message));
    if (!message) {
        LOGM(LOGG_ERROR, "Failed to allocate memory for new message on queue!");
        return 1;
    }

    message->msg = msg;
    message->size = size;
    message->prev = 0;
    if (!queue->back) {
        // queue is empty so set front and back to point newly pushed message
        queue->front = message;
        queue->back = message;
    } else {
        // attach new message to back and move back pointer to point new message
        queue->back->prev = message;
        queue->back = message;
    }
    return 0;
}

int msgqueue_pop(msgqueue_Queue *queue, void **msg, size_t *size) {
    int result = 0;
    if (queue->front) {
        Message* message = queue->front;
        *msg = message->msg;
        *size = message->size;
        // move front to point previous one
        queue->front = message->prev;
        if (!queue->front) {
            // if queue is empty cleare back as well
            queue->back = 0;
        }
        free(message);
        result = 1;
    }
    return result;
}


int msgqueue_empty(const msgqueue_Queue *queue) {
    if (!queue)
        return 1;
    return (queue->front == 0);
}
