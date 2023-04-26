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
        char user[256]; // user
        char port_and_ip[256]; // port
        char receiver[256]; // receiver user
        char message[256]; // message
        int s; // socket descriptor
        
    } petition_t;
#endif