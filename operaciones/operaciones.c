#include "operaciones.h"
#include "../sockets/sockets.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

int registration(char *nombre, char *alias, char *fecha_nac) {
    int res, err;
    // ver si el usuario ya existe

    res = search_user(nombre);
    // solicitamos el resto de datos o enviamos un error
    if (res == 1) return res;
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
    char *tempuser = "database/temp_connected.txt";
    char *line = malloc(256);
    size_t len = 0;
    ssize_t read;
    FILE *f, *f2;
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
    while ((read = getline(&line, &len, f)) != -1) {
        if (strstr(line, alias) == NULL) {
            fprintf(f2, "%s", line);
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
        fclose(f);
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
    int result = 0;

    sprintf(filename, "database/%s.txt", alias);
    
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        if (strstr(line, " conectado") != NULL){
            result = 1;
            break;
        }
    }
    fclose(f);
    if (result == 0) {
        free(filename);
        return 0; // usuario NO conectado
    }
    free(filename);
    return 2; // usuario conectado
}

int establish_connection(char *ip, char *port) {
    int err, res;
    int sd;
    struct sockaddr_in server_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == 1) {
		printf("Error en socket\n");
		return -1;
	}
    
   	server_addr.sin_family  = AF_INET;
   	server_addr.sin_port    = htons(atoi(port));
    server_addr.sin_addr.s_addr = inet_addr(ip);
    err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
	if (err == -1) {
		perror("connect");
		return -1;
	}
    return sd;
}

int create_socket(char *user) {
    // open its file, and get ip and port
    char *ip = malloc(16);
    char *port = malloc(5);
    int length = strlen(user);
    char *filename = malloc(length + 14);
    FILE *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int socket;
    int err;

    sprintf(filename, "database/%s.txt", user);
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(ip);
        free(port);
        free(filename);
        return 2;
    }

    while (read = getline(&line, &len, f) != -1) {
        if (strstr(line, "IP") != 0) {
            sscanf(line, "IP: %s", ip);
        }
        if (strstr(line, "Puerto:") != 0) {
            sscanf(line, "Puerto: %s", port);
        }
    }
    socket = establish_connection(ip, port);
    if (socket == -1) {
        free(ip);
        free(port);
        free(filename);
        fclose(f);
        perror("server: establish_connection");
        return -1;
    }
    free(ip);
    free(port);
    free(filename);
    fclose(f);
    return socket;
}

int messages_connected_user(char *receiver) {
    int err, res;
    char *filename = malloc(strlen(receiver) + 14);
    char *tempuser = malloc(strlen(receiver) + 19);
    sprintf(filename, "database/%s.txt", receiver);
    sprintf(tempuser, "database/temp_%s.txt", receiver);
    FILE *f, *f2;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        free(tempuser);
        return 2;
    }
    f2 = fopen(tempuser, "w");
    if (f2 == NULL) {
        perror("server-fileopen: write\n");
        free(filename);
        free(tempuser);
        fclose(f);
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        fprintf(f2, "%s", line);
        if (strstr (line, "Mensajes:") != NULL) {
            break;
        }
    }
    // check message by message if the sender is connected
    message_t *message = malloc(sizeof(message_t));
    while ((read = getline(&line, &len, f)) != -1) {
        sscanf(line, "%d: %s | %s", &message->id, message->from, message->message);
        err = is_connected(message->from);
        if (err == 0) {
            // do nothing
            fprintf(f2, "%s", line);
            continue;
        }
        // send message to receiver
        err = send_message_to_receiver(receiver, message->from);
        if (err != 0) {
            fprintf(f2, "%s", line);
            break;
        }
    }
    while ((read = getline(&line, &len, f)) != -1) {
        fprintf(f2, "%s", line);
    }
    err = remove(filename);
    if (err != 0) {
        free(filename);
        free(tempuser);
        free(line);
        free(message);
        fclose(f);
        fclose(f2);
        return 2;
    }
    err = rename(tempuser, filename);
    if (err != 0) {
        free(filename);
        free(tempuser);
        free(line);
        free(message);
        fclose(f);
        fclose(f2);
        return 2;
    }
    fclose(f);
    fclose(f2);
    return 0;
}
int connection(char *alias, char *port_and_ip, int socket) {
    int err;
    // ver si el usuario no existe
    err = search_user(alias);
    if (err == 0) return err;

    // ver si el usuario ya esta conectado
    err = is_connected(alias);
    if (err == 2) return 2;

    // si existe, cambiar el estado a conectado
    char *ip = malloc(16);
    char *port = malloc(5);
    char *token;
    token = strtok(port_and_ip, "\n");
    strcpy(ip, token);
    token = strtok(NULL, "\0");
    strcpy(port, token);

    /* YA TENEMOS LOS DATOS NECESARIOS */
    
    /* ACTUALIZAR BASE DE DATOS */

    err = change_state(alias, 1, ip, port);
    if (err != 0) {
        free(ip);
        free(port);
        free(token);
        return 3;
    }
    free(ip);
    free(port);
    err = connect_user_database(alias);
    err = messages_connected_user(alias);
    if (err != 0) {
        free(ip);
        free(port);
        return err;
    }
    return err;
}

