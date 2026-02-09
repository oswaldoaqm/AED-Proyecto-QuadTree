#include "../include/Quadtree.h"

Quadtree::Quadtree(AABB bounds, int lvl) {
    boundary = bounds;
    level = lvl;
    divided = false;
    for (int i = 0; i < 4; i++) children[i] = nullptr;
}

Quadtree::~Quadtree() {
    clear();
}

void Quadtree::clear() {
    objects.clear();
    if (divided) {
        for (int i = 0; i < 4; i++) {
            if (children[i]) {
                delete children[i];
                children[i] = nullptr;
            }
        }
        divided = false;
    }
}

void Quadtree::subdivide() {
    float x = boundary.x;
    float y = boundary.y;
    float hw = boundary.halfW;
    float hh = boundary.halfH;
    float qHW = hw / 2.0f;
    float qHH = hh / 2.0f;

    children[0] = new Quadtree(AABB(x - qHW, y - qHH, qHW, qHH), level + 1); // NW
    children[1] = new Quadtree(AABB(x + qHW, y - qHH, qHW, qHH), level + 1); // NE
    children[2] = new Quadtree(AABB(x - qHW, y + qHH, qHW, qHH), level + 1); // SW
    children[3] = new Quadtree(AABB(x + qHW, y + qHH, qHW, qHH), level + 1); // SE

    divided = true;
}

int Quadtree::getIndex(const AABB& itemBounds) {
    int index = -1;
    float verticalMidpoint = boundary.x;
    float horizontalMidpoint = boundary.y;

    bool topQuadrant = (itemBounds.y + itemBounds.halfH < horizontalMidpoint);
    bool bottomQuadrant = (itemBounds.y - itemBounds.halfH > horizontalMidpoint);
    bool leftQuadrant = (itemBounds.x + itemBounds.halfW < verticalMidpoint);
    bool rightQuadrant = (itemBounds.x - itemBounds.halfW > verticalMidpoint);

    if (leftQuadrant) {
        if (topQuadrant) index = 0;
        else if (bottomQuadrant) index = 2;
    }
    else if (rightQuadrant) {
        if (topQuadrant) index = 1;
        else if (bottomQuadrant) index = 3;
    }

    return index;
}

bool Quadtree::insert(Entity* entity) {
    if (!boundary.intersects(entity->bounds)) return false;

    if (divided) {
        int index = getIndex(entity->bounds);
        if (index != -1) return children[index]->insert(entity);
    }

    objects.push_back(entity);

    if (objects.size() > CAPACITY && level < MAX_DEPTH) {
        if (!divided) subdivide();

        SimpleList<Entity*> movedObjects;

        objects.remove_if([&](Entity* obj) {
            int idx = getIndex(obj->bounds);
            if (idx != -1) {
                children[idx]->insert(obj);
                return true;
            }
            return false;
        });
    }
    return true;
}

void Quadtree::query(AABB range, SimpleList<Entity*>& found) {
    if (!boundary.intersects(range)) return;

    for (auto& obj : objects) {
        if (obj->active && range.intersects(obj->bounds)) {
            found.push_back(obj);
        }
    }

    if (divided) {
        for (int i = 0; i < 4; i++) {
            children[i]->query(range, found);
        }
    }
}

void Quadtree::debugRender(sf::RenderWindow& window) {
    sf::RectangleShape rect;

    rect.setSize(sf::Vector2f(boundary.halfW * 2.0f, boundary.halfH * 2.0f));

    rect.setPosition(boundary.x - boundary.halfW, boundary.y - boundary.halfH);

    rect.setFillColor(sf::Color::Transparent);

    sf::Uint8 colorIntensity = std::min(255, 50 + (level * 25));
    rect.setOutlineColor(sf::Color(0, colorIntensity, 255, 150));
    rect.setOutlineThickness(1.0f / (level + 1));

    window.draw(rect);

    if (divided) {
        for (int i = 0; i < 4; i++) {
            if (children[i]) {
                children[i]->debugRender(window);
            }
        }
    }
}