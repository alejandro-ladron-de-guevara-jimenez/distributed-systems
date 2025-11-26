#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "claves.h"

#define FILE_NAME "store.txt"
#define MAX_VALUE2 32

int exist(int key) {
	/*Se abre el fichero en lectura*/
	FILE *file = fopen(FILE_NAME, "r");

	if (!file) {
		return 0;
	}

	int existing_key;
	char line[256];

	/*Lee línea a línea el contenido de "store.tx"*/
	while (fgets(line, sizeof(line), file)) {
		/*Si encuentra la línea donde hay una clave*/
		if (sscanf(line, "Clave: %d", &existing_key)) {
			/*Compara esa clave con la de la variable pasada*/
			if (existing_key == key) {
				fclose(file);
				return 1;
			}
		}
	}

	fclose(file);
	return 0;
}

int destroy() {
	/*Elimina el archivo "store.txt"*/
	if (remove(FILE_NAME) == 0) {
		printf("Inicializando servicio de almacenamiento\n");
		return 0;
	} else {
		perror("Error al borrar fichero de tuplas");
		return -1;
	}
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {	
	/*Comprueba que existe una tupla con clave "key" y que N_value2 está en el rango (1, 32)*/
	if (exist(key) == 1 || N_value2 < 1 || N_value2 > MAX_VALUE2) {
		printf("Error: Instancia con clave '%d' existente\n", key);
		return -1;
	}

	/*Se abre el fichero en escritura al final*/
	FILE *file = fopen(FILE_NAME, "a");
	if (!file) {
		perror("Error opening file");
		return -1;
	}

	/*Se escriben los valores introducidos en la función*/
	fprintf(file, "Clave: %d\n", key);
	fprintf(file, "'%s'\n", value1);
	/*Cada valor de V_value2 separado por espacios*/
	for (int i = 0; i < N_value2; i++) {
		fprintf(file, "%.2lf", V_value2[i]);
		if (i < N_value2 - 1) {
			fprintf(file, " ");
		}
	}
	fprintf(file, "\n");

	/*Se añade un espacio extra de separación "\n"*/
	fprintf(file, "Coordenadas: (%d, %d)\n\n", value3.x, value3.y);

	fclose(file);
 
	return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
	/*Se abre el fichero en lectura*/
	FILE *file = fopen(FILE_NAME, "r");

	if (!file) {
		perror("Error opening file");
		return -1;
	}

	int existing_key, found = 0;
	char line[512];

	/*Se busca línea a línea la clave pasada*/
	while (found != 1 && fgets(line, sizeof(line), file)) {
		/*Si existe, se indica con "found" que la clave ha sido encontrada*/
		if (sscanf(line, "Clave: %d", &existing_key) == 1 && existing_key == key) {
			found = 1;
		}
	}
	/*En caso de no encontrarse, da error*/
	if (found != 1) {
		perror("Error al encontrar clave");
		return -1;
	}
	
	/*Para los valores siguientes de la tupla, se introduce cada uno en su respectiva variable*/
	/*value1*/
	if (!fgets(line, sizeof(line), file) || sscanf(line, "'%[^\']", value1) != 1) {
		fclose(file);
		return -1;
	}

	/*V_value2*/
	if (!fgets(line, sizeof(line), file)) {
		fclose(file);
		return -1;
	}

	/*Introducción de los valores de V_value2 uno a uno*/
	char *val2 = strtok(line, " ");
	for (int i = 0; i < *N_value2; i++) {
		if (!val2) {
			fclose(file);
			return -1;
		}
		V_value2[i] = atof(val2);
		val2 = strtok(NULL, " ");
	}

	/*value3*/
	if (!fgets(line, sizeof(line), file) || sscanf(line, "Coordenadas: (%d, %d)", &value3->x, &value3->y) != 2) {
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
	/*Se comprueba el rango de N_value2, (1, 32)*/
	if (N_value2 < 1 || N_value2 > 32) {
		return -1;
	}
	
	/*Se elimina la tupla con dicha "key"*/
	if (delete_key(key) == -1) {
		return -1;
	}

	/*Se añade la tupla con dicha "key" y sus valores actualizados*/
	if (set_value(key, value1, N_value2, V_value2, value3) == -1) {
		return -1;
	}

	return 0;
}

int delete_key(int key) {
	/*Se abre el fichero como lectura*/
	FILE *file = fopen(FILE_NAME, "r");
	if (!file) {
		return -1;
	}

	/*Se crea fichero auxiliar para escritura*/
	FILE *temp = fopen("temp.txt", "w");
	if (!temp) {
		return -1;
	}

	int existing_key, found = 0;
	char line[512];

	/*Se leen las líneas del fichero "store.txt"*/
	while (fgets(line, sizeof(line), file)) {
		/*Si es la línea en que se indica la clave "key"*/
		if (sscanf(line, "Clave: %d", &existing_key) == 1 && existing_key == key) {
			/*Indicar que ha sido encontrado*/
			found = 1;
			/*Ignorar value1, V_value3, value3 y el espacio de separación "\n"*/
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
			fgets(line, sizeof(line), file);
		/*Si no lo es, escribir las líneas al fichero auxiliar*/
		} else {
			fputs(line, temp);
		}
	}
	
	fclose(file);
	fclose(temp);

	/*Si no ha sido encontrado, eliminar fichero auxiliar*/
	if (found != 1) {
		remove("temp.txt");
		return -1;
	}

	/*Si encontrado, reemplazar el fichero original por el auxiliar*/
	remove(FILE_NAME);
	rename("temp.txt", FILE_NAME);
	return 0;
}	
