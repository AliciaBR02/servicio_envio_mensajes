#ifndef SOCKETS_H
#define SOCKETS_H

    #include <unistd.h>
    #include <stdint.h>

    int sendMessage(int socket, char *buffer, int len);
    int recvMessage(int socket, char *buffer, int len);
    ssize_t readLine(int fd, void *buffer, size_t n);

    typedef struct petition
    {
        char op[256];
        char string[256];
        char string2[256];
        int s; // socket descriptor
        
    } petition_t;
#endif