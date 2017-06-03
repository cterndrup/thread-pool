//
//  thread_pool_task.c
//  
//
//  Created by COLIN TERNDRUP on 6/3/17.
//
//

#include "thread_pool_task.h"

/*
 * Populates a thread pool task with its function, callback,
 * and associated arguments.
 */
int
thread_pool_task_create
(
    struct thread_pool_task *t,
    thread_function_t       *fun,
    void                    *fun_arg,
    thread_callback_t       *callback,
    void                    *callback_arg
)
{
    if (t == NULL)
        return -1;
    
    t->function     = fun;
    t->function_arg = fun_arg;
    t->callback     = callback;
    t->callback_arg = callback_arg;
    
    return 0;
}
