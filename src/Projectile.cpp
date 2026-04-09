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
    if (!active) return;

    // ── Laser projectile: use sprite if available ────────
    if (sourceType == TowerType::LASER && assets) {
        Texture2D* tex = assets->Get("projectile_laser_test");
        if (tex && tex->id > 0) {
            // Rotate sprite to face movement direction, centered on position
            float angle = atan2f(direction.y, direction.x) * RAD2DEG;
            Rectangle src = {0, 0, (float)tex->width, (float)tex->height};
            Rectangle dst = {position.x, position.y, (float)tex->width, (float)tex->height};
            Vector2 origin = {tex->width / 2.0f, tex->height / 2.0f};
            DrawTexturePro(*tex, src, dst, origin, angle, WHITE);

            // Subtle glow trail behind the sprite
            Vector2 trail = {position.x - direction.x*10, position.y - direction.y*10};
            DrawLineEx(position, trail, 2.0f, Fade(color, 0.3f));
            return;
        }
    }

    // ── Fallback: procedural shape for all other types ───
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
