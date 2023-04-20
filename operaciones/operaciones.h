#ifndef OPERACIONES_H
#define OPERACIONES_H

    int registration(char *buffer, int len);

    typedef struct {
        char *nombre;
        char *alias;
        char *fecha_nac;
        char *estado;
        char *ip;
        char *puerto;
    } user_t;
#endif