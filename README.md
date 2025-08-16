# 🏓 NetPong – CS3205 Assignment 2 (Holi'25)

A simple **two-player networked Ping Pong game** implemented in **C** using **ncurses** for graphics, **POSIX threads** for concurrency, and **sockets** for networking.  

The game allows one player to host as a **server** and another to join as a **client**, synchronizing gameplay in real-time over TCP.  

---

## ✨ Features
- Terminal-based graphics using `ncurses`.
- Two-player gameplay over network (Server ↔ Client).
- Threaded architecture:
  - **Ball thread** (runs only on server).
  - **Network thread** (sends/receives game state).
- Paddle movement with arrow keys.
- Live scoreboard showing Player A and Player B scores.
- Clean exit with `q`.

---

## ⚙️ How It Works
- **Server**:  
  - Runs the ball movement logic.  
  - Sends the full game state (ball + paddles + scores) to the client.  
  - Receives paddle input updates from the client.  

- **Client**:  
  - Sends paddle movement updates to the server.  
  - Receives the synchronized game state from the server.  
  - Displays the same graphics as the server.  

---

## 🖥️ Controls
- **Arrow Keys**: Move your paddle left and right.  
- **q**: Quit the game.  

- **Player A** (bottom paddle) → plays on **Server**.  
- **Player B** (top paddle) → plays on **Client**.  

---

## 🚀 How to Run

### 1. Compile
```bash
gcc -o netpong netpong.c -lncurses -lpthread
