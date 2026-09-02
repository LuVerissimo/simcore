#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int CAPACITY = 10;
int N = 20;
int NUM_THREADS = 2;
int PRODUCER_SLEEP = 10000;
int CONSUMER_SLEEP = 3000;

long elasped_ns(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000000000L) + (end.tv_nsec - start.tv_nsec);
}

struct boundedqueue {
    int buf[10];
    int head, tail, count;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};


void boundedqueue_init(struct boundedqueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void* producer(void *arg) {
    struct boundedqueue *q = arg;
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
    struct boundedqueue *q = arg;
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


int main() {
    struct boundedqueue q;
    struct timespec t0, t1;
    boundedqueue_init(&q);
    pthread_t producer_tid, consumer_tid;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < 2; i++) {
        if (i % 2 == 0)
            pthread_create(&producer_tid, NULL, producer, &q);
        else
            pthread_create(&consumer_tid, NULL, consumer, &q);
    }
    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    
    printf("Elasped Time for Producer: %ld\n", elasped_ns(t0, t1));
}