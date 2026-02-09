#include "../include/Player.h"
#include <cmath>

Player::Player(int id, float x, float y)
    : Entity(id, PLAYER, x, y, 40.0f, 40.0f) { // Glóbulo un poco más grande para 1080p

    speed = 450.0f; // Velocidad ajustada para mayor resolución

    shape.setRadius(20.0f);
    shape.setOrigin(20.0f, 20.0f);
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::Cyan);
    shape.setOutlineThickness(3);
    shape.setOutlineColor(sf::Color::White);
}

void Player::update(float deltaTime) {
    float dx = 0, dy = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dy -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dy += 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dx -= 1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dx += 1;

    if (dx != 0 || dy != 0) {
        float length = std::sqrt(dx*dx + dy*dy);
        dx /= length;
        dy /= length;

        bounds.x += dx * speed * deltaTime;
        bounds.y += dy * speed * deltaTime;
    }

    // LÍMITES PARA 1920 x 1080
    if (bounds.x < 20) bounds.x = 20;
    if (bounds.x > 1900) bounds.x = 1900;
    if (bounds.y < 20) bounds.y = 20;
    if (bounds.y > 1060) bounds.y = 1060;

    shape.setPosition(bounds.x, bounds.y);
}

void Player::render(sf::RenderWindow& window) {
    window.draw(shape);
}

void Player::setColor(sf::Color color) {
    shape.setFillColor(color);
}

sf::Vector2f Player::getPosition() const {
    return {bounds.x, bounds.y};
}