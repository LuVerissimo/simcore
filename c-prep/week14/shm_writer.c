#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include "sharedqueue.h"


long elapsed_ns(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000000000L) + (end.tv_nsec - start.tv_nsec);
}

int main() {
    // Create/Open the shared memory obj
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed\n");
        return EXIT_FAILURE;
    }

    // Configure the phys size of the seg
    if (ftruncate(shm_fd, PAGE_SIZE) == -1) {
        perror("ftruncate failed\n");
        return EXIT_FAILURE;
    }

    off_t target_offset = 5000;
    off_t aligned_offset = (target_offset / PAGE_SIZE) * PAGE_SIZE;
    off_t diff = target_offset - aligned_offset;
    size_t map_length = sizeof(struct sharedqueue) + diff;
    
    // Map Call to shared mem obj into virual address
    char *map = mmap(0, PAGE_SIZE,  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, aligned_offset);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    struct sharedqueue *q = (struct sharedqueue *)(map + diff);

    // boilerplate
    sharedqueue_init_shm(q);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    //producer logic
    printf("Writer Process: Launching imported producer engine.\n");
    producer(q);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Elapsed Time for Producer Process: %ld ns\n", elapsed_ns(t0, t1));

    // Unmap to remove the mem mapping
    if (munmap(map, map_length) == -1) {
        perror("munmap failed\n");
        return EXIT_FAILURE;
    }

    // Close the shm_fd
    close(shm_fd);    
    printf("Shared memory cleaned up successfully.\n");
    return EXIT_SUCCESS;
}