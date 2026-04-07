#include "Projectile.h"
#include "Enemy.h"
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

void Projectile::Draw() const {
    if (!active) return;
    DrawCircleV(position, PROJECTILE_RADIUS+3, Fade(color, 0.2f));
    DrawCircleV(position, PROJECTILE_RADIUS, color);
    DrawCircleV(position, PROJECTILE_RADIUS*0.4f, WHITE);
    Vector2 trail = {position.x - direction.x*8, position.y - direction.y*8};
    DrawLineEx(position, trail, 2.0f, Fade(color, 0.4f));
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
