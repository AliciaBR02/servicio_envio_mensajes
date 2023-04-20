#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "comunicacion.h"
#include "../sockets/sockets.h"
#include <stdio.h>

struct sockaddr_in server_addr, client_addr;
int sd; // socket descriptor of the server

int set_connection() {
    int err;

    sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == 1) {
		printf("Error en socket\n");
		return -1;
	}
    
   	server_addr.sin_family  = AF_INET;
   	server_addr.sin_port    = htons(8888);
    // server_addr.sin_addr.s_addr = inet_addr(strcmp(ip_tuplas, "localhost") == 0 ? "127.0.0.1" : ip_tuplas);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
	if (err == -1) {
		perror("connect");
		return -1;
	}
    return 0;
}
int client_register(char *nombre, char *alias, char *fecha) {
    int err;
    char *send = malloc(strlen(nombre) + strlen(alias) + strlen(fecha) + 4);
    char recv;

    // establecer conexión con el servidor
    err = set_connection();
    if (err == -1) {
        perror("client: set_connection");
        return -1;
    }

    sprintf(send, "1\n%s\n%s\n%s\n", nombre, alias, fecha); //añade un byte null despues de cada argumento  y el servidor no lo puede leer todo

    err = sendMessage(sd, send, strlen(send) + 1);

    if (err == -1) {
        perror("client: sendmsg");
        return -1;
    }

    // recibir respuesta
    err = recvMessage(sd, &recv, sizeof(recv));
    if (err == -1) {
        perror("client: recvmsg");
        return -1;
    }

    // cerrar conexión
    close(sd);
    free(send);
    return 0; // return respuesta del servidor 
}

int client_unregister(char *alias) {
    int err;
    char *send = malloc(strlen(alias) + 4);
    char recv;

    // establecer conexión con el servidor
    err = set_connection();
    if (err == -1) {
        perror("client: set_connection");
        return -1;
    }

    sprintf(send, "2\n\n%s\n\n", alias); //añade un byte null despues de cada argumento  y el servidor no lo puede leer todo

    err = sendMessage(sd, send, strlen(send) + 1);

    if (err == -1) {
        perror("client: sendmsg");
        return -1;
    }

    // recibir respuesta
    err = recvMessage(sd, &recv, sizeof(recv));
    if (err == -1) {
        perror("client: recvmsg");
        return -1;
    }

    // cerrar conexión
    close(sd);
    free(send);
    return 0; // return respuesta del servidor 
}