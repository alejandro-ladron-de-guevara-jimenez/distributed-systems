#include <stdio.h>
#include <stdlib.h>
#include "claves.h"

int main (int argc, char **argv)
{   
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
    	/* Error servicio colas*/
	printf("Error al enviar mensaje a servidor\n");
    }

    /* Le damos nuevos valores a todas las variables de entrada de la tupla*/
    char *v1_1 = "segundo ejemplo de 1";
    double v2_2[] = {3.5, 5.1};
    v3.x = 2;
    v3.y = 6;
    /* Empleamos modify_value(), implicitamente delete_key(), para modificar los valores de la tupla anterior*/
    err = modify_value(key, v1_1, 2, v2_2, v3);
    if (err == -1) {
    	/* Error servicio tuplas*/
        printf("Error al modificar la tupla\n");
    } else if (err == -2) {
    	/* Error servicio colas*/
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
    	/* Error servicio colas*/
        printf("Error al enviar mensaje a servidor\n");
    }

    /* Imprimimos los valores recibidos*/
    printf("Char v4 es '%s'\n", v4);
    printf("Int v5 es %d\n", v5);
    printf("Double v6 es {");
    for (int i = 0; i < v5; i++) {
	 printf("%.2lf", v6[i]);
	 if (i < v5 - 1) {
		printf(", ");
	} else {
		printf("}\n");
	}
    }
    printf("Coord v7 es (%d, %d)\n", v7.x, v7.y);

    /* Liberamos espacio en memoria*/
    free(v4);
    free(v6);

    return 0;
}
