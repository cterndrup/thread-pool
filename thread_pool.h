/* Header file for thread pool */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "thread_pool_task.h"

int
thread_pool_create(unsigned int n_threads);

int
thread_pool_submit(thread_function_t  *function,
                   void               *function_arg,
                   thread_callback_t  *callback,
                   void               *callback_arg);

void
thread_pool_wait(void);

#endif // _THREAD_POOL_H_
