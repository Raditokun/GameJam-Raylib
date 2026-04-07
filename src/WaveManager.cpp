#include "WaveManager.h"
#include <cstdio>

WaveManager::WaveManager() : currentWave(-1), spawnTimer(0), spawnedThisWave(0), waveActive(false) {}

void WaveManager::Init() {
    currentWave = -1;
    spawnTimer = 0;
    spawnedThisWave = 0;
    waveActive = false;
    waves = {
        { 8, 1.0f, EnemyType::GRUNT, 1.0f},
        {12, 0.8f, EnemyType::GRUNT, 1.2f},
        { 8, 0.6f, EnemyType::FAST,  1.0f},
        { 5, 1.5f, EnemyType::TANK,  1.0f},
        {15, 0.5f, EnemyType::GRUNT, 1.5f},
        {10, 0.5f, EnemyType::FAST,  1.3f},
        { 8, 1.0f, EnemyType::TANK,  1.2f},
        { 1, 2.0f, EnemyType::BOSS,  1.0f},
        {20, 0.3f, EnemyType::FAST,  1.5f},
        { 3, 2.0f, EnemyType::BOSS,  1.5f},
    };
}

void WaveManager::Update(float dt, std::vector<Enemy>& enemies, const std::vector<Vector2>& path) {
    if (!waveActive || currentWave < 0 || currentWave >= (int)waves.size()) return;
    WaveDef& w = waves[currentWave];

    if (spawnedThisWave < w.count) {
        spawnTimer -= dt;
        if (spawnTimer <= 0) {
            enemies.emplace_back(w.type, path[0], w.hpMult);
            spawnedThisWave++;
            spawnTimer = w.interval;
        }
    } else {
        // All spawned — check if wave is cleared
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
    snprintf(buf, sizeof(buf), "Wave %d/%d", currentWave+1, (int)waves.size());
    DrawText(buf, SCREEN_WIDTH - 180, UI_PANEL_Y + 15, 18, COLOR_TEXT_MAIN);

    if (!waveActive && currentWave < (int)waves.size()-1) {
        float pulse = 0.6f + 0.4f * sinf((float)GetTime()*3.0f);
        DrawText("[SPACE] Next Wave", SCREEN_WIDTH-210, UI_PANEL_Y+45, 14,
                 Fade(COLOR_CARD_SEL, pulse));
    } else if (waveActive) {
        DrawText("INCOMING!", SCREEN_WIDTH-160, UI_PANEL_Y+45, 14, Fade(COLOR_HEALTH_BAR, 0.8f));
    }
}
