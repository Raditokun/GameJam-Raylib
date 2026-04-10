#include "Projectile.h"
#include "Enemy.h"
#include "AssetManager.h"
#include <cmath>

Projectile::Projectile(Vector2 pos, Vector2 dir, float dmg, float spd, TowerType src, Color col, bool freeze)
    : position(pos), direction(dir), damage(dmg), speed(spd),
      active(true), isFreeze(freeze), sourceType(src), color(col), lifetime(3.0f) {}

void Projectile::Update(float dt) {
    if (!active) return;
    position.x += direction.x * speed * dt;
    position.y += direction.y * speed * dt;
    lifetime -= dt;
    if (position.x<-20||position.x>SCREEN_WIDTH+20||position.y<-20||position.y>SCREEN_HEIGHT+20||lifetime<=0)
        active = false;
}

void Projectile::Draw(AssetManager* assets) const {
    if (!active || !assets) return;

    const char* texKey = nullptr;
    switch (sourceType) {
        case TowerType::LASER:   texKey = "proj_laser"; break;
        case TowerType::MISSILE: texKey = "proj_missile"; break;
        case TowerType::FREEZE:  texKey = "proj_freeze"; break;
        case TowerType::TESLA:   texKey = "proj_tesla"; break;
        case TowerType::PLASMA:  texKey = "proj_plasma"; break;
    }

    if (texKey) {
        Texture2D* tex = assets->Get(texKey);
        if (tex && tex->id > 0) {
            float angle = atan2f(direction.y, direction.x) * RAD2DEG;
            Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
            Rectangle dst = {position.x, position.y, (float)tex->width, (float)tex->height};
            Vector2 origin = {tex->width / 2.0f, tex->height / 2.0f};
            DrawTexturePro(*tex, src, dst, origin, angle, WHITE);
        }
    }
}

bool Projectile::CheckCollision(Enemy& enemy) {
    if (!active || !enemy.alive) return false;
    float dx = position.x-enemy.position.x, dy = position.y-enemy.position.y;
    if (sqrtf(dx*dx+dy*dy) < PROJECTILE_RADIUS + enemy.radius) {
        enemy.TakeDamage(damage);
        if (isFreeze) enemy.ApplySlow(0.4f, 2.0f);
        active = false;
        return true;
    }
    return false;
}
