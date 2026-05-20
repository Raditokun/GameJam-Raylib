# How To Play — *Soaring Caraka*

## Introduction

Welcome to **Soaring Caraka** — a tower-defense game with no keyboard, no mouse, and no gamepad.

Everything is controlled with **your hands in front of the webcam**. A Python vision server (powered by MediaPipe Hands) watches your camera in real time, identifies your right and left hands separately, and streams gesture data to the game over a UDP bridge. The result: you place towers by pointing, fire your Ultimate by clenching a fist, and sell with an open palm. No peripherals — just you, your camera, and the aliens.

> **Quick Start:** Launch `vision/tracker.py`, then run `game.exe`. Watch for the **green crosshair** — that means the bridge is live. (If it's **red**, the Python server isn't broadcasting.)

---

## Right Hand Controls — *Primary Actions*

Your right hand drives the cursor and the moment-to-moment combat decisions.

- **Move Cursor** — Point with your **Index Finger**. The tip of your index finger *is* the cursor.
- **Click / Drag & Drop** — **Pinch** your Index Finger and Thumb together. Pinching once selects cards, places towers, and confirms menu choices.
- **Start Next Wave** — Hold up exactly **2 fingers** (Index + Middle, like a "✌️ Peace" sign).
- **Use Hero Ultimate** — Make a **closed fist** (0 fingers extended). Only fires when your Ult meter is charged.

---

## Left Hand Controls — *Tactical Actions*

Your left hand handles tower management. The right hand still aims — left-hand gestures fire on whatever your right-hand cursor is hovering over.

- **Upgrade Tower** — Hold up exactly **1 finger** (Index only).
  - *Note: ensure your right-hand cursor is hovering over the tower you want to upgrade.*
- **Sell Tower** — **Open your hand completely** (all 5 fingers extended).
  - *Note: ensure your right-hand cursor is hovering over the tower you want to sell.*

---

## Tips for Best Tracking

A little setup goes a long way toward reliable gesture recognition.

- **Light your room well.** MediaPipe relies on clear contrast between your skin and the background. Sunlight or a warm desk lamp in front of you works far better than a backlit silhouette.
- **Keep both hands visible.** If a hand leaves the camera frame, its gestures stop registering until it returns. Sit so the camera can see from your knuckles to your fingertips.
- **Use a plain, non-distracting background.** Cluttered backgrounds with skin-tone objects can confuse the tracker.
- **Hold gestures clearly and deliberately.** Make your peace sign or fist crisp — partial poses can be misread (e.g., a lazy fist with a half-extended thumb won't fire the Ultimate).
- **Keep your hands ~30–80 cm from the camera.** Too close and fingers leave the frame; too far and the landmarks get noisy.
- **Watch the crosshair color.**
  - 🟢 **Green** — UDP bridge is live, gestures are flowing.
  - 🔴 **Red** — Python tracker is offline; the game has fallen back to mouse + keyboard.
- **One hand at a time is fine.** Only need to upgrade? Just raise your left index — your right hand can rest. The game treats each hand independently.

---

*Good luck, commander. The waves are coming.*
