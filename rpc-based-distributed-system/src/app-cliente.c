#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "claves.h"

struct ThreadData {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];
    struct Coord value3;
};

void *thread_function_set(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int err = set_value(data->key, data->value1, data->N_value2, data->V_value2, data->value3);
    if (err == -1) {
        printf("Error al insertar la tupla con key %d\n", data->key);
    } else {
        printf("Tupla con key %d insertada correctamente\n", data->key);
    }
    pthread_exit(NULL);
}

void *thread_function_mod(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    int err = modify_value(data->key, data->value1, data->N_value2, data->V_value2, data->value3);
    if (err == -1) {
        printf("Error al modificar la tupla con key %d\n", data->key);
    } else {
        printf("Tupla con key %d modificada correctamente\n", data->key);
    }
    pthread_exit(NULL);
}

int prueba1(){
    /* Empleamos destroy() para borrar las tuplas*/
    destroy();

    /* Le damos valores a todas las variables de entrada de la tupla*/
    int key = 4;
    char *v1 = "ejemplo de valor 1";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3;
    v3.x = 10;
    v3.y = 5;
    /* Llamamos a set_value(), e implicitamente a exist(), para introducir una tupla a la base datos*/
    int err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
    	/* Error servicio tuplas*/
        printf("Error al insertar la tupla\n");
    } else if (err == -2) {
    	/* Error servicio sockets*/
	printf("Error al enviar mensaje a servidor\n");
    }

    /* Creamos variables contenedores para recibir una tupla de la base de datos*/
    /* Guardamos espacio en memoria para value1 (v4) y V_value2 (v6)*/
    char *v4 = (char *)malloc(255 * sizeof(char));
    int v5;
    double *v6 = (double *)malloc(32 * sizeof(double));
    struct Coord v7;

    /* Comprobamos que se ha guardado el espacio en memoria*/
    if (v4 == NULL || v6 == NULL) {
        printf("Error al asignar memoria\n");
	return -1;
    }

    /* Empleamos get_value() para recibir los datos de la tupla, con clave (key)*/
    err = get_value(key, v4, &v5, v6, &v7);
    if (err == -1) {
    	/* Error servicio tuplas*/
	printf("Error al encontar la tupla\n");
    } else if (err == -2) {
    	/* Error servicio sockets*/
        printf("Error al enviar mensaje a servidor\n");
    }

    /* Imprimimos los valores recibidos*/
    printf("Value 1: '%s'\n", v4);
    printf("Tamaño Value 2: %d\n", v5);
    printf("Value 2: {");
    for (int i = 0; i < v5; i++) {
	 printf("%.2lf", v6[i]);
	 if (i < v5 - 1) {
		printf(", ");
	} else {
		printf("}\n");
	}
    }
    printf("Coords: (%d, %d)\n", v7.x, v7.y);

    /* Liberamos espacio en memoria*/
    free(v4);
    free(v6);

    return 0;
}

int prueba2(){
    /* Le damos nuevos valores a todas las variables de entrada de la tupla*/
    int key = 4;
    char *v1_1 = "segundo ejemplo de 1";
    double v2_2[] = {3.5, 5.1};
    struct Coord v3;
    v3.x = 2;
    v3.y = 6;
    int err;
    /* Empleamos modify_value(), implicitamente delete_key(), para modificar los valores de la tupla anterior*/
    err = modify_value(key, v1_1, 2, v2_2, v3);
    if (err == -1) {
    	/* Error servicio tuplas*/
        printf("Error al modificar la tupla\n");
    } else if (err == -2) {
    	/* Error servicio sockets*/
        printf("Error al enviar mensje a servidor\n");
    }

    /* Creamos variables contenedores para recibir una tupla de la base de datos*/
    /* Guardamos espacio en memoria para value1 (v4) y V_value2 (v6)*/
    char *v4 = (char *)malloc(255 * sizeof(char));
    int v5;
    double *v6 = (double *)malloc(32 * sizeof(double));
    struct Coord v7;

    /* Comprobamos que se ha guardado el espacio en memoria*/
    if (v4 == NULL || v6 == NULL) {
        printf("Error al asignar memoria\n");
	return -1;
    }

    /* Empleamos get_value() para recibir los datos de la tupla, con clave (key)*/
    err = get_value(key, v4, &v5, v6, &v7);
    if (err == -1) {
    	/* Error servicio tuplas*/
	printf("Error al encontar la tupla\n");
    } else if (err == -2) {
    	/* Error servicio sockets*/
        printf("Error al enviar mensaje a servidor\n");
    }

    /* Imprimimos los valores recibidos*/
    printf("Value 1: '%s'\n", v4);
    printf("Tamaño Value 2: %d\n", v5);
    printf("Value 2: {");
    for (int i = 0; i < v5; i++) {
	 printf("%.2lf", v6[i]);
	 if (i < v5 - 1) {
		printf(", ");
	} else {
		printf("}\n");
	}
    }
    printf("Coords: (%d, %d)\n", v7.x, v7.y);

    /* Liberamos espacio en memoria*/
    free(v4);
    free(v6);

    return 0;
}

