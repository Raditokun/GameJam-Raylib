#include "Tower.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Constants.h"
#include "AssetManager.h"
#include <cmath>
#include <cstdio>

// ─── Konstanta Upgrade Lapangan ────────────────────────────
constexpr int   MAX_FIELD_LEVEL = 5;
// Biaya upgrade: biaya base * multiplier per level
constexpr float UPGRADE_COST_MULT[] = {0, 0.5f, 0.75f, 1.0f, 1.5f}; // indeks 0-4 untuk level 1→2, 2→3, dst.

Tower::Tower(TowerType t, int tier, Vector2 pos)
    : type(t), baseTier(tier), fieldLevel(1), synergyMultiplier(1.0f),
      providedDamageMultiplier(1.0f), providedFireRateBonus(0), providedRangeBonus(0),
      position(pos), fireCooldown(0), rotation(0),
      currentDir(FacingDir::FRONT), animTimer(0), currentFrame(0) {
    baseStats = GetTierStats(t, tier);
    RecalcSynergyContributions();
    RecalcEffectiveStats();
}

// ─── Kontribusi Sinergi per Tier ────────────────
// Dipanggil saat tower dibuat atau naik level.
// T1: Tanpa kontribusi sinergi (pekerja grunt, untuk diri sendiri).
// T2: Support. Bonus aditif ke stack yang skala dengan level.
// T3: Amplifier. Boost damage multiplikatif ke stack yang skala dengan level.
void Tower::RecalcSynergyContributions() {
    providedDamageMultiplier = 1.0f;
    providedFireRateBonus    = 0;
    providedRangeBonus       = 0;

    switch (baseTier) {
    case 1: // T1: grunt — tanpa kontribusi stack
        break;
    case 2: // T2: support — bonus aditif
        // Tiap level menambah +10% fire rate dan +15 range ke semua tower di stack
        providedFireRateBonus = 0.10f * fieldLevel;
        providedRangeBonus    = 15.0f * fieldLevel;
        break;
    case 3: // T3: amplifier — damage multiplikatif
        // Lv1: 1.2×, Lv2: 1.4×, Lv3: 1.6×, Lv4: 1.8×, Lv5: 2.0×
        providedDamageMultiplier = 1.0f + 0.2f * fieldLevel;
        break;
    }
}

// ─── Stats Efektif (dipanggil oleh CalculateSynergy) ───────
// synergyMultiplier di-set dari luar oleh GridNode::CalculateSynergy()
// sebelum method ini dipanggil.
void Tower::RecalcEffectiveStats() {
    effectiveStats = baseStats;

    // Skala-diri dari level lapangan (semua tier dapat sedikit peningkatan)
    float selfDmgMult, selfRateMult, selfRangeMult;
    switch (baseTier) {
    case 1: // T1: skala-diri kuat
        selfDmgMult   = 1.0f + 0.20f * (fieldLevel - 1); // +20% dmg tiap level
        selfRateMult  = 1.0f + 0.10f * (fieldLevel - 1); // +10% rate tiap level
        selfRangeMult = 1.0f + 0.05f * (fieldLevel - 1); // +5% range tiap level
        break;
    case 2: // T2: skala-diri sedang
        selfDmgMult   = 1.0f + 0.12f * (fieldLevel - 1);
        selfRateMult  = 1.0f + 0.08f * (fieldLevel - 1);
        selfRangeMult = 1.0f + 0.04f * (fieldLevel - 1);
        break;
    case 3: // T3: skala-diri ringan (kekuatan diberikan ke stack)
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
    // spriteScale tumbuh sedikit seiring level
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
    // Catatan: RecalcEffectiveStats dipanggil oleh GridNode::CalculateSynergy() setelah upgrade
}

// ═══════════════════════════════════════════════════════════
// ─── Kamus Data Sprite ─────────────────────────────
// Mengembalikan key AssetManager dan jumlah frame untuk
// tipe tower + tier + arah hadap tertentu.
// FALLBACK: Jika suatu arah tidak ada, kembalikan FRONT/base.
// ═══════════════════════════════════════════════════════════
SpriteData Tower::GetTowerSpriteData(TowerType type, int tier, FacingDir dir) {
    // Helper suffix arah
    auto dirSuffix = [](FacingDir d) -> const char* {
        switch (d) {
            case FacingDir::FRONT:  return "_f";
            case FacingDir::BEHIND: return "_b";
            case FacingDir::LEFT:   return "_l";
            case FacingDir::RIGHT:  return "_r";
        }
        return "_f";
    };

    switch (type) {
    // ── FREEZE: Semua tier punya keempat arah ─────────
    case TowerType::FREEZE: {
        int frames = (tier == 1) ? 3 : 4; // T1=3, T2/T3=4
        char buf[32];
        snprintf(buf, sizeof(buf), "tower_freeze_%d%s", tier, dirSuffix(dir));
        return { std::string(buf), frames };
    }

    // ── LASER: Semua tier punya keempat arah, 6 frame ─
    case TowerType::LASER: {
        char buf[32];
        snprintf(buf, sizeof(buf), "tower_laser_%d%s", tier, dirSuffix(dir));
        return { std::string(buf), 6 };
    }

    // ── MISSILE: Semua tier punya keempat arah ────────
    case TowerType::MISSILE: {
        int frames = (tier == 1) ? 3 : 5; // T1=3, T2/T3=5
        char buf[32];
        snprintf(buf, sizeof(buf), "tower_missile_%d%s", tier, dirSuffix(dir));
        return { std::string(buf), frames };
    }

    // ── PLASMA: T1 punya 4 arah (F/B=1fr, L/R=3fr). T2/T3 base saja, 6fr ─
    case TowerType::PLASMA: {
        if (tier == 1) {
            int frames;
            if (dir == FacingDir::LEFT || dir == FacingDir::RIGHT)
                frames = 3;
            else
                frames = 1; // FRONT & BEHIND satu frame
            char buf[32];
            snprintf(buf, sizeof(buf), "tower_plasma_1%s", dirSuffix(dir));
            return { std::string(buf), frames };
        } else {
            // T2 dan T3: satu sprite base, 6 frame, tanpa varian berarah
            char buf[32];
            snprintf(buf, sizeof(buf), "tower_plasma_%d", tier);
            return { std::string(buf), 6 };
        }
    }

    // ── TESLA: Tanpa varian berarah sama sekali ───────────
    case TowerType::TESLA: {
        int frames;
        if (tier == 1)      frames = 4;
        else if (tier == 2) frames = 11;
        else                frames = 5;
        char buf[32];
        snprintf(buf, sizeof(buf), "tower_tesla_%d", tier);
        return { std::string(buf), frames };
    }
    }

    // Fallback terakhir (mestinya tak pernah terjadi)
    return { "tower_laser_1_f", 6 };
}

// ─── Tempur ─────────────────────────────────────────────
void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& proj) {
    fireCooldown -= dt;
    Enemy* tgt = FindTarget(enemies);

    if (tgt) {
        float dx = tgt->position.x - position.x;
        float dy = tgt->position.y - position.y;
        rotation = atan2f(dy, dx) * RAD2DEG;

        // Tentukan arah hadap dari sudut ke target
        float absDx = fabsf(dx);
        float absDy = fabsf(dy);
        if (absDx >= absDy) {
            currentDir = (dx >= 0) ? FacingDir::RIGHT : FacingDir::LEFT;
        } else {
            currentDir = (dy >= 0) ? FacingDir::FRONT : FacingDir::BEHIND;
        }

        if (fireCooldown <= 0) { Shoot(tgt, proj); fireCooldown = 1.0f / effectiveStats.fireRate; }
    }

    // ── Animasi sprite ─────────────────────────────────
    SpriteData sd = GetTowerSpriteData(type, baseTier, currentDir);
    animTimer += dt;
    if (animTimer >= ANIM_FRAME_TIME) {
        animTimer -= ANIM_FRAME_TIME;
        currentFrame = (currentFrame + 1) % sd.maxFrames;
    }
    // Clamp jika maxFrames berubah karena ganti arah
    if (currentFrame >= sd.maxFrames) currentFrame = 0;
}

