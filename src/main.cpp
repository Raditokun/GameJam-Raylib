#include "Game.h"
#include "UDPReceiver.h"

int main() {
    Game game;
    game.Init();

    UDPReceiver udp(5005);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        udp.Poll();

        InputState in{};
        in.udpAlive          = udp.IsAlive();
        in.rightClickPressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

        if (in.udpAlive) {
            // Re-scale normalized UDP coords to the live window size every
            // frame so resizing the Raylib window still aligns with the hand.
            in.cursor           = { udp.NormalizedX() * (float)GetScreenWidth(),
                                    udp.NormalizedY() * (float)GetScreenHeight() };
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
            in.cursor           = GetMousePosition();
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
