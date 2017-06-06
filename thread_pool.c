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

#define EXPORT __attribute__((visibility("default")))
#define THREAD_SLEEP_TIME 1

extern struct thread_pool_task *
thread_pool_task_create(thread_function_t *fun,
                        void              *fun_arg,
                        thread_callback_t *callback,
                        void              *callback_arg);

extern void
thread_pool_task_destroy(struct thread_pool_task *task);

extern thread_function_t *
thread_pool_task_get_function(struct thread_pool_task *task);

extern void *
thread_pool_task_get_function_arg(struct thread_pool_task *task);

extern thread_callback_t *
thread_pool_task_get_callback(struct thread_pool_task *task);

extern void *
thread_pool_task_get_callback_arg(struct thread_pool_task *task);

extern struct thread_pool_queue *
thread_pool_queue_create(void);

extern void
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

static int    outstanding_tasks = 0;
static struct thread_pool *pool = NULL;

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

        fn = thread_pool_task_get_function(task);
        cb = thread_pool_task_get_callback(task);

        if (fn != NULL) {
            DPRINTF("running task\n");
            fn(thread_pool_task_get_function_arg(task));
        }

        if (cb != NULL) {
            DPRINTF("running callback\n");
            cb(thread_pool_task_get_callback_arg(task));
        }

        thread_pool_task_destroy(task);

        __atomic_sub_fetch(&outstanding_tasks, 1, __ATOMIC_SEQ_CST);

        DPRINTF("task completed\n");
    }

    return NULL;
}

/* Creates a struct thread_pool */
EXPORT
int
thread_pool_create
(
    unsigned int n_threads
)
{
    DPRINTF("entered thread_pool_create\n");

    static unsigned int create_calls = 1;
    if (create_calls++ > 1)
        return -1;

    unsigned int i;
    pool = (struct thread_pool *)malloc(sizeof(struct thread_pool));
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
    return (pool == NULL) ? -1 : 0;
}

/* Destroys a struct thread_pool */
static void
thread_pool_destroy(void)
{
    DPRINTF("entered thread_pool_destroy\n");

    unsigned int n_threads;
    pthread_t *threads;

    if (pool == NULL)
        return;

    n_threads = pool->n_threads;
    threads   = pool->threads;
    for (unsigned int i = 0; i < n_threads; ++i) {
        // Threads running thread_task won't terminate since
        // running in infinite loop, so must be cancelled.
        DPRINTF("cancelling and joining thread\n");
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }
    pool->n_threads = 0;

    free(threads);
    thread_pool_queue_destroy(pool->submission_queue);
    free(pool);

    DPRINTF("thread pool destroyed successfully\n");
}

/* Submits a struct thread_pool_task * to the thread pool's submission
 * queue
 */
EXPORT
int
thread_pool_submit
(
    thread_function_t  *function,
    void               *function_arg,
    thread_callback_t  *callback,
    void               *callback_arg
)
{
    DPRINTF("entered thread_pool_submit\n");

    int err;
    struct thread_pool_task *task;

    if (pool == NULL)
        return -1;

    task = thread_pool_task_create(function, function_arg,
                                   callback, callback_arg);
    if (task == NULL)
        return -1;

    err = thread_pool_queue_enqueue(pool->submission_queue, task);
    if (err)
        return err;

    __atomic_add_fetch(&outstanding_tasks, 1, __ATOMIC_SEQ_CST);

    DPRINTF("task enqueued successfully\n");

    return 0;
}

/* Blocks until all submitted tasks have been completed */
EXPORT
void
thread_pool_wait(void)
{
    int remaining_tasks;
    do {
        __atomic_load(&outstanding_tasks, &remaining_tasks,
                      __ATOMIC_SEQ_CST);
    } while (remaining_tasks);
}

/* Destroys the thread pool on exit */
__attribute__((destructor))
static void
thread_pool_destructor(void)
{
    thread_pool_destroy();
}
