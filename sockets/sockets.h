#ifndef SOCKETS_H
#define SOCKETS_H

    #include <unistd.h>
    #include <stdint.h>

    int sendMessage(int socket, char *buffer, int len);
    int recvMessage(int socket, char *buffer, int len);
    ssize_t readLine(int fd, void *buffer, size_t n);

    typedef struct petition
    {
        char op[256]; // operation
        char ip[256]; // ip
        int s; // socket descriptor
        
    } petition_t;

#endif