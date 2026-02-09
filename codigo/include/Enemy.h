#ifndef AED_PROYECTO_QUADTREE_ENEMY_H
#define AED_PROYECTO_QUADTREE_ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
private:
    sf::RectangleShape shape;
    float speed;
    float targetX, targetY;

public:
    int generation;

    Enemy(int id, float startX, float startY, float targetX, float targetY, int gen = 0, bool isBoss = false);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void setTarget(float x, float y);
};

#endif