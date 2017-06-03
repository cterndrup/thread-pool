//
//  thread_pool.c
//  
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "thread_pool.h"
#include "thread_pool_debug.h"
#include "thread_pool_queue.h"

#define THREAD_SLEEP_TIME 2

struct thread_pool {
    // Number of threads in the pool
    unsigned int n_threads;
    
    // The threads in the pool
    pthread_t *threads;
    
    // Submission queue
    struct thread_pool_queue *submission_queue;
};

/* Function each thread initially runs */
void * thread_task(void *arg)
{
    struct thread_pool_queue *queue = (struct thread_pool_queue *)arg;

    while (1) {
        struct thread_pool_task *task;
        thread_function_t       *fn;
        thread_callback_t       *cb;
        
        DPRINT("attempting to dequeue task\n");
        
        while ((task = thread_pool_queue_dequeue(queue)) == NULL) {
            sleep(THREAD_SLEEP_TIME);
        }
        
        DPRINT("task acquired\n");
        
        fn = task->function;
        cb = task->callback;

        if (fn != NULL) {
            DPRINT("running task\n");
            fn(task->function_arg);
        }
        
        if (cb != NULL) {
            DPRINT("running callback\n");
            cb(task->callback_arg);
        }
        
        DPRINT("task completed\n");
    }
    
    return NULL;
}

/* Creates and returns a struct thread_pool * */
struct thread_pool *
thread_pool_create(unsigned int n_threads)
{
    DPRINT("entered thread_pool_create\n");
    
    struct thread_pool *pool = (struct thread_pool *)malloc(sizeof(struct thread_pool));
    if (pool == NULL)
        goto DONE;
    
    DPRINT("pool allocated succesfully\n");
    
    pool->n_threads = n_threads;
    
    // Create submission queue
    pool->submission_queue = thread_pool_queue_create();
    if (pool->submission_queue == NULL)
        goto FAIL_QUEUE_CREATE;
    
    DPRINT("queue created successfully\n");
    
    // Allocate memory for threads
    pool->threads = (pthread_t *)malloc(n_threads*sizeof(pthread_t));
    if (pool->threads == NULL)
        goto FAIL_THREADS_ALLOC;
    
    DPRINT("threads allocated succesfully\n");
    
    // Create each thread with thread_task
    for (unsigned int i = 0; i < n_threads; ++i) {
        if (pthread_create(&pool->threads[i], NULL, thread_task, (void *)pool->submission_queue))
            goto FAIL_THREADS_CREATE;
    }
    
    DPRINT("threads created successfully\n");
    
    goto DONE;
    
FAIL_THREADS_CREATE:
    free(pool->threads);
FAIL_THREADS_ALLOC:
    thread_pool_queue_destroy(pool->submission_queue);
FAIL_QUEUE_CREATE:
    free(pool);
    pool = NULL;
DONE:
    return pool;
}

/* Destroys a struct thread_pool */
int
thread_pool_destroy(struct thread_pool *p)
{
    DPRINT("entered thread_pool_destroy\n");
    
    int err = 0;
    unsigned int n_threads;
    pthread_t *threads;
    
    if (p == NULL)
        return -1;
    
    n_threads = p->n_threads;
    threads   = p->threads;
    for (unsigned int i = 0; i < n_threads; ++i) {
        // Threads running thread_task won't terminate since
        // running in infinite loop, so must be cancelled.
        DPRINT("cancelling and joining thread\n");
        if ((err = pthread_cancel(threads[i])))
            return err;
        if ((err = pthread_join(threads[i], NULL)))
            return err;
    }
    p->n_threads = 0;
    
    free(threads);
    err = thread_pool_queue_destroy(p->submission_queue);
    free(p);
    DPRINT("thread pool destroyed succesfully\n");
    
    return err;
}

/* Submits a struct thread_pool_task * to the thread pool's submission
 * queue
 */
int
thread_pool_submit
(
    struct thread_pool      *p,
    struct thread_pool_task *t
)
{
    DPRINT("entered thread_pool_submit\n");
    
    int err;
    
    if (p == NULL)
        return -1;
    
    if ((err = thread_pool_queue_enqueue(p->submission_queue, t)))
        return err;
    
    DPRINT("task enqueued successfully\n");
        
    return 0;
}