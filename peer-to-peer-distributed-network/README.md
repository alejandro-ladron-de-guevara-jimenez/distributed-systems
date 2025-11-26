# Distributed System with Python Clients, C Server, Web Service, and RPC

This repository contains the implementation of a distributed system developed as part of the Distributed Systems course. The project demonstrates a hybrid client-server architecture where Python clients interact with a C-based server, supported by a SOAP web service and an RPC subsystem. The system simulates a peer-to-peer network for file publication and retrieval.

---

## Project Structure

/FinalDistributedSystem/
│── client.py             # Python client application with all user commands
│── web_service.py        # Python SOAP web service providing date/time
│── server.c              # Main server in C handling client requests
│── rpc.x                 # RPC specification file (XDR definitions)
│── rpc.h                 # Generated header with RPC definitions
│── rpc_clnt.c            # Generated client-side RPC stubs
│── rpc_svc.c             # Generated server-side RPC stubs
│── rpc_xdr.c             # Generated serialization/deserialization functions
│── rpc_server.c          # RPC server implementation
│── Makefile              # Compilation instructions
│── store.txt             # Database file (generated at runtime)
│── README.md             # Project explanation

---

## System Overview

The system implements a distributed architecture combining sockets, web services, and RPC:

- **Python clients (`client.py`)** send requests to the C server using sockets.  
- **Server (`server.c`)** processes each request, manages user registration, connections, publications, and file transfers.  
- **Web service (`web_service.py`)** provides date and time information via SOAP, which is included in every client request.  
- **RPC subsystem** (`rpc.x`, `rpc_server.c`, generated stubs) handles remote printing of client operations for monitoring.  

Database storage is implemented as a text file `store.txt`, where user information and publications are recorded.

---

## Key Functions

### Client (`client.py`)
- `REGISTER`: Registers a new user. Returns 0 if successful, 1 if already exists, 2 for errors.  
- `UNREGISTER`: Removes a user. Returns 0 if successful, 1 if not found, 2 for errors.  
- `CONNECT`: Connects a user, opening a thread to listen for incoming file requests. Returns 0 if successful, 1 if not registered, 2 if already connected.  
- `DISCONNECT`: Disconnects a user, closing the thread. Returns 0 if successful, 1 if not found, 2 if not connected.  
- `PUBLISH`: Publishes a file with description. Returns 0 if successful, 2 if not connected, 3 if duplicate, 4 for errors.  
- `DELETE`: Deletes a publication. Returns 0 if successful, 2 if not connected, 3 if not found, 4 for errors.  
- `LIST_USERS`: Lists all connected users with IP and port.  
- `LIST_CONTENT`: Lists publications of a specific user.  
- `GET_FILE`: Retrieves a file from another connected user via sockets.  
- `QUIT`: Exits the system, disconnecting if necessary.  

### Server (`server.c`)
- Handles all client requests via threads (`tratar_mensaje`).  
- Manages user registration, connection states, publications, and file transfers.  
- Stores user and publication data in text files.  

### Web Service (`web_service.py`)
- Provides `get_datetime()` via SOAP.  
- Implemented with Spyne and WSGI, running on `localhost:8000`.  
- Each client request includes the current date/time string obtained from this service.  

### RPC (`rpc.x` and generated files)
- Defines program `IMPRIMIR_PROG` with function `IMPRIMIR_STRINGS`.  
- RPC server (`rpc_server.c`) prints received operations with user, operation, file, and timestamp.  
- Generated stubs (`rpc_clnt.c`, `rpc_svc.c`, `rpc_xdr.c`) handle serialization and communication.  

---

## Client/Server Service

- **Client side**: Python client encodes requests in XDR format, sends them via sockets, and includes date/time from the web service.  
- **Server side**: C server receives requests and returns results.  
- **RPC service**: Runs separately, printing operations for monitoring.  
- **Concurrency**: Threads are used both in the client (for listening to incoming file requests) and in the server (for handling multiple client requests simultaneously).  

---

## Compilation and Execution

- The project includes a `Makefile` that compiles:
  - Server executable: `servidor` (from `server.c`)
  - RPC server executable: `servidor_rpc` (from `rpc_server.c` + generated stubs)
- Python scripts (`client.py`, `web_service.py`) do not require compilation.  
- To compile everything, simply run:  
  `make -f Makefile.rpc`
- To run the system:
  1. Start the C server: `./servidor <port>`
  2. Start the web service: `python3 web_service.py`
  3. Start the RPC server: `./servidor_rpc`
  4. Start the client: `python3 client.py -s localhost -p <port>`
- Clients receive feedback on operations, including date/time from the web service. The server prints operations, and the RPC server logs them separately.

---

## Flow Diagram

- The server starts and listens on a specified port.  
- The web service runs independently, providing date/time.  
- Clients connect to the server, sending requests with date/time included.  
- The server processes requests, updates `store.txt`, and returns results.  
- The RPC server prints operations remotely for monitoring.  
- Clients wait for responses before sending new requests.  

---

## Test Scenarios

1. **REGISTER/UNREGISTER**: Register a user, verify success, attempt duplicate registration, unregister, and verify removal.  
2. **CONNECT/DISCONNECT**: Connect a user, verify success, attempt duplicate connection, disconnect, and verify removal.  
3. **PUBLISH/DELETE**: Publish files with descriptions, verify success, attempt duplicate publication, delete files, and verify removal.  
4. **LIST_USERS**: List all connected users with IP and port.  
5. **LIST_CONTENT**: List publications of a specific user, verify correct output and error handling.  
6. **GET_FILE**: Retrieve files from another connected user, verify correct transfer and error handling.  

Expected results are printed in the client terminal, including date/time from the web service. The server prints operations, and the RPC server logs them separately.

---

## Notes

- `destroy()` is not explicitly included but file operations simulate database reset.  
- The system combines sockets, SOAP web services, and RPC, demonstrating integration of multiple distributed technologies.  
- Server and client are concurrent, avoiding race conditions with threads.  
- System was successfully compiled and executed on the Debian Virtual Lab ("Guernika").  

---

Created by Alejandro Ladrón de Guevara Jiménez
