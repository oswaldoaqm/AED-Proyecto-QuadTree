#ifndef AED_PROYECTO_QUADTREE_FLOATINGTEXT_H
#define AED_PROYECTO_QUADTREE_FLOATINGTEXT_H

#include <SFML/Graphics.hpp>
#include <string>

class FloatingText {
public:
    sf::Text text;
    float lifetime;
    float maxLifetime;
    bool active;

    FloatingText(sf::Font& font, std::string value, float x, float y, sf::Color color) {
        text.setFont(font);
        text.setString(value);
        text.setCharacterSize(24);
        text.setFillColor(color);
        text.setPosition(x, y);

        lifetime = 1.0f;
        maxLifetime = 1.0f;
        active = true;
    }

    void update(float deltaTime) {
        lifetime -= deltaTime;
        if (lifetime <= 0) {
            active = false;
            return;
        }

        text.move(0, -50.0f * deltaTime);

        sf::Color c = text.getFillColor();
        c.a = static_cast<sf::Uint8>(255 * (lifetime / maxLifetime));
        text.setFillColor(c);
    }

    void render(sf::RenderWindow& window) {
        window.draw(text);
    }
};

#endif