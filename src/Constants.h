#pragma once
#include <raylib.h>
#include <vector>
#include <cmath>
#include <cstdio>

// ─── Screen ─────────────────────────────────────────────
constexpr int SCREEN_WIDTH  = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr int TARGET_FPS    = 60;

// ─── Grid ───────────────────────────────────────────────
constexpr int GRID_COLS   = 30;
constexpr int GRID_ROWS   = 14;
constexpr int CELL_SIZE   = 64;
constexpr int GRID_HEIGHT = GRID_ROWS * CELL_SIZE;

// ─── UI Panel ───────────────────────────────────────────
constexpr int UI_PANEL_HEIGHT = SCREEN_HEIGHT - GRID_HEIGHT;
constexpr int UI_PANEL_Y     = GRID_HEIGHT;
constexpr int HAND_SIZE      = 5;
constexpr int CARD_WIDTH     = 130;
constexpr int CARD_HEIGHT    = 140;
constexpr int CARD_SPACING   = 15;

// ─── Drafting UI ────────────────────────────────────────
constexpr int DRAFT_CARD_W  = 140;
constexpr int DRAFT_CARD_H  = 170;
constexpr int DRAFT_COLS    = 5;
constexpr int DRAFT_ROWS    = 3;
constexpr int DRAFT_SPACING = 12;

// ─── Gameplay ───────────────────────────────────────────
constexpr int   STARTING_HEALTH   = 20;
constexpr int   STARTING_CURRENCY = 200;
constexpr int   MAX_TIER          = 3;
constexpr float PROJECTILE_RADIUS = 4.0f;

// ─── Enums ──────────────────────────────────────────────
enum class TowerType  { LASER, MISSILE, FREEZE, TESLA, PLASMA };
enum class EnemyType  { GRUNT, FAST, TANK, BOSS };
enum class GameState  { MAIN_MENU, DRAFTING, PLAYING, SHOP, GAME_OVER, VICTORY };

// ─── Tower Stats ────────────────────────────────────────
struct TowerStats {
    float damage;
    float range;
    float fireRate;
    int   cost;
    float spriteScale;
};

inline TowerStats GetBaseTowerStats(TowerType type) {
    switch (type) {
        case TowerType::LASER:   return {8.0f,  180.0f, 4.0f, 50,  1.0f};
        case TowerType::MISSILE: return {40.0f, 200.0f, 0.8f, 75,  1.0f};
        case TowerType::FREEZE:  return {3.0f,  160.0f, 2.0f, 60,  1.0f};
        case TowerType::TESLA:   return {15.0f, 150.0f, 1.5f, 80,  1.0f};
        case TowerType::PLASMA:  return {25.0f, 120.0f, 2.0f, 65,  1.0f};
        default:                 return {10.0f, 150.0f, 1.0f, 50,  1.0f};
    }
}

inline TowerStats GetTierStats(TowerType type, int tier) {
    TowerStats b = GetBaseTowerStats(type);
    if (tier == 2) return {b.damage*1.75f, b.range*1.2f, b.fireRate*1.3f, b.cost*2,   1.4f};
    if (tier == 3) return {b.damage*2.5f,  b.range*1.4f, b.fireRate*1.6f, b.cost*4,   1.8f};
    return b;
}

// ─── Enemy Stats ────────────────────────────────────────
struct EnemyStats {
    float hp;
    float speed;
    int   reward;
    float radius;
};

inline EnemyStats GetBaseEnemyStats(EnemyType type) {
    switch (type) {
        case EnemyType::GRUNT: return {60.0f,  60.0f, 10, 10.0f};
        case EnemyType::FAST:  return {30.0f, 120.0f, 15,  8.0f};
        case EnemyType::TANK:  return {200.0f, 35.0f, 25, 14.0f};
        case EnemyType::BOSS:  return {800.0f, 40.0f,100, 20.0f};
        default:               return {50.0f,  50.0f, 10, 10.0f};
    }
}

// ─── Card Pool Definition ───────────────────────────────
struct CardDef {
    int       cardId;
    TowerType towerType;
    int       baseTier;
};

constexpr int POOL_SIZE = 15;
constexpr CardDef CARD_POOL[POOL_SIZE] = {
    { 0, TowerType::LASER,   1}, { 1, TowerType::LASER,   2}, { 2, TowerType::LASER,   3},
    { 3, TowerType::MISSILE, 1}, { 4, TowerType::MISSILE, 2}, { 5, TowerType::MISSILE, 3},
    { 6, TowerType::FREEZE,  1}, { 7, TowerType::FREEZE,  2}, { 8, TowerType::FREEZE,  3},
    { 9, TowerType::TESLA,   1}, {10, TowerType::TESLA,   2}, {11, TowerType::TESLA,   3},
    {12, TowerType::PLASMA,  1}, {13, TowerType::PLASMA,  2}, {14, TowerType::PLASMA,  3},
};

// ─── Path Waypoints (grid col, row) ─────────────────────
constexpr int PATH_WP_COUNT = 10;
constexpr int PATH_WP[PATH_WP_COUNT][2] = {
    {0,6},{6,6},{6,11},{12,11},{12,2},{18,2},{18,11},{24,11},{24,6},{29,6}
};

