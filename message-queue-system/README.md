# Message Queue System (Sistema de Colas de Mensajes)

This repository contains the implementation of a distributed system based on message queues, developed as part of the Distributed Systems course. The project demonstrates a client-server architecture where multiple clients interact with a server to manipulate a simple text-based database.

---

## Project Structure

/MessageQueueSystem/
│── app-cliente-1.c       # Client 1 example code
│── app-cliente-2.c       # Client 2 example code
│── proxy-mq.c            # Client-side message queue functions
│── servidor-mq.c         # Server-side code
│── claves.c              # Core functions for database manipulation
│── claves.h              # Header file for shared structures and functions
│── Makefile              # Compilation instructions
│── store.txt             # Database file (generated at runtime)
│── README.md             # Project explanation

---

## System Overview

The system implements a client-server architecture using message queues:

- Clients send requests to the server through a server queue.
- Server processes each request and sends back results through the client's individual queue.
- Database storage is implemented as a text file `store.txt`.

Each client has a unique queue following the pattern `/cliente_%d_%ld`, where `%d` is the client process ID and `%ld` is the thread ID. The server queue is named using a group identifier, e.g., `/100495773`.

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

- `servidor-mq.c`: Simulates the server, receives requests, creates a thread for each request, calls the appropriate function from `claves.c`, and returns results to the client.
- `proxy-mq.c`: Contains functions that allow clients to send requests and communicate with the server.
- Each request is encapsulated in a `Petición` struct with key, tuple values, operation type, client queue name, and result.
- Mutexes and condition variables are used to avoid race conditions between threads.

---

## Compilation and Execution

- The project includes a `Makefile` that compiles:
  - `libclaves.so`: Shared library with core functions.
  - Server executable: `servidor` (from `claves.c` + `servidor-mq.c`)
  - Client executables: `cliente1` and `cliente2` (from `app-cliente-1.c` or `app-cliente-2.c` + `libclaves.so`)
- To compile everything (server, clients, and shared library), simply run: `make`
- To run the system:
  1. Start the server: `./servidor`
  2. Start any client: `./cliente1` or `./cliente2`
- Clients receive feedback on operations, and the server prints operation results.

---

## Flow Diagram

- Each client sends requests to the server.
- The server creates a thread per request.
- Threads process the request, access the database, and send results back to the corresponding client.
- Clients wait for responses before sending new requests.

---

## Test Scenarios

1. **Client1**: Tests all functions in `claves.c` (`destroy`, `set_value`, `modify_value`, `get_value`) sequentially to verify correctness.
2. **Client2**: Tests concurrency with 12 threads, each inserting a tuple using `set_value`, to verify thread safety and server handling.

Expected results are printed in the client terminal. The server prints 0 for successful operations and -1 for errors. The database `store.txt` reflects the changes made.

---

## Notes

- `destroy()` may print an error if the database file does not exist initially.
- Server is concurrent and avoids race conditions, but request processing order may vary.
- System was successfully compiled and executed on the Debian Virtual Lab ("Guernika").


---

Created by Alejandro Ladrón de Guevara Jiménez
