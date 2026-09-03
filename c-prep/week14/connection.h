#ifndef CONNECTION_H
#define CONNECTION_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <pthread.h>

#define SHM_NAME "/my_sharded_queue"
#define PAGE_SIZE 8192

#define CAPACITY 10
#define N 20
#define PRODUCER_SLEEP 10000
#define CONSUMER_SLEEP 3000
#define PORT "8080"
#define MAXDATASIZE 100
#define BACKLOG 10

struct sharedqueue {
    int buf[CAPACITY];
    int head, tail, count;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};

struct Client {
    int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    struct sharedqueue queue;
    int rv;
    char s[INET6_ADDRSTRLEN];
};

struct Server {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; //connector's addr info
    struct sharedqueue queue;
    socklen_t sin_size;
    struct sigaction sa;
    int yes;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[MAXDATASIZE];
};


void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa) ;
int client_init(struct Client *client, int argc, char *argv[]);
int client_run(int argc, char *argv[]);

int server_init(struct Server *s);
int server_run(void);

#endif