int disconnection(char *alias) {
    int err;
    // ver si el usuario ya existe
    int res = search_user(alias);
    if (res == 0) return 1;
    // si existe, cambiar el estado a desconectado
    err = is_connected(alias);
    if (err == 0) return 2; // usuario no conectado
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
    return 0;
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
    fprintf(f2, "%d: %s | %s\n", id, from, message);
    
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
    int err, err2, res, id;
    err = search_user(from);
    err2 = search_user(to);
    if (err != 0 && err2 != 0) {
        // get last message id
        id = get_message_id(to);
        // save message
        err = save_message(id, from, to, message);
        if (err != 0) return 2;
        res = 0;
    } else {
        res = 1;
    }
    // send id to client
    err = sendMessage(socket, (char *)&id, sizeof(int));
    if (err != 0) return 2;

    return res;
}

int send_stored_message(char *receiver, char *sender) {
    return 0;
}
int send_message_to_receiver(char *receiver, char *sender) {
    int socket_receiver, err;

    dprintf(1, "a punto de enviar el mensaje\n");
    if (is_connected(sender) == 0) return 1;
    int socket_sender = create_socket(sender);
    if (socket_sender == -1) {
        disconnection(sender);
        return 2;
    }
    if (is_connected(receiver) == 0) {
        send_stored_message(receiver, sender);
        return 1;
    }
    socket_receiver = create_socket(receiver);
    if (socket_receiver == -1) {
        disconnection(receiver);
        return 2;
    }

    dprintf(1, "socket creado\n");
    // look in the receiver database messages from the sender
    char *filename = malloc(strlen(receiver) + 14);
    char *tempuser = malloc(strlen(receiver) + 19);
    sprintf(filename, "database/%s.txt", receiver);
    sprintf(tempuser, "database/temp_%s.txt", receiver);
    FILE *f, *f2;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int flag, flag2 = 0;

    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(filename);
        free(tempuser);
        return 2;
    }
    f2 = fopen(tempuser, "w");
    if (f2 == NULL) {
        perror("server-fileopen: write\n");
        free(filename);
        free(tempuser);
        fclose(f);
        return 2;
    }
    while (read = getline(&line, &len, f) != -1) {
        fprintf(f2, "%s", line);
        if (strstr (line, "Mensajes:") != NULL) {
            break;
        }
    }
    dprintf(1, "SENDER %s\n", sender);
    message_t *message = malloc(sizeof(message_t));
    while (read = getline(&line, &len, f) != -1) {
        dprintf(1, "LINE %s\n", line);
        if (strstr(line, sender) != NULL) {
            // id: from "message"
            sscanf(line, "%d: %s | %s", &message->id, message->from, message->message);
            err = sendMessage(socket_receiver, "SEND_MESSAGE", strlen("SEND_MESSAGE") + 1);
            if (err != 0) {
                close(socket_receiver);
                flag = 1;
                break;
            }
            strcat(message->from, "\0");
            dprintf(1, "from %s\n", message->from);
            err = sendMessage(socket_receiver, message->from, 256);
            if (err != 0) {
                close(socket_receiver);
                flag = 1;
                break;
            }
            err = sendMessage(socket_receiver, (char *)&(message->id), 1);
            if (err != 0) {
                close(socket_receiver);
                flag = 1;
                break;
            }
            strcat(message->message, "\0");
            dprintf(1, "message %s\n", message->message);
            err = sendMessage(socket_receiver, message->message, 256);
            if (err != 0) {
                close(socket_receiver);
                flag = 1;
                break;
            }

            /* SEND ACK TO THE SENDER */
            err = sendMessage(socket_sender, "SEND_MESS_ACK", strlen("SEND_MESS_ACK") + 1);
            if (err != 0) {
                close(socket_sender);
                flag2 = 1;
                break;
            }
            err = sendMessage(socket_sender, (char *)&message->id, 1);
            if (err != 0) {
                close(socket_sender);
                flag2 = 1;
                break;
            }
            strcat(sender, "\0");
            err = sendMessage(socket_sender, sender, 256);
            if (err != 0) {
                close(socket_sender);
                flag2 = 1;
                break;
            }
            strcat(receiver, "\0");
            err = sendMessage(socket_sender, receiver, 256);
            if (err != 0) {
                close(socket_sender);
                flag2 = 1;
                break;
            }
        } else {
            fprintf(f2, "%s", line);
        }
    }
    // copy the rest of the file
    if (line != NULL) {fprintf(f2, "%s", line);}
    while (read = getline(&line, &len, f) != -1) {
        fprintf(f2, "%s", line);
    }
    if (flag == 0) { // communication problems -> disconnect user
        disconnection(receiver);
    }
    err = remove(filename);
    if (err == -1) {
        perror("server: remove");
        free(filename);
        free(tempuser);
        fclose(f);
        fclose(f2);
        close(socket_receiver);
        return 2;
    }
    err = rename(tempuser, filename);
    if (err == -1) {
        perror("server: rename");
        free(filename);
        free(tempuser);
        fclose(f);
        fclose(f2);
        close(socket_receiver);
        return 2;
    }
    close(socket_receiver);
    fclose(f);
    fclose(f2);
    free(filename);
    free(tempuser);
    free(message);
    return 0;
}

