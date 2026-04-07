#include "Enemy.h"
#include <cmath>

Enemy::Enemy(EnemyType t, Vector2 spawn, float hpMult)
    : type(t), position(spawn), pathIndex(1), alive(true),
      slowTimer(0), slowFactor(1.0f)
{
    EnemyStats s = GetBaseEnemyStats(t);
    hp = s.hp * hpMult;  maxHp = hp;
    speed = s.speed;     reward = s.reward;
    radius = s.radius;
}

void Enemy::Update(float dt, const std::vector<Vector2>& path) {
    if (!alive || pathIndex >= (int)path.size()) return;
    if (slowTimer > 0) { slowTimer -= dt; if (slowTimer <= 0) slowFactor = 1.0f; }

    Vector2 tgt = path[pathIndex];
    float dx = tgt.x - position.x, dy = tgt.y - position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 2.0f) { pathIndex++; return; }

    float mv = speed * slowFactor * dt;
    position.x += (dx/dist)*mv;
    position.y += (dy/dist)*mv;
}

void Enemy::Draw() const {
    if (!alive) return;
    Color col = GetEnemyColor(type);

    switch (type) {
    case EnemyType::GRUNT:
        DrawCircleV(position, radius, col);
        DrawLineEx({position.x-4, position.y-radius}, {position.x-6, position.y-radius-6}, 2, col);
        DrawLineEx({position.x+4, position.y-radius}, {position.x+6, position.y-radius-6}, 2, col);
        DrawCircle((int)(position.x-3), (int)(position.y-2), 2, BLACK);
        DrawCircle((int)(position.x+3), (int)(position.y-2), 2, BLACK);
        break;
    case EnemyType::FAST:
        DrawPoly(position, 4, radius, 45.0f, col);
        DrawLineEx({position.x-radius-5, position.y-3},{position.x-radius-12, position.y-3}, 1, Fade(col,0.5f));
        DrawLineEx({position.x-radius-3, position.y+3},{position.x-radius-10, position.y+3}, 1, Fade(col,0.3f));
        break;
    case EnemyType::TANK:
        DrawRectangle((int)(position.x-radius),(int)(position.y-radius),(int)(radius*2),(int)(radius*2), col);
        DrawRectangleLinesEx({position.x-radius+2, position.y-radius+2, radius*2-4, radius*2-4}, 2, Fade(BLACK,0.5f));
        break;
    case EnemyType::BOSS: {
        float pulse = 1.0f + 0.15f * sinf((float)GetTime()*4.0f);
        DrawCircleV(position, radius*pulse+4, Fade(col,0.2f));
        DrawPoly(position, 8, radius*pulse, 22.5f, col);
        DrawPoly(position, 8, radius*pulse*0.6f, 22.5f, Fade(BLACK,0.3f));
    } break;
    }

    // HP bar
    if (hp < maxHp) {
        float bw = radius*2.5f, bx = position.x-bw/2, by = position.y-radius-8;
        float r = hp/maxHp;
        DrawRectangle((int)bx,(int)by,(int)bw,3, CLITERAL(Color){40,40,40,200});
        Color hc = r>0.5f ? GREEN : (r>0.25f ? YELLOW : RED);
        DrawRectangle((int)bx,(int)by,(int)(bw*r),3, hc);
    }
    if (slowTimer > 0) DrawCircleV(position, radius+2, Fade(COLOR_FREEZE, 0.3f));
}

void Enemy::TakeDamage(float dmg) { hp -= dmg; if (hp <= 0) { hp=0; alive=false; } }
void Enemy::ApplySlow(float f, float d) { slowFactor=f; slowTimer=d; }
bool Enemy::ReachedEnd(const std::vector<Vector2>& path) const { return pathIndex >= (int)path.size(); }
