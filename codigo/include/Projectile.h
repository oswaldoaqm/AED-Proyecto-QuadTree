#ifndef AED_PROYECTO_QUADTREE_PROJECTILE_H
#define AED_PROYECTO_QUADTREE_PROJECTILE_H

#include "Entity.h"

class Projectile : public Entity {
private:
    sf::CircleShape shape;
    float speed;
    float vx, vy;

public:
    Projectile(int id, float x, float y, float dirX, float dirY);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

#endif