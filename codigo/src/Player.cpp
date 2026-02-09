#include "../include/Player.h"
#include <cmath>

Player::Player(int id, float x, float y)
    : Entity(id, PLAYER, x, y, 40.0f, 40.0f, 100) {

    speed = 500.0f;
    invulnerableTimer = 0.0f;
    tripleShotTimer = 0.0f;
    maxHealth = 100;

    shape.setRadius(20.0f);
    shape.setOrigin(20.0f, 20.0f);
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color(245, 245, 255, 200));
    shape.setOutlineThickness(-4.0f);
    shape.setOutlineColor(sf::Color(200, 200, 255));
}

void Player::update(float deltaTime) {
    if (invulnerableTimer > 0) invulnerableTimer -= deltaTime;
    if (tripleShotTimer > 0) tripleShotTimer -= deltaTime;

    float dx = 0, dy = 0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dy -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dy += 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dx -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dx += 1;

    if (dx != 0 || dy != 0) {
        float length = std::sqrt(dx*dx + dy*dy);
        bounds.x += (dx / length) * speed * deltaTime;
        bounds.y += (dy / length) * speed * deltaTime;
    }

    float radius = shape.getRadius();
    if (bounds.x < radius) bounds.x = radius;
    if (bounds.x > 1920 - radius) bounds.x = 1920 - radius;
    if (bounds.y < radius) bounds.y = radius;
    if (bounds.y > 1080 - radius) bounds.y = 1080 - radius;

    shape.setPosition(bounds.x, bounds.y);

    if (isInvulnerable()) {
        sf::Color c = shape.getFillColor();
        c.a = (static_cast<int>(invulnerableTimer * 15) % 2 == 0) ? 100 : 255;
        shape.setFillColor(c);
    } else {
        sf::Color c = shape.getFillColor();
        c.a = 255;
        shape.setFillColor(c);
    }
}

void Player::takeDamage(int amount) {
    if (!isInvulnerable() && active) {
        health -= amount;
        if (health <= 0) {
            health = 0;
            active = false;
        }
        invulnerableTimer = 1.0f;
    }
}

void Player::render(sf::RenderWindow& window) {
    if (active) {
        window.draw(shape);
    }
}

void Player::setColor(sf::Color color) {
    shape.setFillColor(color);
}

sf::Vector2f Player::getPosition() const {
    return {bounds.x, bounds.y};
}