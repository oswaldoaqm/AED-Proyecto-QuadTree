#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>

#include "../include/Quadtree.h"
#include "../include/Player.h"
#include "../include/Enemy.h"
#include "../include/Projectile.h"
#include "../include/Particle.h"

float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

enum GameState { PLAYING, GAME_OVER };

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const int WIDTH = 1920;
    const int HEIGHT = 1080;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cellular Defense - 1080p", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = false;
    std::vector<std::string> fontPaths = {"../codigo/assets/arial.ttf", "C:/Windows/Fonts/arial.ttf"};
    for (const auto& path : fontPaths) { if (font.loadFromFile(path)) { fontLoaded = true; break; } }

    sf::Text scoreText, debugText, gameOverText, restartText;
    if (fontLoaded) {
        scoreText.setFont(font); scoreText.setCharacterSize(30); scoreText.setPosition(20, 20);
        debugText.setFont(font); debugText.setCharacterSize(24); debugText.setPosition(20, 60);
        gameOverText.setFont(font); gameOverText.setString("GAME OVER");
        gameOverText.setCharacterSize(100); gameOverText.setFillColor(sf::Color::Red);
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.width/2, textRect.height/2);
        gameOverText.setPosition(WIDTH/2.0f, HEIGHT/2.0f - 100);
        restartText.setFont(font); restartText.setString("Presiona 'R' para Reiniciar");
        restartText.setCharacterSize(40);
        sf::FloatRect rRect = restartText.getLocalBounds();
        restartText.setOrigin(rRect.width/2, rRect.height/2);
        restartText.setPosition(WIDTH/2.0f, HEIGHT/2.0f + 50);
    }

    AABB worldBounds(WIDTH / 2.0f, HEIGHT / 2.0f, WIDTH / 2.0f, HEIGHT / 2.0f);
    Quadtree quadtree(worldBounds);
    SimpleList<Entity*> entities;
    SimpleList<Particle*> particles;

    Player* player = new Player(0, WIDTH / 2.0f, HEIGHT / 2.0f);
    entities.push_back(player);

    sf::Clock clock;
    float spawnTimer = 0.0f;
    float spawnInterval = 1.0f;
    float shootTimer = 0.0f;
    const float shootInterval = 0.12f;

    int entityIdCounter = 1;
    int score = 0;
    int nextBossScore = 250;
    float difficultyMultiplier = 1.0f;

    GameState state = PLAYING;
    bool showDebug = false;
    bool useQuadtree = true;

    auto spawnExplosion = [&](float x, float y, sf::Color c, int num = 15) {
        for (int i = 0; i < num; i++) {
            particles.push_back(new Particle(x, y, randomFloat(80, 250), randomFloat(0, 360), c));
        }
    };

    while (window.isOpen()) {
        sf::Time dt = clock.restart();
        float deltaTime = dt.asSeconds();
        float fps = (deltaTime > 0) ? 1.0f / deltaTime : 0;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (state == GAME_OVER && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                for (auto& e : entities) delete e;
                entities.clear();
                for (auto& p : particles) delete p;
                particles.clear();
                entityIdCounter = 1; score = 0; nextBossScore = 250; difficultyMultiplier = 1.0f; spawnInterval = 1.0f;
                player = new Player(0, WIDTH / 2.0f, HEIGHT / 2.0f);
                entities.push_back(player);
                state = PLAYING;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::B) showDebug = !showDebug;
                if (event.key.code == sf::Keyboard::Space) useQuadtree = !useQuadtree;
            }
        }

        if (state == PLAYING) {
            shootTimer += deltaTime;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && shootTimer >= shootInterval) {
                shootTimer = 0.0f;
                sf::Vector2i m = sf::Mouse::getPosition(window);
                entities.push_back(new Projectile(entityIdCounter++, player->getPosition().x, player->getPosition().y, (float)m.x - player->getPosition().x, (float)m.y - player->getPosition().y));
            }

            spawnTimer += deltaTime;
            if (spawnTimer >= spawnInterval) {
                spawnTimer = 0.0f;
                float x = (rand()%2==0)?(rand()%2?-50:WIDTH+50):randomFloat(0,WIDTH);
                float y = (rand()%2!=0)?(rand()%2?-50:HEIGHT+50):randomFloat(0,HEIGHT);
                float prob = randomFloat(0, 1);
                int gen = (prob < 0.25f) ? 0 : (prob < 0.60f ? 1 : 2);
                entities.push_back(new Enemy(entityIdCounter++, x, y, player->bounds.x, player->bounds.y, gen, false));
            }

            if (score >= nextBossScore) {
                nextBossScore += 250;
                entities.push_back(new Enemy(entityIdCounter++, -100, -100, player->bounds.x, player->bounds.y, 0, true));
                difficultyMultiplier += 0.1f;
                spawnInterval *= 0.95f;
            }

            for (auto& entity : entities) {
                if (!entity->active) continue;
                if (entity->type == ENEMY || entity->type == BOSS)
                    static_cast<Enemy*>(entity)->setTarget(player->bounds.x, player->bounds.y);
                entity->update(deltaTime);
            }

            particles.remove_if([&](Particle*& p) { if (!p->update(deltaTime)) { delete p; return true; } return false; });

            if (useQuadtree) {
                quadtree.clear();
                for (auto& e : entities) if (e->active) quadtree.insert(e);
            }

            SimpleList<Entity*> newSpawns;
            int checks = 0;
            for (auto& entity : entities) {
                if (!entity->active || (entity->type != PLAYER && entity->type != PROJECTILE)) continue;

                // MODO DE COLISIÓN CORREGIDO
                if (useQuadtree) {
                    SimpleList<Entity*> candidates;
                    quadtree.query(entity->bounds, candidates);
                    for (auto& other : candidates) {
                        if (entity->id == other->id || !other->active) continue;
                        checks++;
                        if (entity->bounds.intersects(other->bounds)) {
                            if (entity->type == PROJECTILE && (other->type == ENEMY || other->type == BOSS)) {
                                entity->active = false;
                                other->health--;
                                if (other->health <= 0) {
                                    other->active = false;
                                    spawnExplosion(other->bounds.x, other->bounds.y, sf::Color(255, 165, 0), other->type == BOSS ? 60 : 15);
                                    if (other->type == ENEMY) {
                                        Enemy* e = static_cast<Enemy*>(other);
                                        if (e->generation == 2) score += 10;
                                        else {
                                            newSpawns.push_back(new Enemy(entityIdCounter++, e->bounds.x - 15, e->bounds.y, player->bounds.x, player->bounds.y, e->generation + 1, false));
                                            newSpawns.push_back(new Enemy(entityIdCounter++, e->bounds.x + 15, e->bounds.y, player->bounds.x, player->bounds.y, e->generation + 1, false));
                                        }
                                    } else score += 100;
                                }
                            } else if (entity->type == PLAYER && (other->type == ENEMY || other->type == BOSS)) {
                                state = GAME_OVER;
                                spawnExplosion(player->bounds.x, player->bounds.y, sf::Color::Cyan, 40);
                            }
                        }
                    }
                } else {
                    // MODO FUERZA BRUTA CORREGIDO (Itera sobre entities directamente)
                    for (auto& other : entities) {
                        if (entity->id == other->id || !other->active) continue;
                        checks++;
                        if (entity->bounds.intersects(other->bounds)) {
                            if (entity->type == PROJECTILE && (other->type == ENEMY || other->type == BOSS)) {
                                entity->active = false; other->health--;
                                if (other->health <= 0) {
                                    other->active = false;
                                    spawnExplosion(other->bounds.x, other->bounds.y, sf::Color(255, 165, 0), other->type == BOSS ? 60 : 15);
                                    if (other->type == ENEMY) {
                                        Enemy* e = static_cast<Enemy*>(other);
                                        if (e->generation == 2) score += 10;
                                        else {
                                            newSpawns.push_back(new Enemy(entityIdCounter++, e->bounds.x, e->bounds.y, player->bounds.x, player->bounds.y, e->generation + 1, false));
                                            newSpawns.push_back(new Enemy(entityIdCounter++, e->bounds.x, e->bounds.y, player->bounds.x, player->bounds.y, e->generation + 1, false));
                                        }
                                    } else score += 100;
                                }
                            } else if (entity->type == PLAYER && (other->type == ENEMY || other->type == BOSS)) {
                                state = GAME_OVER;
                                spawnExplosion(player->bounds.x, player->bounds.y, sf::Color::Cyan, 40);
                            }
                        }
                    }
                }
            }

            for (auto& s : newSpawns) entities.push_back(s);
            entities.remove_if([](Entity*& e) { if (!e->active) { delete e; return true; } return false; });

            if (fontLoaded) {
                scoreText.setString("Puntaje: " + std::to_string(score) + "  Dificultad: x" + std::to_string(difficultyMultiplier).substr(0,3));
                debugText.setString("FPS: " + std::to_string((int)fps) + "\nModo: " + std::string(useQuadtree?"Quadtree":"FB") + "\nComparaciones: " + std::to_string(checks) + "\nEntidades: " + std::to_string(entities.size()));
            }
        }

        window.clear(sf::Color::Black);
        if (state == PLAYING) {
            if (showDebug && useQuadtree) quadtree.debugRender(window);
            for (auto& e : entities) e->render(window);
            for (auto& p : particles) p->render(window);
            if (fontLoaded) { window.draw(scoreText); window.draw(debugText); }
        } else {
            if (fontLoaded) { window.draw(gameOverText); window.draw(restartText); window.draw(scoreText); }
        }
        window.display();
    }
    for (auto& e : entities) delete e;
    for (auto& p : particles) delete p;
    return 0;
}