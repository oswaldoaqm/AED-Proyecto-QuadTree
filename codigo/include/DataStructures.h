#ifndef AED_PROYECTO_QUADTREE_DATASTRUCTURES_H
#define AED_PROYECTO_QUADTREE_DATASTRUCTURES_H

#include <cmath>
#include <cstddef>
#include <functional> // Necesario para usar lambdas en remove_if

// =========================================================
// 1. Estructura AABB (Sin cambios)
// =========================================================
struct AABB {
    float x, y, halfW, halfH;
    AABB() : x(0), y(0), halfW(0), halfH(0) {}
    AABB(float _x, float _y, float _hw, float _hh)
        : x(_x), y(_y), halfW(_hw), halfH(_hh) {}

    bool intersects(const AABB& other) const {
        if (std::abs(x - other.x) > (halfW + other.halfW)) return false;
        if (std::abs(y - other.y) > (halfH + other.halfH)) return false;
        return true;
    }

    bool contains(const AABB& other) const {
        return (other.x - other.halfW >= x - halfW &&
                other.x + other.halfW <= x + halfW &&
                other.y - other.halfH >= y - halfH &&
                other.y + other.halfH <= y + halfH);
    }
};

// =========================================================
// 2. SimpleList (Con remove_if)
// =========================================================
template <typename T>
class SimpleList {
private:
    struct Node {
        T data;
        Node* next;
        Node* prev;
        Node(T val) : data(val), next(nullptr), prev(nullptr) {}
    };

    Node* head;
    Node* tail;
    int count;

public:
    SimpleList() : head(nullptr), tail(nullptr), count(0) {}

    ~SimpleList() { clear(); }

    SimpleList(const SimpleList&) = delete;
    SimpleList& operator=(const SimpleList&) = delete;

    void push_back(T value) {
        Node* newNode = new Node(value);
        if (!tail) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        count++;
    }

    void clear() {
        Node* current = head;
        while (current) {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = tail = nullptr;
        count = 0;
    }

    // NUEVO: Elimina nodos que cumplan una condición (Predicado)
    // Útil para borrar enemigos muertos o balas fuera de pantalla.
    void remove_if(std::function<bool(T&)> predicate) {
        Node* current = head;
        while (current) {
            Node* nextNode = current->next;

            if (predicate(current->data)) {
                // Si cumple la condición, lo sacamos de la lista
                if (current->prev) current->prev->next = current->next;
                if (current->next) current->next->prev = current->prev;

                if (current == head) head = current->next;
                if (current == tail) tail = current->prev;

                delete current; // Borramos el NODO (no necesariamente el dato T si es puntero)
                count--;
            }
            current = nextNode;
        }
    }

    int size() const { return count; }
    bool empty() const { return count == 0; }

    struct Iterator {
        Node* ptr;
        Iterator(Node* p) : ptr(p) {}
        T& operator*() { return ptr->data; }
        Iterator& operator++() { ptr = ptr->next; return *this; }
        bool operator!=(const Iterator& other) { return ptr != other.ptr; }
    };

    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
};

#endif