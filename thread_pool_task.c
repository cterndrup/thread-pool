//
//  thread_pool_task.c
//
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#include <stdlib.h>
#include "thread_pool_debug.h"
#include "thread_pool_task.h"

#define EXPORT __attribute__((visibility("default")))

/*
 * Populates a thread pool task with its function, callback,
 * and associated arguments.
 */
EXPORT
struct thread_pool_task *
thread_pool_task_create
(
    thread_function_t *fun,
    void              *fun_arg,
    thread_callback_t *callback,
    void              *callback_arg
)
{
    DPRINTF("entered thread_pool_task_create\n");

    struct thread_pool_task *task =
        (struct thread_pool_task *)malloc(sizeof(struct thread_pool_task));
    if (task == NULL)
        return NULL;

    task->function     = fun;
    task->function_arg = fun_arg;
    task->callback     = callback;
    task->callback_arg = callback_arg;

    DPRINTF("thread_pool_task created successfully\n");

    return task;
}

/* Destroys a thread_pool_task */
EXPORT
void
thread_pool_task_destroy(struct thread_pool_task *task)
{
    free(task);
}
