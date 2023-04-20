#include "comunicacion/comunicacion.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    dprintf(1, "argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        dprintf(1, "argv[%d]: %s\n", i, argv[i]);
    }
    if (strcmp(argv[1], "REGISTER") == 0) {
        if (argc == 5) client_register(argv[2], argv[3], argv[4]);
        else printf("Error: REGISTER <nombre> <alias> <fecha>\n");
    }
    else if (strcmp(argv[1], "UNREGISTER") == 0) {
        if (argc == 3) client_unregister(argv[2]);
        else printf("Error: UNREGISTER <alias>\n");
    }
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