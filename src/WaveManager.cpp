#include "WaveManager.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

WaveManager::WaveManager() : currentWave(-1), spawnTimer(0), spawnedThisWave(0), waveActive(false) {}

// ─── Tabel Wave: 25 wave dengan skala Chaos ────────────
// Wave 1–10: progres yang di-tuning manual
// Wave 11–25: diskalakan prosedural dengan multiplier chaos

void WaveManager::Init() {
    currentWave = -1;
    spawnTimer = 0;
    spawnedThisWave = 0;
    waveActive = false;
    waves.clear();

    // ── Fase 1: Wave 1–10 tuning manual ───────────────────
    waves.push_back({ 8, 1.0f, EnemyType::GRUNT, 1.0f});  // Wave  1
    waves.push_back({12, 0.8f, EnemyType::GRUNT, 1.2f});  // Wave  2
    waves.push_back({ 8, 0.6f, EnemyType::FAST,  1.0f});  // Wave  3  → SHOP
    waves.push_back({ 5, 1.5f, EnemyType::TANK,  1.0f});  // Wave  4
    waves.push_back({15, 0.5f, EnemyType::GRUNT, 1.5f});  // Wave  5
    waves.push_back({10, 0.5f, EnemyType::FAST,  1.3f});  // Wave  6  → SHOP
    waves.push_back({ 8, 1.0f, EnemyType::TANK,  1.2f});  // Wave  7
    waves.push_back({ 1, 2.0f, EnemyType::BOSS,  1.0f});  // Wave  8
    waves.push_back({20, 0.3f, EnemyType::FAST,  1.5f});  // Wave  9  → SHOP
    waves.push_back({ 3, 2.0f, EnemyType::BOSS,  1.5f});  // Wave 10

    // ── Fase 2: Wave Chaos 11–25 ───────────────────────
    // Tiap wave setelah 10 makin sulit secara progresif:
    //   - Multiplier jumlah: lerp dari 1.5x ke 2.5x
    //   - Interval: menyusut 30–50%
    //   - Multiplier HP: majemuk 1.2x per wave setelah 10
    //   - Tipe musuh bergeser dari dominan Grunt ke dominan Tank/Boss

    const int CHAOS_START = 10;      // wave indeks-0 ke-10 = "wave 11" bagi pemain
    const int TOTAL_WAVES = 25;
    const int CHAOS_COUNT = TOTAL_WAVES - CHAOS_START;

    for (int i = 0; i < CHAOS_COUNT; i++) {
        int waveIndex = CHAOS_START + i;          // 10..24 (indeks-0)
        float progress = (float)i / (float)(CHAOS_COUNT - 1); // 0.0 → 1.0

        // ── Skala jumlah: base 15, dikali 1.5x → 2.5x ──
        float countMult = 1.5f + progress * 1.0f; // 1.5 → 2.5
        int baseCount = 12 + i * 2;               // 12, 14, 16, ... 40
        int count = (int)(baseCount * countMult);

        // ── Interval spawn: menyusut dari 0.5s ke 0.2s ──
        float interval = 0.5f - progress * 0.3f;  // 0.5 → 0.2
        if (interval < 0.15f) interval = 0.15f;

        // ── Multiplier HP: majemuk 1.2x per wave chaos ──
        float hpMult = powf(1.2f, (float)(i + 1)); // 1.2, 1.44, 1.73, ...

        // ── Pergeseran distribusi tipe musuh ──
        // Chaos awal   (11-15): mayoritas Fast/Tank
        // Chaos tengah (16-20): campuran Tank/Boss
        // Chaos akhir  (21-25): gerombolan dominan Boss
        EnemyType type;
        if (waveIndex == TOTAL_WAVES - 1) {
            // Wave terakhir 25: dijamin gerombolan BOSS
            type = EnemyType::BOSS;
            count = 8;
            interval = 1.5f;
            hpMult *= 2.0f;
        } else {
            // Undian acak berbobot berdasarkan progress
            int roll = rand() % 100;
            int bossChance  = (int)(10 + progress * 40);   // 10% → 50%
            int tankChance  = (int)(30 + progress * 20);   // 30% → 50%
            int fastChance  = (int)(40 - progress * 20);   // 40% → 20%
            // gruntChance = sisanya                          // 20% → ~0%

            if (roll < bossChance) {
                type = EnemyType::BOSS;
                count = (int)(count * 0.3f);               // boss lebih sedikit tapi lebih kuat
                if (count < 2) count = 2;
                interval = 1.8f - progress * 0.5f;         // 1.8 → 1.3
            } else if (roll < bossChance + tankChance) {
                type = EnemyType::TANK;
                count = (int)(count * 0.6f);
                if (count < 3) count = 3;
            } else if (roll < bossChance + tankChance + fastChance) {
                type = EnemyType::FAST;
                interval *= 0.7f;                           // fast muncul lebih cepat
            } else {
                type = EnemyType::GRUNT;
                count = (int)(count * 1.2f);                // grunt datang dalam rombongan lebih besar
            }
        }

        waves.push_back({count, interval, type, hpMult});
    }
}

