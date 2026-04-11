#include "Enemy.h"
#include "AssetManager.h"
#include <cmath>

Enemy::Enemy(EnemyType t, Vector2 spawn, float hpMult)
    : type(t), position(spawn), pathIndex(1), alive(true),
      slowTimer(0), slowFactor(1.0f),
      animTimer(0.0f), currentFrame(0)
{
    EnemyStats s = GetBaseEnemyStats(t);
    hp = s.hp * hpMult;  maxHp = hp;
    speed = s.speed;     reward = s.reward;
    radius = s.radius;

    // Tank has 6 frames; all others have 4
    maxFrames = (t == EnemyType::TANK) ? 6 : 4;
}

void Enemy::Update(float dt, const std::vector<Vector2>& path) {
    if (!alive || pathIndex >= (int)path.size()) return;
    if (slowTimer > 0) { slowTimer -= dt; if (slowTimer <= 0) slowFactor = 1.0f; }

    // ── Animation tick ───────────────────────────────────
    animTimer += dt;
    if (animTimer >= frameTime) {
        animTimer = 0.0f;
        currentFrame = (currentFrame + 1) % maxFrames;
    }

    Vector2 tgt = path[pathIndex];
    float dx = tgt.x - position.x, dy = tgt.y - position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 2.0f) { pathIndex++; return; }

    float mv = speed * slowFactor * dt;
    position.x += (dx/dist)*mv;
    position.y += (dy/dist)*mv;
}

void Enemy::Draw(AssetManager* assets) const {
    if (!alive) return;

    // ── Determine texture key ────────────────────────────
    const char* texKey = "enemy_grunt";
    switch (type) {
        case EnemyType::GRUNT: texKey = "enemy_grunt"; break;
        case EnemyType::FAST:  texKey = "enemy_fast";  break;
        case EnemyType::TANK:  texKey = "enemy_tank";  break;
        case EnemyType::BOSS:  texKey = "enemy_boss";  break;
    }

    Texture2D* tex = assets ? assets->Get(texKey) : nullptr;

    if (tex && tex->id > 0) {
        // ── Sprite sheet frame math ──────────────────────
        float frameWidth  = (float)tex->width / maxFrames;
        float frameHeight = (float)tex->height;

        Rectangle sourceRec = {
            (float)currentFrame * frameWidth, 0.0f,
            frameWidth, frameHeight
        };

        // Scale sprite to roughly match the enemy's radius
        float drawSize = radius * 4.0f;
        Rectangle destRec = {
            position.x, position.y,
            drawSize, drawSize
        };
        Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };

        DrawTexturePro(*tex, sourceRec, destRec, origin, 0.0f, WHITE);
    } else {
        // ── Procedural fallback (keeps game playable without sprites) ──
        Color col = GetEnemyColor(type);
        DrawCircleV(position, radius, col);
    }

    // ── HP bar (always drawn) ────────────────────────────
    if (hp < maxHp) {
        float bw = radius*2.5f, bx = position.x-bw/2, by = position.y-radius-8;
        float r = hp/maxHp;
        DrawRectangle((int)bx,(int)by,(int)bw,3, CLITERAL(Color){40,40,40,200});
        Color hc = r>0.5f ? GREEN : (r>0.25f ? YELLOW : RED);
        DrawRectangle((int)bx,(int)by,(int)(bw*r),3, hc);
    }

    // ── Slow effect overlay ──────────────────────────────
    if (slowTimer > 0) DrawCircleV(position, radius+2, Fade(COLOR_FREEZE, 0.3f));
}

void Enemy::TakeDamage(float dmg) { hp -= dmg; if (hp <= 0) { hp=0; alive=false; } }
void Enemy::ApplySlow(float f, float d) { slowFactor=f; slowTimer=d; }
bool Enemy::ReachedEnd(const std::vector<Vector2>& path) const { return pathIndex >= (int)path.size(); }
