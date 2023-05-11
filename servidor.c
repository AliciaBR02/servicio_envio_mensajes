#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include "sockets/sockets.h"
#include "operaciones/operaciones.h"

// threads and mutexes' global variables
pthread_cond_t cond_message;
pthread_mutex_t mutex_message;
int not_message_copied = 1;


// function to procSess the message and execute the requested operation
void process_message(petition_t *pet) {
    petition_t pet_local = *pet;
    // copy the descriptor to a local variable
    int s_local;
    pthread_mutex_lock(&mutex_message);
    // copy the message to the global variable
    s_local = pet_local.s;
    not_message_copied = 0;
    pthread_cond_signal(&cond_message);
    pthread_mutex_unlock(&mutex_message);

    int err, res;
    char *buffer = malloc(50); // to read the length of the strings
    int length;
    
    if (strcmp("REGISTER",  pet_local.op) == 0) {
        // receive length
        err = readLine(s_local, buffer, sizeof(uint16_t));
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        length = atoi(buffer);
        dprintf(1, "length: %d\n", length);
        char *user = malloc(length + 1);

        // receive alias
        err = readLine(s_local, user, 257);
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        dprintf(1, "alias: %s\n", user);
        res = registration(user, s_local);
        free(buffer);
        free(user);

    } else if (strcmp("UNREGISTER", pet_local.op) == 0) { 
        err = readLine(s_local, buffer, sizeof(uint16_t));
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        length = atoi(buffer);
        char *user = malloc(length + 1);
        // receive alias
        err = readLine(s_local, user, length + 1);
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        dprintf(1, "alias: %s\n", user);
        res = unregistration(user);
        free(buffer);
        free(user);

    } else if (strcmp("CONNECT", pet_local.op) == 0) {
        // alias
        err = readLine(s_local, buffer, sizeof(uint16_t));
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        length = atoi(buffer);
        char *user = malloc(length + 1);
        // receive alias
        err = readLine(s_local, user, length + 1);
        if (err == -1) {
            perror("recv");
            close(s_local);
            free(buffer);
            pthread_exit(NULL);
        }
        dprintf(1, "alias: %s\n", user);

        char *port_and_ip = malloc(10);
        err = readLine(s_local, port_and_ip, 10);
        if (err == -1) {
            perror("recv");
            close(s_local);
            pthread_exit(NULL);
        }
        // add a zero
        strcat(pet_local.ip, "\0");
        // add pet.ip to port_and_ip
        strcat(pet_local.ip, port_and_ip);
        dprintf(1, "port and ip: %s\n", pet_local.ip);
        res = connection(user, pet_local.ip, s_local);
        free(user);
        free(port_and_ip);
        free(buffer);

    } else if (strcmp("DISCONNECT", pet_local.op) == 0) {

        char *user = malloc(256);
        // receive alias
        err = readLine(s_local, user, 257);
        if (err == -1) {
            perror("recv");
            close(s_local);
            pthread_exit(NULL);
        }
        dprintf(1, "alias: %s\n", user);
        res = disconnection(user);
        free(user);
    //} else if (strcmp("CONNECTEDUSERS", pet_local.op) == 0) {
    //    res = connected_users(pet_local.user);
    } else if (strcmp("SEND_MESSAGE", pet_local.op) == 0) {

        char *user = malloc(256);
        char *receiver = malloc(256);
        char *message = malloc(256);
        // receive alias
        err = readLine(s_local, user, 257);
        if (err == -1) {
            perror("recv");
            close(s_local);
            pthread_exit(NULL);
        }
        dprintf(1, "alias: %s\n", user);
        // receive receiver
        err = readLine(s_local, receiver, 257);
        if (err == -1) {
            perror("recv");
            close(s_local);
            pthread_exit(NULL);
        }
        dprintf(1, "receiver: %s\n", receiver);
        // receive message
        err = readLine(s_local, message, 257);
        if (err == -1) {
            perror("recv");
            close(s_local);
            pthread_exit(NULL);
        }
        dprintf(1, "message: %s\n", message);
        res = send_message(user, receiver, message, s_local);
        free(user);
        free(receiver);
        free(message);
    } else {
        res = -1;
    }
    dprintf(1, " end result: %d\n", res);
    // send the result to the client
    err = sendMessage(s_local, (char *)&res, sizeof(char));
    if (err == -1) {
        perror("server: sendmsg");
        close(s_local);
        pthread_exit(NULL);
    }
    close(s_local);
    pthread_exit(NULL);
}

// ESTRUCTURA SOCKET TCP DEL SERVIDOR
// 1- Abrir el socket
// 2- Asignarle una dirección
// 3- Escuchar en el socket
// 4- Aceptar conexiones
// 5- Recibir datos
// 6- Enviar datos
// 7- Cerrar el socket

int main(int argc, char *argv[]) {
    // auxiliar variablesserver_addr.sin_addr.s_addr
    int err;
    ssize_t err2;

    struct sockaddr_in server_addr,  client_addr;
    socklen_t size;
    int sd; // socket descriptor of the server
    int sc; // socket connection with client
    char *port;

    char buffer[1024];

    if (argc != 3) {
        printf("Usage: %s -p <port>\n", argv[0]);
        return -1;
    }
    port = argv[2]; 

    // we create the socket
    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("SERVER: Error en el socket");
        return (0);
    }
    
    int optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(atoi(port));

    // we assign the server address to the socket created
    err = bind(sd, (const struct sockaddr *)&server_addr,
			sizeof(server_addr));
	if (err == -1) {
		perror("bind");
        close(sd);
		return -1;
	}
    

    size = sizeof(client_addr);

     // ***concurrency***

    pthread_t thread;
    pthread_attr_t attr;

    // initialize the mutex and condition variables
    pthread_mutex_init(&mutex_message, NULL);
    pthread_cond_init(&cond_message, NULL);

    pthread_attr_init(&attr);
    // attributes => independent threads
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // socket starts listening
    err = listen(sd, SOMAXCONN);
	if (err == -1) {
		perror("listen");
        close(sd);
		return -1;
	}

    petition_t pet;
    // infinite loop waiting for requests


    
    while (1) {
        dprintf(1, "waiting for conection...\n");
        // accept connection
        sc = accept(sd, (struct sockaddr *) &client_addr, &size);
        if (sc == -1) {
            perror("accept");
            close(sc);
            return -1;
        }
        dprintf(1, "conection accepted\n");
        dprintf(1, "client ip: %s\n", inet_ntoa(client_addr.sin_addr));

        // receive data
        // first parameter
        err = readLine(sc, pet.op, 257);
        if (err == -1) {
            perror("recv");
            close(sd);
            close(sc);
            return -1;
        }
        dprintf(1, "operation: %s\n", pet.op);
        pet.s = sc;
        // we copy the ip of the client
        strcpy(pet.ip, inet_ntoa(client_addr.sin_addr));
        // add a newline
        strcat(pet.ip, "\n");
        dprintf(1, "ip: %s\n", pet.ip);
        // then we process the message
        pthread_create(&thread, &attr, (void *)process_message, (void *)&pet);

        // mutex for copying msg and socket descriptor of the client
        pthread_mutex_lock(&mutex_message);
        while (not_message_copied == 1) {
            pthread_cond_wait(&cond_message, &mutex_message);
        }
        not_message_copied = 1;
        pthread_mutex_unlock(&mutex_message);
        

    }
    // close socket
    close(sd);
    return 0;

}