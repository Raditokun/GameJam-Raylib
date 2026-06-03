#include "Game.h"
#include "UDPReceiver.h"

// Inter-packet smoothing factor for the hand cursor. The webcam tracker runs
// ~30 fps while the game renders at 60, so each frame we lerp the crosshair a
// fraction of the way toward the latest UDP position for fluid motion. Python
// already applies EMA smoothing upstream, so keep this light; raise toward 1.0
// to effectively disable it if the cursor ever feels laggy.
constexpr float CURSOR_LERP = 0.5f;

int main() {
    Game game;
    game.Init();

    UDPReceiver udp(5005);

    // Smoothed, screen-space cursor carried across frames (starts centered).
    Vector2 smoothedCursor = { GetScreenWidth()  / 2.0f,
                               GetScreenHeight() / 2.0f };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        udp.Poll();

        InputState in{};
        in.udpAlive          = udp.IsAlive();
        in.rightClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

        if (in.udpAlive) {
            in.handPresent      = udp.HandPresent();
            // Re-scale normalized UDP coords to the live window size every
            // frame so resizing the Raylib window still aligns with the hand.
            Vector2 target      = { udp.NormalizedX() * (float)GetScreenWidth(),
                                    udp.NormalizedY() * (float)GetScreenHeight() };
            if (in.handPresent) {
                // Ease toward the latest sample for smooth, jitter-free motion.
                smoothedCursor.x += (target.x - smoothedCursor.x) * CURSOR_LERP;
                smoothedCursor.y += (target.y - smoothedCursor.y) * CURSOR_LERP;
            } else {
                // Hand left the frame (crosshair is being hidden) — snap so it
                // doesn't visibly slide in from a stale spot when it returns.
                smoothedCursor = target;
            }
            in.cursor           = smoothedCursor;
            in.clickDown        = udp.ClickDown();
            in.clickPressed     = udp.ClickPressed();
            in.clickReleased    = udp.ClickReleased();
            in.startWavePressed = udp.StartWavePressed();
            in.upgradePressed   = udp.UpgradePressed();
            in.sellPressed      = udp.SellPressed();
            in.ultPressed       = udp.UltPressed();
        } else {
            // No Python feed -> hardware mouse fallback for dev-without-camera.
            // Flips back automatically the moment tracker.py starts broadcasting.
            in.handPresent      = true;            // mouse cursor is always present
            smoothedCursor      = GetMousePosition();
            in.cursor           = smoothedCursor;
            in.clickDown        = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            in.clickPressed     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            in.clickReleased    = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
            // No mouse analog for these gestures — keyboard/right-mouse
            // fallbacks are applied inside HandleInput via ||.
            in.startWavePressed = false;
            in.upgradePressed   = false;
            in.sellPressed      = false;
            in.ultPressed       = false;
        }

        game.Update(dt, in);

        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}
