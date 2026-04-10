#include "Hero.h"
#include "Constants.h"
#include <cmath>
#include <cstdio>

Hero::Hero()
    : position({0,0}), radius(20.0f),
      maxHP(STARTING_HEALTH), currentHP(STARTING_HEALTH),
      currentUltCharge(0), maxUltCharge(100),
      ultDuration(0.5f), ultActiveTimer(0),
      ultDamage(9999.0f), ultRadius(250.0f),
      isUltFiring(false), pulseTimer(0) {}

void Hero::Init(Vector2 basePos) {
    position = basePos;
    currentHP = maxHP;
    currentUltCharge = 0;
    ultActiveTimer = 0;
    isUltFiring = false;
    pulseTimer = 0;
}

void Hero::Update(float dt, bool isWaveActive) {
    pulseTimer += dt;

    // Only the laser visual timer ticks down — no passive charging
    if (ultActiveTimer > 0) {
        ultActiveTimer -= dt;
        if (ultActiveTimer <= 0) {
            ultActiveTimer = 0;
        }
    }
}

void Hero::Draw() const {
    float p = 1.0f + 0.08f * sinf(pulseTimer * 2.0f);

    // Outer glow
    DrawCircleV(position, 28*p, Fade(COLOR_BASE, 0.15f));
    DrawCircleV(position, 20*p, Fade(COLOR_BASE, 0.3f));

    // Core shape
    DrawPoly(position, 6, 14*p, 0, COLOR_BASE);
    DrawPoly(position, 6, 8*p, 30, Fade(WHITE, 0.4f));

    // HP bar above hero
    float barW = 40.0f, barH = 5.0f;
    float hpRatio = (float)currentHP / (float)maxHP;
    float bx = position.x - barW/2, by = position.y - 36;
    DrawRectangle((int)bx, (int)by, (int)barW, (int)barH, CLITERAL(Color){40,40,40,200});
    DrawRectangle((int)bx, (int)by, (int)(barW * hpRatio), (int)barH,
                  hpRatio > 0.5f ? COLOR_BASE : COLOR_HEALTH_BAR);

    // Label
    DrawText("HERO", (int)(position.x-16), (int)(position.y+24), 10, Fade(COLOR_BASE, 0.8f));

    // Ult indicator
    if (IsUltReady()) {
        float pulse = 0.5f + 0.5f * sinf(pulseTimer * 4.0f);
        DrawCircleLines((int)position.x, (int)position.y, 32, Fade(COLOR_CURRENCY, pulse));
        DrawText("[Q] ULT READY", (int)(position.x-42), (int)(position.y+38), 10,
                 Fade(COLOR_CURRENCY, pulse));
    } else if (IsUltActive()) {
        // Active — pulsing glow
        float flash = 0.5f + 0.5f * sinf(pulseTimer * 12.0f);
        DrawCircleV(position, 30, Fade(COLOR_CURRENCY, 0.3f * flash));
    } else {
        // Charge display (kill-based)
        float pct = GetUltChargePercent();
        char buf[24];
        snprintf(buf, sizeof(buf), "ULT: %d/%d", currentUltCharge, maxUltCharge);
        DrawText(buf, (int)(position.x-30), (int)(position.y+38), 9, Fade(COLOR_TEXT_DIM, 0.7f));

        // Mini charge bar under hero label
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
