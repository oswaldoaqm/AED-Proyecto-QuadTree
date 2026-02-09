#include "../include/Projectile.h"
#include <cmath>

Projectile::Projectile(int id, float x, float y, float dirX, float dirY)
    : Entity(id, PROJECTILE, x, y, 12.0f, 12.0f) {

    speed = 750.0f; // Más rápido para cruzar 1920 píxeles rápido

    float length = std::sqrt(dirX*dirX + dirY*dirY);
    if (length != 0) {
        dirX /= length;
        dirY /= length;
    }

    // Almacenamos velocidad para update
    // Nota: Como no tenemos miembros de velocidad en Entity,
    // podrías agregarlos o calcularlos aquí.
    // Usaremos un truco simple guardándolos en variables locales
    // o calculándolos cada vez (menos eficiente) o añadiendo campos.
    // Asumiremos que Projectile tiene campos privados vx, vy.
    vx = dirX * speed;
    vy = dirY * speed;

    shape.setRadius(6.0f);
    shape.setOrigin(6.0f, 6.0f);
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::Yellow);
}

void Projectile::update(float deltaTime) {
    bounds.x += vx * deltaTime;
    bounds.y += vy * deltaTime;

    shape.setPosition(bounds.x, bounds.y);

    // LÍMITES DE DESACTIVACIÓN PARA 1920 x 1080
    if (bounds.x < -100 || bounds.x > 2020 || bounds.y < -100 || bounds.y > 1180) {
        active = false;
    }
}

void Projectile::render(sf::RenderWindow& window) {
    window.draw(shape);
}