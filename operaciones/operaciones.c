#include "operaciones.h"
#include "../sockets/sockets.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int search_user(char *alias) {
    // user is until the first newline

    char *filename = malloc(256);
    sprintf(filename, "database/%s.txt", alias);
    if (access(filename, F_OK) != -1) {
        free(filename);
        return 1;
    }

    free(filename);
    return 0;
}

int register_user(char *nombre, char *alias, char *fecha_nac) {
    FILE *f, *f2;
    char *filename = malloc(256);
    char *namedatabase = "database/users.txt";
    sprintf(filename, "database/%s.txt", alias);
    // create the file if it doesn't exist
    f = fopen(filename, "a");
    if (f == NULL) {
        perror("server-fileopen: write\n");
        free(filename);
        return 2;
    }

    fprintf(f, "Nombre: %s \nAlias: %s \nFecha de nacimiento: %s\nEstado: desconectado\nIP:\nPuerto:\nMensajes:\n", nombre, alias, fecha_nac);
    
    f2 = fopen(namedatabase, "a");
    if (f2 == NULL) {
        perror("server-fileopen: write\n");
        free(filename);
        return 2;
    }
    fprintf(f2, "%s\t%s\n", nombre, alias);

    fclose(f2);
    fclose(f);
    free(filename);
    return 0;
}

int registration(char *nombre, int socket) {
    int res, err;
    // ver si el usuario ya existe

    res = search_user(nombre);
    // solicitamos el resto de datos o enviamos un error
    err = sendMessage(socket, (char *)&res, sizeof(int));
    if (err == 1) {
        perror("server: sendmsg");
        return err;
    }
    if (res == 1) return res;
    // de lo contrario, registrarlo
    char *alias = malloc(256);
    char *fecha_nac = malloc(256);
    err = readLine(socket, alias, 256);
    if (err == -1) {
        perror("server: readLine");
        free(alias);
        free(fecha_nac);
        return 2;
    }
    err = readLine(socket, fecha_nac, 256);
    if (err == -1) {
        perror("server: readLine");
        free(alias);
        free(fecha_nac);
        return 2;
    }
    err = register_user(nombre, alias, fecha_nac);
    free(alias);
    free(fecha_nac);
    return err;
}

int delete_user(char *alias) {
    int err;
    char *filename = malloc(256);
    sprintf(filename, "database/%s.txt", alias);
    err = remove(filename);
    if (err == -1) {
        perror("server: remove");
        free(filename);
        return 2;
    }
    free(filename);
    return 0;
}
int delete_alias(char *alias) {
    int err;
    FILE *f, *f2;
    char *namedatabase = "database/users.txt";
    char *temp = "database/temp.txt";
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *token;
    char *name;
    char *alias2;


    f = fopen(namedatabase, "r");
    f2 = fopen(temp, "w");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        return 2;
    }
    if (f2 == NULL) {
        perror("server-fileopen: write\n");
        return 2;
    }


    while ((read = getline(&line, &len, f)) != -1) {
        token = strtok(line, "\t");
        name = token;
        token = strtok(NULL, "\n");
        alias2 = token;
        if (strcmp(alias, alias2) != 0) {
            fprintf(f2, "%s\t%s\n", name, alias2);
        }
    }

    fclose(f);
    fclose(f2);
    free(line);

    err = remove(namedatabase);
    if (err == -1) {
        perror("server: remove");
        return 2;
    }
    err = rename(temp, namedatabase);
    if (err == -1) {
        perror("server: rename");
        return 2;
    }
    return 0;

}
int unregistration(char *alias) {
    // search the user in the database
    int err = search_user(alias);
    if (err == 0) return err; // if user isn't in the database, return error
    // delete the file of the user
    err = delete_alias(alias);
    if (err != 0) return err;
    // delete the user from the users database
    err = delete_user(alias);
    if (err != 0) return err;
    return 0;

}
int change_state(char *alias, int state, char *ip, char *port) {
    FILE *f, *f2;
    char *filename = malloc(256);
    char *tempuser = malloc(256);
    int err;
    sprintf(tempuser, "database/temp_%s.txt", alias);
    sprintf(filename, "database/%s.txt", alias);

    f = fopen(filename, "r+");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        return 2;
    }
    f2 = fopen(tempuser, "w");
    if (f2 == NULL) {
        perror("server-fileopen: write\n");
        free(filename);
        return 2;
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *token;
    char *title;
    char *value;
    char *string = malloc(1024);
    char *connected = "conectado";
    char *disconnected = "desconectado";
    while ((read = getline(&line, &len, f)) != -1) {
        token = strtok(line, ":");
        title = token;
        token = strtok(NULL, ":");
        value = token;
        if (strcmp(title, "Estado") == 0)
            sprintf(string, "%s: %s", title, (state == 1) ? connected : disconnected);
        else if (strcmp(title, "IP") == 0)
            sprintf(string, "%s: %s", title, (state == 1) ? ip : "");
        else if (strcmp(title, "Puerto") == 0)
            sprintf(string, "%s: %s", title, (state == 1) ? port : "");
        else
            sprintf(string, "%s: %s", title, value);
        fprintf(f2, "%s", string);
    }
    err = remove(filename);
    if (err == -1) {
        fclose(f);
        fclose(f2);
        free(line);
        free(string);
        free(filename);
        free(tempuser);
        perror("server: remove");
        return 2;
    }
    err = rename(tempuser, filename);
    if (err == -1) {
        fclose(f);
        fclose(f2);
        free(line);
        free(string);
        free(filename);
        free(tempuser);
        perror("server: rename");
        return 2;
    }
    fclose(f);
    fclose(f2);
    free(line);
    free(filename);
    free(string);
    free(tempuser);
    return 0;
}

int is_connected(char *alias) {
    // open file of the user, and search the word "conectado"
    FILE    *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *filename = malloc(256);
    char *result;

    sprintf(filename, "database/%s.txt", alias);
    
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        result = strstr(line, " conectado\n");
    }
    fclose(f);
    if (result == NULL) {
        free(filename);
        return 0; // usuario NO conectado
    }
    free(filename);
    return 2; // usuario conectado
}
int connection(char *alias, char *port_and_ip) {
    int err;
    // ver si el usuario ya existe
    int res = search_user(alias);
    if (res == 0) return 1;

    // ver si el usuario ya esta conectado
    err = is_connected(alias);
    if (err == 2) return 2;

    // si existe, cambiar el estado a conectado
    char *ip = malloc(16);
    char *port = malloc(5);
    char *token;
    token = strtok(port_and_ip, "\n");
    strcpy(port, token);
    token = strtok(NULL, "\n");
    strcpy(ip, token);
    err = change_state(alias, 1, ip, port);
    if (err != 0) {
        free(ip);
        free(port);
        free(token);
        return 3;
    }
    free(ip);
    free(port);
    return 0;
}

int disconnection(char *alias) {
    int err;
    // ver si el usuario ya existe
    int res = search_user(alias);
    if (res == 0) return 1;
    // si existe, cambiar el estado a desconectado
    err = is_connected(alias);
    if (err == 2) return err; // usuario no conectado
    err = change_state(alias, 0, "", "");
    if (err != 0) return 3;
    return 0;
}