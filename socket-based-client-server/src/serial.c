#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "claves.h"
#include "serial.h"

#define BUFFER_SIZE 1024

void sendMessage(int sockfd, struct Peticion *p) {
    char buffer[BUFFER_SIZE];
    char temp[BUFFER_SIZE];  // Buffer temporal para `V_value2`
    
    // Limpiar buffers
    memset(buffer, 0, BUFFER_SIZE);
    memset(temp, 0, BUFFER_SIZE);

    // Serializar `V_value2`
    int offset = 0;
    for (int i = 0; i < p->N_value2; i++) {
        if (i == 0) {
            offset += snprintf(temp + offset, BUFFER_SIZE - offset, "%.2f", p->V_value2[i]);
        } else {
            offset += snprintf(temp + offset, BUFFER_SIZE - offset, ",%.2f", p->V_value2[i]);
        }

        // Verificar si el buffer está lleno
        if (offset >= BUFFER_SIZE) {
            fprintf(stderr, "Error: Buffer temp sobrepasado en serialización de V_value2\n");
            return;
        }
    }

    // Construcción del mensaje final
    int bytes_written = snprintf(buffer, BUFFER_SIZE, "%s|%d|%s|%d|%s|%d,%d|%d",
             p->op, p->key, p->value1, p->N_value2, temp,
             p->value3.x, p->value3.y, p->result);

    // Verificar si hubo truncamiento
    if (bytes_written >= BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer buffer sobrepasado en serialización de mensaje\n");
        return;
    }

    // Enviar el mensaje
    send(sockfd, buffer, strlen(buffer), 0);
}

void recvMessage(int sockfd, struct Peticion *p) {
    char buffer[BUFFER_SIZE];
    char value2_str[BUFFER_SIZE];  // Para `V_value2`

    // Limpiar buffers
    memset(buffer, 0, BUFFER_SIZE);
    memset(value2_str, 0, BUFFER_SIZE);

    // Recibir datos
    recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    buffer[BUFFER_SIZE - 1] = '\0'; // Evitar problemas con terminación de cadena

    // Parseo de la estructura
    sscanf(buffer, "%[^|]|%d|%[^|]|%d|%[^|]|%d,%d|%d", 
           p->op, &p->key, p->value1, &p->N_value2, value2_str,
           &p->value3.x, &p->value3.y, &p->result);

    // Deserializar `V_value2`
    char *token = strtok(value2_str, ",");
    int i = 0;
    while (token != NULL && i < p->N_value2) {
        p->V_value2[i++] = atof(token);
        token = strtok(NULL, ",");
    }
}

/*
void sendMessage(int sockfd, struct Peticion *p) {
    struct Peticion temp;
    memcpy(&temp, p, sizeof(struct Peticion));

    // Convertir enteros a Network Byte Order
    temp.key = htonl(p->key);
    temp.N_value2 = htonl(p->N_value2);
    temp.value3.x = htonl(p->value3.x);
    temp.value3.y = htonl(p->value3.y);
    temp.result = htonl(p->result);

    // Enviar la estructura completa
    send(sockfd, &temp, sizeof(temp), 0);
}

void recvMessage(int sockfd, struct Peticion *p) {
    struct Peticion temp;
    recv(sockfd, &temp, sizeof(temp), 0);

    // Convertir enteros a Host Byte Order
    p->key = ntohl(temp.key);
    p->N_value2 = ntohl(temp.N_value2);
    p->value3.x = ntohl(temp.value3.x);
    p->value3.y = ntohl(temp.value3.y);
    p->result = ntohl(temp.result);

    memcpy(p->op, temp.op, sizeof(p->op));
    memcpy(p->value1, temp.value1, sizeof(p->value1));
    memcpy(p->V_value2, temp.V_value2, sizeof(p->V_value2));
}
*/