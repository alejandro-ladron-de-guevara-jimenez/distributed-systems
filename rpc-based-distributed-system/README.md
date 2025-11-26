# RPC-Based Distributed System (Sistema de RPC)

This repository contains the implementation of a distributed system based on Remote Procedure Calls (RPC), developed as part of the Distributed Systems course. The project demonstrates a client-server architecture where multiple clients interact with a server to manipulate a simple text-based database using C structures and RPC-generated code.

---

## Project Structure

/RPCSystem/
│── app-cliente.c         # Client application with test scenarios
│── proxy-rpc.c           # Client-side RPC functions
│── servidor-rpc.c        # Server-side code
│── claves.c              # Core functions for database manipulation
│── claves.h              # Header file for shared structures and functions
│── tuplas.x              # RPC specification file
│── tuplas.h              # Generated header with shared definitions
│── tuplas_xdr.c          # Generated serialization/deserialization functions
│── tuplas_clnt.c         # Generated client-side RPC stubs
│── tuplas_svc.c          # Generated server-side RPC stubs
│── Makefile.tuplas       # Compilation instructions
│── store.txt             # Database file (generated at runtime)
│── README.md             # Project explanation

---

## System Overview

The system implements a client-server architecture using RPC:

- Clients send requests to the server through RPC calls defined in `tuplas.x`.
- Server processes each request and sends back results using serialized data structures.
- Database storage is implemented as a text file `store.txt`.

RPC structures include:
- `CoordAux`: Auxiliary coordinates structure.
- `Petición`: Input structure for client requests.
- `Respuesta`: Container structure for server responses.
- `RespuestaGetValue`: Structure for returning values from `get_value()`.

Generated files (`tuplas.h`, `tuplas_xdr.c`, `tuplas_clnt.c`, `tuplas_svc.c`) provide serialization, client stubs, and server stubs to support communication.

---

## Key Functions (`claves.c`)

- `destroy()`: Deletes the database file to reset all stored tuples.
- `set_value(key, value1, N_value2, V_value2, value3)`: Adds a new tuple if the key does not exist and N_value2 is between 1 and 32.
- `exist(key)`: Checks if a tuple with the given key exists; returns 1 if found, 0 otherwise.
- `get_value(key, value1, N_value2, V_value2, value3)`: Retrieves values of a tuple by key; returns an error if not found.
- `delete_key(key)`: Deletes a tuple with the given key using a temporary file for safe replacement.
- `modify_value(key, value1, N_value2, V_value2, value3)`: Modifies a tuple by deleting the old one and adding the updated version.

---

## Client/Server Service

- `servidor-rpc.c`: Simulates the server, receives requests, deserializes them using `tuplas_xdr.c`, calls the appropriate function from `claves.c`, and returns results to the client.
- `proxy-rpc.c`: Contains functions that allow clients to send requests and communicate with the server through RPC.
- `tuplas_clnt.c`: Provides client-side stubs for invoking remote functions.
- `tuplas_svc.c`: Provides server-side stubs for handling remote calls.

The connection between client and server is managed by the **portmapper**, which associates RPC programs with communication ports.  
- The server registers automatically with the portmapper when started.  
- The client initializes a connection with `rpc_init()`, sends requests, and closes the connection with `rpc_close()` after each call.  
- Concurrency is handled by mutexes and condition signals to avoid race conditions.

---

## Compilation and Execution

- The project includes a `Makefile.tuplas` that compiles:
  - `libclaves.so`: Shared library with core functions and RPC stubs (`proxy-rpc.c`, `tuplas_clnt.c`, `tuplas_xdr.c`)
  - Server executable: `servidor` (from `claves.c` + `servidor-rpc.c` + `tuplas_svc.c` + `tuplas_xdr.c`)
  - Client executable: `cliente` (from `app-cliente.c` + `libclaves.so`)
- To compile everything (server, client, and shared library), simply run:  
  `make -f Makefile.tuplas`
- To run the system:
  1. Start the server: `./servidor`
  2. Start the client: `./cliente`
- Clients receive feedback on operations, and the server prints operation results.

---

## Flow Diagram

- The server registers with the portmapper when started.
- Clients connect to the server using RPC stubs.
- Each client sends requests to the server.
- The server deserializes requests, processes them with `claves.c`, and serializes results.
- Clients receive responses before sending new requests.

---

## Test Scenarios

1. **Basic insertion and modification**: Tests `set_value()` and `modify_value()` sequentially to verify correctness.
2. **Concurrent insertion and modification**: Tests concurrency with 12 threads, each inserting or modifying tuples, to verify thread safety.
3. **Existence and deletion**: Tests `exist()` and `delete_key()` to verify correct tuple removal.
4. **Mass insertion**: Inserts 1000 tuples using 1000 threads, testing scalability and performance.

Expected results are printed in the client terminal. The server prints feedback for each operation. The database `store.txt` reflects the changes made.

---

## Notes

- `destroy()` may print an error if the database file does not exist initially.
- Server is concurrent and avoids race conditions, but request processing order may vary.
- System was successfully compiled and executed on the Debian Virtual Lab ("Guernika").

---

Created by Alejandro Ladrón de Guevara Jiménez