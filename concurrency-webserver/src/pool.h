#ifndef QSC_THREAD_POOL_INCLUDE
#define QSC_THREAD_POOL_INCLUDE

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    void * arg;
    void * (*func) (void *);
}JobParam;

typedef struct {
    JobParam ** data;
    size_t size;
    int front, tail;
    int stop;
    pthread_cond_t empty, full;
    pthread_mutex_t lock;
} MQueue;

typedef struct {
    pthread_t * tid_list;
    size_t pool_size;
    MQueue * job_queue;
    pthread_mutex_t lock;
    sem_t wait_sem;
    int stop;
}ThreadPool;

// public
ThreadPool * make_thread_pool(int thread_nums, int buffers);
void submit(ThreadPool * pool, JobParam * job);
void destroy_thread_pool(ThreadPool * pool);

// private
void * worker(void * arg);
MQueue * make_queue(int buffers);
void destroy_queue(MQueue *q);
int empty_queue(MQueue * q);
int full_queue(MQueue * q);
void insert_queue(MQueue * q, JobParam * job);
JobParam * pop_queue(MQueue * q);

//
#endif