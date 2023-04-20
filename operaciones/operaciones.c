#include "operaciones.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int search_user(char *buffer) {
    // user is until the first newline
    FILE *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    char *user = strtok(buffer, "\n");
    char *filename = malloc(strlen(user) + 4);
    sprintf(filename, "%s.txt", user);

    f = fopen(filename, "r");
    if (f == NULL) {
        perror("fileopen: read\n");
        return -1;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        if (strstr(line, user)) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    free(line);
    free(filename);
    return 0;
}

int register_user(char *buffer) {
    FILE *f;
    user_t user_data;

    sscanf(buffer, "%s\n%s\n%s\n", user_data.nombre, user_data.alias, user_data.fecha_nac);

    char *filename = malloc(strlen(user_data.nombre) + 4);
    sprintf(filename, "%s.txt", user_data.nombre);

    f = fopen(filename, "a");
    if (f == NULL) {
        perror("server-fileopen: write\n");
        return 2;
    }

    fprintf(f, "Nombre: %s \n\tAlias: %s \n\tFecha de nacimiento: %s\n\n", user_data.nombre, user_data.alias, user_data.fecha_nac);
    
    fclose(f);
    free(filename);
    return 0;
}

int registration(char *buffer, int len) {
    int err;
    
    // ver si el usuario ya existe
    err = search_user(buffer);
    if (err == 1) {
        printf("Error: user already exists\n");
        return 1;
    }
    // de lo contrario, registrarlo
    err = register_user(buffer);
    return err;
}