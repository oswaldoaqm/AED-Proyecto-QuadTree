#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>

#include "../include/Quadtree.h"
#include "../include/Player.h"
#include "../include/Enemy.h"
#include "../include/Projectile.h"
#include "../include/Particle.h"
#include "../include/PowerUp.h"
#include "../include/FloatingText.h"

struct BackgroundCell {
    sf::CircleShape shape;
    float speed;
    float amplitude;
    float offset;
};

float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

enum GameState { MENU, PLAYING, GAME_OVER };

// CLASE DEL BOT
class BotController {
public:
    float perceptionRadius = 450.0f;
    float safeDistance = 250.0f;
    float speed = 500.0f;

    float timeSinceLastShot = 0.0f;
    float fireRate = 0.15f;

    void update(Player* player, Quadtree& quadtree, SimpleList<Entity*>& entities,
                float deltaTime, int& idCounter, int mapWidth, int mapHeight) {

        float px = player->bounds.x;
        float py = player->bounds.y;
        AABB perceptionBox(px, py, perceptionRadius, perceptionRadius);

        SimpleList<Entity*> threats;
        quadtree.query(perceptionBox, threats);

        float moveX = 0, moveY = 0;
        int threatCount = 0;
        Entity* nearestTarget = nullptr;
        float minDistSq = 10000000.0f;

        for (auto& e : threats) {
            if (!e->active || e->id == player->id) continue;

            float dx = px - e->bounds.x;
            float dy = py - e->bounds.y;
            float distSq = dx*dx + dy*dy;

            if (e->type == ENEMY || e->type == BOSS) {
                if (distSq < perceptionRadius * perceptionRadius) {
                    float dist = sqrt(distSq);
                    float strength = (perceptionRadius - dist) / perceptionRadius;

                    moveX += (dx / dist) * strength * 8.0f;
                    moveY += (dy / dist) * strength * 8.0f;
                    threatCount++;
                }

                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    nearestTarget = e;
                }
            }
            else if (e->type == POWERUP) {
                float dist = sqrt(distSq);
                float attraction = (player->health < 50) ? 4.0f : 1.5f;
                moveX -= (dx / dist) * attraction;
                moveY -= (dy / dist) * attraction;
            }
        }

        float margin = 150.0f;
        if (px < margin) moveX += 3.0f * ((margin - px) / margin);
        if (px > mapWidth - margin) moveX -= 3.0f * ((px - (mapWidth - margin)) / margin);
        if (py < margin) moveY += 3.0f * ((margin - py) / margin);
        if (py > mapHeight - margin) moveY -= 3.0f * ((py - (mapHeight - margin)) / margin);

        if (threatCount == 0) {
            float dx = (mapWidth / 2.0f) - px;
            float dy = (mapHeight / 2.0f) - py;
            moveX += dx * 0.001f;
            moveY += dy * 0.001f;
        }

        float length = sqrt(moveX*moveX + moveY*moveY);

        if (length > 0.05f) {

            float inputMag = std::min(length, 1.0f);

            float dirX = moveX / length;
            float dirY = moveY / length;

            player->bounds.x += dirX * inputMag * speed * deltaTime;
            player->bounds.y += dirY * inputMag * speed * deltaTime;
        }

        // DISPARO
        timeSinceLastShot += deltaTime;
        bool panicMode = (threatCount > 15);
        bool shouldShoot = true;

        if (panicMode && nearestTarget && nearestTarget->type == ENEMY) {
            Enemy* en = static_cast<Enemy*>(nearestTarget);
            if (en->variant == NORMAL && !player->hasTripleShot()) shouldShoot = false;
        }

        if (nearestTarget && timeSinceLastShot >= fireRate && shouldShoot) {
            timeSinceLastShot = 0.0f;
            float dx = nearestTarget->bounds.x - px;
            float dy = nearestTarget->bounds.y - py;

            entities.push_back(new Projectile(idCounter++, px, py, dx, dy));
            if (player->hasTripleShot()) {
                entities.push_back(new Projectile(idCounter++, px, py, dx + dy*0.2f, dy - dx*0.2f));
                entities.push_back(new Projectile(idCounter++, px, py, dx - dy*0.2f, dy + dx*0.2f));
            }
        }
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const int WIDTH = 1920;
    const int HEIGHT = 1080;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cellular Defense - Final", sf::Style::Default, settings);
    window.setFramerateLimit(165);

