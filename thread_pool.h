/* Header file for thread pool */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

struct thread_pool_task;

struct thread_pool *
     thread_pool_create(unsigned int n_threads);
void thread_pool_destroy(struct thread_pool *p);
int  thread_pool_submit(struct thread_pool *p, struct thread_pool_task *t);

#endif // _THREAD_POOL_H_
