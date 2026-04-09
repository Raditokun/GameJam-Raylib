#pragma once
#include <raylib.h>
#include "Constants.h"

class Enemy; // forward declare
class AssetManager; // forward declare

class Projectile {
public:
    Vector2 position, direction;
    float damage, speed;
    bool active, isFreeze;
    TowerType sourceType;
    Color color;
    float lifetime;

    Projectile(Vector2 pos, Vector2 dir, float dmg, float spd, TowerType src, Color col, bool freeze = false);
    void Update(float dt);
    void Draw(AssetManager* assets = nullptr) const;
    bool CheckCollision(Enemy& enemy);
};
