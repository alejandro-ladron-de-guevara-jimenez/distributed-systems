#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "claves.h"
#include "serial.h"

struct ThreadArgs {
    struct Peticion pet;
    int sc;
};

/*Inicialización de mutex, condicionales y colas*/
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;

void *tratar_mensaje(void *args){
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    struct Peticion mensaje = threadArgs->pet;	/*Mensaje local */
    int sc = threadArgs->sc;
    free(threadArgs);

    /*Lock del mutex*/
    pthread_mutex_lock(&mutex_mensaje);

    /*Se indica que ya se puede despertar al servidor*/
    mensaje_no_copiado = false;

    /*Ejecutar la función de la petición del cliente y preparar respuesta*/
    if (strcmp(mensaje.op, "destroy") == 0) {
        mensaje.result = destroy();  // Llamada a destroy
    }
    else if (strcmp(mensaje.op, "set_value") == 0) {
        mensaje.result = set_value(mensaje.key, mensaje.value1, mensaje.N_value2, mensaje.V_value2, mensaje.value3);  // Llamada a set_value
    }
    else if (strcmp(mensaje.op, "get_value") == 0) {
        mensaje.result = get_value(mensaje.key, mensaje.value1, &mensaje.N_value2, mensaje.V_value2, &mensaje.value3);  // Llamada a get_value
    }
    else if (strcmp(mensaje.op, "modify_value") == 0) {
        mensaje.result = modify_value(mensaje.key, mensaje.value1, mensaje.N_value2, mensaje.V_value2, mensaje.value3);  // Llamada a modify_value
    }
    else if (strcmp(mensaje.op, "delete_key") == 0) {
        mensaje.result = delete_key(mensaje.key);  // Llamada a delete_key
    }
    else if (strcmp(mensaje.op, "exist") == 0) {
        mensaje.result = exist(mensaje.key);  // Llamada a exist
    }
    else {
        mensaje.result = -1;  // Si la operación no es reconocida o falla
    }

    /*Despierta al servidor*/
    pthread_cond_signal(&cond_mensaje);

    /*Unlock del mutex*/
    pthread_mutex_unlock(&mutex_mensaje);

    sendMessage(sc, &mensaje);  // Envía el resultado
    close(sc);
    pthread_exit(0);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Comando incorrecto, el formato debe ser ./servidor (num de puerto)");
        return -1;
    }

    pthread_attr_t t_attr;        // Atributos threads
    pthread_t thid;
    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&t_attr);

    int puerto = atoi(argv[1]);
    if (puerto <= 0) {
        printf("Error: puerto inválido\n");
        return -1;
    }
    
    struct Peticion pet;               // Estructura de la petición
    struct sockaddr_in server_addr,  client_addr;
    socklen_t size = sizeof(client_addr);
    int sd, sc;
	int err;

	if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0){
			printf ("SERVER: Error en el socket");
			return (0);
	}
	int val = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

	bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(puerto);

    err = bind(sd, (const struct sockaddr *)&server_addr,
			sizeof(server_addr));
	if (err == -1) {
		printf("Error en bind\n");
		return -1;
	}

    err = listen(sd, SOMAXCONN);
    if (err == -1) {
	printf("Error en listen\n");
	return -1;
    }

    while (1){
	printf("Esperando conexion\n");
	sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);

	if (sc == -1) {
		printf("Error en accept\n");
		return -1;
	}
	printf("Conexión aceptada de IP: %s   Puerto: %d\n",
	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
       	if (!args) {
        	perror("Error al asignar memoria");
        	close(sc);
        	return -1;
     	}
        
     	recvMessage(sc, &args->pet);
     	args->sc = sc;

     	pthread_t thread;
    	if (pthread_create(&thread, NULL, tratar_mensaje, (void *)args) == 0) {
        	pthread_detach(thread);
     	} else {
        	perror("Error al crear thread");
        	free(args);
      		close(sc);
		return -1;
   	}
    }
    close (sd);
    return(0);
}
