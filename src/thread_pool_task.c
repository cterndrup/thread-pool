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

struct thread_pool_task {
    // A function to execute once task is scheduled to a thread
    thread_function_t *function;

    // The argument to above function.
    void *function_arg;

    // A function to execute on fun's completion
    thread_callback_t *callback;

    // The argument to above callback
    void *callback_arg;
};

/*
 * Populates a thread pool task with its function, callback,
 * and associated arguments.
 */
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
void
thread_pool_task_destroy(struct thread_pool_task *task)
{
    DPRINTF("entered thread_pool_task_destroy\n");

    free(task);

    DPRINTF("thread_pool_task destroyed successfully\n");
}

/* Returns the function from a struct thread_pool_task */
thread_function_t *
thread_pool_task_get_function(struct thread_pool_task *task)
{
    return task->function;
}

/* Returns the function arg from a struct thread_pool_task */
void *
thread_pool_task_get_function_arg(struct thread_pool_task *task)
{
    return task->function_arg;
}

/* Returns the callback from a struct thread_pool_task */
thread_callback_t *
thread_pool_task_get_callback(struct thread_pool_task *task)
{
    return task->callback;
}

/* Returns the callback arg from a struct thread_pool_task */
void *
thread_pool_task_get_callback_arg(struct thread_pool_task *task)
{
    return task->callback_arg;
}
