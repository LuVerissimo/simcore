#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include "connection.h"

void sigchld_handler(int s) 
{
    (void)s;
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) 
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

long elapsed_ns(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000000000L) + (end.tv_nsec - start.tv_nsec);
}


void c_sharedqueue_init_shm(struct sharedqueue *q) {
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

void* c_producer(void *arg) {
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

void* c_consumer(void *arg) {
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

/* ========================================================================== // 
                        CLIENT LOGIC                               
// ========================================================================== */

int client_init(struct Client *client, int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    memset(&client->hints, 0, sizeof client->hints);
    client->hints.ai_family = AF_UNSPEC;
    client->hints.ai_socktype = SOCK_STREAM;

    if ((client->rv = getaddrinfo(argv[1], PORT, &client->hints, &client->servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(client->rv));
        exit(1);
    }
    return 0;
}


int client_shm_queue(struct Client *client) {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    // dynamic alignment
    off_t target_offset = 5000;
    off_t aligned_offset = (target_offset / PAGE_SIZE) * PAGE_SIZE;
    off_t diff = target_offset - aligned_offset;
    size_t map_length = sizeof(client->queue) + diff;
    
    // map the aligned mem window
    char *map  = mmap(0, map_length,  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, aligned_offset);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return EXIT_FAILURE;
    }
    
    struct sharedqueue *queue = (struct sharedqueue *)(map + diff);
    printf("Reader Process: Launching imported consumer engine.");
    
    c_consumer(queue);
    
    munmap(map, map_length);
    close(shm_fd);
    printf("Reader Process: Shared memory object successfully unlinked.\n");
    return EXIT_SUCCESS;
}

int client_run(int argc, char *argv[]) {
    int return_status = 0;
    struct Client client;

    int servinfo_allocated = 0;
    int socket_opened = 0;

    // if (argc > 1 && strcmp(argv[1], "--deadlock") == 0) {
    //     printf("[TEST] Forcing a real client deadlock scenario...\n");
        
    //     pthread_mutex_t deadlock_mutex;
    //     pthread_mutex_init(&deadlock_mutex, NULL);

    //     // First lock acquisition: Success
    //     pthread_mutex_lock(&deadlock_mutex);
    //     printf("[TEST] Acquired lock first time. Attempting second lock to freeze...\n");

    //     // Second lock acquisition on a non-recursive mutex: Infinite Block / Deadlock
    //     pthread_mutex_lock(&deadlock_mutex); 

    //     // This line will NEVER be reached
    //     pthread_mutex_unlock(&deadlock_mutex);
    //     pthread_mutex_destroy(&deadlock_mutex);
    //     return 0;
    // }

    client_init(&client, argc, argv);
    if (client.servinfo != NULL) {
        servinfo_allocated = 1;
    }


    //loop thru all the results and connect to the first possible
    for (client.p = client.servinfo; client.p != NULL; client.p = client.p->ai_next) {
        if ((client.sockfd = socket(client.p->ai_family, client.p->ai_socktype, client.p->ai_protocol)) == -1) {
            perror("client: socket");
            return_status = 1;
            goto cleanup;
        }

        socket_opened = 1;

        inet_ntop(client.p->ai_family, get_in_addr((struct sockaddr *)client.p->ai_addr), client.s, sizeof client.s);
        printf("client: attempting connection to %s\n", client.s);

        if (connect(client.sockfd, client.p->ai_addr, client.p->ai_addrlen) == -1) {
            perror("client: connect");
            close(client.sockfd);
            socket_opened = 0;
            continue;
        }

        break;
    }

    if (client.p == NULL) {
        fprintf(stderr, "client: failed to connect");
        return_status = 3;
        goto cleanup;
    }

    inet_ntop(client.p->ai_family, get_in_addr((struct sockaddr *)client.p->ai_addr), client.s, sizeof client.s);
    fprintf(stderr, "client: connected to %s\n", client.s);
    freeaddrinfo(client.servinfo);

    printf("Type a message to send to the server (Ctrl+D to quit):\n");
    while (fgets(client.buf, sizeof client.buf, stdin) != NULL) {
        size_t len = strlen(client.buf);
        ssize_t total_sent = 0;

        while (total_sent < (ssize_t)len) {
            ssize_t bytes_sent = send(client.sockfd, client.buf + total_sent, len - total_sent, 0);
            if (bytes_sent == -1) {
                perror("send");
                return_status = 4;
                goto cleanup;
            }
            total_sent += bytes_sent;
        }

        memset(client.buf, 0, sizeof client.buf);
        ssize_t numbytes = recv(client.sockfd, client.buf, sizeof(client.buf) -1, 0);
        
        if (numbytes < 0) {
            perror("recv");
            return_status = 6;
            goto cleanup;
        } else if (numbytes == 0) {
            printf("client: server closed connection\n");
            return_status = 7;
            goto cleanup;
        }

        client.buf[numbytes] = '\0';
        printf("client: received '%s'\n", client.buf);

        // queue and buf handler
        if (client_shm_queue(&client) != 0) {
            fprintf(stderr, "Failed to push network data into shared memory queue\n");
            return_status = 8;
            goto cleanup;
        }
    }
    
    return_status = EXIT_SUCCESS;
    goto cleanup;

cleanup:
    if (socket_opened) {
        close(client.sockfd);
    }
    if (servinfo_allocated) {
        freeaddrinfo(client.servinfo);
    }

    return return_status;
}


/* ========================================================================== // 
                        SERVER LOGIC                               
// ========================================================================== */

int server_init(struct Server *server) {
    memset(&server->hints, 0, sizeof server->hints);
    server->hints.ai_family = AF_INET;
    server->hints.ai_socktype = SOCK_STREAM;
    server->hints.ai_flags = AI_PASSIVE; // use my IP
    server->yes=1;

    if ((server->rv = getaddrinfo(NULL, PORT, &server->hints, &server->servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(server->rv));
        return 1;
    }
    return 0;
}

int server_shm_queue(struct Server *server) {
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
    size_t map_length = sizeof(server->queue) + diff;
    
    // Map Call to shared mem obj into virual address
    char *map = mmap(0, PAGE_SIZE,  PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, aligned_offset);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return EXIT_FAILURE;
    }

    struct sharedqueue *queue = (struct sharedqueue *)(map + diff);

    // boilerplate
    c_sharedqueue_init_shm(queue);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    //producer logic
    printf("Writer Process: Launching imported producer engine.\n");
    c_producer(queue);

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

int server_run(void){
    int return_status = 0;
    struct Server server;
    server_init(&server);

    int servinfo_allocated = (server.servinfo != NULL); 
    int socket_opened = 0;

    for(server.p = server.servinfo; server.p != NULL; server.p = server.p->ai_next) {
        if ((server.sockfd = socket(server.p->ai_family, server.p->ai_socktype, server.p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(server.sockfd, SOL_SOCKET, SO_REUSEADDR, &server.yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            return_status = 1;
            goto cleanup;
        }

        if (bind(server.sockfd, server.p->ai_addr, server.p->ai_addrlen) == -1) {
            close(server.sockfd);
            socket_opened = 0;
            perror("server: bind");
            continue;
        }

        break;
    }
    socket_opened = 1;

    
    if (server.p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return_status = 1;
        goto cleanup;
    }
    freeaddrinfo(server.servinfo); // all done with this structure
    servinfo_allocated = 0;

    if (listen(server.sockfd, BACKLOG) == -1) {
        perror("listen");
        return_status = 1;
        goto cleanup;
    }

    server.sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&server.sa.sa_mask);
    server.sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &server.sa, NULL) == -1) {
        perror("sigaction");
        return_status = 1;
        goto cleanup;
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        server.sin_size = sizeof server.their_addr;
        server.new_fd = accept(server.sockfd, (struct sockaddr *)&server.their_addr,
            &server.sin_size);
        if (server.new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(server.their_addr.ss_family,
            get_in_addr((struct sockaddr *)&server.their_addr),
            server.s, sizeof server.s);
        printf("server: got connection from %s\n", server.s);

        if (!fork()) { // this is the child process
            close(server.sockfd);
            
            for (;;) {
                ssize_t n = recv(server.new_fd, server.buf, sizeof server.buf, 0);
                printf("server: echoed %zd bytes: %.*s", n, (int)n, server.buf);

                if (n < 0) {
                    perror("recv");
                    break;
                } 
                if (n == 0) {
                    break;
                }

                ssize_t total_sent = 0;
                while (total_sent < n) {
                    ssize_t bytes_sent = send(server.new_fd, server.buf + total_sent, n - total_sent, 0);
                    if (bytes_sent == -1) {
                        perror("send");
                        goto done;
                    }
                    total_sent += bytes_sent;
                }
                if (server_shm_queue(&server) != 0) {
                    fprintf(stderr, "Failed to push network data into shared memory queue\n");
                }
            }
        done:
            close(server.new_fd);
            exit(0);
        }
        close(server.new_fd);  // parent doesn't need this
    }

    return_status = EXIT_SUCCESS;
    goto cleanup;

cleanup:
    if (socket_opened) {
        close(server.sockfd);
    }
    if (servinfo_allocated) {
        freeaddrinfo(server.servinfo);
    }

    return return_status;
}


/* ========================================================================== // 
                             MAIN EXECUTIVE                               
// ========================================================================== */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  To start Server: %s server\n", argv[0]);
        fprintf(stderr, "  To start Client: %s <server_hostname>\n", argv[0]); 
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "server") == 0) {
        return server_run();
    } else {
        return client_run(argc, argv);
    }
}