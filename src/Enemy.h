#pragma once
#include "Constants.h"
#include <raylib.h>
#include <vector>

class AssetManager;

class Enemy {
public:
  EnemyType type;
  Vector2 position;
  float speed, hp, maxHp, radius;
  int reward, pathIndex;
  bool alive;
  float slowTimer, slowFactor;

  // ── Animation ──────────────────────────────────────────
  float animTimer = 0.0f;
  int   currentFrame = 0;
  int   maxFrames = 4;
  static constexpr float frameTime = 0.15f;

  Enemy(EnemyType t, Vector2 spawn, float hpMult = 1.0f);
  void Update(float dt, const std::vector<Vector2> &path);
  void Draw(AssetManager* assets) const;
  void TakeDamage(float dmg);
  void ApplySlow(float factor, float duration);
  bool ReachedEnd(const std::vector<Vector2> &path) const;
};
