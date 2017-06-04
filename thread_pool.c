//
//  thread_pool.c
//
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "thread_pool.h"
#include "thread_pool_debug.h"

#define EXPORT __attribute__((visibility("default")))
#define THREAD_SLEEP_TIME 1

extern struct thread_pool_queue *
thread_pool_queue_create(void);

extern int
thread_pool_queue_destroy(struct thread_pool_queue *queue);

extern int
thread_pool_queue_enqueue(struct thread_pool_queue *queue,
                          struct thread_pool_task  *task);

extern struct thread_pool_task *
thread_pool_queue_dequeue(struct thread_pool_queue *queue);

struct thread_pool {
    // Number of threads in the pool
    unsigned int n_threads;

    // The threads in the pool
    pthread_t *threads;

    // Submission queue
    struct thread_pool_queue *submission_queue;
};

/* Function each thread initially runs until pool is destroyed */
static void *
thread_task(void *arg)
{
    struct thread_pool_queue *queue = (struct thread_pool_queue *)arg;

    while (1) {
        struct thread_pool_task *task;
        thread_function_t       *fn;
        thread_callback_t       *cb;

        DPRINTF("attempting to dequeue task\n");

        while ((task = thread_pool_queue_dequeue(queue)) == NULL) {
            sleep(THREAD_SLEEP_TIME);
        }

        DPRINTF("task acquired\n");

        fn = task->function;
        cb = task->callback;

        if (fn != NULL) {
            DPRINTF("running task\n");
            fn(task->function_arg);
        }

        if (cb != NULL) {
            DPRINTF("running callback\n");
            cb(task->callback_arg);
        }

        thread_pool_task_destroy(task);

        DPRINTF("task completed\n");
    }

    return NULL;
}

/* Creates and returns a struct thread_pool * */
EXPORT
struct thread_pool *
thread_pool_create
(
    unsigned int       n_threads,
    unsigned int       timeout,
    timeout_handler_t *timeout_handler
)
{
    DPRINTF("entered thread_pool_create\n");

    unsigned int i;
    struct thread_pool *pool =
        (struct thread_pool *)malloc(sizeof(struct thread_pool));
    if (pool == NULL)
        goto DONE;

    DPRINTF("pool allocated successfully\n");

    pool->n_threads = n_threads;

    // Create submission queue
    pool->submission_queue = thread_pool_queue_create();
    if (pool->submission_queue == NULL)
        goto FAIL_QUEUE_CREATE;

    DPRINTF("queue created successfully\n");

    // Allocate memory for threads
    pool->threads = (pthread_t *)malloc(n_threads*sizeof(pthread_t));
    if (pool->threads == NULL)
        goto FAIL_THREADS_ALLOC;

    DPRINTF("threads allocated successfully\n");

    // Create each thread with thread_task
    for (i = 0; i < n_threads; ++i) {
        if (pthread_create(&pool->threads[i], NULL, thread_task,
            (void *)pool->submission_queue))
        {
            goto FAIL_THREADS_CREATE;
        }
    }

    DPRINTF("threads created successfully\n");

    // Start timer if applicable
    if (timeout > 0 && timeout_handler != NULL) {
        if (signal(SIGALRM, timeout_handler) == SIG_ERR)
            goto FAIL_TIMEOUT_REGISTER;
        alarm(timeout);
    }

    goto DONE;

FAIL_TIMEOUT_REGISTER:
    for (unsigned int j = 0; j < i; ++j) {
        pthread_cancel(pool->threads[j]);
        pthread_join(pool->threads[j], NULL);
    }
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
EXPORT
int
thread_pool_destroy(struct thread_pool *p)
{
    DPRINTF("entered thread_pool_destroy\n");

    int err;
    unsigned int n_threads;
    pthread_t *threads;

    if (p == NULL)
        return -1;

    n_threads = p->n_threads;
    threads   = p->threads;
    for (unsigned int i = 0; i < n_threads; ++i) {
        // Threads running thread_task won't terminate since
        // running in infinite loop, so must be cancelled.
        DPRINTF("cancelling and joining thread\n");
        if ((err = pthread_cancel(threads[i])))
            return err;
        if ((err = pthread_join(threads[i], NULL)))
            return err;
    }
    p->n_threads = 0;

    free(threads);
    err = thread_pool_queue_destroy(p->submission_queue);
    free(p);

    DPRINTF("thread pool destroyed successfully\n");

    return err;
}

/* Submits a struct thread_pool_task * to the thread pool's submission
 * queue
 */
EXPORT
int
thread_pool_submit
(
    struct thread_pool      *p,
    struct thread_pool_task *t
)
{
    DPRINTF("entered thread_pool_submit\n");

    int err;

    if (p == NULL)
        return -1;

    if ((err = thread_pool_queue_enqueue(p->submission_queue, t)))
        return err;

    DPRINTF("task enqueued successfully\n");

    return 0;
}
