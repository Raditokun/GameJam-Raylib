#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include "Constants.h"

class Enemy;
class Projectile;

// ── Data sprite hasil lookup berarah ──────
struct SpriteData {
    std::string key;
    int maxFrames;
};

class Tower {
public:
    TowerType  type;
    int        baseTier;
    int        fieldLevel;           // level upgrade di lapangan (1-5)
    float      synergyMultiplier;    // di-set oleh GridNode::CalculateSynergy()

    // Kontribusi sinergi (dari tower ini ke stack)
    float providedDamageMultiplier;  // T3: boost damage multiplikatif ke stack
    float providedFireRateBonus;     // T2: bonus fire rate aditif ke stack
    float providedRangeBonus;        // T2: bonus range aditif ke stack

    TowerStats baseStats;
    TowerStats effectiveStats;       // base * sinergi * level
    Vector2    position;
    float      fireCooldown, rotation;

    // ── Animasi Sprite ─────────────────────────────────
    FacingDir  currentDir;
    float      animTimer;
    int        currentFrame;
    static constexpr float ANIM_FRAME_TIME = 0.12f;
    static constexpr int   SPRITE_H = 64;

    Tower(TowerType t, int tier, Vector2 pos);
    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw(float yOffset = 0.0f) const;
    void RecalcEffectiveStats();
    bool CanUpgrade() const;
    int  GetUpgradeCost() const;
    void Upgrade();                  // menaikkan fieldLevel, hitung ulang kontribusi sinergi

    // ── Lookup Sprite ────────────────────────────────────
    static SpriteData GetTowerSpriteData(TowerType type, int tier, FacingDir dir);

private:
    void RecalcSynergyContributions(); // update provided* berdasarkan baseTier + fieldLevel
    Enemy* FindTarget(std::vector<Enemy>& enemies);
    void   Shoot(Enemy* target, std::vector<Projectile>& projectiles);
    void   DrawShape(Vector2 p, float scale) const; // fallback prosedural
};
