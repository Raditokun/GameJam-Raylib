#pragma once
#include <raylib.h>
#include <vector>
#include "Tower.h"

class Enemy;
class Projectile;

class GridNode {
public:
    int row, col;
    Vector2 worldPos;
    bool isPathTile;
    std::vector<Tower> towerStack;

    GridNode();
    GridNode(int r, int c, bool isPath);
    bool CanPlaceTower(int tier) const;
    void PlaceTower(const Tower& tower);
    int  SellTopTower();
    void UpdateAll(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& proj);
    void DrawAll() const;
    bool IsEmpty() const;
    int  GetTopTier() const;
    void CalculateSynergy();   // sets synergy on all towers based on tier contributions
    bool CanUpgradeTop() const;
    int  GetTopUpgradeCost() const;
    bool UpgradeTopTower(int& currency); // upgrades top tower if affordable, returns true on success
};