int connected_users(char *user, int socket) {
    int err, res;
    err = search_user(user);
    if (err == 0) {
        res = 2;
        err = sendMessage(socket, (char *)&res, sizeof(int));
        if (err != 0) return 2;
        return 2;
    }
    err = is_connected(user);
    if (err == 0) {
        res = 1;
        err = sendMessage(socket, (char *)&res, sizeof(int));
        if (err != 0) return 2;
        return 1;
    }
    res = 0;
    err = sendMessage(socket, (char *)&res, sizeof(int));
    if (err != 0) return 2;
    char *filename = "database/connected.txt";
    FILE *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *username = malloc(255);
    //count_connected, read_connected, return 0
    res = count_connected();
    err = sendMessage(socket, (char *)&res, sizeof(int));
    if (err != 0) {
        free(username);
        return 2;
    }
    f = fopen(filename, "r");
    if (f == NULL) {
        perror("server-fileopen: read\n");
        free(username);
        return 2;
    }
    while ((read = getline(&line, &len, f)) != -1) {
        sscanf(line, "%s\n", username);
        dprintf(1, "username %s\n", username);
        strcat(username, "\0");
        err = sendMessage(socket, username, 256);
        if (err != 0) {
            fclose(f);
            free(username);
            return 2;
        }
    }
    fclose(f);
    free(username);
    return 0;
}