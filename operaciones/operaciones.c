#include "operaciones.h"
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

    fprintf(f, "Nombre: %s \nAlias: %s \nFecha de nacimiento: %s\n\n", nombre, alias, fecha_nac);
    
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

int registration(char *nombre, char *alias, char *fecha_nac) {
    int err;
    // ver si el usuario ya existe
    err = search_user(alias);
    if (err == 1) return err;
    // de lo contrario, registrarlo
    err = register_user(nombre, alias, fecha_nac);
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
    if (err == 0) return err;
    
    // delete the file of the user
    err = delete_alias(alias);
    if (err != 0) return err;
    // delete the user from the users database
    err = delete_user(alias);
    if (err != 0) return err;
    return 0;

}