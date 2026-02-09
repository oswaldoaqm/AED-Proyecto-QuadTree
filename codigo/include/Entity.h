#ifndef AED_PROYECTO_QUADTREE_ENTITY_H
#define AED_PROYECTO_QUADTREE_ENTITY_H

#include <SFML/Graphics.hpp>
#include "DataStructures.h"

enum EntityType {
    PLAYER,
    PROJECTILE,
    ENEMY,
    BOSS
};

class Entity {
public:
    int id;
    EntityType type;
    AABB bounds;
    bool active;
    int health;

    Entity(int _id, EntityType _type, float x, float y, float width, float height, int hp = 1)
        : id(_id), type(_type), active(true), health(hp) {
        bounds = AABB(x, y, width / 2.0f, height / 2.0f);
    }

    virtual ~Entity() {}
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};

#endif