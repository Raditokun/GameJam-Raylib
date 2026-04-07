#pragma once
#include <raylib.h>
#include <vector>
#include "Constants.h"
#include "GridNode.h"
#include "Enemy.h"
#include "Projectile.h"
#include "DeckManager.h"
#include "WaveManager.h"

class Game {
public:
    GameState state;
    int playerHealth, currency;

    GridNode grid[GRID_ROWS][GRID_COLS];
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;

    DeckManager deck;
    WaveManager waves;
    std::vector<Vector2> pathPoints;
    bool pathCells[GRID_ROWS][GRID_COLS];

    Game();
    void Init();
    void Update(float dt);
    void Draw() const;

private:
    void InitGrid();
    void UpdateDrafting();
    void DrawDrafting() const;
    void HandleInput();
    void UpdateProjectiles(float dt);
    void CheckEnemyReachedBase();
    void CleanupDead();
    void DrawGrid() const;
    void DrawPath() const;
    void DrawUI() const;
    void DrawBase() const;
    void DrawPortal() const;
    void DrawGameOver() const;
    void DrawVictory() const;
};
