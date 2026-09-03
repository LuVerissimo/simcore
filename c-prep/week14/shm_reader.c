<<<<<<< HEAD
#include <pthread.h>
=======
>>>>>>> ec8c9f9d299c93c68f39a22106b54a0e9608f1a8
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
<<<<<<< HEAD
#include "sharedqueue.h"

int main () {
    // open shared mem seg
=======

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 4096

int main () {

    // get/create
>>>>>>> ec8c9f9d299c93c68f39a22106b54a0e9608f1a8
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }
<<<<<<< HEAD

    // dynamic alignment
    off_t target_offset = 5000;
    off_t aligned_offset = (target_offset / PAGE_SIZE) * PAGE_SIZE;
    off_t diff = target_offset - aligned_offset;
    size_t map_length = sizeof(struct sharedqueue) + diff;
    
    // map the aligned mem window
    char *map  = mmap(0, PAGE_SIZE,  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, aligned_offset);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return EXIT_FAILURE;
    }
    
    // consumer logic
    struct sharedqueue *q = (struct sharedqueue *)(map + diff);
    printf("Reader Process: Launching imported consumer engine.");
    consumer(q);
    
    munmap(map, map_length);
    close(shm_fd);
    
    printf("Reader Process: Shared memory object successfully unlinked.\n");
=======
    
    // attach
    char *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }

    // use 
    printf("Consumer read: %s\n", ptr);

    // detach
    if (munmap(ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        return EXIT_FAILURE;
    }

    // control / delete
    close(shm_fd);

>>>>>>> ec8c9f9d299c93c68f39a22106b54a0e9608f1a8
    return EXIT_SUCCESS;
}