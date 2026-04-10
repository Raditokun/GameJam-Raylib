#include "Tower.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Constants.h"
#include "AssetManager.h"
#include <cmath>
#include <cstdio>

// ─── Field Upgrade Constants ────────────────────────────
constexpr int   MAX_FIELD_LEVEL = 5;
// Upgrade cost: base cost * multiplier per level
constexpr float UPGRADE_COST_MULT[] = {0, 0.5f, 0.75f, 1.0f, 1.5f}; // indices 0-4 for levels 1→2, 2→3, etc.

Tower::Tower(TowerType t, int tier, Vector2 pos)
    : type(t), baseTier(tier), fieldLevel(1), synergyMultiplier(1.0f),
      providedDamageMultiplier(1.0f), providedFireRateBonus(0), providedRangeBonus(0),
      position(pos), fireCooldown(0), rotation(0) {
    baseStats = GetTierStats(t, tier);
    RecalcSynergyContributions();
    RecalcEffectiveStats();
}

// ─── Tier-Specific Synergy Contributions ────────────────
// Called when a tower is created or leveled up.
// T1: No synergy contributions (grunt worker, self-only).
// T2: Support. Additive bonuses to the stack that scale with level.
// T3: Amplifier. Multiplicative damage boost to the stack that scales with level.
void Tower::RecalcSynergyContributions() {
    providedDamageMultiplier = 1.0f;
    providedFireRateBonus    = 0;
    providedRangeBonus       = 0;

    switch (baseTier) {
    case 1: // T1: grunt — no stack contributions
        break;
    case 2: // T2: support — additive bonuses
        // Each level adds +10% fire rate and +15 range to all towers in the stack
        providedFireRateBonus = 0.10f * fieldLevel;
        providedRangeBonus    = 15.0f * fieldLevel;
        break;
    case 3: // T3: amplifier — multiplicative damage
        // Lv1: 1.2×, Lv2: 1.4×, Lv3: 1.6×, Lv4: 1.8×, Lv5: 2.0×
        providedDamageMultiplier = 1.0f + 0.2f * fieldLevel;
        break;
    }
}

// ─── Effective Stats (called by CalculateSynergy) ───────
// synergyMultiplier is set externally by GridNode::CalculateSynergy()
// before this method is called.
void Tower::RecalcEffectiveStats() {
    effectiveStats = baseStats;

    // Self-scaling from field level (all tiers get some self-improvement)
    float selfDmgMult, selfRateMult, selfRangeMult;
    switch (baseTier) {
    case 1: // T1: strong self-scaling
        selfDmgMult   = 1.0f + 0.20f * (fieldLevel - 1); // +20% dmg per level
        selfRateMult  = 1.0f + 0.10f * (fieldLevel - 1); // +10% rate per level
        selfRangeMult = 1.0f + 0.05f * (fieldLevel - 1); // +5% range per level
        break;
    case 2: // T2: moderate self-scaling
        selfDmgMult   = 1.0f + 0.12f * (fieldLevel - 1);
        selfRateMult  = 1.0f + 0.08f * (fieldLevel - 1);
        selfRangeMult = 1.0f + 0.04f * (fieldLevel - 1);
        break;
    case 3: // T3: light self-scaling (power goes to the stack)
        selfDmgMult   = 1.0f + 0.10f * (fieldLevel - 1);
        selfRateMult  = 1.0f + 0.05f * (fieldLevel - 1);
        selfRangeMult = 1.0f + 0.03f * (fieldLevel - 1);
        break;
    default:
        selfDmgMult = selfRateMult = selfRangeMult = 1.0f;
    }

    effectiveStats.damage   *= selfDmgMult * synergyMultiplier;
    effectiveStats.fireRate *= selfRateMult;
    effectiveStats.range    *= selfRangeMult;
    // spriteScale grows slightly with level
    effectiveStats.spriteScale = baseStats.spriteScale * (1.0f + 0.05f * (fieldLevel - 1));
}

// ─── Upgrade ────────────────────────────────────────────
bool Tower::CanUpgrade() const { return fieldLevel < MAX_FIELD_LEVEL; }

int Tower::GetUpgradeCost() const {
    if (fieldLevel >= MAX_FIELD_LEVEL) return 0;
    return (int)(baseStats.cost * UPGRADE_COST_MULT[fieldLevel - 1]);
}

void Tower::Upgrade() {
    if (fieldLevel >= MAX_FIELD_LEVEL) return;
    fieldLevel++;
    RecalcSynergyContributions();
    // Note: RecalcEffectiveStats is called by GridNode::CalculateSynergy() after upgrade
}

