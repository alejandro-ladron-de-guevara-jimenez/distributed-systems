# Socket-Based Distributed System (Sistema de Sockets)

This repository contains the implementation of a distributed system based on sockets, developed as part of the Distributed Systems course. The project demonstrates a client-server architecture where multiple clients interact with a server to manipulate a simple text-based database using serialized data types.

---

## Project Structure

/SocketSystem/
│── app-cliente.c         # Client application with test scenarios
│── proxy-sock.c          # Client-side socket functions
│── servidor-sock.c       # Server-side code
│── claves.c              # Core functions for database manipulation
│── claves.h              # Header file for shared structures and functions
│── serial.c              # Serialization and deserialization functions
│── serial.h              # Header file for serialization functions
│── Makefile              # Compilation instructions
│── store.txt             # Database file (generated at runtime)
│── README.md             # Project explanation

---

## System Overview

The system implements a client-server architecture using sockets:

- Clients send requests to the server through a socket connection.
- Server processes each request and sends back results using serialized data types.
- Database storage is implemented as a text file `store.txt`.

Each request is encapsulated in a `Petición` struct, serialized into a character buffer with delimiters (`|` and `,`) to ensure compatibility across different architectures.

---

## Key Functions (`claves.c`)

- `destroy()`: Deletes the database file to reset all stored tuples.
- `set_value(key, value1, N_value2, V_value2, value3)`: Adds a new tuple if the key does not exist and N_value2 is between 1 and 32.
- `exist(key)`: Checks if a tuple with the given key exists; returns 1 if found, 0 otherwise.
- `get_value(key, value1, N_value2, V_value2, value3)`: Retrieves values of a tuple by key; returns an error if not found.
- `delete_key(key)`: Deletes a tuple with the given key using a temporary file for safe replacement.
- `modify_value(key, value1, N_value2, V_value2, value3)`: Modifies a tuple by deleting the old one and adding the updated version.

Additional functions for message exchange are defined in `serial.c` and `serial.h`:
- `sendMessage(int sockfd, struct Peticion *p)`: Serializes and sends a request through a socket.
- `recvMessage(int sockfd, struct Peticion *p)`: Receives and deserializes a request from a socket.

---

## Client/Server Service

- `servidor-sock.c`: Simulates the server, receives requests, creates a thread for each request, calls the appropriate function from `claves.c`, and returns results to the client.
- `proxy-sock.c`: Contains functions that allow clients to send requests and communicate with the server.
- Each request is processed concurrently using threads. Mutexes are used to avoid race conditions between threads.

The connection between client and server is established via `localhost` and a port number:
- The client reads the port from the environment variable `PORT_TUPLAS`.
- The server receives the port as a command-line argument (`./servidor <port>`).

---

## Compilation and Execution

- The project includes a `Makefile` that compiles:
  - `libclaves.so`: Shared library with core functions (from `proxy-sock.c` + `serial.c`)
  - Server executable: `servidor` (from `claves.c` + `servidor-sock.c` + `serial.c`)
  - Client executable: `cliente` (from `app-cliente.c` + `libclaves.so`)
- To compile everything (server, client, and shared library), simply run: `make`
- To run the system:
  1. Set environment variables in the client terminal:  
     `export IP_TUPLAS=localhost`  
     `export PORT_TUPLAS=<port>`  
  2. Start the server in another terminal:  
     `./servidor <port>`  
  3. Run the client:  
     `./cliente`
- Clients receive feedback on operations, and the server prints operation results.


---

## Flow Diagram

- Each client sends requests to the server.
- The server creates a thread per request.
- Threads process the request, access the database, and send results back to the corresponding client.
- Clients wait for responses before sending new requests.

---

## Test Scenarios

1. **Basic insertion and modification**: Tests `set_value()` and `modify_value()` sequentially to verify correctness.
2. **Concurrent insertion and modification**: Tests concurrency with 12 threads, each inserting or modifying tuples, to verify thread safety.
3. **Mass insertion**: Inserts 1000 tuples using 1000 threads, testing scalability and performance.
4. **Extra test**: Compares performance between sending raw C structs and serialized buffers.

Expected results are printed in the client terminal. The server prints 0 for successful operations and -1 for errors. The database `store.txt` reflects the changes made.

---

## Notes

- `destroy()` may print an error if the database file does not exist initially.
- Server is concurrent and avoids race conditions, but request processing order may vary.
- System was successfully compiled and executed on the Debian Virtual Lab ("Guernika").

---

Created by Alejandro Ladrón de Guevara Jiménez
