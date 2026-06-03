# Project Overview: C++ Raylib Game with Python Gesture Bridge
This is a high-performance, gesture-controlled game. The architecture is split into two distinct parts:
1. **The Game Client (C++):** Built with Raylib. Handles all rendering, state machines, and game logic.
2. **The Vision Server (Python):** Uses OpenCV and MediaPipe Hands to track the user's hand and transmits the data to the C++ client in real-time.

**CORE ARCHITECTURE RULE:** The two systems communicate EXCLUSIVELY via a local UDP Socket Bridge. Do not attempt to compile MediaPipe into the C++ project.

## 1. Tech Stack
* **Game Engine:** C++17 (or newer) using Raylib.
* **Computer Vision:** Python 3.x, OpenCV (`cv2`), Google MediaPipe Hands.
* **Networking:** UDP Sockets (`sys/socket.h` or `Winsock2` on C++, `socket` module in Python).

## 2. The UDP Bridge Protocol
* **One-Way Traffic:** Python sends data, C++ receives data. C++ does not need to talk back.
* **Packet Format:** Send data as a simple, comma-separated string. The bridge has grown to 8 fields: `"X,Y,IS_CLICKING,START_WAVE,UPGRADE,SELL,ULT,HAND_PRESENT"`. (Example: `"0.450,0.620,1,0,0,0,0,1"`). All flags are `0`/`1`; `HAND_PRESENT` is `1` while the right (cursor) hand is in frame so the C++ client can hide the crosshair instead of freezing it. The C++ receiver strictly rejects any packet that is not exactly this field count, which keeps both halves in sync.
* **Normalized Coordinates:** Python must send `X` and `Y` as normalized values between `0.0` and `1.0`. The C++ client will multiply these by the current Raylib screen width/height. This prevents scaling bugs if the Raylib window is resized.
* **Non-Blocking C++:** The C++ UDP receiver MUST be configured as non-blocking. The game loop must never wait for a network packet, otherwise the game will freeze if the Python script drops a frame.

## 3. Directory Structure
Ensure code generation respects this separation of concerns:
* `game/` -> Contains all C++ source code (`main.cpp`, game logic, UDP listener classes).
* `vision/` -> Contains all Python source code (`tracker.py`, MediaPipe logic).
* `assets/` -> Shared graphics, audio, and UI elements for the C++ client.

## 4. Coding Style & AI Instructions
* **C++:** Write modern, clean C++ (use `std::string`, smart pointers where applicable, avoid raw `new/delete`). Keep Raylib drawing calls strictly between `BeginDrawing()` and `EndDrawing()`.
* **Python:** Implement Exponential Moving Average (EMA) smoothing on the MediaPipe coordinates before sending the UDP packet to prevent cursor jitter.
* **Read Before Writing:** Always review the existing files before proposing major changes. Provide only the specific blocks of code that need modification.

## 5. Output & Handoff Protocol
Whenever you complete a coding task, create new files, or modify existing logic, you MUST output a "Changelog Summary" in Markdown at the end of your response. Use this exact format:

### Changelog Summary
* **Files Created:** [List files]
* **Files Modified:** [List files]
* **Logic Implemented:** [1-2 sentences explaining what the new code does, especially noting any network protocol changes or C++ memory management].