// ─── Draw ───────────────────────────────────────────────
void Tower::Draw(float yOffset) const {
    Vector2 dp = {position.x, position.y + yOffset};

    // ── Coba sprite sheet ─────────────────────────────────
    SpriteData sd = GetTowerSpriteData(type, baseTier, currentDir);
    Texture2D* tex = AssetManager::GetTextureStatic(sd.key);

    if (tex && tex->id > 0) {
        float frameW = (float)tex->width / (float)sd.maxFrames;
        float frameH = (float)tex->height;

        Rectangle src = {
            (float)currentFrame * frameW, 0,
            frameW, frameH
        };
        // Dest: jangkar bawah-tengah pada posisi grid
        float scale = effectiveStats.spriteScale;
        float dstW = frameW * scale;
        float dstH = frameH * scale;
        Rectangle dst = {
            dp.x, dp.y,
            dstW, dstH
        };
        // Origin di bawah-tengah
        Vector2 origin = { dstW / 2.0f, dstH };

        DrawTexturePro(*tex, src, dst, origin, 0.0f, WHITE);
    } else {
        // ── Fallback prosedural ──────────────────────────
        float s = effectiveStats.spriteScale;
        float bs = 12.0f * s;
        DrawRectangle((int)(dp.x-bs), (int)(dp.y-bs/2), (int)(bs*2), (int)bs, Fade(GetTowerColor(type), 0.3f));
        DrawShape(dp, s);
    }

    // Indikator sinergi (cincin glow jika multiplier > 1)
    if (synergyMultiplier > 1.05f) {
        float pulse = 0.3f + 0.15f * sinf((float)GetTime()*3.0f);
        float s = effectiveStats.spriteScale;
        DrawCircleLines((int)dp.x, (int)dp.y, 14*s, Fade(COLOR_CARD_SEL, pulse));
    }

    // Pip level di bawah tower
    for (int i = 0; i < fieldLevel; i++) {
        float px = dp.x + (i - (fieldLevel-1)*0.5f) * 5.0f;
        Color pipCol = (baseTier == 3) ? CLITERAL(Color){255,200,0,255} :
                        (baseTier == 2) ? CLITERAL(Color){0,200,255,255} :
                        CLITERAL(Color){200,200,200,255};
        float s = effectiveStats.spriteScale;
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
    // Titik tier (indikator baseTier)
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
