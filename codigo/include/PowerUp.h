#ifndef AED_PROYECTO_QUADTREE_POWERUP_H
#define AED_PROYECTO_QUADTREE_POWERUP_H

#include "Entity.h"

enum PowerUpType {
    HEAL,
    SHIELD,
    TRIPLE,
    BOMB
};

class PowerUp : public Entity {
private:
    sf::CircleShape shape;
    PowerUpType puType;
    float lifetime;

public:
    PowerUp(int id, float x, float y, PowerUpType type);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    PowerUpType getPUType() const { return puType; }
};

#endif