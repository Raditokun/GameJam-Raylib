#include "Game.h"
#include "UDPReceiver.h"

// Faktor smoothing antar-paket untuk kursor tangan. Tracker webcam berjalan
// ~30 fps sedangkan game render di 60, jadi tiap frame crosshair di-lerp
// sebagian menuju posisi UDP terbaru agar gerakan mulus. Python sudah
// menerapkan smoothing EMA di hulu, jadi buat ini ringan; naikkan ke 1.0
// untuk praktis menonaktifkannya jika kursor terasa lambat.
constexpr float CURSOR_LERP = 0.5f;

int main() {
    Game game;
    game.Init();

    UDPReceiver udp(5005);

    // Kursor ruang-layar yang sudah dihaluskan, dibawa antar-frame (mulai di tengah).
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
            // Sesuaikan skala koordinat UDP ke ukuran window aktif tiap
            // frame agar resize window Raylib tetap selaras dengan tangan.
            Vector2 target      = { udp.NormalizedX() * (float)GetScreenWidth(),
                                    udp.NormalizedY() * (float)GetScreenHeight() };
            if (in.handPresent) {
                // Tarik menuju sampel terbaru agar gerakan mulus tanpa jitter.
                smoothedCursor.x += (target.x - smoothedCursor.x) * CURSOR_LERP;
                smoothedCursor.y += (target.y - smoothedCursor.y) * CURSOR_LERP;
            } else {
                // Tangan keluar frame (crosshair sedang disembunyikan) — snap agar
                // tidak terlihat menggeser dari posisi lama saat tangan kembali.
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
            // Tidak ada data Python -> fallback ke mouse hardware untuk dev tanpa kamera.
            // Otomatis kembali begitu tracker.py mulai mengirim siaran.
            in.handPresent      = true;            // kursor mouse selalu dianggap ada
            smoothedCursor      = GetMousePosition();
            in.cursor           = smoothedCursor;
            in.clickDown        = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            in.clickPressed     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            in.clickReleased    = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
            // Tidak ada padanan mouse untuk gestur ini — fallback keyboard/klik-kanan
            // diterapkan di dalam HandleInput via ||.
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
