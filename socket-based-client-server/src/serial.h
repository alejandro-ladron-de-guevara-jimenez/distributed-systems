#ifndef SERIAL_H
#define SERIAL_H


struct Peticion {
    char op[32];         // Código de operación
    int key;             // Clave para operaciones key-value
    char value1[256];    // Cadena de caracteres (255 + '\0')
    int N_value2;        // Tamaño del vector de doubles
    double V_value2[32]; // Vector de doubles
    struct Coord value3; // Estructura con dos enteros
    int result;		 // Resultado peticion (0 / -1)
};

void sendMessage(int sockfd, struct Peticion *p);
void recvMessage(int sockfd, struct Peticion *p);

#endif