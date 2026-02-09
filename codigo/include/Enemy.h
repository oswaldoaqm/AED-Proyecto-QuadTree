#ifndef AED_PROYECTO_QUADTREE_ENEMY_H
#define AED_PROYECTO_QUADTREE_ENEMY_H

#include "Entity.h"

enum EnemyVariant {
    NORMAL,
    KAMIKAZE,
    TANK
};

class Enemy : public Entity {
private:
    sf::RectangleShape shape;
    float speed;
    float targetX, targetY;

public:
    int generation;
    EnemyVariant variant;

    Enemy(int id, float x, float y, float tx, float ty, int gen = 0, bool isBoss = false, EnemyVariant var = NORMAL);

    void setTarget(float x, float y);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};


#endif