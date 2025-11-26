#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "claves.h"

struct Peticion {
    char name[32];	 // Identificador cola del usuario
    char op[32];         // Código de operación
    int key;             // Clave para operaciones key-value
    char value1[256];    // Cadena de caracteres (255 + '\0')
    int N_value2;        // Tamaño del vector de doubles
    double V_value2[32]; // Vector de doubles
    struct Coord value3; // Estructura con dos enteros
    int result;		 // Resultado peticion (0 / -1)
};

/*Inicialización función cliente() y struct Petición*/
int cliente(struct Peticion pet);
struct Peticion pet;

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    /*Se indica tipo de petición*/
    strcpy(pet.op, "set_value");
    /*Se pasan los datos correspondientes*/
    pet.key = key;
    strcpy(pet.value1, value1);
    pet.N_value2 = N_value2;
    memcpy(pet.V_value2, V_value2, N_value2 * sizeof(V_value2));
    pet.value3 = value3;
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        return 0;
    }
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    /*Se indica el tipo de petición*/
    strcpy(pet.op, "get_value");
    /*Se pasan los datos correspondientes*/
    pet.key = key;
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        /*Modificar los valores de las variables de entrada corresponientes*/
        strcpy(value1, pet.value1);
	*N_value2 = pet.N_value2;
	for (int i = 0; i < *N_value2; i++) {
		V_value2[i] = pet.V_value2[i];
	}
	*value3 = pet.value3;
        return 0;
    }
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    /*Se indica el tipo de petición*/
    strcpy(pet.op, "modify_value");
    /*Se pasan los datos correspondientes*/
    pet.key = key;
    strcpy(pet.value1, value1);
    pet.N_value2 = N_value2;
    memcpy(pet.V_value2, V_value2, N_value2 * sizeof(double));
    pet.value3 = value3;
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        return 0;
    }
}

int exist(int key) {
    /*Se indica el tipo de petición*/
    strcpy(pet.op, "exist");
    /*Se pasan los datos correspondientes*/
    pet.key = key;
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        return 0;
    }
}

int destroy() {
    /*Se indica el tipo de petición*/
    strcpy(pet.op, "destroy");
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        return 0;
    }
}

int delete_key(int key) {
    /*Se indica el tipo de petición*/
    strcpy(pet.op, "delete_key");
    /*Se pasan los datos correspondientes*/
    pet.key = key;
    /*Se envía mensaje a servidor*/
    int res = cliente(pet);
    /*Si error en servicio de tuplas*/
    if (res == -1) {
        perror("Error procesando petición");
        return -1;
    }
    /*Si error en servicio cliente/servidor*/
    else if (res == -2) {
	perror("Error enviando petición");
    } else {
        return 0;
    }
}

int cliente(struct Peticion pet) {
    /* Comprobacion validez de N_value2*/
    if (pet.N_value2) {
    	if (pet.N_value2 < 1 || pet.N_value2 > 32) {
    		printf("Valor N_value2 inválido\n");
    		return -1;
    	}
    }
    /* Comprobacion validez de value1*/
    if (pet.value1) {
    	if (strlen(pet.value1) > 255) {
    		printf("Tamaño de value1 incorrecto");
    		return -1;
    	}
    }
    
    mqd_t q_servidor;       // Cola de mensajes del proceso servidor
    mqd_t q_cliente;        // Cola de mensajes para el proceso cliente
    
    struct mq_attr attr;	// Inicialización cola
    char queuename[256];	// Contenendor nombre cola

    /*Definir atributos de la cola*/
    attr.mq_maxmsg = 1;     
    attr.mq_msgsize = sizeof(struct Peticion);

    /*Definir nombre de la cola con ID de proceso y hilo*/
    snprintf(queuename, sizeof(queuename), "/cliente_%d_%ld", getpid(), pthread_self());
    /*Introducirlo en la petición*/
    strcpy(pet.name, queuename);

    /*Crear la cola para el cliente*/
    q_cliente = mq_open(queuename, O_CREAT | O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -2;
    }

    /*Abrir la cola del servidor*/
    q_servidor = mq_open("/100495773", O_WRONLY);
    if (q_servidor == -1) {
        mq_close(q_cliente);
	mq_unlink(queuename);
        perror("mq_open 2");
        return -2;
    }

    /* Enviar la petición al servidor */
    if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0) {
	perror("mq_send");
	mq_close(q_cliente);
	mq_close(q_servidor);
	mq_unlink(queuename);
	return -2;
    }

    /* Recibir la respuesta del servidor */
    if (mq_receive(q_cliente, (char *)&pet, sizeof(pet), 0) < 0) {
        perror("mq_recv");
	mq_close(q_cliente);
	mq_close(q_servidor);
	mq_unlink(queuename);
	return -2;
    }

    /*Si cliente recibe error del servidor por el tratamiento de tuplas*/
    if (pet.result == -1) {
        perror("Error en servidor");
	mq_close(q_cliente);
	mq_close(q_servidor);
	mq_unlink(queuename);
        return -1;
    }

    /* Cerrar las colas */
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename); // Eliminar la cola del cliente

    return 0;
}
