#ifndef OPERACIONES_H
#define OPERACIONES_H

    int registration(char *alias, char *nombre, char *fecha_nac);
    int unregistration(char *alias);
    int connection(char *alias, char *port_and_ip, int socket);
    int disconnection(char *alias);
    int send_message(char *from, char *to, char *message, int socket);
    int send_message_to_receiver(char *receiver, char *sender);
    int connected_users(char *user, int socket);

    typedef struct {
        char *nombre;
        char *alias;
        char *fecha_nac;
        char *estado;
        char *ip;
        char *puerto;
    } user_t;

    typedef struct {
        int id;
        char from[256];
        char message[256];
    } message_t;

#endif