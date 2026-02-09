#include "../include/PowerUp.h"

PowerUp::PowerUp(int id, float x, float y, PowerUpType type)
    : Entity(id, POWERUP, x, y, 30.0f, 30.0f, 1), puType(type) {

    lifetime = 10.0f;
    shape.setRadius(15.0f);
    shape.setOrigin(15.0f, 15.0f);
    shape.setPosition(x, y);

    switch(puType) {
        case HEAL:   shape.setFillColor(sf::Color::Green); break;
        case SHIELD: shape.setFillColor(sf::Color::Blue); break;
        case TRIPLE: shape.setFillColor(sf::Color::Magenta); break;
        case BOMB:   shape.setFillColor(sf::Color(150, 0, 255)); break;
        default:     shape.setFillColor(sf::Color::White); break;
    }

    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::White);
}

void PowerUp::update(float deltaTime) {
    if (!active) return;

    lifetime -= deltaTime;
    if (lifetime <= 0) {
        active = false;
        return;
    }

    float scale = 1.0f + std::sin(lifetime * 5.0f) * 0.1f;
    shape.setScale(scale, scale);

    shape.rotate(45.0f * deltaTime);
}

void PowerUp::render(sf::RenderWindow& window) {
    if (active) {
        window.draw(shape);
    }
}