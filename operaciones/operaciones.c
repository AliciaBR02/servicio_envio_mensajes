#include "operaciones.h"
#include "../sockets/sockets.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int search_user(char *alias) {
    // user is until the first newline

    char *filename = malloc(strlen(alias) + 14);
    sprintf(filename, "database/%s.txt", alias);
    if (access(filename, F_OK) != -1) {
        free(filename);
        return 1;
    } // if the user doesn't exist, return 0

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
    int length;
    char *buffer = malloc(20);
    // ver si el usuario ya existe

    res = search_user(nombre);
    // solicitamos el resto de datos o enviamos un error
    err = sendMessage(socket, (char *)&res, sizeof(int));
    if (err == 1) {
        perror("server: sendmsg");
        free(buffer);
        return err;
    }
    if (res == 1) return res;
    // de lo contrario, registrarlo
    err = readLine(socket, buffer, sizeof(int));
    if (err == -1) {
        perror("server: readLine");
        free(buffer);
        return 2;
    }
    length = atoi(buffer);
    char *alias = malloc(length + 1);
    err = readLine(socket, alias, length + 1);
    if (err == -1) {
        perror("server: readLine");
        free(alias);
        free(buffer);
        return 2;
    }
    err = readLine(socket, buffer, sizeof(int));
    if (err == -1) {
        perror("server: readLine");
        free(alias);
        free(buffer);
        return 2;
    }
    length = atoi(buffer);
    char *fecha_nac = malloc(length + 1);
    err = readLine(socket, fecha_nac, length + 1);
    if (err == -1) {
        perror("server: readLine");
        free(alias);
        free(fecha_nac);
        free(buffer);
        return 2;
    }
    dprintf(1, "Nombre: %s\nAlias: %s\nFecha de nacimiento: %s\n", nombre, alias, fecha_nac);
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
int connect_user_database(char *alias) {
    char *connected_database = "database/connected.txt";

    FILE *f;
    f = fopen(connected_database, "a");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        return 2;
    }
    fprintf(f, "%s\n", alias);
    fclose(f);
    return 0;
}

int disconnect_user_database(char *alias) {
    char *connected_database = "database/connected.txt";

    FILE *f, *f2;
    char *tempuser = "database/temp_connected.txt";
    char c;
    int err;
    // copy file except for the line containing alias

    f = fopen(connected_database, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        return 2;
    }
    f2 = fopen(tempuser, "w");
    if (f2 == NULL) {
        perror("server-fileopen: read\n");
        fclose(f);
        return 2;
    }
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') {
            char *line = malloc(256);
            fgets(line, 256, f);
            if (strcmp(line, alias) != 0) {
                fprintf(f2, "%s\n", line);
            }
            free(line);
        } else {
            fputc(c, f2);
        }
    }
    fclose(f);
    fclose(f2);
    err = remove(connected_database);
    if (err == -1) {
        perror("server: remove");
        return 2;
    }
    err = rename(tempuser, connected_database);
    if (err == -1) {
        perror("server: rename");
        return 2;
    }
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
            sprintf(string, "%s: %s\n", title, (state == 1) ? connected : disconnected);
        else if (strcmp(title, "IP") == 0)
            sprintf(string, "%s: %s\n", title, (state == 1) ? ip : "");
        else if (strcmp(title, "Puerto") == 0)
            sprintf(string, "%s: %s\n", title, (state == 1) ? port : "");
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
    dprintf(1, "empezando connection\n");
    dprintf(1, "ALIAS: %s\n", alias);
    int err;
    // ver si el usuario no existe
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
    dprintf(1, "antes de changestate\n");
    err = change_state(alias, 1, ip, port);
    if (err != 0) {
        free(ip);
        free(port);
        free(token);
        return 3;
    }
    dprintf(1, "despues de changestate\n");
    free(ip);
    free(port);
    dprintf(1, "antes de connect_user_database\n");
    err = connect_user_database(alias);
    return err;
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
    err = disconnect_user_database(alias);
    return err;
}

int count_connected() {
    char *filename = "database/connected.txt";
    FILE *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    //open file
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        count += 1;
    }
    fclose(f);
    return count;

}

int read_connected() {
    
}

int connected_users(char *alias) {
    int err, res;
    //comprobar si está registrado, si no return 2
    err = search_user(alias);
    if (err == 0) return 2;
    //comprobar si está conectado, si no return 1
    err = is_connected(alias);
    if (err == 0) return 1;
    //count_connected, read_connected, return 0
    res = count_connected();
}

int get_message_id(char *to) {
    FILE *f;
    char *filename = malloc(256);
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int id = 0;

    sprintf(filename, "database/%s.txt", to);
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        free(line);
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        if (isdigit(line[0])) {
            id++;
        }
    }
    fclose(f);
    free(filename);
    free(line);
    return id;
}
int save_message(int id, char *from, char *to, char *message) {
    FILE *f, *f2;
    int len_to = strlen(to);
    char *filename = malloc(len_to + 13);
    char *tempuser = malloc(strlen(to) + 19);
    char c;
    int err;

    sprintf(tempuser, "database/temp_%s.txt", to);
    sprintf(filename, "database/%s.txt", to);

    f = fopen(filename, "r+");
    if (f == NULL) {
        perror("server-fileopen\n");
        free(filename);
        free(tempuser);
        return 2;
    }
    f2 = fopen(tempuser, "w");
    if (f2 == NULL) {
        perror("server-fileopen\n");
        free(filename);
        free(tempuser);
        fclose(f);
        return 2;
    }

    while ((c = fgetc(f)) != EOF) {
        fputc(c, f2);
    }
    fprintf(f2, "%d: %s %s\n", id, from, message);
    
    err = remove(filename);
    if (err == -1) {
        fclose(f);
        fclose(f2);
        free(filename);
        free(tempuser);
        perror("server: remove");
        return 2;
    }
    err = rename(tempuser, filename);
    if (err == -1) {
        fclose(f);
        fclose(f2);
        free(filename);
        free(tempuser);
        perror("server: rename");
        return 2;
    }
    fclose(f);
    fclose(f2);
    free(filename);
    free(tempuser);    
    return 0;
}
int send_message(char *from, char *to, char *message, int socket) {
    // check if the users exist
    int err, err2, res;
    err = search_user(from);
    err2 = search_user(to);
    if (err != 0 && err2 != 0) {
        // get last message id
        err = get_message_id(to);
        // save message
        err = save_message(err, from, to, message);
        if (err != 0) return 2;
        res = 0;
    } else {
        res = 1;
    }
    // send id to client
    err = sendMessage(socket, (char *)&err, sizeof(int));
    if (err != 0) return 2;

    return res;
}

