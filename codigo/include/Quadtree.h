#ifndef AED_PROYECTO_QUADTREE_QUADTREE_H
#define AED_PROYECTO_QUADTREE_QUADTREE_H

#include <SFML/Graphics.hpp>
#include "DataStructures.h"
#include "Entity.h"

class Quadtree {
private:
    static const int CAPACITY = 4;
    static const int MAX_DEPTH = 8;

    AABB boundary;
    SimpleList<Entity*> objects;
    Quadtree* children[4];

    bool divided;
    int level;

public:
    Quadtree(AABB bounds, int lvl = 0);

    ~Quadtree();

    void clear();

    bool insert(Entity* entity);

    void query(AABB range, SimpleList<Entity*>& found);

    void debugRender(sf::RenderWindow& window);

private:
    void subdivide();

    int getIndex(const AABB& itemBounds);
};

#endif