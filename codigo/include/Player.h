#ifndef AED_PROYECTO_QUADTREE_PLAYER_H
#define AED_PROYECTO_QUADTREE_PLAYER_H

#include "Entity.h"

class Player : public Entity {
private:
    sf::CircleShape shape;
    float speed;

public:
    Player(int id, float x, float y);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    // Métodos específicos del jugador
    void setColor(sf::Color color);
    sf::Vector2f getPosition() const;
};

#endif