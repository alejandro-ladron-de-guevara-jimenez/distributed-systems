#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "claves.h"
#include "serial.h"

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
	if (strchr(pet.value1, '|')) {
     		printf("Value 1 contiene el caracter especial '|'\n");
		return -1;
	}
    }
    
    int sd;
    struct sockaddr_in server_addr;
    struct hostent *hp;        
	int err;
    char *direccion_servidor = getenv("IP_TUPLAS");
    char *puerto_servidor = getenv("PORT_TUPLAS");

    if (!direccion_servidor || !puerto_servidor) {
        printf("Error: variables de entorno no definidas\n");
        return -1;
    }

    if (direccion_servidor <= 0) {
        printf("Error: puerto inválido\n");
        return -1;
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		printf("Error en socket\n");
		return -1;
	}

	bzero((char *)&server_addr, sizeof(server_addr));
   	hp = gethostbyname (direccion_servidor);
	if (hp == NULL) {
		printf("Error en gethostbyname\n");
		return -1;
	}

   	memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
   	server_addr.sin_family  = AF_INET;
	int puerto = atoi(puerto_servidor);
   	server_addr.sin_port    = htons(puerto);
   	
	// se establece la conexión
   	err = connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr));
	if (err == -1) {
		printf("Error en connect\n");
		return -1;
	}

    	/// A PARTIR DE AQUI CAMBIAR LOS SENDMESSAGE POR LO QUE SE QUIERE ENVIAR ///

    	sendMessage(sd, &pet);  // Envía la operacion
    	recvMessage(sd, &pet);  // Recibe la respuesta

   	close (sd);
   	return(0);
}
