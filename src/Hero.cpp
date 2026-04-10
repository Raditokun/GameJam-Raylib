#include "Hero.h"
#include "AssetManager.h"
#include "Constants.h"
#include <cmath>
#include <cstdio>

Hero::Hero()
    : position({0,0}), radius(20.0f),
      maxHP(STARTING_HEALTH), currentHP(STARTING_HEALTH),
      currentUltCharge(0), maxUltCharge(100),
      ultDuration(0.5f), ultActiveTimer(0),
      ultDamage(9999.0f), ultRadius(250.0f),
      isUltFiring(false), pulseTimer(0),
      animTimer(0), currentFrame(0),
      ultAnimTimer(0), currentUltFrame(9) {}

void Hero::Init(Vector2 basePos) {
    position = basePos;
    currentHP = maxHP;
    currentUltCharge = 0;
    ultActiveTimer = 0;
    isUltFiring = false;
    pulseTimer = 0;
    animTimer = 0;
    currentFrame = 0;
    ultAnimTimer = 0;
    currentUltFrame = 9;
}

void Hero::Update(float dt, bool isWaveActive) {
    pulseTimer += dt;

    // ── Idle sprite animation ────────────────────────────
    animTimer += dt;
    if (animTimer >= FRAME_TIME) {
        animTimer -= FRAME_TIME;
        currentFrame = (currentFrame + 1) % IDLE_FRAMES;
    }

    // ── Ultimate visual timer ────────────────────────────
    if (ultActiveTimer > 0) {
        ultActiveTimer -= dt;
        if (ultActiveTimer <= 0) {
            ultActiveTimer = 0;
        }

        // Ult sprite animation (right-to-left, then loop 2→0)
        ultAnimTimer += dt;
        if (ultAnimTimer >= ULT_FRAME_TIME) {
            ultAnimTimer -= ULT_FRAME_TIME;
            currentUltFrame--;
            // After initial 9→0 sweep, loop the last 3 frames (2, 1, 0)
            if (currentUltFrame < 0) {
                currentUltFrame = 2;
            }
        }
    }
}

