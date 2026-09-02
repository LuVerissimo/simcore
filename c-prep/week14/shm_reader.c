#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 4096

int main () {

    // get/create
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }
    
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

    return EXIT_SUCCESS;
}