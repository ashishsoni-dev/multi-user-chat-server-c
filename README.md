# Multi-User Chat Server in C (Winsock)

## About
A simple multi-user chat server built in C using Winsock.  
Supports multiple clients, private messaging, and real-time communication.

---
## 📂 Project Structure
- chat_server.c → Main server file
---

## Features
- Handles multiple clients using threads (CreateThread)
- Broadcast messages to all users
- Send private messages using `/msg`
- View connected users with `/list`
- Exit chat using `/exit`
- Handles TCP stream properly (no broken messages)
- Uses basic synchronization (Critical Sections)

---

## Tech Stack
- C
- Winsock2 (TCP sockets)
- Windows API (threads and synchronization)

---

## How to Run

1. Compile:
gcc server.c -o server.exe -lws2_32

2. Start server:
server.exe

3. Connect clients:
telnet localhost 8080

---

## Commands
- `/msg <username> <message>` → send private message  
- `/list` → show users  
- `/exit` → leave chat  

---

## What I Learned
- How TCP actually works (stream-based)
- Handling multiple clients using threads
- Avoiding data conflicts using synchronization
- Building a basic real-time system from scratch

---

## Future Improvements
- Create a custom client instead of telnet
- Add a proper UI
- Improve error handling

---

## Author
Ashish Soni
