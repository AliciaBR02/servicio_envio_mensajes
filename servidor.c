#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>


// ESTRUCTURA SOCKET TCP DEL SERVIDOR
// 1- Abrir el socket
// 2- Asignarle una dirección
// 3- Escuchar en el socket
// 4- Aceptar conexiones
// 5- Recibir datos
// 6- Enviar datos
// 7- Cerrar el socket

int main(int argc, char *argv[]) {
    // auxiliar variables
    int err;
    ssize_t err2;

    struct sockaddr_in server_addr,  client_addr;
    socklen_t size;
    int sd; // socket descriptor of the server
    int sc; // socket connection with client

    // we create the socket
    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("SERVER: Error en el socket");
        return (0);
    }
    
    int optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(8888);

    // we assign the server address to the socket created
    err = bind(sd, (const struct sockaddr *)&server_addr,
			sizeof(server_addr));
	if (err == -1) {
		perror("bind");
        close(sd);
		return -1;
	}
    

    size = sizeof(client_addr);


    // socket starts listening
    err = listen(sd, SOMAXCONN);
	if (err == -1) {
		perror("listen");
        close(sd);
		return -1;
	}

    // infinite loop waiting for requests
    
    while (1) {
        dprintf(1, "waiting for conection...\n");
        // accept connection
        sc = accept(sd, (struct sockaddr *) &client_addr, &size);
        if (sc == -1) {
            perror("accept");
            close(sd);
            return -1;
        }
        dprintf(1, "conection accepted\n");
        // receive data
        int receive;
        err2 = recv(sc, &receive, sizeof(receive), 0);
        if (err2 == -1) {
            perror("recv");
            close(sd);
            return -1;
        }
        dprintf(1, "received: %d\n", receive);

    }
    // close socket
    close(sd);
    return 0;

}