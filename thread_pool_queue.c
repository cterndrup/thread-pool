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
#include "thread_pool_queue.h"

/* Creates and returns a struct thread_pool_queue_node * */
static struct thread_pool_queue_node *
thread_pool_queue_node_create
(
    struct thread_pool_task *task
)
{
    struct thread_pool_queue_node *node =
    (struct thread_pool_queue_node *)malloc(sizeof(struct thread_pool_queue_node));
    if (node == NULL)
        return NULL;
    
    node->task = task;
    node->next = NULL;
    
    return node;
}

/* Destroys a struct thread_pool_queue_node */
static void
thread_pool_queue_node_destroy
(
    struct thread_pool_queue_node *node
)
{
    if (node == NULL)
        return;
    
    node->next = NULL;
    free(node);
}

/* Creates and returns a struct thread_pool_queue * */
struct thread_pool_queue *
thread_pool_queue_create(void)
{
    struct thread_pool_queue *queue = (struct thread_pool_queue *)malloc(sizeof(struct thread_pool_queue));
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
    
    return queue;
}

/* Destroys a struct thread_pool_queue */
int
thread_pool_queue_destroy
(
    struct thread_pool_queue *queue
)
{
    int err = 0;
    struct thread_pool_queue_node *node;
    
    if (queue == NULL)
        return -1;
    
    // Destroy all nodes in the queue
    err = pthread_mutex_lock(&queue->qmutex);
    if (err)
        return err;
    node = queue->head_node;
    while (node) {
        struct thread_pool_queue_node *temp = node->next;
        free(node);
        node = temp;
    }
    queue->n_tasks = 0;
    pthread_mutex_unlock(&queue->qmutex);
    
    // Destroy the mutex
    while ((err = pthread_mutex_destroy(&queue->qmutex)) == EBUSY);
    if (err)
        return err;
    
    free(queue);
    
    return err;
}

/* Places a struct thread_pool_task * on the thread pool queue */
int
thread_pool_queue_enqueue
(
    struct thread_pool_queue *queue,
    struct thread_pool_task  *task
)
{
    int err = 0;
    struct thread_pool_queue_node *node;
    
    if (queue == NULL)
        return -1;
    
    node = thread_pool_queue_node_create(task);
    if (node == NULL)
        return -1;
    
    err = pthread_mutex_lock(&queue->qmutex);
    if (err) {
        thread_pool_queue_node_destroy(node);
        return err;
    }

    if (queue->tail_node != NULL)
        queue->tail_node->next = node;
    queue->tail_node = node;
    
    if (queue->head_node == NULL)
        queue->head_node = node;
    
    ++queue->n_tasks;
    
    err = pthread_mutex_unlock(&queue->qmutex);
    
    return err;
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
    struct thread_pool_queue_node *node;
    struct thread_pool_task       *task;
    
    if (queue == NULL)
        return NULL;
    
    if (pthread_mutex_trylock(&queue->qmutex))
        return NULL;
    
    node = queue->head_node;
    if (node != NULL)
        queue->head_node = node->next;
    
    if (queue->tail_node == node)
        queue->tail_node = NULL;
    
    --queue->n_tasks;
    
    pthread_mutex_unlock(&queue->qmutex);
    
    task = (node == NULL) ? node->task : NULL;
    
    free(node);
    
    return task;
}