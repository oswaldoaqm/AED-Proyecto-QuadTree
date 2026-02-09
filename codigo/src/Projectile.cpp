#include "../include/Projectile.h"
#include <cmath>

Projectile::Projectile(int id, float x, float y, float dirX, float dirY)
    : Entity(id, PROJECTILE, x, y, 12.0f, 12.0f, 1) {

    speed = 750.0f;

    float length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0.0001f) {
        dirX /= length;
        dirY /= length;
    } else {
        dirX = 1.0f;
        dirY = 0.0f;
    }

    vx = dirX * speed;
    vy = dirY * speed;

    shape.setRadius(6.0f);
    shape.setOrigin(6.0f, 6.0f);
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::Yellow);
}

void Projectile::update(float deltaTime) {
    if (!active) return;

    bounds.x += vx * deltaTime;
    bounds.y += vy * deltaTime;

    shape.setPosition(bounds.x, bounds.y);

    const float margin = 50.0f;
    if (bounds.x < -margin || bounds.x > 1920 + margin ||
        bounds.y < -margin || bounds.y > 1080 + margin) {
        active = false;
        }
}

void Projectile::render(sf::RenderWindow& window) {
    if (active) {
        window.draw(shape);
    }
}