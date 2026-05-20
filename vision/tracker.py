import cv2
import mediapipe as mp
import socket
import math

mp_hands_mod = mp.solutions.hands
mp_draw_mod  = mp.solutions.drawing_utils

# --- Config ---
EMA_ALPHA       = 0.5
CLICK_THRESHOLD = 0.05
UDP_IP          = "127.0.0.1"
UDP_PORT        = 5005
DEBUG_INTERVAL  = 10.\

# --- Init ---
cap  = cv2.VideoCapture(0)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

hands = mp_hands_mod.Hands(
    max_num_hands=1,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.5,
)

smoothed_x = 0.5
smoothed_y = 0.5
frame_count = 0

print(f"[tracker] UDP -> {UDP_IP}:{UDP_PORT}  |  press 'q' to quit")

while cap.isOpened():
    ok, frame = cap.read()
    if not ok:
        break

    frame = cv2.flip(frame, 1)
    rgb   = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    is_clicking = 0

    if result.multi_hand_landmarks:
        lm = result.multi_hand_landmarks[0].landmark

        # Landmark 4 = Thumb Tip, Landmark 8 = Index Finger Tip
        tx, ty = lm[4].x, lm[4].y
        ix, iy = lm[8].x, lm[8].y

        dist = math.sqrt((ix - tx) ** 2 + (iy - ty) ** 2)
        is_clicking = 1 if dist < CLICK_THRESHOLD else 0

        # EMA smoothing
        smoothed_x = EMA_ALPHA * ix + (1.0 - EMA_ALPHA) * smoothed_x
        smoothed_y = EMA_ALPHA * iy + (1.0 - EMA_ALPHA) * smoothed_y

        mp_draw_mod.draw_landmarks(frame, result.multi_hand_landmarks[0], mp_hands_mod.HAND_CONNECTIONS)

    packet = f"{smoothed_x:.4f},{smoothed_y:.4f},{is_clicking}"
    sock.sendto(packet.encode(), (UDP_IP, UDP_PORT))

    frame_count += 1
    if frame_count % DEBUG_INTERVAL == 0:
        print(f"[UDP] {packet}")

    cv2.imshow("Hand Tracker — press q to quit", frame)
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
sock.close()
print("[tracker] Exited cleanly.")
