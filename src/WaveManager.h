#pragma once
#include <vector>
#include "Enemy.h"

struct WaveDef {
    int count;
    float interval;
    EnemyType type;
    float hpMult;
};

class WaveManager {
public:
    int currentWave;
    float spawnTimer;
    int spawnedThisWave;
    bool waveActive;
    std::vector<WaveDef> waves;

    WaveManager();
    void Init();
    void Update(float dt, std::vector<Enemy>& enemies, const std::vector<Vector2>& path);
    void StartNextWave();
    bool AllWavesComplete() const;
    bool IsWaveCleared(const std::vector<Enemy>& enemies) const;
    void Draw() const;
};
