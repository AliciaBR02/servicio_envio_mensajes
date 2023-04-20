#include "comunicacion.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // print all arguments
    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    printf("%d", argc);
    client_register(argv[1], argv[2], argv[3]);
    // 1 - cogemos datos de la interfaz

    // a - ip y socket
    // b - operacion
    // c - datos necesarios
    // ejecutar register_communication()
    // 2 - creamos el socket
    // 3 - conectamos con el servidor
    // 4 - enviamos los datos
    // 5 - recibimos la respuesta
    // 6 - cerramos el socket
    // 7 - mandamos la respueta a la interfaz
    return 0;
}