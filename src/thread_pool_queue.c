//
//  thread_pool_queue.c
//
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "thread_pool_debug.h"
#include "thread_pool_error.h"

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

/* Creates and returns a struct thread_pool_queue_node * */
static struct thread_pool_queue_node *
thread_pool_queue_node_create
(
    struct thread_pool_task *task
)
{
    DPRINTF("entered thread_pool_queue_node_create\n");

    struct thread_pool_queue_node *node = (struct thread_pool_queue_node *)
        malloc(sizeof(struct thread_pool_queue_node));
    if (node == NULL)
        return NULL;

    node->task = task;
    node->next = NULL;

    DPRINTF("thread_pool_queue_node created successfully\n");

    return node;
}

/* Destroys a struct thread_pool_queue_node */
static void
thread_pool_queue_node_destroy
(
    struct thread_pool_queue_node *node
)
{
    DPRINTF("entered thread_pool_queue_node_destroy\n");

    if (node == NULL)
        return;

    node->next = NULL;
    free(node);

    DPRINTF("thread_pool_queue_node destroyed successfully\n");
}

/* Creates and returns a struct thread_pool_queue * */
struct thread_pool_queue *
thread_pool_queue_create(void)
{
    DPRINTF("entered thread_pool_queue_create\n");

    struct thread_pool_queue *queue =
        (struct thread_pool_queue *)malloc(sizeof(struct thread_pool_queue));
    if (queue == NULL)
        return NULL;

    // Initialize the queue
    queue->n_tasks = 0;
    queue->head_node = NULL;
    queue->tail_node = NULL;

    // Initialize the queue's mutex
    if (pthread_mutex_init(&queue->qmutex, NULL)) {
        free(queue);
        return NULL;
    }

    DPRINTF("thread_pool_queue created successfully\n");

    return queue;
}

/* Destroys a struct thread_pool_queue */
void
thread_pool_queue_destroy
(
    struct thread_pool_queue *queue
)
{
    DPRINTF("entered thread_pool_queue_destroy\n");

    struct thread_pool_queue_node *node;

    if (queue == NULL)
        return;

    node = queue->head_node;
    while (node) {
        struct thread_pool_queue_node *temp = node->next;
        free(node);
        node = temp;
    }
    queue->n_tasks = 0;

    // Destroy the mutex
    pthread_mutex_destroy(&queue->qmutex);

    free(queue);

    DPRINTF("thread_pool_queue destroyed successfully\n");
}

/* Places a struct thread_pool_task * on the thread pool queue */
int
thread_pool_queue_enqueue
(
    struct thread_pool_queue *queue,
    struct thread_pool_task  *task
)
{
    DPRINTF("entered thread_pool_queue_enqueue\n");

    struct thread_pool_queue_node *node;

    if (queue == NULL)
        return THREAD_POOL_INVALID_PTR;

    node = thread_pool_queue_node_create(task);
    if (node == NULL)
        return THREAD_POOL_ALLOC_ERROR;

    if (pthread_mutex_lock(&queue->qmutex)) {
        thread_pool_queue_node_destroy(node);
        return THREAD_POOL_ERROR;
    }

    if (queue->tail_node != NULL)
        queue->tail_node->next = node;
    queue->tail_node = node;

    if (queue->head_node == NULL)
        queue->head_node = node;

    ++queue->n_tasks;

    if (pthread_mutex_unlock(&queue->qmutex))
        return THREAD_POOL_ERROR;

    DPRINTF("thread_pool_task enqueued successfully\n");

    return 0;
}

/*
 * Removes and returns a struct thread_pool_task * from the thread
 * pool queue
 */
struct thread_pool_task *
thread_pool_queue_dequeue
(
    struct thread_pool_queue *queue
)
{
    DPRINTF("entered thread_pool_queue_dequeue\n");

    struct thread_pool_queue_node *node;
    struct thread_pool_task       *task;

    if (queue == NULL)
        return NULL;

    if (pthread_mutex_trylock(&queue->qmutex))
        return NULL;

    DPRINTF("queue mutex acquired successfully\n");

    node = queue->head_node;
    if (node != NULL)
        queue->head_node = node->next;

    if (queue->tail_node == node)
        queue->tail_node = NULL;

    --queue->n_tasks;

    pthread_mutex_unlock(&queue->qmutex);

    task = (node == NULL) ? NULL : node->task;

    free(node);

    DPRINTF("thread_pool_task dequeued successfully\n");

    return task;
}
