#pragma once
#include <raylib.h>

// ─── Hero: Replaces the basic "base" with an entity ─────
// The Hero sits at the end of the path. It has HP (separate
// from the old playerHealth), a visual representation, and
// an Ultimate ability on a long cooldown.

class Hero {
public:
    // ── Identity ──────────────────────────────────────────
    Vector2 position;       // world position (set to last path point)
    float   radius;         // collision / visual radius

    // ── Health ────────────────────────────────────────────
    int   maxHP;
    int   currentHP;

    // ── Ultimate Ability ─────────────────────────────────
    float ultCooldownMax;   // total cooldown in seconds (e.g. 45s)
    float ultCooldownTimer; // counts DOWN to 0; <= 0 means ready
    float ultDuration;      // how long the ult effect lasts
    float ultActiveTimer;   // counts DOWN while ult is firing; <= 0 means inactive
    float ultDamage;        // damage dealt by the ultimate
    float ultRadius;        // area-of-effect radius

    // ── Visual ────────────────────────────────────────────
    float pulseTimer;       // internal animation timer

    // ── Lifecycle ─────────────────────────────────────────
    Hero();
    void Init(Vector2 basePos);
    void Update(float dt);
    void Draw() const;

    // ── Ultimate ──────────────────────────────────────────
    bool  IsUltReady() const;
    bool  IsUltActive() const;
    void  ActivateUlt();            // starts the ult (called on player input)
    float GetUltCooldownPercent() const; // 0.0 = ready, 1.0 = full cooldown

    // ── Damage ────────────────────────────────────────────
    void TakeDamage(int amount);
    bool IsAlive() const;
};
