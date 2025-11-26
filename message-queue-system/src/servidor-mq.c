#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "claves.h"

struct Peticion {
    char name[32];       // Identificador cola usuario
    char op[32];         // Código de operación
    int key;             // Clave para operaciones key-value
    char value1[256];    // Cadena de caracteres (255 + '\0')
    int N_value2;        // Tamaño del vector de doubles
    double V_value2[32]; // Vector de doubles
    struct Coord value3; // Estructura con dos enteros
    int result;		 // Resultado (0 / -1)
};

/*Inicialización de mutex, condicionales y colas*/
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t  q_servidor;

void tratar_mensaje(void *mess){
    struct Peticion mensaje;	/*Mensaje local */
    mqd_t q_cliente;		/*Cola del cliente */

    /*Lock del mutex*/
    pthread_mutex_lock(&mutex_mensaje);

    /*Copia de la petición en local*/
    mensaje = (*(struct Peticion *) mess);

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
        /*Imprimir valores de comprobación*/
	printf("Servidor - get_value ejecutado para clave: %d\n", mensaje.key);
	printf("Servidor - value1: %s\n", mensaje.value1);
	printf("Servidor - N_value2: %d\n", mensaje.N_value2);
	printf("Servidor - V_value2: ");
	for (int i = 0; i < mensaje.N_value2; i++) {
	    printf("%.2lf ", mensaje.V_value2[i]);
	}
	printf("\n");
	printf("Servidor - Coordenadas: (%d, %d)\n", mensaje.value3.x, mensaje.value3.y);
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
    /* Se devuelve el resultado al cliente */
    /* Se abre la coladel cliente*/
    q_cliente = mq_open(mensaje.name, O_WRONLY);
    if (q_cliente == -1){
	perror("No se puede abrir la cola del cliente");
	mq_close(q_servidor);
	mq_unlink("/100495773");
    }
    else {
	/*Imrpimir resultado de la operación*/
	printf("Resultado: %d\n", mensaje.result);
	/*Enviar petición a cliente con el resultado de la operación*/
	if (mq_send(q_cliente, (char *)&mensaje, sizeof(mensaje), 0) < 0) {
		perror("mq_send");
		mq_close(q_cliente);
		mq_close(q_servidor);
		mq_unlink("/100495773");
	}
	mq_close(q_cliente);
    }

    /*Despierta al servidor*/
    pthread_cond_signal(&cond_mensaje);

    /*Unlock del mutex*/
    pthread_mutex_unlock(&mutex_mensaje);


    pthread_exit(0);
}


int main(void) {
    struct Peticion mess;               // Estructura de la petición
    struct mq_attr attr;		// Estructura de cola servidor
    pthread_attr_t t_attr;		// Atributos threads
    pthread_t thid;			// Threads

    /*Definir parámetros de la cola*/
    attr.mq_maxmsg = 10;                
    attr.mq_msgsize = sizeof(struct Peticion);

    /*Inicialización de cola servidor*/
    q_servidor = mq_open("/100495773", O_CREAT|O_RDONLY, 0700, &attr);
    if (q_servidor == -1) {
	perror("mq_open");
	return -1;
    }

    /*Inicialización de mutex, conds y threads*/
    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&t_attr);

    /*Atributos de los threads*/
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    while(1) {
	/*Feedback estado del sistema*/
  	printf("Esperando petición...\n");
	/*Recibir mensaje por cola servidor*/
	if (mq_receive(q_servidor, (char *)&mess, sizeof(mess), 0) < 0 ){
		perror("mq_recev");
		return -1;
	}

	/*Crear thread que trate el mensaje*/
	if (pthread_create(&thid, &t_attr, (void *)tratar_mensaje, (void *)&mess) == 0) {
		/*Se espera a que el thread copie el mensaje*/
		pthread_mutex_lock(&mutex_mensaje);
		while (mensaje_no_copiado) {
			pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
		}
		mensaje_no_copiado = true;
		pthread_mutex_unlock(&mutex_mensaje);
	}   
    }
    return 0;
}
