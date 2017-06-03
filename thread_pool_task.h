/* Header file for thread pool task */

#ifndef _THREAD_POOL_TASK_H_
#define _THREAD_POOL_TASK_H_

typedef void * thread_function_t(void *);
typedef void * thread_callback_t(void *);

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

struct thread_pool_task *
    thread_pool_task_create(thread_function_t       *fun,
                            void                    *fun_arg,
                            thread_callback_t       *callback,
                            void                    *callback_arg);

void thread_pool_task_destroy(struct thread_pool_task *task);

#endif // _THREAD_POOL_TASK_H_
