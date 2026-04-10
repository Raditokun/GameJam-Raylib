#pragma once
#include <raylib.h>

// ─── Hero: Replaces the basic "base" with an entity ─────
// The Hero sits at the end of the path. It has HP, a visual
// representation, and a Kill-Charged Ultimate ability.

class AssetManager; // forward declaration

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

    // ── Visual / Idle Animation ──────────────────────────
    float pulseTimer;       // internal animation timer (legacy, kept for UI pulse)
    float animTimer;        // sprite sheet frame timer
    int   currentFrame;     // current idle animation frame (0-4)
    static constexpr float FRAME_TIME = 0.15f;
    static constexpr int   IDLE_FRAMES = 5;
    static constexpr int   SPRITE_SIZE = 64;

    // ── Ultimate Animation ───────────────────────────────
    float ultAnimTimer;     // ult sprite frame timer
    int   currentUltFrame;  // current ult animation frame (starts at 9, loops 2→0)
    static constexpr float ULT_FRAME_TIME = 0.05f;
    static constexpr int   ULT_TOTAL_FRAMES = 10;

    // ── Lifecycle ─────────────────────────────────────────
    Hero();
    void Init(Vector2 basePos);
    void Update(float dt, bool isWaveActive);
    void Draw(AssetManager* assets = nullptr) const;

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
