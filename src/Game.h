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

    std::vector<Vector2> pathPoints;
    bool pathCells[GRID_ROWS][GRID_COLS];

    Game();
    void Init();
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
    void CheckWaveEndShop();       // after a wave clears, check if shop should open

    // ── Shop ─────────────────────────────────────────────
    void UpdateShop();
    void DrawShop() const;

    // ── Rendering ────────────────────────────────────────
    void DrawGrid() const;
    void DrawPath() const;
    void DrawUI() const;
    void DrawGameOver() const;
    void DrawVictory() const;
    // DrawBase() and DrawPortal() removed — Hero::Draw() replaces DrawBase()
    void DrawPortal() const;
};
