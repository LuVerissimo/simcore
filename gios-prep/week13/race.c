#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int global_counter = 0;
const int NUM_THREADS = 8;
const int N = 100000;


long elasped_ns(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000000000L) + (end.tv_nsec - start.tv_nsec);
}

void *thread_lock_helper(void *arg) {
    for (int i = 0; i < N; i++) {
        (*(int *)arg)++;
    }
    return NULL;
}


struct thread_info {
    pthread_t thread_id;
    int thread_num;
    char *argv_string;
};

struct Thread_mutex_info {
    pthread_mutex_t * mutex;
    void *data;
};

void* mutex_lock_helper(void *arg) {
    struct Thread_mutex_info *args = arg;
    pthread_mutex_t *mutex = args->mutex;
    int *counter = args->data;
    
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(mutex);
        (*counter)++;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}


int main() {
    struct timespec t0, t1, t2, t3;
    pthread_t threads[NUM_THREADS];

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    
    struct Thread_mutex_info args;
    args.mutex = &lock;
    args.data = &global_counter;
    
    
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_lock_helper, &global_counter);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("global_counter = %d (expected: %d) with time:  %ldns\n", global_counter, NUM_THREADS * N, elasped_ns(t0, t1));
    
    global_counter = 0;
    clock_gettime(CLOCK_MONOTONIC, &t2);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mutex_lock_helper, &args);
    
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t3);

    printf("global_counter = %d (expected: %d) with time:  %ldns\n", global_counter, NUM_THREADS * N, elasped_ns(t2, t3));

    pthread_mutex_destroy(&lock);
    return 0;
}