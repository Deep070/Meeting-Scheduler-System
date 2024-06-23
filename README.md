# Meeting-Scheduler-System
 

EE450 Socket Programming Project - Spring 2023

Project Overview:
This project implements a meeting scheduler using UNIX socket programming. The scheduler helps efficiently schedule meetings by maintaining user availability on backend servers and processing client requests to find common available times for meetings.

Objective:
To familiarize students with UNIX socket programming and develop a system that handles user availability and schedules meetings based on intersection of available times.

System Components:
- Client: Initiates requests for scheduling meetings.
- Main Server (serverM): Coordinates with backend servers to retrieve and process user availability data.
- Backend Servers (serverA and serverB): Store user availability and compute common available time slots.

Project Structure:
Meeting Scheduler System 

│

├── client.cpp

├── serverM.cpp

├── serverA.cpp

├── serverB.cpp

├── Makefile

├── README.txt

├── a.txt

└── b.txt

Source Files:
- client.cpp: Contains the client-side code for initiating meeting requests.
- serverM.cpp: Contains the main server code that coordinates between the client and backend servers.
- serverA.cpp and serverB.cpp: Contain the backend server codes that store user availability and compute time intersections.

Input Files:
- a.txt and b.txt: Files containing user availability data for backend servers A and B, respectively. Each file lists user availability in the format username;[[start_time, end_time], ...].

Running the Project:
1. Compile the Project:
   Use the provided Makefile to compile the project.
   ```
   make all
   ```

2. Start the Servers and Client:
   Open four separate terminals and start the servers and client in the following order:
   ```
   ./serverM
   ./serverA
   ./serverB
   ./client
   ```

On-Screen Messages:
Each component prints specific messages to the screen to indicate the progress and status of operations:

Main Server (serverM)
- "Main Server is up and running."
- "Main Server received the username list from server<A or B> using UDP over port <port number>."
- "Main Server received the request from client using TCP over port <port number>."
- "Found <username1, username2, ...> located at Server <A or B>. Send to Server <A or B>."
- "Main Server received from server <A or B> the intersection result using UDP over port <port number>: <result>."
- "Main Server sent the result to the client."

Backend Servers (serverA and serverB)
- "Server <A or B> is up and running using UDP on port <port number>."
- "Server <A or B> finished sending a list of usernames to Main Server."
- "Server <A or B> received the usernames from Main Server using UDP over port <port number>."
- "Found the intersection result: <result> for <username1, username2, ...>."
- "Server <A or B> finished sending the response to Main Server."

Client
- "Client is up and running."
- "Please enter the usernames to check schedule availability: <username1> <username2> ..."
- "Client finished sending the usernames to Main Server."
- "Client received the reply from Main Server using TCP over port <port number>: <result>."
