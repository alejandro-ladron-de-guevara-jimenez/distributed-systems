#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/xdr.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "rpc.h"

// Definición del fichero de usuarios y el máximo de línea empleado
#define USER_FILE "users.txt"
#define MAX_LEN 256

struct ThreadArgs {
    char *ip;
    int sc;
};

/*Inicialización de mutex, condicionales y colas*/
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;

// Thread que trata los mensajes recibidos de los clientes
void *tratar_mensaje(void *args) {
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    // Cargamos el socket y la ip pasadas
    int sc = threadArgs->sc;
    // La IP del cliente se pasó para utilizarse en ciertos contextos
    char *ip = threadArgs->ip;
    // Se libera el contenedor de los argumentos pasados
    free(args);

    // Se lee la info mandada por socket y se guarda en un buffer
    char buffer[1024];
    int n = read(sc, buffer, sizeof(buffer));
    if (n <= 0) {
        close(sc);
        pthread_exit(NULL);
    }

    // Se crea un interpretador de XDR para desempacar
    XDR xdrs_in;
    xdrmem_create(&xdrs_in, buffer, n, XDR_DECODE);

    // Inicialización de los valores a usar en las funciones y RPC
    // La operación, el usuario, el fichero para PUBLISH y DELETE, fecha
    char *op = NULL;
    char *user = NULL;
    char *fileName = "";
    char *datetime = NULL;
    int result = 4;  // Default: ERROR

    // Se desempacan la operación, nombre del usuario y la fecha
    xdr_string(&xdrs_in, &op, MAX_LEN);
    xdr_string(&xdrs_in, &user, MAX_LEN);
    xdr_string(&xdrs_in, &datetime, MAX_LEN);

    // Se crea un interpretador de XDR para empacar y enviar
    char outbuf[1024];
    XDR xdrs_out;
    xdrmem_create(&xdrs_out, outbuf, sizeof(outbuf), XDR_ENCODE);

    /*Lock del mutex*/
    pthread_mutex_lock(&mutex_mensaje);

    /*Se indica que ya se puede despertar al servidor*/
    mensaje_no_copiado = false;

    // Si operación REGISTER
    if (strcmp(op, "REGISTER") == 0) {
        // Abrir archivo en opción append
        FILE *file = fopen(USER_FILE, "a+");
        if (file) {
            char line[MAX_LEN];
            int exists = 0;
	    // Leer si existe dicho usuario registrado
            while (fgets(line, sizeof(line), file)) {
                line[strcspn(line, "\n")] = 0;  // Eliminar salto de línea
		// Si existe
                if (strcmp(line, user) == 0) {
                    exists = 1;
                }
            }
	    // Si se ha comprobado que no existe
            if (!exists) {
		// Hacer append del usuario
                fprintf(file, "%s\n", user);
                result = 0;  // OK
            } else {
		// Sino, error 1
                result = 1;  // USERNAME IN USE
            }
            fclose(file);
        } else {
	    // En otros casos, error 2
            result = 2;  // FILE ERROR
        }
	// Enviar respuesta
        xdr_int(&xdrs_out, &result);
    // Si operación UNREGISTER
    } else if (strcmp(op, "UNREGISTER") == 0) {
       // Abrir fichero lectura
       FILE *file = fopen(USER_FILE, "r");
       if (file) {
	    // Crear auxiliar escritura
            FILE *temp = fopen("temp_users.txt", "w");
            if (temp) {
                char line[MAX_LEN];
                int found = 0;
		// Pasar cada línea de un fichero a otro
                while (fgets(line, sizeof(line), file)) {
                    line[strcspn(line, "\n")] = 0;
		    // De existir el usuario
                    if (strcmp(line, user) == 0) {
			// Abstenerse de pasar la línea al otro archivo
                        found = 1;  // Usuario encontrado, no se escribe
		    // Si no lo es, pasarla al otro archivo
                    } else {
                        fprintf(temp, "%s\n", line);
                    }
                }
                fclose(file);
                fclose(temp);
		// Si se ha detectado al usuario en el archivo
                if (found) {
		    // Reemplazar el fichero antiguo con el nuevo
                    remove(USER_FILE);
                    rename("temp_users.txt", USER_FILE);
                    result = 0;  // OK
                } else {
		    // Sino, mantener fichero antiguo
                    remove("temp_users.txt");
                    result = 1;  // USER NOT FOUND
                }
            } else {
		// Otros casos de error
                fclose(file);
                result = 2;  // TEMP FILE ERROR
            }
        } else {
	    // Otros casos de error
            result = 2;  // FILE ERROR
        }
	// Mandar resultado
        xdr_int(&xdrs_out, &result);
    // Si operación CONNECT
    } else if (strcmp(op, "CONNECT") == 0) {
	char *port = NULL;
	// Se desempaca el valor del puerto
        xdr_string(&xdrs_in, &port, MAX_LEN);

	// Se abre el fichero de usuarios
        FILE *file = fopen(USER_FILE, "r");
        if (file) {
	    // Se crea uno auxiliar
            FILE *temp = fopen("temp_users.txt", "w");
            if (temp) {
                char line[3 * MAX_LEN];
                int found = 0;
		int connected = 0;

		// Se leen las líneas del fichero
                while (fgets(line, sizeof(line), file)) {
                    char name[MAX_LEN];
		    char aux_ip[MAX_LEN];
		    char *aux_port = NULL;
		    // Se escanean los valores de la línea: nombre, ip y puerto
                    int err = sscanf(line, "%s %s %s\n", name, aux_ip, aux_port);

		    // Si el nombre concuerda
                    if (strcmp(name, user) == 0) {
			// Y existe un valor para el puerto
			if (err == 2) {
			    // El usuario ya está conectado
			    connected = 1;
			// Sino
		        } else {
                            // Sobrescribir la línea con nueva información
                            fprintf(temp, "%s    %s    %s\n", user, ip, port);
			}
                        found = 1;
		    // Sino, se pasa sin modificar
                    } else {
                        fputs(line, temp);  // Mantener línea original
                    }
                }

                fclose(file);
                fclose(temp);

		// Se ha encontrado pero no está conectado
                if (found && !connected) {
		    // Reemplazar antiguo archivo por nuevo
                    remove(USER_FILE);
                    rename("temp_users.txt", USER_FILE);
                    result = 0;  // OK
                } 
		// Se ha encontrado y está conectado
		else if (found && connected) {
		    // Mantener archivo antiguo
                    remove("temp_users.txt");
                    result = 2;  // USER ALREADY CONNECTED
		}
		// No se ha encontrado
		else {
		    // Mantener archivo antiguo
                    remove("temp_users.txt");
                    result = 1;  // USER NOT FOUND
                }

            } else {
		// Otros errores
                fclose(file);
                result = 3;  // TEMP FILE ERROR
            }

        } else {
	    // Otros errores
            result = 3;  // FILE ERROR
        }
	// Mandar resultado
        xdr_int(&xdrs_out, &result);
    // Si operación DISCONNECT
    } else if (strcmp(op, "DISCONNECT") == 0) {
	int port = 0;
        xdr_int(&xdrs_in, &port); // Se extrae un entero del flujo XDR de entrada (el puerto)

        FILE *file = fopen(USER_FILE, "r"); // Abrimos el archivo de usuarios en modo lectura.
        if (file) {
            FILE *temp = fopen("temp_users.txt", "w"); // Abrimos un archivo temporal en modo escritura.
            if (temp) {
                char line[3 * MAX_LEN]; // Buffer para leer cada línea del archivo.
                int found = 0; // Indicador para saber si se encontró al usuario.

                // Leemos línea por línea el archivo de usuarios.
                while (fgets(line, sizeof(line), file)) {
                    char name[MAX_LEN], ip_tmp[MAX_LEN];
                    int port_tmp;

                    // Leemos línea por línea el archivo de usuarios.
                    sscanf(line, "%s %s %d", name, ip_tmp, &port_tmp);

                    if (strcmp(name, user) == 0) {
                        // Si el nombre coincide con el usuario que quiere desconectarse,
                        // Escribimos solo el nombre del usuario, eliminando IP y puerto
                        fprintf(temp, "%s\n", name);
                        found = 1;
                    } else {
                        // Si no es el usuario que se desconecta, se copia la línea original tal cual.
                        fputs(line, temp);  
                    }
                }

                fclose(file); // Cerramos el archivo original.
                fclose(temp); // Cerramos el archivo temporal.

                if (found) {
                     // Si el usuario fue encontrado, reemplazamos el archivo original con el temporal.
                    remove(USER_FILE); // Eliminamos el archivo original.
                    rename("temp_users.txt", USER_FILE); // Renombramos el temporal como el original.
                    result = 0;  // OK
                } else {
                    // Si no se encontró al usuario, eliminamos el archivo temporal y devolvemos error.
                    remove("temp_users.txt");
                    result = 1;  // USER NOT FOUND
                }

            } else {
                  // Si no se pudo abrir el archivo temporal, cerramos el original y devolvemos error.
                fclose(file);
                result = 2;  // TEMP FILE ERROR
            }

        } else {
            // Si no se pudo abrir el archivo original, devolvemos error.
            result = 2;  // FILE ERROR
        }
	// Enviar resultado
        xdr_int(&xdrs_out, &result);
    // Si operación PUBLISH
    } else if (strcmp(op, "PUBLISH") == 0) {
        char *description = NULL;
        fileName = NULL;

        // Extraer desde el canal XDR de entrada el nombre del fichero y la descripción
        xdr_string(&xdrs_in, &fileName, MAX_LEN);
        xdr_string(&xdrs_in, &description, MAX_LEN);

        // Verificar si el usuario está conectado (tiene IP y puerto)
        FILE *file = fopen(USER_FILE, "r");
        int user_connected = 0; // Indicador de conexión del usuario

        if (file) {
            char line[3 * MAX_LEN]; // Buffer para leer líneas del archivo

            // Leemos el archivo de usuarios línea por línea
            while (fgets(line, sizeof(line), file)) {
                char name[MAX_LEN], ip_tmp[MAX_LEN];
                int port_tmp;
                // Extraemos nombre, IP y puerto
                if (sscanf(line, "%s %s %d", name, ip_tmp, &port_tmp) == 3 && strcmp(name, user) == 0) {
                    // Si coincide el nombre con el usuario actual, está conectado
                    user_connected = 1;
                }
            }
        }
	fclose(file); // Cerramos el archivo de usuarios
	
        if (!user_connected) {
            // Si el usuario no está conectado, devolvemos error
            result = 2;  // USER NOT CONNECTED
        } else {
            // Si el usuario está conectado, continuamos
	    struct stat st = {0};
        // Comprobamos si existe el directorio "files", si no, lo creamos
            if (stat("files", &st) == -1) {
                mkdir("files", 0700);
	    }
            // Construir ruta: users/<usuario>.txt
            char path[MAX_LEN * 2];
            snprintf(path, sizeof(path), "files/%s.txt", user);

            // Abrir archivo para añadir publicación
            FILE *ufile = fopen(path, "a");
            if (ufile) {
                fprintf(ufile, "Path: %s\n", fileName);
                fprintf(ufile, "'%s'\n\n", description);
                result = 0;  // OK
            } else {
                result = 4;  // FILE ERROR
            }
            fclose(ufile); // Cerramos el archivo del usuario
        }
    // Enviamos el resultado al cliente a través del canal XDR de salida
	xdr_int(&xdrs_out, &result);
    } else if (strcmp(op, "DELETE") == 0) {
        fileName = NULL;
        // Recibimos desde el canal XDR el nombre del fichero que se desea eliminar
        xdr_string(&xdrs_in, &fileName, MAX_LEN);

        // Verificar si el usuario está conectado (tiene IP y puerto)
        FILE *file = fopen(USER_FILE, "r");
        int user_connected = 0;

        if (file) {
            char line[3 * MAX_LEN];
            // Leemos línea a línea el archivo de usuarios conectados
            while (fgets(line, sizeof(line), file)) {
                char name[MAX_LEN], ip_tmp[MAX_LEN];
                int port_tmp;
                // Extraemos nombre, IP y puerto de la línea actual
                if (sscanf(line, "%s %s %d", name, ip_tmp, &port_tmp) == 3 && strcmp(name, user) == 0) {
                    user_connected = 1;
                }
            }
            fclose(file); // Cerramos el archivo después de leerlo
        }

        if (!user_connected) {
            result = 2;  // USER NOT CONNECTED
        } else {
            // Nos aseguramos de que exista el directorio "files"
	    struct stat st = {0};
            if (stat("files", &st) == -1) {
                mkdir("files", 0700);
	    }
            // Construir ruta: files/<usuario>.txt
            char path[MAX_LEN * 2];
            snprintf(path, sizeof(path), "files/%s.txt", user);

            // Abrir archivo para añadir publicación
            FILE *ufile = fopen(path, "r");
            if (ufile) {
        // Abrimos un archivo temporal donde copiaremos las publicaciones que se mantendrán
		FILE *temp = fopen("files/temp.txt", "w");
		if (!temp) {
            fclose(ufile);
		    result = 4;
	        }
		else {
       		int found = 0; // Bandera para saber si se encontró la publicación
		    char line[512];
		    char name[256];

            // Leemos línea a línea el archivo del usuario
		    while (fgets(line, sizeof(line), ufile)) {
                // Comprobamos si esta línea es una entrada que corresponde al archivo a borrar
		        if (sscanf(line, "Path: %s", name) == 1 && strcmp(name, fileName) == 0) {
			    found = 1;
                // Saltamos las dos líneas siguientes (descripción y línea en blanco)
			    fgets(line, sizeof(line), ufile);
			    fgets(line, sizeof(line), ufile);
		        } else {
                // Si no es el archivo a borrar, copiamos la línea al archivo temporal
			    fputs(line, temp);
		        }
	            }
            
            // Cerramos ambos archivos
		    fclose(ufile);
		    fclose(temp);

		    if (found != 1) {
                // Si no se encontró el archivo a borrar
		        remove("files/temp.txt");
		        result = 3; // FILE NOT FOUND
		    } else {
                // Si se encontró y eliminó, reemplazamos el archivo original por el temporal
                remove(path);
		        rename("files/temp.txt", path);
		        result = 0;  // OK
		    }
                }
	    } else {
                result = 4;  // FILE ERROR
            }
        }
    // Enviamos el resultado al cliente a través del canal XDR de salida
	xdr_int(&xdrs_out, &result);
    } else if (strcmp(op, "LIST_USERS") == 0) {
	FILE *tfile = fopen(USER_FILE, "r");  // Abrimos el archivo de usuarios conectados para contar usuarios distintos del solicitante
        int count = 0; // Contador de usuarios conectados (excluyendo al usuario que hace la solicitud)
	int user_exist = 0; // Bandera para verificar si el usuario solicitante está registrado
        char line[3 * MAX_LEN];
        char name[MAX_LEN], ip[MAX_LEN], port[MAX_LEN];
        // Recorremos el archivo línea a línea
        while (fgets(line, sizeof(line), tfile)) {
            // Extraemos nombre, IP y puerto
            if (sscanf(line, "%s %s %s", name, ip, port) == 3) {
	        if (strcmp(name, user) == 0) {
		    user_exist = 1; // El usuario solicitante está conectado
	        } else {
            	    count++; // Usuario válido distinto al solicitante
		}
	    }
        }
        fclose(tfile); // Cerramos archivo de conteo

	FILE *ufile = fopen(USER_FILE, "r"); // Lo abrimos de nuevo para enviar los datos
	if (!user_exist) {
        // Si el usuario que pide la lista no está conectado
	    result = 1;
	    xdr_int(&xdrs_out, &result); // Enviamos el código de error
        } else {
	    FILE *file = fopen(USER_FILE, "r");
	    if (!file) {
    		result = 3;
	    	xdr_int(&xdrs_out, &result);
	    } else {
	        result = 0;
	        xdr_int(&xdrs_out, &result); // Enviamos confirmación de éxito

        	char count_str[12];  // suficientemente grande para cualquier int
        	snprintf(count_str, sizeof(count_str), "%d", count);
        	char *count_ptr = count_str;
        	xdr_string(&xdrs_out, &count_ptr, MAX_LEN);  // Enviar número de usuarios como string

             // Recorremos nuevamente el archivo y enviamos los datos de cada usuario distinto al solicitante
                while (fgets(line, sizeof(line), ufile)) {
                    if (sscanf(line, "%s %s %s", name, ip, port) == 3 && strcmp(name, user) != 0) {
                        char *name_ptr = name, *ip_ptr = ip, *port_ptr = port;
			xdr_string(&xdrs_out, &name_ptr, MAX_LEN); // Enviamos nombre
			xdr_string(&xdrs_out, &ip_ptr, MAX_LEN); // Enviamos IP
			xdr_string(&xdrs_out, &port_ptr, MAX_LEN); // Enviamos puerto
                    }
		}
            }
	    fclose(file); 
	}
	fclose(ufile); // Cerramos archivo abierto para el envío de datos
    } else if (strcmp(op, "LIST_CONTENT") == 0) {
        // Verificar si el usuario está conectado (tiene IP y puerto)
	char *t_user = NULL;
	xdr_string(&xdrs_in, &t_user, MAX_LEN);

        FILE *file = fopen(USER_FILE, "r"); // Abrir archivo de usuarios conectados
        int user_connected = 0;
	int target_exist = 0;
	int user_exist = 0;

        if (file) {
            char line[3 * MAX_LEN];
            while (fgets(line, sizeof(line), file)) {
                char name[MAX_LEN], ip_tmp[MAX_LEN], port_tmp[MAX_LEN];
		int err = sscanf(line, "%s %s %s", name, ip_tmp, port_tmp);
                if (err == 3) {
		    if (strcmp(name, t_user) == 0) {
			target_exist = 1; // El usuario destino está conectado
                    } else if (strcmp(name, user) == 0) {
                    	user_connected = 1; // El usuario solicitante está conectado
			user_exist = 1;
		    }
		} else if (err == 1) {
		    if (strcmp(name, t_user) == 0) {
			target_exist = 1;
		    }
		}
            }
            fclose(file);
        }
	if (!user_exist) {
            result = 1;  // USER DON'T EXIST
            xdr_int(&xdrs_out, &result);
        } else if (user_exist && !user_connected) {
            result = 2;  // USER NOT CONNECTED
            xdr_int(&xdrs_out, &result);
        } else if (user_exist && user_connected && !target_exist) {
            result = 3;  // REMOTE USER NOT CONNECTED
            xdr_int(&xdrs_out, &result);
        } else {
             // Verificar o crear directorio "files"
	    struct stat st = {0};
            if (stat("files", &st) == -1) {
                mkdir("files", 0700);
	    }

            char path[MAX_LEN * 2];
	    char title[MAX_LEN];
            snprintf(path, sizeof(path), "files/%s.txt", t_user);  // Ruta del archivo de publicaciones del usuario destino

            // Primera pasada: contar cuántas publicaciones hay
            FILE *ufile = fopen(path, "r");
	    int count = 0;
            if (ufile) {
        	char line[3 * MAX_LEN];
      		while (fgets(line, sizeof(line), ufile)) {
            	    if (sscanf(line, "Path: %s", title) == 1) {
            	        count++;
		    }
	        }
	        fclose(ufile);
            }
        // Segunda pasada: enviar los títulos de las publicaciones
	    FILE *file = fopen(path, "r");
	    if (file) {
		result = 0;
	        xdr_int(&xdrs_out, &result);
		char line[MAX_LEN];

        	char count_str[12];  // suficientemente grande para cualquier int
        	snprintf(count_str, sizeof(count_str), "%d", count);
        	char *count_ptr = count_str;
        	xdr_string(&xdrs_out, &count_ptr, MAX_LEN);  // Enviar número de usuarios como string
                // Enviar los títulos (paths) de las publicaciones
                for (int i = 0; i < count; ++i) {
                    if (fgets(line, sizeof(line), file) && sscanf(line, "Path: %s", title) == 1) {
                        char *title_ptr = title;
			xdr_string(&xdrs_out, &title_ptr, MAX_LEN); // Enviar título
            // Saltar las siguientes dos líneas (description y category)
		    	fgets(line, sizeof(line), file);
		    	fgets(line, sizeof(line), file);
                    }
		}
		fclose(file);
	    }
	}
    } else if (strcmp(op, "GET_FILE") == 0) {
        // Verificar si el usuario está conectado (tiene IP y puerto)
    // Leer nombre del usuario objetivo (el que publicó el archivo)
	char *t_user = NULL;
	xdr_string(&xdrs_in, &t_user, MAX_LEN);

        FILE *file = fopen(USER_FILE, "r"); // Abrir archivo de usuarios conectados
	int target_connected = 0;
        int user_connected = 0;
	int target_exist = 0;
	int user_exist = 0;
	char *ip = NULL;
	char *port = NULL;

        if (file) {
            char line[3 * MAX_LEN];
            while (fgets(line, sizeof(line), file)) {
                char name[MAX_LEN], ip_tmp[MAX_LEN], port_tmp[MAX_LEN];
		int err = sscanf(line, "%s %s %s", name, ip_tmp, port_tmp);
                if (err == 3) {  // Usuario conectado (tiene IP y puerto)
		    if (strcmp(name, t_user) == 0) {
			target_connected = 1;
			target_exist = 1;
            // Asignar punteros a ip y puerto temporal
			ip = ip_tmp;
			port = port_tmp;
                    } else if (strcmp(name, user) == 0) {
                    	user_connected = 1;
			user_exist = 1;
		    }
		} else if (err == 1) { // Usuario listado pero sin IP/puerto
		    if (strcmp(name, t_user) == 0) {
			target_exist = 1;
		    }
		}
            }
        // Validaciones de conexión y existencia
	    if (!user_exist || !user_connected || !target_exist || !target_connected) {
		result = 2; // Validaciones de conexión y existencia
	        xdr_int(&xdrs_out, &result);
	    } else {
		result = 0;
	    	xdr_int(&xdrs_out, &result);
		xdr_string(&xdrs_out, &ip, MAX_LEN);
		xdr_string(&xdrs_out, &port, MAX_LEN);
	    }
	    fclose(file);
        } else {
        // Si no se pudo abrir el archivo de usuarios
	    result = 2;
	    xdr_int(&xdrs_out, &result);
	}
    }

    /*Despierta al servidor*/
    pthread_cond_signal(&cond_mensaje);

    /*Unlock del mutex*/
    pthread_mutex_unlock(&mutex_mensaje);

    xdr_destroy(&xdrs_in);

    // Enviar respuesta
    int len = xdr_getpos(&xdrs_out);
    write(sc, outbuf, len);
    xdr_destroy(&xdrs_out);

    printf("OPERATION %s FROM %s\n", op, user);
    // Atributos para activar RPC
    CLIENT *clnt; 
    enum clnt_stat retval_1;
    void *result_1;
    char *host = getenv("LOG_RPC_IP");
    // Creamos conexión RPC
    clnt = clnt_create (host, IMPRIMIR_PROG, IMPRIMIR_VERS, "tcp");
    if (clnt == NULL) {
	clnt_pcreateerror (host);
	exit (1);
    }
    // Llamamos a la función RPC
    retval_1 = imprimir_strings_1(user, op, fileName, datetime, &result_1, clnt);
    if (retval_1 != RPC_SUCCESS) {
	clnt_perror (clnt, "call failed");
    }
    // Destruimos RPC
    clnt_destroy (clnt);

    close(sc);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s -p <puerto>\n", argv[0]);
        return 1;
    }

    int puerto = atoi(argv[2]);
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket");
        return 1;
    }

    // Activamos un socket temporal para acceder a nuestra IP
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Creamos una estructura por defecto de un socket
    struct sockaddr_in remoto;
    memset(&remoto, 0, sizeof(remoto));
    remoto.sin_family = AF_INET;
    remoto.sin_port = htons(80);    // Puerto
    inet_pton(AF_INET, "8.8.8.8", &remoto.sin_addr);    // IP de escucha

    // Conectamos el socket
    connect(sock, (struct sockaddr *)&remoto, sizeof(remoto));

    // Guardamos el valor de la IP en la estructura "local"
    struct sockaddr_in local;
    socklen_t len = sizeof(local);
    getsockname(sock, (struct sockaddr *)&local, &len);

    // Cerramos el socket temporal
    close(sock);

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    getsockname(sd, (struct sockaddr *)&local_addr, &addr_len);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(puerto);

    int opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    char *server_ip = inet_ntoa(local.sin_addr);
    printf("init server %s|%d\n", server_ip, puerto);

    listen(sd, 5);

    /*Inicialización de mutex, conds y threads*/
    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int sc = accept(sd, (struct sockaddr *)&client_addr, &len);
        if (sc < 0) {
            perror("accept");
            continue;
        }

	char *ip = inet_ntoa(client_addr.sin_addr);

	struct ThreadArgs *args = malloc(sizeof(struct ThreadArgs));
       	if (!args) {
        	perror("Error al asignar memoria");
        	close(sc);
        	return -1;
     	}

     	args->sc = sc;
	args->ip = ip;

	/*Crear thread que trate el mensaje*/
        pthread_t tid;
        if (pthread_create(&tid, NULL, tratar_mensaje, args) == 0) {
		/*Se espera a que el thread copie el mensaje*/
		pthread_mutex_lock(&mutex_mensaje);
		while (mensaje_no_copiado) {
			pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
		}
		mensaje_no_copiado = true;
		pthread_mutex_unlock(&mutex_mensaje);
	}
        pthread_detach(tid);
    }

    close(sd);
    return 0;
}
