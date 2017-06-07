/* Header file for thread pool */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "thread_pool_error.h"
#include "thread_pool_task.h"

/*
 * Creates a thread pool with n_threads
 *
 * @param[in] n_threads     Number of threads in the pool
 *
 * @return  0 if successful
 *          THREAD_POOL_ALLOC_ERROR on a resource allocation error
 *          THREAD_POOL_ERROR on other error
 */
int
thread_pool_create(unsigned int n_threads);

/*
 * Submits a task to a thread pool created with thread_pool_create
 *
 * @param[in] function     A function executed by one of the threads
 *                         in the pool
 * @param[in] function_arg The argument to function
 * @param[in] callback     A function executed by the assigned thread after
 *                         execution of function has completed
 * @param[in] callback_arg The argument to callback
 *
 * @return  0 if successful
 *          THREAD_POOL_ALLOC_ERROR on a resource allocation error
 *          THREAD_POOL_INVALID_PTR if thread_pool_create() has not
 *              been called or failed
 *          THREAD_POOL_ERROR on other error
 */
int
thread_pool_submit(thread_function_t  *function,
                   void               *function_arg,
                   thread_callback_t  *callback,
                   void               *callback_arg);

/*
 * Blocks until all tasks submitted to the thread pool have completed.
 * Tasks should not be submitted concurrently to the execution of this
 * function. This allows the library to deallocate all necessary resources
 * safely and at the appropriate time.
 */
void
thread_pool_wait(void);

#endif // _THREAD_POOL_H_
