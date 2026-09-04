#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

#include <pthread.h>

#define SHM_NAME "/my_sharded_queue"
#define PAGE_SIZE 8192

#define CAPACITY 10
#define N 20
#define PRODUCER_SLEEP 10000
#define CONSUMER_SLEEP 3000

struct sharedqueue {
    int buf[CAPACITY];
    int head, tail, count;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};

// Declaring the engines so reader/writer can import them
void sharedqueue_init_shm(struct sharedqueue *q);
void* producer(void *arg);
void* consumer(void *arg);

#endif