void Hero::Draw(AssetManager* assets) const {
    // ── Try sprite sheet first ───────────────────────────
    if (assets) {
        Texture2D* tex = assets->Get("hero_marine");
        if (tex && tex->id > 0) {
            // Source rect: pick current frame from horizontal strip
            Rectangle src = {
                (float)(currentFrame * SPRITE_SIZE), 0,
                (float)SPRITE_SIZE, (float)SPRITE_SIZE
            };
            // Dest rect: centered on hero position
            Rectangle dst = {
                position.x, position.y,
                (float)SPRITE_SIZE, (float)SPRITE_SIZE
            };
            Vector2 origin = { SPRITE_SIZE / 2.0f, SPRITE_SIZE / 2.0f };
            DrawTexturePro(*tex, src, dst, origin, 0.0f, WHITE);

            // HP bar above hero
            float barW = 40.0f, barH = 5.0f;
            float hpRatio = (float)currentHP / (float)maxHP;
            float bx = position.x - barW/2, by = position.y - 40;
            DrawRectangle((int)bx, (int)by, (int)barW, (int)barH, CLITERAL(Color){40,40,40,200});
            DrawRectangle((int)bx, (int)by, (int)(barW * hpRatio), (int)barH,
                          hpRatio > 0.5f ? COLOR_BASE : COLOR_HEALTH_BAR);

            // Ult indicator
            if (IsUltReady()) {
                float pulse = 0.5f + 0.5f * sinf(pulseTimer * 4.0f);
                DrawCircleLines((int)position.x, (int)position.y, 36, Fade(COLOR_CURRENCY, pulse));
                DrawText("[Q] ULT READY", (int)(position.x-42), (int)(position.y+38), 10,
                         Fade(COLOR_CURRENCY, pulse));
            } else if (IsUltActive()) {
                float flash = 0.5f + 0.5f * sinf(pulseTimer * 12.0f);
                DrawCircleV(position, 34, Fade(COLOR_CURRENCY, 0.3f * flash));
            } else {
                float pct = GetUltChargePercent();
                char buf[24];
                snprintf(buf, sizeof(buf), "ULT: %d/%d", currentUltCharge, maxUltCharge);
                DrawText(buf, (int)(position.x-30), (int)(position.y+38), 9, Fade(COLOR_TEXT_DIM, 0.7f));

                float cbW = 40.0f, cbH = 3.0f;
                float cbx = position.x - cbW/2, cby = position.y + 50;
                DrawRectangle((int)cbx, (int)cby, (int)cbW, (int)cbH, CLITERAL(Color){40,40,40,180});
                DrawRectangle((int)cbx, (int)cby, (int)(cbW * pct), (int)cbH,
                              Fade(COLOR_CURRENCY, 0.7f));
            }
            return; // sprite drawn, skip procedural fallback
        }
    }

    // ── Procedural fallback (no sprite) ──────────────────
    float p = 1.0f + 0.08f * sinf(pulseTimer * 2.0f);

    DrawCircleV(position, 28*p, Fade(COLOR_BASE, 0.15f));
    DrawCircleV(position, 20*p, Fade(COLOR_BASE, 0.3f));
    DrawPoly(position, 6, 14*p, 0, COLOR_BASE);
    DrawPoly(position, 6, 8*p, 30, Fade(WHITE, 0.4f));

    float barW = 40.0f, barH = 5.0f;
    float hpRatio = (float)currentHP / (float)maxHP;
    float bx = position.x - barW/2, by = position.y - 36;
    DrawRectangle((int)bx, (int)by, (int)barW, (int)barH, CLITERAL(Color){40,40,40,200});
    DrawRectangle((int)bx, (int)by, (int)(barW * hpRatio), (int)barH,
                  hpRatio > 0.5f ? COLOR_BASE : COLOR_HEALTH_BAR);

    DrawText("HERO", (int)(position.x-16), (int)(position.y+24), 10, Fade(COLOR_BASE, 0.8f));

    if (IsUltReady()) {
        float pulse = 0.5f + 0.5f * sinf(pulseTimer * 4.0f);
        DrawCircleLines((int)position.x, (int)position.y, 32, Fade(COLOR_CURRENCY, pulse));
        DrawText("[Q] ULT READY", (int)(position.x-42), (int)(position.y+38), 10,
                 Fade(COLOR_CURRENCY, pulse));
    } else if (IsUltActive()) {
        float flash = 0.5f + 0.5f * sinf(pulseTimer * 12.0f);
        DrawCircleV(position, 30, Fade(COLOR_CURRENCY, 0.3f * flash));
    } else {
        float pct = GetUltChargePercent();
        char buf[24];
        snprintf(buf, sizeof(buf), "ULT: %d/%d", currentUltCharge, maxUltCharge);
        DrawText(buf, (int)(position.x-30), (int)(position.y+38), 9, Fade(COLOR_TEXT_DIM, 0.7f));

        float cbW = 40.0f, cbH = 3.0f;
        float cbx = position.x - cbW/2, cby = position.y + 50;
        DrawRectangle((int)cbx, (int)cby, (int)cbW, (int)cbH, CLITERAL(Color){40,40,40,180});
        DrawRectangle((int)cbx, (int)cby, (int)(cbW * pct), (int)cbH,
                      Fade(COLOR_CURRENCY, 0.7f));
    }
}

bool Hero::IsUltReady() const {
    return currentUltCharge >= maxUltCharge && ultActiveTimer <= 0;
}

bool Hero::IsUltActive() const {
    return ultActiveTimer > 0;
}

void Hero::FireUltimate() {
    if (!IsUltReady()) return;
    isUltFiring = true;
    ultActiveTimer = ultDuration;
    currentUltCharge = 0; // drain charge on fire
    // Reset ult animation to start from frame 9 (right-to-left sweep)
    currentUltFrame = 9;
    ultAnimTimer = 0.0f;
}

void Hero::AddUltCharge(int amount) {
    currentUltCharge += amount;
    if (currentUltCharge > maxUltCharge)
        currentUltCharge = maxUltCharge;
}

float Hero::GetUltChargePercent() const {
    if (maxUltCharge <= 0) return 0;
    return (float)currentUltCharge / (float)maxUltCharge;
}

void Hero::TakeDamage(int amount) {
    currentHP -= amount;
    if (currentHP < 0) currentHP = 0;
}

bool Hero::IsAlive() const {
    return currentHP > 0;
}
