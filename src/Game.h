#pragma once
#include <raylib.h>
#include <vector>
#include "Constants.h"
#include "GridNode.h"
#include "Enemy.h"
#include "Projectile.h"
#include "DeckManager.h"
#include "WaveManager.h"
#include "ShopManager.h"
#include "Hero.h"
#include "AssetManager.h"

// ─── Snapshot input virtual ─────────────────────────────
// Dibuat sekali per frame di main.cpp dari penerima UDP
// (dengan fallback mouse hardware saat bridge Python
// diam). Semua UI yang dulu digerakkan mouse hardware kini
// membaca dari struct ini.
struct InputState {
    Vector2 cursor;          // piksel ruang-layar
    bool clickDown;          // sedang ditahan (pinch ditahan)
    bool clickPressed;       // edge: baru ditekan di frame ini
    bool clickReleased;      // edge: baru dilepas di frame ini
    bool rightClickPressed;  // klik kanan mouse hardware (UDP tidak punya klik kanan)
    bool udpAlive;           // true jika paket UDP tiba dalam ~1s
    bool handPresent;        // true jika tangan kanan (kursor) ada di frame

    // ── Edge aksi dari gestur (tangan kanan/kiri) ────
    bool startWavePressed;   // tangan kanan: 2 jari (telunjuk + tengah)
    bool upgradePressed;     // tangan kiri : 1 jari (telunjuk)
    bool sellPressed;        // tangan kiri : telapak terbuka (5 jari)
    bool ultPressed;         // tangan kanan: kepalan (0 jari)
};

class Game {
public:
    GameState state;
    int currency;
    // playerHealth kini Hero::currentHP — disimpan di sini sebagai alias praktis
    int& playerHealth;

    GridNode grid[GRID_ROWS][GRID_COLS];
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;

    DeckManager  deck;
    WaveManager  waves;
    ShopManager  shop;
    Hero         hero;
    AssetManager assets;

    std::vector<Vector2> pathPoints;
    bool pathCells[GRID_ROWS][GRID_COLS];

    // ── Screen Shake ─────────────────────────────────────
    Camera2D camera;
    float screenShakeTimer;

    // ── Background Map Beranimasi ──────────────────────────
    float mapAnimTimer = 0.0f;
    int   currentMapFrame = 0;
    static constexpr float MAP_FRAME_TIME = 0.1f;
    static constexpr int   MAP_TOTAL_FRAMES = 12;
    static constexpr int   MAP_FRAME_W = 1920;
    static constexpr int   MAP_FRAME_H = 1080;

    // ── Menu Utama Beranimasi ───────────────────────────────
    Image menuGifImage;
    Texture2D menuGifTexture;
    int menuAnimFrames = 0;
    int menuCurrentFrame = 0;
    float menuFrameTimer = 0.0f;
    const float menuFrameDelay = 0.1f;

    // ── Audio ────────────────────────────────────────────
    Music bgMusic;
    std::vector<std::string> playlist;
    int currentTrackIndex = 0;
    Rectangle skipBtn;
    void PlayNextTrack();

    // Input dari panggilan Update() terakhir di-cache agar
    // Draw() yang const bisa memakainya (hover highlight, crosshair, dll).
    InputState lastInput{};

    // Visibilitas crosshair, ditarik ke 1 saat tangan ada dan ke 0 saat
    // tangan keluar frame agar kursor fade in/out, bukan muncul mendadak.
    float crosshairAlpha_ = 0.0f;

    Game();
    void Init();
    void Shutdown();     // panggil sebelum CloseWindow untuk membebaskan tekstur GPU
    void Update(float dt, const InputState& input);
    void Draw() const;

private:
    void InitGrid();

    // ── Drafting ──────────────────────────────────────────
    void UpdateDrafting(const InputState& in);
    void DrawDrafting() const;

    // ── Playing ──────────────────────────────────────────
    void HandleInput(const InputState& in);
    void UpdateProjectiles(float dt);
    void CheckEnemyReachedBase();
    void CleanupDead();
    void CheckWaveEndShop();

    // ── Shop ─────────────────────────────────────────────
    void UpdateShop(const InputState& in);
    void DrawShop() const;

    // ── Rendering ────────────────────────────────────────
    void DrawGrid() const;
    void DrawPath() const;
    void DrawUltLaser() const;      // visual laser penelusur jalur
    void DrawUI() const;
    void DrawGameOver() const;
    void DrawVictory() const;
    void DrawPortal() const;
    void DrawCrosshair() const;     // kursor virtual; dipanggil dari MAIN_MENU + akhir Draw()
};