int prueba3() {
    pthread_t threads[12];
    struct ThreadData threadData[12];

    destroy();  // Limpiamos la base de datos

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
        if (pthread_create(&threads[i], NULL, thread_function_set, &threadData[i]) != 0) {
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

int prueba4(){
    pthread_t threads[12];
    struct ThreadData threadData[12];

    // Datos para cada hilo
    int keys[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    char *values[] = {
        "Valor 13", "Valor 14", "Valor 15", "Valor 16", "Valor 17", "Valor 18",
        "Valor 19", "Valor 20", "Valor 21", "Valor 22", "Valor 23", "Valor 24"
    };

    double vectors[][12] = {
        {37.0, 38.0, 39.0},
        {40.0, 41.0, 42.0},
        {43.0, 44.0, 45.0},
        {46.0, 47.0, 48.0},
        {49.0, 50.0, 51.0},
        {52.0, 53.0, 54.0},
        {55.0, 56.0, 57.0},
        {58.0, 59.0, 60.0},
        {61.0, 62.0, 63.0},
        {64.0, 65.0, 66.0},
        {67.0, 68.0, 69.0},
        {70.0, 71.0, 72.0}
    };

    for (int i = 0; i < 12; i++) {
        threadData[i].key = keys[i];
        strcpy(threadData[i].value1, values[i]);
        threadData[i].N_value2 = 3;
        memcpy(threadData[i].V_value2, vectors[i], 3 * sizeof(double));
        threadData[i].value3.x = (i + 12) * 10;
        threadData[i].value3.y = (i + 12) * 20;

        // Crear hilo
        if (pthread_create(&threads[i], NULL, thread_function_mod, &threadData[i]) != 0) {
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

int prueba5() {
    int key = 4;
    // Búsqueda de la tupla con clave "key"
    int err = exist(key);
    // Devuelve 1 si se ha realizado correctamente
    // Devuelve 0 si no lo ha hecho
    if (err != 0) {
	printf("Tupla con clave %d encontrada\n", key);
    } else {
        printf("Error: Tupla no encontrada\n");
    }
    
    // Eliminación de dicha tupla
    err = delete_key(key);
    if (err == 0) {
	printf("Tupla con clave %d eliminada\n", key);
    } else {
        printf("Error al eliminar tupla\n");
    }
}

int prueba_extra(){
    pthread_t threads[1000];
    struct ThreadData threadData[1000];

    destroy();

    double vector[] = {1.0, 2.0, 3.0};

    for (int i = 0; i < 1000; i++) {
        threadData[i].key = i + 1;
        strcpy(threadData[i].value1, "Probing efficiency");
        threadData[i].N_value2 = 3;
        memcpy(threadData[i].V_value2, vector, 3 * sizeof(double));
        threadData[i].value3.x = i;
        threadData[i].value3.y = i * 2;

        // Crear hilo
        if (pthread_create(&threads[i], NULL, thread_function_set, &threadData[i]) != 0) {
            perror("Error al crear el hilo");
            return -1;
        }
    }

    // Esperar a que terminen los hilos
    for (int i = 0; i < 1000; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

int main (int argc, char **argv)
{   
    printf("--------------------------------------------------------------\n");
    printf("PRUEBA 1: Comprobación Funcionamiento de Funciones: Inserción\n");
    printf("--------------------------------------------------------------\n");
    prueba1();
    printf("-----------------------------------------------------------------\n");
    printf("PRUEBA 2: Comprobación Funcionamiento de Funciones: Modificación\n");
    printf("-----------------------------------------------------------------\n");
    prueba2();
    printf("-----------------------------------------------------------\n");
    printf("PRUEBA 3: Comprobación Concurrencia con Threads: Inserción\n");
    printf("-----------------------------------------------------------\n");
    prueba3();
    printf("--------------------------------------------------------------\n");
    printf("PRUEBA 4: Comprobación Concurrencia con Threads: Modificación\n");
    printf("--------------------------------------------------------------\n");
    prueba4();
    printf("-----------------------------------------------------------------------------\n");
    printf("PRUEBA 5: Comprobación Funcionamiento de Funciones: Existencia y Eliminación\n");
    printf("-----------------------------------------------------------------------------\n");
    prueba5();
    /*
    struct timespec start, end;
    double elapsed_time;
    printf("--------------------------------------------------------------\n");
    printf("PRUEBA EXTRA: Compraración Eficiencia || Struct vs Buffer Serializado\n");
    printf("--------------------------------------------------------------\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    prueba_extra();
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Tiempo transcurrido: %.9f segundos\n", elapsed_time);
    */
    return 0;
}
