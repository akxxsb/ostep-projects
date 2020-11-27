# include "pool.h"
# include "util.h"
# include "assert.h"

void *
worker(void * arg)
{
    pthread_t tid = pthread_self();
    //pthread_detach(tid);
    ThreadPool * pool = (ThreadPool *) arg;

    pthread_mutex_lock(&pool->lock);
    printf("worker %lu started ...\n", tid);
    pthread_mutex_unlock(&pool->lock);

    while (1) {
        pthread_mutex_lock(&pool->lock);
        int stop = pool->stop;
        pthread_mutex_unlock(&pool->lock);
        if (stop) {
            break;
        }
        JobParam * job = pop_queue(pool->job_queue);
        if (job && job->func) {
            job->func(job->arg);
            print_id();
            printf("free job at %p\n\n", job);
            free(job);
        }
    }
    //sem_post(&pool->wait_sem);
    pthread_mutex_lock(&pool->lock);
    printf("worker %lu exited ...\n", tid);
    pthread_mutex_unlock(&pool->lock);
    return (void *)0;
}

MQueue *
make_queue(int buffers)
{
    assert(buffers > 0 && buffers <= 100000);
    ++buffers;
    MQueue * q = (MQueue *) Malloc(sizeof(MQueue));
    q->data = (JobParam **) Malloc(sizeof(JobParam*) * buffers);
    q->size = buffers;
    q->stop = 0;
    q->front = q->tail = 0;
    pthread_cond_init(&q->empty, NULL);
    pthread_cond_init(&q->full, NULL);
    pthread_mutex_init(&q->lock, NULL);
    return q;
}

void
destroy_queue(MQueue *q)
{
    assert(q->stop == 1);
    if(q) {
        if (q->data) {
            free(q->data);
        }
        q->data = NULL;
        free(q);
    }
}

int
empty_queue(MQueue * q)
{
    return q->front == q->tail;
}

int
full_queue(MQueue * q)
{
    return (q->tail + 1) % q->size == q->front;
}

void
insert_queue(MQueue * q, JobParam * job)
{
    pthread_mutex_lock(&q->lock);
    while (q->stop == 0 && full_queue(q)) {
        pthread_cond_wait(&q->empty, &q->lock);
    }

    if (q->stop) {
        pthread_mutex_unlock(&q->lock);
        return;
    }

    assert(q->data[q->tail] == NULL);
    q->data[q->tail] = job;
    q->tail = (q->tail + 1) % q->size;

    pthread_cond_signal(&q->full);
    pthread_mutex_unlock(&q->lock);
}

JobParam *
pop_queue(MQueue * q)
{
    assert(q != NULL);
    pthread_mutex_lock(&q->lock);
    while (q->stop == 0 && empty_queue(q)) {
        pthread_cond_wait(&q->full, &q->lock);
    }

    if (q->stop) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    JobParam * job =  q->data[q->front];
    q->data[q->front] = NULL;
    q->front = (q->front + 1) % q->size;

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
    return job;
}

ThreadPool *
make_thread_pool(int thread_nums, int buffers)
{
    assert(thread_nums >= 1 && thread_nums <= 1000);
    ThreadPool * pool = (ThreadPool *) Malloc(sizeof(ThreadPool));
    pool->pool_size = thread_nums;
    pool->job_queue = make_queue(buffers);
    pool->stop = 0;

    pool->tid_list =  (pthread_t *) Malloc(sizeof(pthread_t) * thread_nums);
    pthread_mutex_init(&pool->lock, NULL);
    //sem_init(&pool->sem, 0, 0);

    for (int i = 0; i < pool->pool_size; ++i) {
        pthread_create(&pool->tid_list[i], NULL, worker, pool);
    }
    return pool;
}

void
submit(ThreadPool * pool, JobParam * job) {
    assert(pool != NULL);
    insert_queue(pool->job_queue, job);
}

void
destroy_thread_pool(ThreadPool * pool)
{
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_mutex_unlock(&pool->lock);

    // 唤醒所有等待队列的线程, 以便线程退出
    MQueue * q = pool->job_queue;
    pthread_mutex_lock(&q->lock);
    q->stop = 1;
    pthread_cond_broadcast(&q->empty);
    pthread_cond_broadcast(&q->full);
    pthread_mutex_unlock(&q->lock);

    for (int i = 0; i < pool->pool_size; ++i) {
        sem_wait(&pool->wait_sem);
    }
    //sem_destroy(&pool->sem);

    for (int i = 0; i < pool->pool_size; ++i) {
        pthread_join(pool->tid_list[i], NULL);
    }
    free(pool->tid_list);
    pool->tid_list = NULL;

    destroy_queue(pool->job_queue);
    pool->job_queue = NULL;

    pool->pool_size = 0;
    free(pool);
}