// ─── Combat ─────────────────────────────────────────────
void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& proj) {
    fireCooldown -= dt;
    Enemy* tgt = FindTarget(enemies);
    if (tgt) {
        rotation = atan2f(tgt->position.y - position.y, tgt->position.x - position.x) * RAD2DEG;
        if (fireCooldown <= 0) { Shoot(tgt, proj); fireCooldown = 1.0f / effectiveStats.fireRate; }
    }
}

void Tower::Draw(float yOffset) const {
    Vector2 dp = {position.x, position.y + yOffset};
    float s = effectiveStats.spriteScale;
    float bs = 12.0f * s;
    DrawRectangle((int)(dp.x-bs), (int)(dp.y-bs/2), (int)(bs*2), (int)bs, Fade(GetTowerColor(type), 0.3f));
    DrawShape(dp, s);

    // Synergy indicator (glow ring if multiplier > 1)
    if (synergyMultiplier > 1.05f) {
        float pulse = 0.3f + 0.15f * sinf((float)GetTime()*3.0f);
        DrawCircleLines((int)dp.x, (int)dp.y, 14*s, Fade(COLOR_CARD_SEL, pulse));
    }

    // Level pips under tower
    for (int i = 0; i < fieldLevel; i++) {
        float px = dp.x + (i - (fieldLevel-1)*0.5f) * 5.0f;
        Color pipCol = (baseTier == 3) ? CLITERAL(Color){255,200,0,255} :
                        (baseTier == 2) ? CLITERAL(Color){0,200,255,255} :
                        CLITERAL(Color){200,200,200,255};
        DrawRectangle((int)(px-1.5f), (int)(dp.y + 10*s + 6), 3, 3, pipCol);
    }
}

void Tower::DrawShape(Vector2 p, float sc) const {
    Color c = GetTowerColor(type);
    float sz = 10.0f * sc;
    switch (type) {
    case TowerType::LASER: {
        DrawTriangle({p.x, p.y-sz},{p.x-sz*0.7f, p.y+sz*0.5f},{p.x+sz*0.7f, p.y+sz*0.5f}, c);
        float rad = rotation * DEG2RAD;
        DrawLineEx(p, {p.x+cosf(rad)*sz*1.5f, p.y+sinf(rad)*sz*1.5f}, 2*sc, c);
    } break;
    case TowerType::MISSILE:
        DrawRectangle((int)(p.x-sz*0.6f),(int)(p.y-sz*0.3f),(int)(sz*1.2f),(int)(sz*0.8f), c);
        DrawCircle((int)p.x,(int)(p.y-sz*0.3f),(int)(sz*0.5f), Fade(c,0.8f));
        break;
    case TowerType::FREEZE:
        DrawPoly(p, 6, sz, 30.0f, c);
        DrawPoly(p, 6, sz*0.5f, 30.0f, Fade(WHITE, 0.3f));
        break;
    case TowerType::TESLA:
        DrawPoly(p, 4, sz, 45.0f, c);
        DrawPoly(p, 4, sz*0.4f, 45.0f, Fade(WHITE, 0.5f));
        break;
    case TowerType::PLASMA:
        DrawPoly(p, 5, sz, 270.0f, c);
        DrawCircle((int)p.x,(int)p.y, sz*0.35f, Fade(WHITE, 0.4f));
        break;
    }
    // Tier dots (baseTier indicator)
    for (int i = 0; i < baseTier; i++) {
        float dx = p.x + (i - (baseTier-1)*0.5f)*6.0f;
        DrawCircle((int)dx, (int)(p.y + sz + 4), 2, Fade(c, 0.8f));
    }
}

Enemy* Tower::FindTarget(std::vector<Enemy>& enemies) {
    Enemy* best = nullptr;
    float minD = effectiveStats.range;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float dx = e.position.x-position.x, dy = e.position.y-position.y;
        float d = sqrtf(dx*dx+dy*dy);
        if (d < minD) { minD = d; best = &e; }
    }
    return best;
}

void Tower::Shoot(Enemy* t, std::vector<Projectile>& proj) {
    if (!t) return;
    float dx = t->position.x-position.x, dy = t->position.y-position.y;
    float d = sqrtf(dx*dx+dy*dy);
    if (d == 0) return;
    Vector2 dir = {dx/d, dy/d};
    proj.emplace_back(position, dir, effectiveStats.damage, 300.0f, type, GetTowerColor(type), type==TowerType::FREEZE);

    switch (type) {
        case TowerType::LASER:   PlaySound(AssetManager::GetSound("sfx_laser")); break;
        case TowerType::MISSILE: PlaySound(AssetManager::GetSound("sfx_missile")); break;
        case TowerType::FREEZE:  PlaySound(AssetManager::GetSound("sfx_freeze")); break;
        case TowerType::TESLA:   PlaySound(AssetManager::GetSound("sfx_tesla")); break;
        case TowerType::PLASMA:  PlaySound(AssetManager::GetSound("sfx_plasma")); break;
    }
}
