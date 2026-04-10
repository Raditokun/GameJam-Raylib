#pragma once
#include <raylib.h>

// ─── Hero: Replaces the basic "base" with an entity ─────
// The Hero sits at the end of the path. It has HP, a visual
// representation, and a Kill-Charged Ultimate ability.

class Hero {
public:
    // ── Identity ──────────────────────────────────────────
    Vector2 position;       // world position (set to last path point)
    float   radius;         // collision / visual radius

    // ── Health ────────────────────────────────────────────
    int   maxHP;
    int   currentHP;

    // ── Ultimate Ability (Kill-Charged) ──────────────────
    int   currentUltCharge; // fills up from enemy kills
    int   maxUltCharge;     // 100 = fully charged, ready to fire
    float ultDuration;      // how long the laser visual stays on screen
    float ultActiveTimer;   // counts DOWN while laser visual is showing
    float ultDamage;        // one-shot damage dealt on fire
    float ultRadius;        // area-of-effect radius (unused now, kept for compat)
    bool  isUltFiring;      // true on the ONE frame damage is applied

    // ── Visual ────────────────────────────────────────────
    float pulseTimer;       // internal animation timer

    // ── Lifecycle ─────────────────────────────────────────
    Hero();
    void Init(Vector2 basePos);
    void Update(float dt, bool isWaveActive);
    void Draw() const;

    // ── Ultimate ──────────────────────────────────────────
    bool  IsUltReady() const;
    bool  IsUltActive() const;      // true while laser visual is showing
    void  FireUltimate();           // one-shot trigger: sets isUltFiring, drains charge
    void  AddUltCharge(int amount); // called on enemy kill
    float GetUltChargePercent() const; // 0.0 = empty, 1.0 = full

    // ── Damage ────────────────────────────────────────────
    void TakeDamage(int amount);
    bool IsAlive() const;
};