    // Vista principal (Cámara)
    sf::View mainView(sf::FloatRect(0, 0, (float)WIDTH, (float)HEIGHT));
    window.setView(mainView);

    // --- CARGA DE RECURSOS ---
    sf::Font font;
    bool fontLoaded = false;
    // Intentamos cargar de varias rutas por si acaso
    std::vector<std::string> fontPaths = {"../codigo/assets/arial.ttf", "C:/Windows/Fonts/arial.ttf"};
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) { fontLoaded = true; break; }
    }

    // UI Setup
    sf::Text scoreText, debugText, gameOverText, restartText;
    sf::Text titleText, startInstructionText;
    if (fontLoaded) {
        scoreText.setFont(font); scoreText.setCharacterSize(35); scoreText.setPosition(30, 30);
        debugText.setFont(font); debugText.setCharacterSize(20); debugText.setPosition(30, 80);

        gameOverText.setFont(font); gameOverText.setString("GAME OVER");
        gameOverText.setCharacterSize(100);
        gameOverText.setFillColor(sf::Color::Red);

        // Centramos el texto de Game Over
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        gameOverText.setPosition(WIDTH/2.0f, HEIGHT/2.0f - 50);

        restartText.setFont(font); restartText.setString("Presiona 'R' para Reiniciar");
        restartText.setCharacterSize(40);
        sf::FloatRect restartRect = restartText.getLocalBounds();
        restartText.setOrigin(restartRect.left + restartRect.width/2.0f, restartRect.top + restartRect.height/2.0f);
        restartText.setPosition(WIDTH/2.0f, HEIGHT/2.0f + 50);

        titleText.setFont(font);
        titleText.setString("CELLULAR DEFENSE");
        titleText.setCharacterSize(80);
        titleText.setFillColor(sf::Color::Cyan);
        titleText.setStyle(sf::Text::Bold);

        sf::FloatRect titleRect = titleText.getLocalBounds();
        titleText.setOrigin(titleRect.left + titleRect.width/2.0f, titleRect.top + titleRect.height/2.0f);
        titleText.setPosition(WIDTH/2.0f, HEIGHT/2.0f - 100);

        startInstructionText.setFont(font);
        startInstructionText.setString("Presiona ENTER para Iniciar");
        startInstructionText.setCharacterSize(40);

        sf::FloatRect startRect = startInstructionText.getLocalBounds();
        startInstructionText.setOrigin(startRect.left + startRect.width/2.0f, startRect.top + startRect.height/2.0f);
        startInstructionText.setPosition(WIDTH/2.0f, HEIGHT/2.0f + 50);
    }

    // Barra de Vida
    sf::RectangleShape healthBarBg(sf::Vector2f(300, 20));
    healthBarBg.setFillColor(sf::Color(50, 50, 50));
    healthBarBg.setPosition(WIDTH - 330, 30);

    sf::RectangleShape healthBar(sf::Vector2f(300, 20));
    healthBar.setFillColor(sf::Color::Green);
    healthBar.setPosition(WIDTH - 330, 30);

    // --- AMBIENTACIÓN BIOLÓGICA ---
    std::vector<BackgroundCell> bgCells;
    for(int i = 0; i < 60; i++) {
        BackgroundCell c;
        float radius = randomFloat(10.0f, 30.0f);
        c.shape.setRadius(radius);
        c.shape.setPointCount(30);
        c.shape.setPosition(randomFloat(0, WIDTH), randomFloat(0, HEIGHT));

        c.shape.setFillColor(sf::Color(150, 20, 40, randomFloat(20, 60)));

        c.speed = randomFloat(30.0f, 80.0f);
        c.amplitude = randomFloat(0.5f, 1.5f);
        c.offset = randomFloat(0.0f, 6.28f);
        bgCells.push_back(c);
    }

    // Definición del mundo y Quadtree
    AABB worldBounds(WIDTH / 2.0f, HEIGHT / 2.0f, WIDTH / 2.0f, HEIGHT / 2.0f);
    Quadtree quadtree(worldBounds);

    SimpleList<Entity*> entities;
    SimpleList<Particle*> particles;
    SimpleList<FloatingText*> floatingTexts;

    // Crear Jugador
    Player* player = new Player(0, WIDTH / 2.0f, HEIGHT / 2.0f);
    entities.push_back(player);

    // Variables de Juego
    sf::Clock clock;
    float spawnTimer = 0.0f;
    float spawnInterval = 1.5f; // Empezamos lento
    float shootTimer = 0.0f;
    float shakeTimer = 0.0f;
    float damageFlashTimer = 0.0f;
    float menuFadeTimer = 0.0f;
    int entityIdCounter = 1;
    int score = 0;
    int nextBossScore = 500;
    int bacteriaKilled = 0;
    const float FADE_DURATION = 2.0f;

    GameState state = MENU;
    bool showDebug = false;
    bool useQuadtree = true;

    // Lambda para crear explosiones
    auto spawnExplosion = [&](float x, float y, sf::Color c, int count) {
        for(int i=0; i<count; i++) {
            particles.push_back(new Particle(x, y, randomFloat(100, 300), randomFloat(0, 360), c));
        }
    };

    BotController bot;
    bool autoPilot = false;

    // BUCLE PRINCIPAL
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        if (deltaTime > 0.1f) deltaTime = 0.1f; // Evitar saltos grandes si se traba

        if (state == MENU) {
            menuFadeTimer += deltaTime;
        }

        if (damageFlashTimer > 0) {
            damageFlashTimer -= deltaTime;
        }

        // 1. INPUT
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (state == MENU && event.key.code == sf::Keyboard::Enter) {
                    state = PLAYING;
                }

                if (event.key.code == sf::Keyboard::K) {
                    for(int i=0; i<50; i++) {
                        float x = randomFloat(0, WIDTH);
                        float y = randomFloat(0, HEIGHT);
                        entities.push_back(new Enemy(entityIdCounter++, x, y, player->getPosition().x, player->getPosition().y, 0, false, KAMIKAZE));
                    }
                }
                if (event.key.code == sf::Keyboard::B) showDebug = !showDebug;
                if (event.key.code == sf::Keyboard::Space) useQuadtree = !useQuadtree; // Comparar rendimiento

                if (event.key.code == sf::Keyboard::P) {
                    autoPilot = !autoPilot;
                    if (fontLoaded) {
                        floatingTexts.push_back(new FloatingText(font, autoPilot ? "BOT ACTIVADO" : "MANUAL", player->bounds.x, player->bounds.y - 50, sf::Color::Cyan));
                    }
                }

                if (state == GAME_OVER && event.key.code == sf::Keyboard::R) {
                    // Reiniciar juego
                    for (auto& e : entities) delete e; entities.clear();
                    for (auto& p : particles) delete p; particles.clear();
                    for (auto& t : floatingTexts) delete t; floatingTexts.clear();

                    player = new Player(0, WIDTH / 2.0f, HEIGHT / 2.0f);
                    entities.push_back(player);
                    bacteriaKilled = 0;
                    score = 0;
                    spawnInterval = 1.5f;
                    nextBossScore = 500;
                    state = PLAYING;
                }
            }
        }

        if (state == PLAYING) {
            // 2. UPDATE (Lógica)

            for(auto& c : bgCells) {
                float moveY = c.speed * deltaTime;
                float moveX = std::sin(menuFadeTimer + c.offset) * c.amplitude;

                c.shape.move(moveX, moveY);

                if (c.shape.getPosition().y > HEIGHT) {
                    c.shape.setPosition(randomFloat(0, WIDTH), -50.0f);
                }
            }

            // Cámara Shake
            if (shakeTimer > 0) {
                shakeTimer -= deltaTime;
                mainView.setCenter(WIDTH/2.0f + randomFloat(-5, 5), HEIGHT/2.0f + randomFloat(-5, 5));
            } else {
                mainView.setCenter(WIDTH/2.0f, HEIGHT/2.0f);
            }
            window.setView(mainView);

            // BOT y accion DISPARAR
            if (autoPilot) {
                // 1. MODO BOT: El bot calcula movimiento y dispara solo
                bot.update(player, quadtree, entities, deltaTime, entityIdCounter, WIDTH, HEIGHT);
            }
            else {
                // 2. MODO MANUAL: Tú controlas el disparo con el mouse
                // (El movimiento WASD se procesa dentro de player->update más abajo)

                shootTimer += deltaTime;
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && shootTimer >= 0.12f) {
                    shootTimer = 0.0f;
                    sf::Vector2i m = sf::Mouse::getPosition(window);

                    // Vector dirección hacia el mouse
                    float dx = (float)m.x - player->getPosition().x;
                    float dy = (float)m.y - player->getPosition().y;

                    // Disparo central
                    entities.push_back(new Projectile(entityIdCounter++, player->getPosition().x, player->getPosition().y, dx, dy));

                    // Si tiene el PowerUp activado (Triple Shot)
                    if (player->hasTripleShot()) {
                        entities.push_back(new Projectile(entityIdCounter++, player->getPosition().x, player->getPosition().y, dx + dy*0.2f, dy - dx*0.2f));
                        entities.push_back(new Projectile(entityIdCounter++, player->getPosition().x, player->getPosition().y, dx - dy*0.2f, dy + dx*0.2f));
                    }
                }
            }

            // Spawner
            spawnTimer += deltaTime;
            if (spawnTimer >= spawnInterval) {
                spawnTimer = 0.0f;
                // Spawnear en los bordes
                float x, y;
                if (rand() % 2 == 0) { x = (rand()%2)*WIDTH; y = randomFloat(0, HEIGHT); }
                else { x = randomFloat(0, WIDTH); y = (rand()%2)*HEIGHT; }

                float prob = randomFloat(0, 1);
                EnemyVariant variant = NORMAL;
                if (prob < 0.2f) variant = KAMIKAZE;
                else if (prob < 0.35f) variant = TANK;

                int gen = (variant == NORMAL) ? 0 : 0; // Solo los normales se dividen al inicio
                entities.push_back(new Enemy(entityIdCounter++, x, y, player->bounds.x, player->bounds.y, gen, false, variant));
            }

            // Boss Spawner
            if (score >= nextBossScore) {
                nextBossScore += 500;
                entities.push_back(new Enemy(entityIdCounter++, 100, 100, player->bounds.x, player->bounds.y, 0, true));
                spawnInterval = std::max(0.5f, spawnInterval - 0.1f); // Dificultad progresiva
            }

            // Actualizar Entidades
            for (auto& e : entities) {
                if (!e->active) continue;

                // Lógica de rastro de toxinas
                if (e->type == ENEMY || e->type == BOSS) {
                    static_cast<Enemy*>(e)->setTarget(player->getPosition().x, player->getPosition().y);

                    // Probabilidad de soltar rastro (5% por frame)
                    if (rand() % 100 < 5) {
                        sf::Color toxinColor = sf::Color(50, 200, 50, 150);
                        float pSize = 10.0f;

                        // Si es un BOSS o un TANK, el rastro es más denso y oscuro
                        if (e->type == BOSS || (e->type == ENEMY && static_cast<Enemy*>(e)->variant == TANK)) {
                            toxinColor = sf::Color(100, 30, 200, 180);
                            pSize = 25.0f;
                        }

                        particles.push_back(new Particle(
                            e->bounds.x, e->bounds.y,
                            randomFloat(5, 15),
                            randomFloat(0, 360),
                            toxinColor
                        ));
                    }
                }

                if (autoPilot && e == player) {
                    if (player->invulnerableTimer > 0) player->invulnerableTimer -= deltaTime;
                    if (player->tripleShotTimer > 0) player->tripleShotTimer -= deltaTime;

                    e->update(0.0f);
                }
                else {
                    e->update(deltaTime);
                }
            }

            // Actualizar Partículas y Textos
            particles.remove_if([&](Particle*& p) { if(!p->update(deltaTime)) { delete p; return true; } return false; });
            floatingTexts.remove_if([&](FloatingText*& t) { t->update(deltaTime); if(!t->active) { delete t; return true; } return false; });

            // 3. QUADTREE & COLISIONES
            if (useQuadtree) {
                quadtree.clear();
                for (auto& e : entities) {
                    if (e->active) quadtree.insert(e);
                }
            }

            // Lista para nuevos enemigos generados por división
            SimpleList<Entity*> bornEntities;
            int collisionChecks = 0;

            // Recorremos entidades buscando colisiones
            for (auto& entity : entities) {
                if (!entity->active || (entity->type != PROJECTILE && entity->type != PLAYER)) continue;

                SimpleList<Entity*> candidates;
                if (useQuadtree) {
                    quadtree.query(entity->bounds, candidates);
                } else {
                    // Fuerza bruta para comparar rendimiento
                    for(auto& other : entities) if(other->active) candidates.push_back(other);
                }

                for (auto& other : candidates) {
                    if (entity->id == other->id || !other->active) continue;

                    collisionChecks++;

                    if (entity->bounds.intersects(other->bounds)) {

                        // CASO A: Bala golpea Enemigo
                        if (entity->type == PROJECTILE && (other->type == ENEMY || other->type == BOSS)) {
                            entity->active = false; // Bala muere

                            Enemy* enemy = static_cast<Enemy*>(other);
                            enemy->health--;

                            // Hit marker visual
                            spawnExplosion(enemy->bounds.x, enemy->bounds.y, sf::Color::Yellow, 3);

                            if (enemy->health <= 0) {
                                enemy->active = false;
                                bacteriaKilled++;

                                // Logica de puntaje
                                int puntosGanados = 0;

                                if (enemy->type == BOSS) {
                                    puntosGanados = 50;
                                }
                                else if (enemy->variant == NORMAL) {
                                    if (enemy->generation == 2) {
                                        puntosGanados = 10;
                                    }
                                }
                                else {
                                    puntosGanados = 10;
                                }

                                if (puntosGanados > 0) {
                                    score += puntosGanados;
                                    if(fontLoaded) floatingTexts.push_back(new FloatingText(font, "+" + std::to_string(puntosGanados), enemy->bounds.x, enemy->bounds.y, sf::Color::Yellow));
                                }

                                spawnExplosion(enemy->bounds.x, enemy->bounds.y, sf::Color::Red, 15);

                                // Mecánica de Mitosis
                                if (enemy->type == ENEMY && enemy->variant == NORMAL && enemy->generation < 2) {
                                    bornEntities.push_back(new Enemy(entityIdCounter++, enemy->bounds.x-10, enemy->bounds.y, player->bounds.x, player->bounds.y, enemy->generation+1));
                                    bornEntities.push_back(new Enemy(entityIdCounter++, enemy->bounds.x+10, enemy->bounds.y, player->bounds.x, player->bounds.y, enemy->generation+1));
                                }

                                // Drop PowerUp (12% prob)
                                if (rand() % 100 < 12) {
                                    bornEntities.push_back(new PowerUp(entityIdCounter++, enemy->bounds.x, enemy->bounds.y, (PowerUpType)(rand() % 4)));
                                }
                            }
                        }

                        // CASO B: Jugador toca PowerUp
                        else if (entity->type == PLAYER && other->type == POWERUP) {
                            PowerUp* pu = static_cast<PowerUp*>(other);
                            PowerUpType type = pu->getPUType();
                            std::string txt = "";
                            sf::Color col = sf::Color::White;

                            if (type == HEAL) {
                                player->health = std::min(player->maxHealth, player->health + 30);
                                txt = "HEALTH UP!"; col = sf::Color::Green;
                            }
                            else if (type == SHIELD) {
                                player->invulnerableTimer = 5.0f; // 5 segundos inmortal
                                txt = "SHIELD!"; col = sf::Color::Cyan;
                            }
                            else if (type == TRIPLE) {
                                player->tripleShotTimer = 8.0f; // 8 segundos de escopeta
                                txt = "TRIPLE SHOT!"; col = sf::Color::Magenta;
                            }
                            else if (type == BOMB) {
                                // Lógica de la BOMBA: Matar a todos los enemigos en pantalla
                                txt = "BOOM!"; col = sf::Color::Red;
                                spawnExplosion(player->bounds.x, player->bounds.y, sf::Color::White, 50); // Mega explosión visual
                                shakeTimer = 0.5f; // Sacudir cámara

                                // Recorremos todas las entidades para matar a los enemigos
                                for (auto& e : entities) {
                                    if (e->type == ENEMY && e->active) {
                                        e->active = false;
                                        spawnExplosion(e->bounds.x, e->bounds.y, sf::Color(255, 100, 0), 10);
                                        score += 10;
                                    }
                                }
                            }

                            if (fontLoaded) floatingTexts.push_back(new FloatingText(font, txt, player->bounds.x, player->bounds.y - 30, col));
                            other->active = false; // El powerup desaparece
                        }

                        // CASO C: Jugador choca con Enemigo
                        else if (entity->type == PLAYER && (other->type == ENEMY || other->type == BOSS)) {
                            if (!player->isInvulnerable()) {
                                player->takeDamage(10);
                                shakeTimer = 0.3f;
                                damageFlashTimer = 0.2f;
                                spawnExplosion(player->bounds.x, player->bounds.y, sf::Color::Red, 10);
                                if (player->health <= 0) state = GAME_OVER;
                            }
                        }
                    }
                }
            }

            // Agregar nuevos spawns a la lista principal
            for (auto& newE : bornEntities) {
                entities.push_back(newE);
            }

            // Limpiar entidades muertas (Garbage Collection)
            entities.remove_if([](Entity*& e) {
                if (!e->active) {
                    delete e;
                    return true;
                }
                return false;
            });

            // Actualizar UI
            healthBar.setSize(sf::Vector2f(3.0f * player->health, 20.0f));
            if(fontLoaded) {
                scoreText.setString("Puntaje: " + std::to_string(score));
                debugText.setString("Entidades: " + std::to_string(entities.size()) +
                                  "\nChecks Colision: " + std::to_string(collisionChecks) +
                                  "\nFPS: " + std::to_string((int)(1.0f/deltaTime)));
            }
        }

        // 4. RENDER
        float heartbeat = (std::sin(menuFadeTimer * 2.0f) + 1.0f) / 2.0f;
        sf::Color bloodColor;

        if (damageFlashTimer > 0) {
            bloodColor = sf::Color(120, 0, 0);
        } else {
            // Oscila suavemente entre un rojo muy oscuro y uno más visible
            sf::Uint8 r = static_cast<sf::Uint8>(25 + (heartbeat * 15));
            bloodColor = sf::Color(r, 5, 5);
        }
        window.clear(bloodColor);

        // Dibujar fondo
        for (auto& s : bgCells) window.draw(s.shape);

        if (state == MENU) {
            if (fontLoaded) {
                // Lógica del Título (Fade-in inicial)
                float titleAlpha = std::min(1.0f, menuFadeTimer / FADE_DURATION);
                titleText.setFillColor(sf::Color(0, 255, 255, static_cast<sf::Uint8>(titleAlpha * 255)));

                // Lógica de la Instrucción (Parpadeo tipo "Breath")
                float blink = (std::sin(menuFadeTimer * 5.0f) + 1.0f) / 2.0f;
                float instAlpha = (blink * 0.8f + 0.2f) * titleAlpha;

                startInstructionText.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(instAlpha * 255)));

                window.draw(titleText);
                window.draw(startInstructionText);
            }
        }
        else if (state == PLAYING) {
            if (showDebug && useQuadtree) {
                quadtree.debugRender(window);

                sf::RectangleShape queryBox;
                float w = player->bounds.halfW * 2.0f;
                float h = player->bounds.halfH * 2.0f;

                queryBox.setSize(sf::Vector2f(w, h));
                queryBox.setOrigin(w / 2.0f, h / 2.0f);
                queryBox.setPosition(player->bounds.x, player->bounds.y);

                queryBox.setFillColor(sf::Color::Transparent);
                queryBox.setOutlineColor(sf::Color::Green);
                queryBox.setOutlineThickness(2.0f);

                window.draw(queryBox);
            }

            for (auto& e : entities) e->render(window);
            for (auto& p : particles) p->render(window);
            for (auto& t : floatingTexts) t->render(window);

            window.draw(healthBarBg);
            window.draw(healthBar);
            if(fontLoaded) {
                window.draw(scoreText);
                if (showDebug) window.draw(debugText);
            }
        }
        else if (state == GAME_OVER) {
            if(fontLoaded) {
                window.draw(gameOverText);

                // Texto dinámico para las estadísticas
                sf::Text statsText;
                statsText.setFont(font);
                statsText.setCharacterSize(30);
                statsText.setFillColor(sf::Color::White);
                statsText.setString("Bacterias eliminadas: " + std::to_string(bacteriaKilled) +
                                   "\nPuntaje Total: " + std::to_string(score));

                sf::FloatRect sRect = statsText.getLocalBounds();
                statsText.setOrigin(sRect.left + sRect.width/2.0f, sRect.top + sRect.height/2.0f);
                statsText.setPosition(WIDTH/2.0f, HEIGHT/2.0f + 150);

                window.draw(statsText);
                window.draw(restartText);
            }
        }

        window.display();
    }

    // Limpieza final de memoria
    for (auto& e : entities) delete e;
    for (auto& p : particles) delete p;
    for (auto& t : floatingTexts) delete t;

    return 0;
}