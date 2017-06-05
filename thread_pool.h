/* Header file for thread pool */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "thread_pool_debug.h"
#include "thread_pool_task.h"

typedef void timeout_handler_t(int);

struct thread_pool *
thread_pool_create(unsigned int       n_threads,
                   unsigned int       timeout,
                   timeout_handler_t *timeout_handler);

int
thread_pool_destroy(struct thread_pool *p);

int
thread_pool_submit(struct thread_pool *p,
                   thread_function_t  *function,
                   void               *function_arg,
                   thread_callback_t  *callback,
                   void               *callback_arg);

#endif // _THREAD_POOL_H_
