#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE 4096

int main() {

    // Create/Open the shared memory obj
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    // Configure the phys size of the seg
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return EXIT_FAILURE;
    }
    
    // Map Call to shared mem obj into virual address
    char *ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }

    // Read and write directly through the returned pointer
    const char *message = "Hello from the GPU kernel engineer";
    int written = snprintf(ptr, SHM_SIZE, "%s: %ld", message, (long)getpid());
    if (written < 0 || (size_t)written >= SHM_SIZE) {
        fprintf(stderr, "message truncated or encoding err\n");
        return EXIT_FAILURE;
    }

    printf("Producer wrote: %s\n", ptr);

    printf("Press enter after running the consumer to clean up\n");
    getchar();

    // Unmap to remove the mem mapping
    if (munmap(ptr, SHM_SIZE) == -1) {
        perror("munmap failed");
        return EXIT_FAILURE;
    }
    // Close the shm_fd
    close(shm_fd);
    
    //Delete to remove object name from the system
    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink failed");
        return EXIT_FAILURE;
    }  
    
    printf("Shared memory cleaned up successfully.\n");
    return EXIT_SUCCESS;
}