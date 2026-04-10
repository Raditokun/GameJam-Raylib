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

class Game {
public:
    GameState state;
    int currency;
    // playerHealth is now Hero::currentHP — kept here as a convenience alias
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

    // ── Animated Map Background ──────────────────────────
    float mapAnimTimer = 0.0f;
    int   currentMapFrame = 0;
    static constexpr float MAP_FRAME_TIME = 0.1f;
    static constexpr int   MAP_TOTAL_FRAMES = 12;
    static constexpr int   MAP_FRAME_W = 1920;
    static constexpr int   MAP_FRAME_H = 1080;

    // ── Audio ────────────────────────────────────────────
    Music bgMusic;
    std::vector<std::string> playlist;
    int currentTrackIndex = 0;
    Rectangle skipBtn;
    void PlayNextTrack();

    Game();
    void Init();
    void Shutdown();     // call before CloseWindow to free GPU textures
    void Update(float dt);
    void Draw() const;

private:
    void InitGrid();

    // ── Drafting ──────────────────────────────────────────
    void UpdateDrafting();
    void DrawDrafting() const;

    // ── Playing ──────────────────────────────────────────
    void HandleInput();
    void UpdateProjectiles(float dt);
    void CheckEnemyReachedBase();
    void CleanupDead();
    void CheckWaveEndShop();

    // ── Shop ─────────────────────────────────────────────
    void UpdateShop();
    void DrawShop() const;

    // ── Rendering ────────────────────────────────────────
    void DrawGrid() const;
    void DrawPath() const;
    void DrawUltLaser() const;      // path-tracing laser visual
    void DrawUI() const;
    void DrawGameOver() const;
    void DrawVictory() const;
    void DrawPortal() const;
};
