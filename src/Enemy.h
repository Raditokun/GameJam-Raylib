#pragma once
#include "Constants.h"
#include <raylib.h>
#include <vector>

class Enemy {
public:
  EnemyType type;
  Vector2 position;
  float speed, hp, maxHp, radius;
  int reward, pathIndex;
  bool alive;
  float slowTimer, slowFactor;

  Enemy(EnemyType t, Vector2 spawn, float hpMult = 1.0f);
  void Update(float dt, const std::vector<Vector2> &path);
  void Draw() const;
  void TakeDamage(float dmg);
  void ApplySlow(float factor, float duration);
  bool ReachedEnd(const std::vector<Vector2> &path) const;
};
