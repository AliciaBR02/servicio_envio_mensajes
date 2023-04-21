#ifndef OPERACIONES_H
#define OPERACIONES_H

    int registration(char *alias, int socket);
    int unregistration(char *alias);
    int connection(char *alias, char *port_and_ip);

    typedef struct {
        char *nombre;
        char *alias;
        char *fecha_nac;
        char *estado;
        char *ip;
        char *puerto;
    } user_t;
#endif