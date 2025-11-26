# Distributed Parking Management System (Conceptual Design)

This project presents the conceptual design of a distributed system for managing access to a public parking facility. Unlike previous practices focused on code implementation, this exercise emphasizes **architecture, graphical representation, and system interactions**. The design illustrates how multiple client devices (gate controllers) interact with a central server to regulate vehicle entry and exit, maintain updated information on available spaces, and ensure smooth operation.

---

## System Overview

The system follows a **client-server architecture**:

- **Clients (Gate Controllers)**: Each gate is controlled by a small computer acting as a client. These devices manage user interactions such as entry/exit buttons, cameras, sensors, and display screens.
- **Server (Central Computer)**: A central computer maintains the database of vehicles and parking availability. It validates access requests, updates the number of free spaces, and synchronizes information across all gate controllers.

Communication between clients and the server is based on **socket message exchange**. Each client sends requests (entry/exit events, license plate images) to the server, which processes them and responds with updated information.

---

## Design Elements

### Client Devices
Each gate controller integrates:
- **Entry/Exit Buttons**: Trigger requests to open or close the gate.
- **Camera**: Captures license plate images for identification.
- **Gate Mechanism**: Opens or closes based on server authorization.
- **Sensor**: Detects vehicle presence to ensure safe operation.
- **Display Screen**: Shows the current number of available spaces.

### Central Server
The server maintains:
- **Vehicle Database**: Stores information about vehicles currently inside the parking facility.
- **Space Availability Counter**: Tracks the number of free spaces.
- **Synchronization Service**: Sends updated availability data to all gate controllers whenever changes occur.

---

## Diagrams (Conceptual)

### Client Workflow
1. User presses entry/exit button.  
2. Camera captures license plate image.  
3. Client sends request + image to server.  
4. Server validates request and responds with authorization.  
5. Gate opens/closes accordingly.  
6. Sensor confirms vehicle passage.  
7. Display updates free space count.  

### Server Workflow
1. Server waits for client requests.  
2. Upon receiving a request, it creates a thread to handle it.  
3. Thread accesses the vehicle database and free space counter.  
4. If spaces are available, the vehicle is registered and the counter updated.  
5. If no spaces are available, access is denied.  
6. Updated information is sent back to the requesting client and broadcast to all others.  
7. When a vehicle exits, its record is removed and the counter is incremented.  

---

## Key Features

- **Concurrency**: Server uses threads to handle multiple simultaneous requests.  
- **Synchronization**: Mutexes and condition signals prevent race conditions when updating the database.  
- **Scalability**: Additional gate controllers can be added without altering the central logic.  
- **Data Types**:  
  - Free spaces stored as integers.  
  - License plate images stored in `.jpg` format.  

---

## Notes and Conclusions

This conceptual design demonstrates how distributed systems can be applied to real-world scenarios such as parking management. By combining **client-server communication, concurrency, and synchronized data sharing**, the system ensures efficient control of vehicle access and accurate tracking of available spaces.  

The graphical representation highlights the interaction between physical devices (buttons, sensors, cameras, gates) and the central server, showing how distributed computing principles can be applied beyond pure software into integrated hardware environments.

---

Created by Alejandro Ladrón de Guevara Jiménez