// ─── Colors (Alien Neon Theme) ──────────────────────────
#define COLOR_BG            CLITERAL(Color){12, 8, 24, 255}
#define COLOR_GRID_LINE     CLITERAL(Color){40, 30, 70, 80}
#define COLOR_VALID_CELL    CLITERAL(Color){0, 255, 100, 30}
#define COLOR_HOVER_VALID   CLITERAL(Color){0, 255, 100, 80}
#define COLOR_HOVER_INVALID CLITERAL(Color){255, 0, 50, 80}
#define COLOR_PATH_FILL     CLITERAL(Color){20, 45, 20, 255}
#define COLOR_PATH_BORDER   CLITERAL(Color){30, 200, 30, 200}
#define COLOR_PATH_GLOW     CLITERAL(Color){30, 255, 30, 25}
#define COLOR_UI_PANEL      CLITERAL(Color){15, 10, 30, 245}
#define COLOR_UI_BORDER     CLITERAL(Color){100, 60, 200, 255}
#define COLOR_CARD_BG       CLITERAL(Color){25, 20, 45, 255}
#define COLOR_CARD_BORDER   CLITERAL(Color){80, 60, 140, 255}
#define COLOR_CARD_SEL      CLITERAL(Color){0, 255, 180, 255}
#define COLOR_CURRENCY      CLITERAL(Color){255, 215, 0, 255}
#define COLOR_HEALTH_BAR    CLITERAL(Color){255, 50, 80, 255}
#define COLOR_TEXT_MAIN     CLITERAL(Color){220, 210, 255, 255}
#define COLOR_TEXT_DIM      CLITERAL(Color){120, 110, 160, 255}
#define COLOR_LASER         CLITERAL(Color){0, 255, 255, 255}
#define COLOR_MISSILE       CLITERAL(Color){255, 120, 0, 255}
#define COLOR_FREEZE        CLITERAL(Color){100, 200, 255, 255}
#define COLOR_TESLA         CLITERAL(Color){200, 100, 255, 255}
#define COLOR_PLASMA        CLITERAL(Color){0, 255, 100, 255}
#define COLOR_GRUNT         CLITERAL(Color){80, 220, 80, 255}
#define COLOR_FAST          CLITERAL(Color){0, 220, 255, 255}
#define COLOR_TANK          CLITERAL(Color){255, 100, 50, 255}
#define COLOR_BOSS          CLITERAL(Color){255, 0, 200, 255}
#define COLOR_BASE          CLITERAL(Color){100, 150, 255, 255}
#define COLOR_PORTAL        CLITERAL(Color){150, 0, 255, 255}
#define COLOR_DRAFT_BG      CLITERAL(Color){8, 5, 18, 255}

// ─── Utility ────────────────────────────────────────────
inline Color GetTowerColor(TowerType t) {
    switch (t) {
        case TowerType::LASER:   return COLOR_LASER;
        case TowerType::MISSILE: return COLOR_MISSILE;
        case TowerType::FREEZE:  return COLOR_FREEZE;
        case TowerType::TESLA:   return COLOR_TESLA;
        case TowerType::PLASMA:  return COLOR_PLASMA;
        default: return WHITE;
    }
}

inline const char* GetTowerName(TowerType t) {
    switch (t) {
        case TowerType::LASER:   return "LASER";
        case TowerType::MISSILE: return "MISSILE";
        case TowerType::FREEZE:  return "FREEZE";
        case TowerType::TESLA:   return "TESLA";
        case TowerType::PLASMA:  return "PLASMA";
        default: return "???";
    }
}

inline const char* GetCardName(TowerType type, int tier) {
    static char buf[32];
    const char* roman[] = {"", "I", "II", "III"};
    snprintf(buf, sizeof(buf), "%s Mk.%s", GetTowerName(type), roman[tier]);
    return buf;
}

inline Color GetTierAccent(int tier) {
    switch (tier) {
        case 1: return CLITERAL(Color){80, 80, 120, 255};
        case 2: return CLITERAL(Color){0, 200, 255, 255};
        case 3: return CLITERAL(Color){255, 200, 0, 255};
        default: return WHITE;
    }
}

inline Color GetEnemyColor(EnemyType t) {
    switch (t) {
        case EnemyType::GRUNT: return COLOR_GRUNT;
        case EnemyType::FAST:  return COLOR_FAST;
        case EnemyType::TANK:  return COLOR_TANK;
        case EnemyType::BOSS:  return COLOR_BOSS;
        default: return WHITE;
    }
}

// ─── Rect Helpers ───────────────────────────────────────
inline Rectangle GetHandSlotRect(int slot) {
    float totalW = HAND_SIZE * CARD_WIDTH + (HAND_SIZE-1) * CARD_SPACING;
    float startX = (SCREEN_WIDTH - totalW) / 2.0f;
    return {startX + slot*(CARD_WIDTH+CARD_SPACING), (float)UI_PANEL_Y + 10, (float)CARD_WIDTH, (float)CARD_HEIGHT};
}

inline Rectangle GetDraftSlotRect(int poolIndex) {
    int col = poolIndex / 3;
    int row = poolIndex % 3;
    float totalW = DRAFT_COLS * DRAFT_CARD_W + (DRAFT_COLS-1) * DRAFT_SPACING;
    float startX = (SCREEN_WIDTH - totalW) / 2.0f;
    float startY = 130.0f;
    return {startX + col*(DRAFT_CARD_W+DRAFT_SPACING), startY + row*(DRAFT_CARD_H+DRAFT_SPACING), (float)DRAFT_CARD_W, (float)DRAFT_CARD_H};
}
