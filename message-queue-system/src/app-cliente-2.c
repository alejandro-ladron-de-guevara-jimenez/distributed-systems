#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

struct ThreadData {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

void *thread_function(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int err = set_value(data->key, data->value1, data->N_value2, data->V_value2, data->value3);
    if (err == -1) {
        printf("Error al insertar la tupla con key %d\n", data->key);
    } else {
        printf("Tupla con key %d insertada correctamente\n", data->key);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[12];
    struct ThreadData threadData[12];

    destroy();

    // Datos para cada hilo
    int keys[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    char *values[] = {"Valor 1", "Valor 2", "Valor 3", "Valor 4", "Valor 5", "Valor 6", "Valor 7", "Valor 8", "Valor 9", "Valor 10", "Valor 11", "Valor 12"};
    double vectors[][12] = {
        {1.1, 2.2, 3.3},
        {4.4, 5.5, 6.6},
        {7.7, 8.8, 9.9},
	{10.0, 11.0, 12.0},
	{13.0, 14.0, 15.0},
	{16.0, 17.0, 18.0},
	{19.0, 20.0, 21.0},
	{22.0, 23.0, 24.0},
	{25.0, 26.0, 27.0},
	{28.0, 29.0, 30.0},
	{31.0, 32.0, 33.0},
	{34.0, 35.0, 36.0},
    };

    for (int i = 0; i < 12; i++) {
        threadData[i].key = keys[i];
        strcpy(threadData[i].value1, values[i]);
        threadData[i].N_value2 = 3;
        memcpy(threadData[i].V_value2, vectors[i], 3 * sizeof(double));
        threadData[i].value3.x = i * 10;
        threadData[i].value3.y = i * 20;

        // Crear hilo
        if (pthread_create(&threads[i], NULL, thread_function, &threadData[i]) != 0) {
            perror("Error al crear el hilo");
            return -1;
        }
    }

    // Esperar a que terminen los hilos
    for (int i = 0; i < 12; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
