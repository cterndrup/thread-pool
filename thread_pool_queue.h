/* Header file for thread pool queue */

#ifndef _THREAD_POOL_QUEUE_H_
#define _THREAD_POOL_QUEUE_H_

#include <pthread.h>
#include "thread_pool_task.h"

struct thread_pool_queue_node {
    // The task
    struct thread_pool_task *task;

    // Pointer to the next node
    struct thread_pool_queue_node *next;
};

struct thread_pool_queue {
    // Number of tasks in the queue
    unsigned int n_tasks;

    // The queue itself
    struct thread_pool_queue_node *head_node;
    struct thread_pool_queue_node *tail_node;

    // The queue locks
    pthread_mutex_t qmutex;
};

struct thread_pool_queue * thread_pool_queue_create(void);
void thread_pool_queue_destroy(struct thread_pool_queue *queue);
int  thread_pool_queue_enqueue(struct thread_pool_queue *queue,
                             struct thread_pool_task *task);
struct thread_pool_task *
     thread_pool_queue_dequeue(struct thread_pool_queue *queue);

#endif // _THREAD_POOL_QUEUE_H_
