/*
* client.c -- a stream socket client. Together with the server, they make a real-time chat server 
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


#define PORT "3490" // the port users will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int active = 1;
    char input[MAXDATASIZE];
    
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all results and connect to first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("client: connecting to %s.\nOnce connected, type messages and Enter (type 'exit' to quit).\n\n", s);

    freeaddrinfo(servinfo);

    while (active) { // main chat loop
        memset(buf, 0, sizeof(buf));
        numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
        if (numbytes > 0) {
            buf[numbytes] = '\0';
        }
        if (numbytes <= 0) {
            if (numbytes == 0) {
                fprintf(stderr, "Error: Lost connection to server\n");
                exit(1);
            } else {
                perror("recv");
            }
            active = 0;
            continue;
        }
        if (strcmp("You're connected!", buf) != 0) {
            printf("Server: %s\n", buf);
        } else {
            printf("%s\n", buf);
        }
        //continue;
        // get input and send to server:
        printf("You: ");
        fgets(input, sizeof(input), stdin); // fgets is a blocking call, so there are better way to implement chat server

        input[strcspn(input, "\n")] = 0; // find nl, replace with null terminator

        if (strcmp(input, "exit") == 0) {
            if (send(sockfd, "exit", 4, 0) == -1) {
                perror("send");
            }

            close(sockfd);
            
            printf("Connection closed.\n");
            exit(0);
        }

        if (send(sockfd, input, strlen(input), 0) == -1) {
            perror("send");
            active = 0;
        }
        
        // send unique message to server, or exit by saying exit  
        
    }
    return 0;
}
