#include "../include/Enemy.h"
#include <cmath>
#include <algorithm>

Enemy::Enemy(int id, float x, float y, float tx, float ty, int gen, bool isBoss, EnemyVariant var)
    : Entity(id, isBoss ? BOSS : ENEMY, x, y, 0.0f, 0.0f, 1), generation(gen), variant(var) {

    targetX = tx;
    targetY = ty;
    float size = 40.0f;

    if (isBoss) {
        size = 150.0f;
        speed = 50.0f;
        health = 50;
        shape.setFillColor(sf::Color(128, 0, 128));
    } else {
        switch(variant) {
            case KAMIKAZE:
                size = 25.0f; speed = 350.0f; health = 2;
                shape.setFillColor(sf::Color(255, 50, 50, 200));
                shape.setOutlineThickness(2.0f);
                shape.setOutlineColor(sf::Color(255, 150, 150));
                break;
            case TANK:
                size = 60.0f; speed = 80.0f; health = 20;
                shape.setFillColor(sf::Color(100, 0, 150, 220));
                shape.setOutlineThickness(3.0f);
                shape.setOutlineColor(sf::Color(200, 50, 255));
                break;
            default: // NORMAL
                size = 40.0f / (gen + 1);
                speed = (120.0f + (static_cast<float>(rand() % 80))) * (1.0f + gen * 0.4f);
                health = (gen == 0) ? 4 : (gen == 1 ? 2 : 1);

                if (gen == 0) shape.setFillColor(sf::Color(50, 200, 50, 180));
                else if (gen == 1) shape.setFillColor(sf::Color(100, 255, 100, 200));
                else shape.setFillColor(sf::Color(180, 255, 180, 255));
                break;
        }
    }

    bounds.halfW = size / 2.0f;
    bounds.halfH = size / 2.0f;
    maxHealth = health;

    shape.setSize({size, size});
    shape.setOrigin(size / 2.0f, size / 2.0f);
    shape.setPosition(x, y);
}

void Enemy::setTarget(float x, float y) {
    targetX = x;
    targetY = y;
}

void Enemy::update(float deltaTime) {
    float dx = targetX - bounds.x;
    float dy = targetY - bounds.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > 0.5f) {
        float moveDist = speed * deltaTime;
        float actualMove = std::min(moveDist, dist);

        bounds.x += (dx / dist) * actualMove;
        bounds.y += (dy / dist) * actualMove;
    }

    shape.setPosition(bounds.x, bounds.y);

    float rotSpeed = (type == BOSS) ? 30.0f : (variant == KAMIKAZE ? 360.0f : 90.0f);
    shape.rotate(rotSpeed * deltaTime);
}

void Enemy::render(sf::RenderWindow& window) {
    if (active) {
        window.draw(shape);
    }
}