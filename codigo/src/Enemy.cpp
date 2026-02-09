#include "../include/Enemy.h"
#include <cmath>

Enemy::Enemy(int id, float startX, float startY, float tX, float tY, int gen, bool isBoss)
    : Entity(id, isBoss ? BOSS : ENEMY, startX, startY, 0, 0), generation(gen) {

    targetX = tX;
    targetY = tY;

    float size;
    if (isBoss) {
        size = 80.0f;
        speed = 30.0f;
        health = 50; // El jefe tiene mucha vida
        shape.setFillColor(sf::Color(128, 0, 128)); // Púrpura para el jefe
    } else {
        size = 40.0f / (gen + 1);
        speed = (50.0f + (rand() % 50)) * (1.0f + gen * 0.5f);

        // Vida según tus requerimientos
        if (gen == 0) health = 4;      // Grande necesita 4 disparos
        else if (gen == 1) health = 2; // Mediano necesita 2 disparos
        else health = 1;               // Pequeño muere de 1

        if (gen == 0) shape.setFillColor(sf::Color::Red);
        else if (gen == 1) shape.setFillColor(sf::Color(255, 165, 0));
        else shape.setFillColor(sf::Color::Yellow);
    }

    bounds.halfW = size / 2.0f;
    bounds.halfH = size / 2.0f;

    shape.setSize(sf::Vector2f(size, size));
    shape.setOrigin(size / 2.0f, size / 2.0f);
    shape.setPosition(startX, startY);
}

void Enemy::setTarget(float x, float y) {
    targetX = x;
    targetY = y;
}

void Enemy::update(float deltaTime) {
    float dx = targetX - bounds.x;
    float dy = targetY - bounds.y;
    float dist = std::sqrt(dx*dx + dy*dy);

    if (dist > 1.0f) {
        dx /= dist; dy /= dist;
        bounds.x += dx * speed * deltaTime;
        bounds.y += dy * speed * deltaTime;
    }

    shape.setPosition(bounds.x, bounds.y);
    // Los jefes no rotan o rotan lento
    float rotationSpeed = (type == BOSS) ? 20.0f : 90.0f * (generation + 1);
    shape.rotate(rotationSpeed * deltaTime);
}

void Enemy::render(sf::RenderWindow& window) {
    window.draw(shape);
}