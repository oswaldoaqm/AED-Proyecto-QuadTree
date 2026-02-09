#ifndef AED_PROYECTO_QUADTREE_PLAYER_H
#define AED_PROYECTO_QUADTREE_PLAYER_H

#include "Entity.h"

class Player : public Entity {
private:
    sf::CircleShape shape;
    float speed;

public:
    float invulnerableTimer;
    float tripleShotTimer;

    Player(int id, float x, float y);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void setColor(sf::Color color);
    sf::Vector2f getPosition() const;

    // Métodos de utilidad
    bool isInvulnerable() const { return invulnerableTimer > 0; }
    bool hasTripleShot() const { return tripleShotTimer > 0; }
    void takeDamage(int amount);
};

#endif