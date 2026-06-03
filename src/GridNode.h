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
    void CalculateSynergy();   // set sinergi semua tower berdasarkan kontribusi tier
    bool CanUpgradeTop() const;
    int  GetTopUpgradeCost() const;
    bool UpgradeTopTower(int& currency); // upgrade tower teratas jika mampu, return true bila sukses
};
