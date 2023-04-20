#ifndef SOCKETS_H
#define SOCKETS_H

    #include <unistd.h>

    int sendMessage(int socket, char *buffer, int len);
    int recvMessage(int socket, char *buffer, int len);
    ssize_t readLine(int fd, void *buffer, size_t n);

    typedef struct petition
    {
        int op;
        char *buffer;
        int len;
        int s; // socket descriptor
        
    } petition_t;
#endif