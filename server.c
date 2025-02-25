/*
* server.c -- a stream socket server
*/
#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT "3490" // the port users will be connecting to 
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0); // while loop waits for child processes to terminate

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    int sockfd, new_fd; // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET_ADDRSTRLEN];
    int rv;
    int numbytes;
    char buf[MAXDATASIZE];
    int active = 1;
    const char* responses[] = { // list of possible responses
        "Interesting point. Tell me more!",
        "I've never thought about that before.",
        "That's a great observation!",
        "How long have you felt that way?",
        "Let's discuss that further.",
        "Go on...",
        "I see what you mean.",
        "That's fascinating!",
        "I would love to talk!"
    };
    int num_responses = sizeof(responses) / sizeof(responses[0]); // divide total bytes of array by total bytes of single elem to get number of responses
    srand(time(NULL));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue; 
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
            close(sockfd);
            perror("server: bind");
            continue; // 
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1) { // main accept() loop
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }


        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

        // send initial message, connection confirmation, to client
        if (send(new_fd, "You're connected!", 17, 0) == -1) {
            perror("send");
            continue;
        } 

        if (!fork()) { // this is the child process
            // this is where conversation happens
            close(sockfd); // child doesnt need listener

            while (active) {
                // receive msg from client:
                memset(buf, 0, sizeof(buf));
                numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0);

                // error and disconnection checking 
                if (numbytes <= 0) {
                    if (numbytes == 0) {
                        printf("Client disconnected\n");
                    } else {
                        perror("recv");
                    }
                    active = 0;
                    continue; // skip to next iteration, which ends loop
                }

                
                buf[numbytes] = '\0'; // null terminate receive msg
                printf("server: received '%s'\n", buf);

                // check for exit
                if (strcmp(buf, "exit") == 0) {
                    send(new_fd, "Goodbye!", 8, 0);
                    active = 0;
                    continue;
                }

                // send back random response 
                int random_index = rand() % num_responses;
                if (send(new_fd, responses[random_index], strlen(responses[random_index]), 0) == -1) {
                    perror("send");
                    active = 0;
                }

            }

            close(new_fd);
            exit(0);
        }
    }

    return 0;
}