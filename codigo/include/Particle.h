#ifndef AED_PROYECTO_QUADTREE_PARTICLE_H
#define AED_PROYECTO_QUADTREE_PARTICLE_H

#include <SFML/Graphics.hpp>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;       // Cuánto tiempo vive (segundos)
    float maxLifetime;    // Para calcular transparencia
    sf::Color color;
    sf::RectangleShape shape;

    Particle(float x, float y, float speed, float angle, sf::Color c) {
        position = {x, y};
        // Convertir ángulo a vector
        float rad = angle * 3.14159f / 180.0f;
        velocity = {std::cos(rad) * speed, std::sin(rad) * speed};

        lifetime = 0.5f + (rand() % 50) / 100.0f; // Entre 0.5 y 1.0 segundos
        maxLifetime = lifetime;
        color = c;

        shape.setSize({4.0f, 4.0f});
        shape.setOrigin(2.0f, 2.0f);
        shape.setPosition(position);
        shape.setFillColor(color);
    }

    // Retorna false si la partícula murió
    bool update(float dt) {
        lifetime -= dt;
        if (lifetime <= 0) return false;

        position += velocity * dt;
        shape.setPosition(position);

        // Efecto Fade-out (Desvanecer)
        float alphaRatio = lifetime / maxLifetime;
        sf::Color faded = color;
        faded.a = static_cast<sf::Uint8>(255 * alphaRatio);
        shape.setFillColor(faded);

        // Rotar un poco
        shape.rotate(360.0f * dt);

        return true;
    }

    void render(sf::RenderWindow& window) {
        window.draw(shape);
    }
};

#endif