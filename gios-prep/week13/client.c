#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT "8080"
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa) 
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[]) {

    int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop thru all the results and connect to the first possible
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
        printf("client: attempting connection to %s\n", s);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    fprintf(stderr, "client: connected to %s\n", s);

    freeaddrinfo(servinfo); // all done w/ structure

    printf("Type a message to send to the server (Ctrl+D to quit):\n");

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        size_t len = strlen(buf);

        // 1. Send the keyboard input to server
        ssize_t total_sent = 0;
        int send_failed = 0;
        while (total_sent < (ssize_t)len) {
            ssize_t bytes_sent = send(sockfd, buf + total_sent, len - total_sent, 0);
            if (bytes_sent == -1) {
                perror("send");
                send_failed = 1;
                break;
            }
            total_sent += bytes_sent;
        }

        if (send_failed) break;

        memset(buf, 0, sizeof buf);
        ssize_t numbytes = recv(sockfd, buf, sizeof(buf) -1, 0);

        if (numbytes < 0) {
            perror("recv");
            break;
        } else if (numbytes == 0) {
            printf("client: server closed connection\n");
            break;
        }

        buf[numbytes] = '\0';
        printf("client: received '%s'\n", buf);
    }      

    close(sockfd);
    return 0;
}