void WaveManager::Update(float dt, std::vector<Enemy>& enemies, const std::vector<Vector2>& path) {
    if (!waveActive || currentWave < 0 || currentWave >= (int)waves.size()) return;
    WaveDef& w = waves[currentWave];

    if (spawnedThisWave < w.count) {
        spawnTimer -= dt;
        if (spawnTimer <= 0) {
            // Skala HP berbasis wave: +15% per wave di atas hpMult WaveDef
            float waveHpScale = w.hpMult * (1.0f + currentWave * 0.15f);
            enemies.emplace_back(w.type, path[0], waveHpScale);
            spawnedThisWave++;
            spawnTimer = w.interval;
        }
    } else {
        // Semua sudah spawn — cek apakah wave selesai
        bool allDead = true;
        for (auto& e : enemies) { if (e.alive) { allDead = false; break; } }
        if (allDead) waveActive = false;
    }
}

void WaveManager::StartNextWave() {
    currentWave++;
    if (currentWave >= (int)waves.size()) return;
    spawnedThisWave = 0;
    spawnTimer = 0.5f;
    waveActive = true;
}

bool WaveManager::AllWavesComplete() const {
    return currentWave >= (int)waves.size() - 1 && !waveActive;
}

bool WaveManager::IsWaveCleared(const std::vector<Enemy>& enemies) const {
    if (waveActive) return false;
    for (auto& e : enemies) if (e.alive) return false;
    return true;
}

void WaveManager::Draw() const {
    char buf[64];
    int chaosWave = currentWave + 1; // indeks-1 untuk tampilan

    if (chaosWave > 10) {
        // Indikator chaos
        snprintf(buf, sizeof(buf), "WAVE %d/%d  [CHAOS]", chaosWave, (int)waves.size());
        float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 5.0f);
        DrawText(buf, SCREEN_WIDTH - 260, UI_PANEL_Y + 15, 18, Fade(COLOR_HEALTH_BAR, pulse));
    } else {
        snprintf(buf, sizeof(buf), "Wave %d/%d", chaosWave, (int)waves.size());
        DrawText(buf, SCREEN_WIDTH - 180, UI_PANEL_Y + 15, 18, COLOR_TEXT_MAIN);
    }

    if (!waveActive && currentWave < (int)waves.size()-1) {
        float pulse = 0.6f + 0.4f * sinf((float)GetTime()*3.0f);
        DrawText("[SPACE] Next Wave", SCREEN_WIDTH-210, UI_PANEL_Y+45, 14,
                 Fade(COLOR_CARD_SEL, pulse));
    } else if (waveActive) {
        DrawText("INCOMING!", SCREEN_WIDTH-160, UI_PANEL_Y+45, 14, Fade(COLOR_HEALTH_BAR, 0.8f));
    }
}
