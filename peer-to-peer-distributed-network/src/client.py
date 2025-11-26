from enum import Enum
import argparse
import socket
import xdrlib
import threading
import sys
import os
import requests
from zeep import Client
from zeep.exceptions import Fault
import time



class client :



    # ******************** TYPES *********************

    # *

    # * @brief Return codes for the protocol methods

    class RC(Enum) :

        OK = 0

        ERROR = 1

        USER_ERROR = 2



    # ****************** ATTRIBUTES ******************

    _server = None

    _port = -1

    _user = None

    _thread_run = threading.Event()

    _thread = None



    # ******************** METHODS *******************

    @staticmethod
    
    def get_datetime_from_service():
        try:
            print("Intentando obtener fecha y hora del servicio web...")
            time.sleep(2)
            # URL del archivo WSDL del servicio web SOAP que define la interfaz
            wsdl = "http://localhost:5500/?wsdl"
            # Crea un cliente SOAP usando la librería zeep y el WSDL dado
            client = Client(wsdl=wsdl)
            # Llama a la función 'get_datetime' del servicio web
            response = client.service.get_datetime()

            # Muestra la respuesta obtenida por consola
            print("Respuesta obtenida:", response)

            # Devuelve la fecha y hora obtenida desde el servicio
            return response

        # Si ocurre un error SOAP específico 
        except Fault as fault:
            print("SOAP Fault ocurrido:", fault)
            # Devuelve una fecha por defecto indicando error
            return "00/00/0000 00:00:00"

        # Si ocurre cualquier otro tipo de error 
        except Exception as e:
            print("Error inesperado al obtener la fecha y hora del servicio web:", e)
            return "00/00/0000 00:00:00"



    @staticmethod

    def register(user):

        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Crear un paquete XDR con la operación REGISTER, el nombre de usuario y la fecha/hora
        packer = xdrlib.Packer()
        packer.pack_string(("REGISTER").encode('utf-8'))
        packer.pack_string(user.encode('utf-8'))
        packer.pack_string(datetime_str.encode('utf-8'))  
        data = packer.get_buffer()

        # Crear conexión con el servidor usando sockets
        with socket.create_connection((client._server, client._port)) as sock:

            # Enviar el paquete al servidor
            sock.sendall(data)

            # Esperar respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)
            num = unpacker.unpack_int()

            # Manejo de errores dependiendo de la respuesta del servidor
            if num == 0:
                print(f"REGISTER OK")
                return client.RC.OK
            elif num == 2:
                print(f"REGISTER FAIL")
                return client.RC.USER_ERROR
            elif num == 1:
                print(f"USERNAME IN USE")
        
        # Si no se reconoce la respuesta, devolver error genérico
        return client.RC.ERROR
   

    @staticmethod

    def unregister(user):

        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Crear un paquete XDR con la operación UNREGISTER, el nombre de usuario y la fecha/hora
        packer = xdrlib.Packer()
        packer.pack_string(("UNREGISTER").encode('utf-8'))
        packer.pack_string(user.encode('utf-8'))
        packer.pack_string(datetime_str.encode('utf-8'))  
        data = packer.get_buffer()

        # Crear conexión con el servidor usando sockets
        with socket.create_connection((client._server, client._port)) as sock:

            # Enviar el paquete al servidor
            sock.sendall(data)

            # Esperar respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)
            num = unpacker.unpack_int()

            # Manejo de errores dependiendo de la respuesta del servidor
            if num == 0:
                print(f"UNREGISTER OK")
                return client.RC.OK
            elif num == 2:
                print(f"UNREGISTER FAIL")
                return client.RC.USER_ERROR
            elif num == 1:
                print(f"USER DOES NOT EXIST")

        # Si no se reconoce la respuesta, devolver error genérico
        return client.RC.ERROR


    @staticmethod

    def file_request_listener(listen_port):
        # Crear un socket TCP
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            print("Llegado")
            # Asociar el socket a todas las interfaces locales y al puerto indicado
            s.bind(('0.0.0.0', listen_port))
            s.listen()  # Poner el socket en modo escucha

            # Establecer un timeout de 1 segundo para aceptar conexiones
            s.settimeout(1.0)

            # Bucle principal mientras la bandera del hilo esté activa
            while client._thread_run.is_set():
                try:
                    # Esperar una conexión entrante
                    conn, addr = s.accept()
                except socket.timeout:
                    # Si no hay conexión en el tiempo establecido, continuar
                    continue
                with conn:
                    print(f"[Listener] Conexión recibida de {addr}")
                    try:
                        # Recibir datos del cliente
                        data = conn.recv(1024)
                        if not data:
                            continue
                        # Desempaquetar el nombre del archivo solicitado
                        unpacker = xdrlib.Unpacker(data)
                        path = unpacker.unpack_string().decode('utf-8')
                        print(f"[Listener] Solicitado archivo: {path}")

                        packer = xdrlib.Packer()
                        if os.path.isfile(path):
                            # Si el archivo existe, leerlo y empaquetarlo
                            with open(path, 'rb') as f:
                                content = f.read()
                            packer.pack_int(0)  # Código 0: archivo encontrado
                            packer.pack_bytes(content)
                            print(f"[Listener] Archivo enviado correctamente.")
                        else:
                            # Si no se encuentra el archivo, enviar código de error
                            packer.pack_int(1)  # Código 1: archivo no encontrado
                            print(f"[Listener] Archivo no encontrado.")

                        # Enviar respuesta al cliente
                        conn.sendall(packer.get_buffer())

                    except Exception as e:
                        # Manejo de errores internos del hilo
                        print(f"[Listener] Error: {e}")
        # Mensaje final al detenerse el hilo
        print("[Listener] Hilo detenido")


    @staticmethod

    def connect(user) :
        # Crear un socket temporal para obtener un puerto libre del sistema
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(('0.0.0.0', 0))  # Se enlaza a cualquier interfaz y puerto 0 
            _, port = s.getsockname()  # Obtener el puerto asignado
            listen_port = str(port)  # Guardar el puerto como string

        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Empaquetar la solicitud CONNECT con: operación, usuario, fecha y puerto de escucha
        packer = xdrlib.Packer()
        packer.pack_string(("CONNECT").encode('utf-8'))
        packer.pack_string(user.encode('utf-8'))
        packer.pack_string(datetime_str.encode('utf-8'))
        packer.pack_string(listen_port.encode('utf-8'))
        data = packer.get_buffer()

        # Iniciar el hilo del listener para recibir peticiones de archivos
        client._thread_run.set()
        client._thread = threading.Thread(target=client.file_request_listener, args=(int(listen_port),), daemon=True)
        client._thread.start()

        # Establecer conexión con el servidor principal
        with socket.create_connection((client._server, client._port)) as sock:

             # Enviar los datos por el socket
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)
            num = unpacker.unpack_int()  # Desempaquetar el código de respuesta

            if num == 0:
                print(f"CONNECT OK")
                client._user = user  # Guardar el nombre de usuario conectado
                return client.RC.OK
            elif num == 2:
                print(f"USER ALREADY CONNECTED")
                client._thread_run.clear()
                client._thread.join()  # Finalizar el hilo de escucha
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"CONNECT FAIL")
                client._thread_run.clear()
                client._thread.join()  # Finalizar el hilo de escucha
            elif num == 1:
                client._thread_run.clear()
                client._thread.join()  # Finalizar el hilo de escucha
                print(f"CONNECT FAIL, USER DOES NOT EXIST")

        return client.RC.ERROR  # Código de error genérico si no se cumplió ninguna condición anterior


    @staticmethod

    def disconnect(user) :

        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Empaquetar la solicitud DISCONNECT con: operación, usuario y fecha
        packer = xdrlib.Packer()
        packer.pack_string(("DISCONNECT").encode('utf-8'))
        packer.pack_string(user.encode('utf-8'))
        packer.pack_string(datetime_str.encode('utf-8'))  
        data = packer.get_buffer()

        # Establecer conexión con el servidor principal
        with socket.create_connection((client._server, client._port)) as sock:

             # Enviar los datos por el socket
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)
            num = unpacker.unpack_int()  # Desempaquetar el código de respuesta

            if num == 0:
                print(f"DISCONNECT OK")
                client._user = None  # Limpiar el usuario actual
                client._thread_run.clear()  # Señal para detener el hilo listener
                client._thread.join()  # Esperar a que el hilo termine
                return client.RC.OK
            elif num == 2:
                print(f"DISCONNECT FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"DISCONNECT FAIL")
            elif num == 1:
                print(f"DISCONNECT FAIL, USER DOES NOT EXIST")

        return client.RC.ERROR  # Código de error genérico si no se cumplió ninguna condición


    @staticmethod

    def publish(fileName, description) :
        # Comprobar si el usuario está conectado antes de continuar
        if client._user == None:
            print(f"PUBLISH FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        
        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Empaquetar la solicitud PUBLISH con: operación, usuario, fecha, nombre del archivo y descripción
        packer = xdrlib.Packer()
        packer.pack_string(("PUBLISH").encode('utf-8'))
        packer.pack_string(client._user.encode('utf-8'))
        packer.pack_string(datetime_str.encode('utf-8'))  
        packer.pack_string(fileName.encode('utf-8'))
        packer.pack_string(description.encode('utf-8'))
        data = packer.get_buffer()

        # Establecer conexión con el servidor
        with socket.create_connection((client._server, client._port)) as sock:

            # Enviar los datos por el socket
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)
            num = unpacker.unpack_int()  # Obtener el código de respuesta

            if num == 0:
                print(f"PUBLISH OK")
                return client.RC.OK
            elif num == 2:
                print(f"PUBLISH FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
            elif num == 4:
                print(f"PUBLISH FAIL")
            elif num == 1:
                print(f"PUBLISH FAIL, USER DOES NOT EXIST")
        
        # Devolver código de error genérico si no se reconoce la respuesta
        return client.RC.ERROR




    @staticmethod

    def delete(fileName) :
        # Comprobar si el usuario está conectado antes de continuar
        if client._user == None:
            print(f"DELETE FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        
        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Crear un objeto packer para empaquetar los datos usando XDR
        packer = xdrlib.Packer()
        packer.pack_string(("DELETE").encode('utf-8'))  # Operación: DELETE
        packer.pack_string(client._user.encode('utf-8'))  # Usuario conectado
        packer.pack_string(datetime_str.encode('utf-8'))  # Fecha y hora actual
        packer.pack_string(fileName.encode('utf-8'))  # Nombre del archivo a eliminar

        data = packer.get_buffer()  # Obtener el buffer con los datos empaquetados

        # Establecer una conexión con el servidor
        with socket.create_connection((client._server, client._port)) as sock:

            # Enviar los datos empaquetados al servidor
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)  # Crear un desempaquetador para la respuesta
            num = unpacker.unpack_int()  # Leer el código de respuesta

            # Interpretar el código de respuesta del servidor
            if num == 0:
                print(f"DELETE OK")
                return client.RC.OK
            elif num == 2:
                print(f"DELETE FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"DELETE FAIL, CONTENT NOT PUBLISHED")
            elif num == 4:
                print(f"DELETE FAIL")
            elif num == 1:
                print(f"DELETE FAIL, USER DOES NOT EXIST")
        
        # Si la respuesta no es válida o reconocida, devolver error genérico
        return client.RC.ERROR


    @staticmethod

    def listusers() :
        # Comprobar si el usuario está conectado antes de continuar
        if client._user == None:
            print(f"LIST_USERS FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        
        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Crear un objeto packer para empaquetar los datos usando XDR
        packer = xdrlib.Packer()
        packer.pack_string(("LIST_USERS").encode('utf-8'))  # Operación: LIST_USERS
        packer.pack_string(client._user.encode('utf-8'))  # Usuario conectado
        packer.pack_string(datetime_str.encode('utf-8'))  # Fecha y hora actual
        data = packer.get_buffer()  # Obtener el buffer con los datos empaquetados

        # Establecer una conexión con el servidor
        with socket.create_connection((client._server, client._port)) as sock:
            # Enviar los datos empaquetados al servidor
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)  # Crear un desempaquetador para la respuesta
            num = unpacker.unpack_int()  # Leer el código de respuesta

            # Interpretar el código de respuesta del servidor
            if num == 0:
                print(f"LIST_USERS OK")
                users = unpacker.unpack_string().decode('utf-8')  # Obtener el número de usuarios
                if int(users) == 0:
                    return client.RC.OK  # No hay usuarios
                # Mostrar la lista de usuarios
                for i in range(int(users)):
                    name = unpacker.unpack_string().decode('utf-8')
                    ip = unpacker.unpack_string().decode('utf-8')
                    port = unpacker.unpack_string().decode('utf-8')
                    print(f"    {name} {ip} {port}")
                return client.RC.OK
            elif num == 2:
                print(f"LIST_USERS FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"LIST_USERS FAIL")
            elif num == 1:
                print(f"LIST_USERS FAIL, USER DOES NOT EXIST")

        # Devolver un código de error si la respuesta no es válida
        return client.RC.ERROR




    @staticmethod

    def listcontent(user) :
        # Comprobar si el usuario está conectado antes de continuar
        if client._user == None:
            print(f"LIST_CONTENT FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        
        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()

        # Crear un objeto packer para empaquetar los datos usando XDR
        packer = xdrlib.Packer()
        packer.pack_string(("LIST_CONTENT").encode('utf-8'))  # Operación: LIST_CONTENT
        packer.pack_string(client._user.encode('utf-8'))  # Usuario conectado
        packer.pack_string(datetime_str.encode('utf-8'))  # Fecha y hora actual
        packer.pack_string(user.encode('utf-8'))  # Usuario remoto para obtener su contenido

        data = packer.get_buffer()  # Obtener el buffer con los datos empaquetados

        # Establecer una conexión con el servidor
        with socket.create_connection((client._server, client._port)) as sock:
            # Enviar los datos empaquetados al servidor
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)  # Crear un desempaquetador para la respuesta
            num = unpacker.unpack_int()  # Leer el código de respuesta

            # Interpretar el código de respuesta del servidor
            if num == 0:
                print(f"LIST_CONTENT OK")
                contents = unpacker.unpack_string().decode('utf-8')  # Obtener el número de contenidos
                if int(contents) == 0:
                    return client.RC.OK  # No hay contenidos
                # Mostrar la lista de contenidos
                for i in range(int(contents)):
                    title = unpacker.unpack_string().decode('utf-8')
                    print(f"    {title}")
                return client.RC.OK
            elif num == 2:
                print(f"LIST_CONTENT FAIL, USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif num == 3:
                print(f"LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
            elif num == 4:
                print(f"LIST_CONTENT FAIL")
            elif num == 1:
                print(f"LIST_CONTENT FAIL, USER DOES NOT EXIST")

        # Devolver un código de error si la respuesta no es válida
        return client.RC.ERROR


    @staticmethod

    def getfile(user, remote_FileName, local_FileName) :
        # Comprobar si el usuario está conectado antes de continuar
        if client._user == None:
            print(f"GET_FILE FAIL, USER NOT CONNECTED")
            return client.RC.USER_ERROR
        
        # Obtener la fecha y hora desde el web service
        datetime_str = client.get_datetime_from_service()
        
        # Crear un objeto packer para empaquetar los datos usando XDR
        packer = xdrlib.Packer()
        packer.pack_string(("GET_FILE").encode('utf-8'))  # Operación: GET_FILE
        packer.pack_string(client._user.encode('utf-8'))  # Usuario conectado
        packer.pack_string(datetime_str.encode('utf-8'))  # Fecha y hora actual
        packer.pack_string(user.encode('utf-8'))  # Usuario remoto
        data = packer.get_buffer()  # Obtener el buffer con los datos empaquetados

        # Establecer una conexión con el servidor
        with socket.create_connection((client._server, client._port)) as sock:
            # Enviar los datos empaquetados al servidor
            sock.sendall(data)

            # Esperar la respuesta del servidor
            respuesta = sock.recv(1024)
            unpacker = xdrlib.Unpacker(respuesta)  # Crear un desempaquetador para la respuesta
            num = unpacker.unpack_int()  # Leer el código de respuesta

            # Si la respuesta es correcta, obtener la dirección y puerto del servidor remoto
            if num == 0:
                ip = unpacker.unpack_string().decode('utf-8')  # IP del servidor remoto
                port = unpacker.unpack_string().decode('utf-8')  # Puerto del servidor remoto

                # Establecer conexión con el servidor remoto para obtener el archivo
                with socket.create_connection((ip, port)) as sc:
                    packer2 = xdrlib.Packer()
                    packer2.pack_string(remote_FileName.encode('utf-8'))  # Nombre del archivo remoto
                    data = packer2.get_buffer()
                    sc.sendall(data)  # Enviar el nombre del archivo al servidor remoto

                    # Esperar la respuesta del servidor remoto
                    response = sc.recv(1024)
                    unpacker2 = xdrlib.Unpacker(response)
                    result = unpacker2.unpack_int()  # Código de resultado de la solicitud de archivo

                    # Si el archivo se encuentra, recibirlo
                    if result == 0:
                        with open(local_FileName, 'wb') as f:  # Abrir archivo local para escribir
                            while True:
                                chunk = sc.recv(4096)  # Recibir porciones del archivo
                                if not chunk:  # Si no hay más datos, terminar
                                    break
                                f.write(chunk)  # Escribir porciones al archivo local
                        print(f"GET_FILE OK")  # Éxito
                        return client.RC.OK
                    elif result == 1:
                        print("GET_FILE FAIL, FILE NOT FOUND")  # Archivo no encontrado
                        return client.RC.ERROR
                    else:
                        print("GET_FILE FAIL")  # Error desconocido
                        return client.RC.ERROR

            elif num == 2:
                print(f"GET_FILE FAIL")
                return client.RC.USER_ERROR

        return client.RC.ERROR  # Error en la solicitud si no se llega a una respuesta válida




    # *

    # **

    # * @brief Command interpreter for the client. It calls the protocol functions.

    @staticmethod

    def shell():



        while (True) :

            try :

                command = input("c> ")

                line = command.split(" ")

                if (len(line) > 0):



                    line[0] = line[0].upper()



                    if (line[0]=="REGISTER") :

                        if (len(line) == 2) :

                            client.register(line[1])

                        else :

                            print("Syntax error. Usage: REGISTER <userName>")



                    elif(line[0]=="UNREGISTER") :

                        if (len(line) == 2) :

                            client.unregister(line[1])

                        else :

                            print("Syntax error. Usage: UNREGISTER <userName>")



                    elif(line[0]=="CONNECT") :

                        if (len(line) == 2) :

                            client.connect(line[1])

                        else :

                            print("Syntax error. Usage: CONNECT <userName>")

                    

                    elif(line[0]=="PUBLISH") :

                        if (len(line) >= 3) :

                            #  Remove first two words

                            description = ' '.join(line[2:])

                            client.publish(line[1], description)

                        else :

                            print("Syntax error. Usage: PUBLISH <fileName> <description>")



                    elif(line[0]=="DELETE") :

                        if (len(line) == 2) :

                            client.delete(line[1])

                        else :

                            print("Syntax error. Usage: DELETE <fileName>")



                    elif(line[0]=="LIST_USERS") :

                        if (len(line) == 1) :

                            client.listusers()

                        else :

                            print("Syntax error. Use: LIST_USERS")



                    elif(line[0]=="LIST_CONTENT") :

                        if (len(line) == 2) :

                            client.listcontent(line[1])

                        else :

                            print("Syntax error. Usage: LIST_CONTENT <userName>")



                    elif(line[0]=="DISCONNECT") :

                        if (len(line) == 2) :

                            client.disconnect(line[1])

                        else :

                            print("Syntax error. Usage: DISCONNECT <userName>")



                    elif(line[0]=="GET_FILE") :

                        if (len(line) == 4) :

                            client.getfile(line[1], line[2], line[3])

                        else :

                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")



                    elif(line[0]=="QUIT") :

                        if (len(line) == 1) :
                            if (client._user != None):
                                err = client.disconnect(client._user)
                                if err == client.RC.OK:
                                    break
                            
                            else:
                                break

                        else :

                            print("Syntax error. Use: QUIT")

                    else :

                        print("Error: command " + line[0] + " not valid.")

            except Exception as e:

                print("Exception: " + str(e))



    # *

    # * @brief Prints program usage

    @staticmethod

    def usage() :

        print("Usage: python3 client.py -s <server> -p <port>")





    # *

    # * @brief Parses program execution arguments

    @staticmethod

    def  parseArguments(argv) :

        parser = argparse.ArgumentParser()

        parser.add_argument('-s', type=str, required=True, help='Server IP')

        parser.add_argument('-p', type=int, required=True, help='Server Port')

        args = parser.parse_args()



        if (args.s is None):

            parser.error("Usage: python3 client.py -s <server> -p <port>")

            return False



        if ((args.p < 1024) or (args.p > 65535)):

            parser.error("Error: Port must be in the range 1024 <= port <= 65535");

            return False;

        

        client._server = args.s

        client._port = args.p



        return True





    # ******************** MAIN *********************

    @staticmethod

    def main(argv) :

        if (not client.parseArguments(argv)) :

            client.usage()

            return



        #  Write code here

        client.shell()

        print("+++ FINISHED +++")

    



if __name__=="__main__":

    client.main([])
