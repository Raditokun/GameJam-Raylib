#pragma once
#include <raylib.h>
#include <vector>
#include "Constants.h"

class Enemy;
class Projectile;

class Tower {
public:
    TowerType  type;
    int        baseTier;
    int        fieldLevel;           // on-field upgrade level (1-5)
    float      synergyMultiplier;    // set by GridNode::CalculateSynergy()

    // Synergy contributions (from this tower to the stack)
    float providedDamageMultiplier;  // T3: multiplicative damage boost to stack
    float providedFireRateBonus;     // T2: additive fire rate bonus to stack
    float providedRangeBonus;        // T2: additive range bonus to stack

    TowerStats baseStats;
    TowerStats effectiveStats;       // base * synergy * level
    Vector2    position;
    float      fireCooldown, rotation;

    Tower(TowerType t, int tier, Vector2 pos);
    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw(float yOffset = 0.0f) const;
    void RecalcEffectiveStats();
    bool CanUpgrade() const;
    int  GetUpgradeCost() const;
    void Upgrade();                  // increments fieldLevel, recalcs synergy contributions

private:
    void RecalcSynergyContributions(); // updates provided* based on baseTier + fieldLevel
    Enemy* FindTarget(std::vector<Enemy>& enemies);
    void   Shoot(Enemy* target, std::vector<Projectile>& projectiles);
    void   DrawShape(Vector2 p, float scale) const;
};
