#include <stdio.h>
#include <unistd.h>
#include "sharedqueue.h"

void sharedqueue_init_shm(struct sharedqueue *q) {
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    
    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);
    
    pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);

    q->head = 0; 
    q->tail = 0; 
    q->count = 0;
    
    pthread_mutex_init(&q->lock, &m_attr);
    pthread_cond_init(&q->not_full, &c_attr);
    pthread_cond_init(&q->not_empty, &c_attr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
}

void* producer(void *arg) {
    struct sharedqueue *q = arg;
    pthread_mutex_t *lock = &q->lock;
    pthread_cond_t *not_full = &q->not_full;
    pthread_cond_t *not_empty = &q->not_empty;

    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(lock);
        while (q->count == CAPACITY) {
            pthread_cond_wait(not_full, lock);
        }
        
        q->buf[q->tail] = i;
        q->tail = (q->tail + 1) % CAPACITY;
        q->count++;
        pthread_cond_signal(not_empty);
       
        pthread_mutex_unlock(lock);
        usleep(PRODUCER_SLEEP);
    }
    return NULL;
}

void* consumer(void *arg) {
    struct sharedqueue *q = arg;
    pthread_mutex_t *lock = &q->lock;
    pthread_cond_t *not_full = &q->not_full;
    pthread_cond_t *not_empty = &q->not_empty;

    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(lock);
        while (q->count == 0) {
            pthread_cond_wait(not_empty, lock);
        }
        
        int item = q->buf[q->head];
        q->head = (q->head + 1) % CAPACITY;
        q->count--;
        printf("Consumed: %d\n", item);
        pthread_cond_signal(not_full);
       
        pthread_mutex_unlock(lock);
        usleep(CONSUMER_SLEEP);
    }
    return NULL;
}
