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
DEBUG_INTERVAL  = 10

# MediaPipe landmark indices (per the official hand model)
#   Thumb : MCP=2, IP=3,   Tip=4
#   Index : MCP=5, PIP=6,  Tip=8
#   Middle: MCP=9, PIP=10, Tip=12
#   Ring  : MCP=13,PIP=14, Tip=16
#   Pinky : MCP=17,PIP=18, Tip=20
FINGER_TIPS = {"thumb": 4, "index": 8, "middle": 12, "ring": 16, "pinky": 20}
FINGER_PIPS = {            "index": 6, "middle": 10, "ring": 14, "pinky": 18}


def count_extended_fingers(lm, actual_hand_label):
    """Return (extended_set, count). `lm` is a list of normalized landmarks.

    `actual_hand_label` is the USER-perspective label ("Left" / "Right") *after*
    we have already compensated for the cv2.flip(frame, 1) mirror — see main loop.
    The thumb test uses it to pick the correct sideways direction.
    """
    extended = set()

    # Non-thumb fingers: tip is above (smaller y) the PIP joint when extended.
    for name in ("index", "middle", "ring", "pinky"):
        if lm[FINGER_TIPS[name]].y < lm[FINGER_PIPS[name]].y:
            extended.add(name)

    # Thumb: tip lies sideways relative to the IP joint. Direction depends on
    # which hand we're looking at (mirror-corrected).
    thumb_tip_x = lm[4].x
    thumb_ip_x  = lm[3].x
    if actual_hand_label == "Right":
        # User's right hand: thumb extends to the LEFT in the (already-mirrored) image
        if thumb_tip_x < thumb_ip_x:
            extended.add("thumb")
    else:  # Left hand
        if thumb_tip_x > thumb_ip_x:
            extended.add("thumb")

    return extended, len(extended)


# --- Init ---
cap  = cv2.VideoCapture(0)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

hands = mp_hands_mod.Hands(
    max_num_hands=2,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.5,
)

smoothed_x = 0.5
smoothed_y = 0.5
frame_count = 0

print(f"[tracker] UDP -> {UDP_IP}:{UDP_PORT}  |  press 'q' to quit")
print("[tracker] Gestures:")
print("  Right hand: index tip = cursor, pinch = click, peace-sign = start wave, fist = ult")
print("  Left  hand: index pointing = upgrade, open palm = sell")

while cap.isOpened():
    ok, frame = cap.read()
    if not ok:
        break

    frame  = cv2.flip(frame, 1)   # mirror — MediaPipe handedness will need swapping below
    rgb    = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    # Per-frame outputs (default off)
    is_clicking = 0
    start_wave  = 0
    upgrade     = 0
    sell        = 0
    ult         = 0
    right_hand_present = 0   # 1 once the right (cursor) hand is seen this frame

    if result.multi_hand_landmarks and result.multi_handedness:
        for hand_lms, handedness in zip(result.multi_hand_landmarks,
                                        result.multi_handedness):
            mp_label = handedness.classification[0].label  # "Left" or "Right"
            # Since the frame was mirrored before processing, MediaPipe's label
            # is the OPPOSITE of the user's actual hand. Swap it.
            actual_label = "Right" if mp_label == "Left" else "Left"

            lm = hand_lms.landmark
            extended, n_ext = count_extended_fingers(lm, actual_label)

            if actual_label == "Right":
                # The right hand drives the cursor, so its presence is what the
                # game uses to decide whether to show or hide the crosshair.
                right_hand_present = 1
                # --- Cursor + click (right hand only) -------------------
                tx, ty = lm[4].x, lm[4].y     # thumb tip
                ix, iy = lm[8].x, lm[8].y     # index tip
                dist = math.sqrt((ix - tx) ** 2 + (iy - ty) ** 2)
                is_clicking = 1 if dist < CLICK_THRESHOLD else 0

                # EMA smoothing on index tip
                smoothed_x = EMA_ALPHA * ix + (1.0 - EMA_ALPHA) * smoothed_x
                smoothed_y = EMA_ALPHA * iy + (1.0 - EMA_ALPHA) * smoothed_y

                # --- Action gestures (right hand) ------------------------
                # Peace sign: exactly index + middle, nothing else.
                if n_ext == 2 and extended == {"index", "middle"}:
                    start_wave = 1
                # Closed fist: nothing extended.
                if n_ext == 0:
                    ult = 1

            else:  # Left hand
                # --- Action gestures (left hand) -------------------------
                # Pointing: exactly the index finger.
                if n_ext == 1 and "index" in extended:
                    upgrade = 1
                # Open palm: all five fingers.
                if n_ext == 5:
                    sell = 1

            mp_draw_mod.draw_landmarks(frame, hand_lms, mp_hands_mod.HAND_CONNECTIONS)

    # 8-field packet: cursor X,Y + 5 action bits + hand-present bit
    packet = (f"{smoothed_x:.4f},{smoothed_y:.4f},"
              f"{is_clicking},{start_wave},{upgrade},{sell},{ult},{right_hand_present}")
    sock.sendto(packet.encode(), (UDP_IP, UDP_PORT))

    frame_count += 1
    if frame_count % DEBUG_INTERVAL == 0:
        print(f"[UDP] {packet}")

    cv2.imshow("Hand Tracker -- press q to quit", frame)
    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()
sock.close()
print("[tracker] Exited cleanly.")
