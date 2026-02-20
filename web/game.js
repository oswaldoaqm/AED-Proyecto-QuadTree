// ============================================================
//  game.js – Game Loop principal (puerto de main.cpp)
// ============================================================

const canvas = document.getElementById('gameCanvas');
const ctx = canvas.getContext('2d');

const WIDTH = 1920;
const HEIGHT = 1080;
canvas.width = WIDTH;
canvas.height = HEIGHT;

// ----------- Estado Global -----------
const GameState = { MENU: 'MENU', PLAYING: 'PLAYING', GAME_OVER: 'GAME_OVER' };
let state = GameState.MENU;

let score = 0;
let bacteriaKilled = 0;
let nextBossScore = 500;
let spawnInterval = 1.5;
let spawnTimer = 0;
let shootTimer = 0;
let shakeTimer = 0;
let damageFlashTimer = 0;
let menuFadeTimer = 0;
let collisionChecks = 0;

let useQuadtree = true;
let showDebug = false;
let autoPilot = false;

const keys = {};
const mouse = { x: WIDTH / 2, y: HEIGHT / 2, down: false };

let player, entities, particles, floatingTexts, bgCells;
let worldBounds, quadtree;
const bot = new BotController();

// ----------- Input Handlers -----------
window.addEventListener('keydown', e => {
    // Evitar que Space y flechas hagan scroll en el navegador
    if ([' ', 'arrowup', 'arrowdown', 'arrowleft', 'arrowright'].includes(e.key.toLowerCase())) {
        e.preventDefault();
    }
    keys[e.key.toLowerCase()] = true;
    handleKeyPress(e.key.toLowerCase());
});
window.addEventListener('keyup', e => { keys[e.key.toLowerCase()] = false; });
canvas.addEventListener('mousemove', e => {
    const rect = canvas.getBoundingClientRect();
    const scaleX = WIDTH / rect.width;
    const scaleY = HEIGHT / rect.height;
    mouse.x = (e.clientX - rect.left) * scaleX;
    mouse.y = (e.clientY - rect.top) * scaleY;
});
canvas.addEventListener('mousedown', e => { if (e.button === 0) mouse.down = true; });
canvas.addEventListener('mouseup', e => { if (e.button === 0) mouse.down = false; });

function handleKeyPress(key) {
    if (state === GameState.MENU && key === 'enter') {
        startGame();
        return;
    }
    if (state === GameState.GAME_OVER && key === 'r') {
        initGame();
        return;
    }
    if (key === 'b') {
        showDebug = !showDebug;
    }
    if (key === ' ') {
        useQuadtree = !useQuadtree;
        spawnFloatingText(useQuadtree ? 'QUADTREE ON' : 'FUERZA BRUTA', player.bounds.x, player.bounds.y - 50, '#0ff');
    }
    if (key === 'p') {
        autoPilot = !autoPilot;
        spawnFloatingText(autoPilot ? 'BOT ACTIVADO' : 'MANUAL', player.bounds.x, player.bounds.y - 50, '#0ff');
    }
    if (key === 'k' && state === GameState.PLAYING) {
        for (let i = 0; i < 50; i++) {
            entities.push(new Enemy(
                Math.random() * WIDTH, Math.random() * HEIGHT,
                player.bounds.x, player.bounds.y,
                0, false, EnemyVariant.KAMIKAZE
            ));
        }
    }
}

// ----------- Helpers -----------
function spawnExplosion(x, y, color, count) {
    for (let i = 0; i < count; i++) {
        particles.push(new Particle(x, y, randomFloat(80, 280), randomFloat(0, 360), color));
    }
}

function spawnFloatingText(text, x, y, color) {
    floatingTexts.push(new FloatingText(text, x, y, color));
}

// ----------- Init Game -----------
function initGame() {
    state = GameState.MENU;
    menuFadeTimer = 0;
    score = 0;
    bacteriaKilled = 0;
    nextBossScore = 500;
    spawnInterval = 1.5;
    spawnTimer = 0;
    shootTimer = 0;
    shakeTimer = 0;
    damageFlashTimer = 0;
    collisionChecks = 0;
    useQuadtree = true;
    showDebug = false;
    autoPilot = false;

    _idCounter = 1;

    player = new Player(WIDTH / 2, HEIGHT / 2);
    entities = [player];
    particles = [];
    floatingTexts = [];

    bgCells = [];
    for (let i = 0; i < 60; i++) bgCells.push(new BackgroundCell(WIDTH, HEIGHT));

    worldBounds = new AABB(WIDTH / 2, HEIGHT / 2, WIDTH / 2, HEIGHT / 2);
    quadtree = new Quadtree(worldBounds);
}

function startGame() {
    state = GameState.PLAYING;
}

// ----------- Main Loop -----------
let lastTime = 0;

function gameLoop(timestamp) {
    let dt = (timestamp - lastTime) / 1000;
    lastTime = timestamp;
    if (dt > 0.1) dt = 0.1;

    // menuFadeTimer siempre avanza (necesario para animaciones en todos los estados)
    menuFadeTimer += dt;

    if (damageFlashTimer > 0) damageFlashTimer -= dt;

    // --- UPDATE ---
    if (state === GameState.PLAYING) {
        update(dt);
    } else if (state === GameState.MENU || state === GameState.GAME_OVER) {
        // Animar fondo en menú y game over
        for (const c of bgCells) c.update(dt, menuFadeTimer, WIDTH, HEIGHT);
    }

    // --- RENDER ---
    render(dt);

    requestAnimationFrame(gameLoop);
}

// ----------- UPDATE -----------
function update(dt) {
    // Fondo animado (ya no actualizamos menuFadeTimer aquí, lo hace gameLoop)
    for (const c of bgCells) c.update(dt, menuFadeTimer, WIDTH, HEIGHT);

    // Cámara shake
    if (shakeTimer > 0) shakeTimer -= dt;

    // Bot o input manual
    if (autoPilot) {
        bot.update(player, quadtree, entities, dt, WIDTH, HEIGHT);
        if (player.invulnerableTimer > 0) player.invulnerableTimer -= dt;
        if (player.tripleShotTimer > 0) player.tripleShotTimer -= dt;
    } else {
        player.update(dt, keys, WIDTH, HEIGHT);

        // Disparo manual con mouse
        shootTimer += dt;
        if (mouse.down && shootTimer >= 0.12) {
            shootTimer = 0;
            const dx = mouse.x - player.bounds.x;
            const dy = mouse.y - player.bounds.y;
            entities.push(new Projectile(player.bounds.x, player.bounds.y, dx, dy));
            if (player.hasTripleShot()) {
                entities.push(new Projectile(player.bounds.x, player.bounds.y, dx + dy * 0.2, dy - dx * 0.2));
                entities.push(new Projectile(player.bounds.x, player.bounds.y, dx - dy * 0.2, dy + dx * 0.2));
            }
        }
    }

    // Spawner de enemigos
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0;
        let ex, ey;
        if (Math.random() < 0.5) { ex = (Math.random() < 0.5 ? 0 : WIDTH); ey = randomFloat(0, HEIGHT); }
        else { ex = randomFloat(0, WIDTH); ey = (Math.random() < 0.5 ? 0 : HEIGHT); }

        const prob = Math.random();
        let variant = EnemyVariant.NORMAL;
        if (prob < 0.20) variant = EnemyVariant.KAMIKAZE;
        else if (prob < 0.35) variant = EnemyVariant.TANK;

        entities.push(new Enemy(ex, ey, player.bounds.x, player.bounds.y, 0, false, variant));
    }

    // Boss spawner
    if (score >= nextBossScore) {
        nextBossScore += 500;
        entities.push(new Enemy(100, 100, player.bounds.x, player.bounds.y, 0, true));
        spawnInterval = Math.max(0.5, spawnInterval - 0.1);
    }

    // Actualizar entidades
    for (const e of entities) {
        if (!e.active) continue;

        if (e.type === EntityType.ENEMY || e.type === EntityType.BOSS) {
            e.setTarget(player.bounds.x, player.bounds.y);

            // Rastro de toxinas (5%)
            if (Math.random() < 0.05) {
                let toxinColor = { r: 50, g: 200, b: 50 };
                if (e.type === EntityType.BOSS || (e.type === EntityType.ENEMY && e.variant === EnemyVariant.TANK)) {
                    toxinColor = { r: 100, g: 30, b: 200 };
                }
                particles.push(new Particle(e.bounds.x, e.bounds.y, randomFloat(5, 15), randomFloat(0, 360), toxinColor));
            }
        }

        if (autoPilot && e === player) {
            // La actualización de timers ya se hizo arriba
        } else {
            if (e.type === EntityType.PROJECTILE) {
                e.update(dt, WIDTH, HEIGHT);
            } else if (e.type === EntityType.POWERUP) {
                e.update(dt);
            } else if (e.type !== EntityType.PLAYER || !autoPilot) {
                if (e.type === EntityType.ENEMY || e.type === EntityType.BOSS) {
                    e.update(dt);
                }
            }
        }
    }

    // Partículas y textos flotantes
    particles = particles.filter(p => p.update(dt));
    floatingTexts = floatingTexts.filter(t => { t.update(dt); return t.active; });

    // Quadtree
    if (useQuadtree) {
        quadtree.clear();
        for (const e of entities) {
            if (e.active) quadtree.insert(e);
        }
    }

    // Colisiones
    const bornEntities = [];
    collisionChecks = 0;

    for (const entity of entities) {
        if (!entity.active) continue;
        if (entity.type !== EntityType.PROJECTILE && entity.type !== EntityType.PLAYER) continue;

        let candidates = [];
        if (useQuadtree) {
            quadtree.query(entity.bounds, candidates);
        } else {
            candidates = entities.filter(e => e.active);
        }

        for (const other of candidates) {
            if (entity.id === other.id || !other.active) continue;
            collisionChecks++;

            if (!entity.bounds.intersects(other.bounds)) continue;

            // Caso A: Bala golpea Enemigo/Boss
            if (entity.type === EntityType.PROJECTILE &&
                (other.type === EntityType.ENEMY || other.type === EntityType.BOSS)) {
                entity.active = false;
                other.health--;
                spawnExplosion(other.bounds.x, other.bounds.y, { r: 255, g: 230, b: 0 }, 3);

                if (other.health <= 0) {
                    other.active = false;
                    bacteriaKilled++;

                    let pts = 0;
                    if (other.type === EntityType.BOSS) {
                        pts = 50;
                    } else if (other.variant === EnemyVariant.NORMAL) {
                        if (other.generation === 2) pts = 10;
                    } else {
                        pts = 10;
                    }

                    if (pts > 0) {
                        score += pts;
                        spawnFloatingText('+' + pts, other.bounds.x, other.bounds.y, '#ffee00');
                    }

                    spawnExplosion(other.bounds.x, other.bounds.y, { r: 255, g: 60, b: 60 }, 15);

                    // Mitosis
                    if (other.type === EntityType.ENEMY &&
                        other.variant === EnemyVariant.NORMAL &&
                        other.generation < 2) {
                        bornEntities.push(new Enemy(other.bounds.x - 10, other.bounds.y, player.bounds.x, player.bounds.y, other.generation + 1));
                        bornEntities.push(new Enemy(other.bounds.x + 10, other.bounds.y, player.bounds.x, player.bounds.y, other.generation + 1));
                    }

                    // Drop PowerUp 12%
                    if (Math.random() < 0.12) {
                        bornEntities.push(new PowerUp(other.bounds.x, other.bounds.y, Math.floor(Math.random() * 4)));
                    }
                }
            }

            // Caso B: Jugador recoge PowerUp
            else if (entity.type === EntityType.PLAYER && other.type === EntityType.POWERUP) {
                const puType = other.getPUType();
                let txt = '', col = '#fff';
                if (puType === PowerUpType.HEAL) {
                    player.health = Math.min(player.maxHealth, player.health + 30);
                    txt = 'HEALTH UP!'; col = '#00ff66';
                } else if (puType === PowerUpType.SHIELD) {
                    player.invulnerableTimer = 5;
                    txt = 'SHIELD!'; col = '#44aaff';
                } else if (puType === PowerUpType.TRIPLE) {
                    player.tripleShotTimer = 8;
                    txt = 'TRIPLE SHOT!'; col = '#ff44ff';
                } else if (puType === PowerUpType.BOMB) {
                    txt = 'BOOM!'; col = '#ff3300';
                    spawnExplosion(player.bounds.x, player.bounds.y, { r: 255, g: 255, b: 255 }, 50);
                    shakeTimer = 0.5;
                    for (const e of entities) {
                        if (e.type === EntityType.ENEMY && e.active) {
                            e.active = false;
                            spawnExplosion(e.bounds.x, e.bounds.y, { r: 255, g: 100, b: 0 }, 10);
                            score += 10;
                        }
                    }
                }
                spawnFloatingText(txt, player.bounds.x, player.bounds.y - 30, col);
                other.active = false;
            }

            // Caso C: Jugador choca con Enemigo
            else if (entity.type === EntityType.PLAYER &&
                (other.type === EntityType.ENEMY || other.type === EntityType.BOSS)) {
                if (!player.isInvulnerable()) {
                    player.takeDamage(10);
                    shakeTimer = 0.3;
                    damageFlashTimer = 0.25;
                    spawnExplosion(player.bounds.x, player.bounds.y, { r: 255, g: 30, b: 30 }, 10);
                    if (player.health <= 0) {
                        state = GameState.GAME_OVER;
                    }
                }
            }
        }
    }

    // Agregar entidades nacidas
    for (const e of bornEntities) entities.push(e);

    // Garbage collection
    entities = entities.filter(e => e.active);
}

// ----------- RENDER -----------
function render(dt) {
    // Shake transform
    ctx.save();
    if (shakeTimer > 0) {
        ctx.translate(randomFloat(-5, 5), randomFloat(-5, 5));
    }

    // Fondo pulsante biológico
    const heartbeat = (Math.sin(menuFadeTimer * 2) + 1) / 2;
    let bgR, bgG, bgB;
    if (damageFlashTimer > 0) {
        bgR = 120; bgG = 0; bgB = 0;
    } else {
        bgR = Math.floor(25 + heartbeat * 15); bgG = 5; bgB = 5;
    }
    ctx.fillStyle = `rgb(${bgR},${bgG},${bgB})`;
    ctx.fillRect(0, 0, WIDTH, HEIGHT);

    // Células de fondo
    for (const c of bgCells) c.render(ctx);

    if (state === GameState.MENU) {
        renderMenu();
    } else if (state === GameState.PLAYING) {
        renderGame();
    } else if (state === GameState.GAME_OVER) {
        renderGameOver();
    }

    ctx.restore();
}

function renderMenu() {
    // Fade-in del título
    const titleAlpha = Math.min(1, menuFadeTimer / 2);
    const blink = (Math.sin(menuFadeTimer * 5) + 1) / 2;
    const instAlpha = (blink * 0.8 + 0.2) * titleAlpha;

    // Glow del título
    ctx.save();
    ctx.shadowColor = '#00ffff';
    ctx.shadowBlur = 40;

    ctx.globalAlpha = titleAlpha;
    ctx.fillStyle = '#00ffff';
    ctx.font = 'bold 90px "Segoe UI", "Arial", sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText('CELLULAR DEFENSE', WIDTH / 2, HEIGHT / 2 - 100);

    ctx.shadowBlur = 0;
    ctx.globalAlpha = instAlpha;
    ctx.fillStyle = '#ffffff';
    ctx.font = '44px "Segoe UI", sans-serif';
    ctx.fillText('Presiona ENTER para Iniciar', WIDTH / 2, HEIGHT / 2 + 50);

    ctx.globalAlpha = titleAlpha * 0.7;
    ctx.font = '24px "Segoe UI", sans-serif';
    ctx.fillStyle = '#aaaaaa';
    ctx.fillText('WASD: Mover  |  Click: Disparar  |  P: Bot  |  B: Debug  |  SPACE: Toggle Quadtree', WIDTH / 2, HEIGHT / 2 + 130);

    ctx.restore();
}

function renderGame() {
    // Debug Quadtree
    if (showDebug && useQuadtree) {
        quadtree.debugRender(ctx);

        // Caja de query del jugador
        const qb = player.bounds;
        ctx.strokeStyle = 'rgba(0, 255, 0, 0.7)';
        ctx.lineWidth = 2;
        ctx.strokeRect(qb.x - qb.halfW, qb.y - qb.halfH, qb.halfW * 2, qb.halfH * 2);
    }

    // Entidades
    for (const e of entities) e.render(ctx);
    for (const p of particles) p.render(ctx);
    for (const t of floatingTexts) t.render(ctx);

    // HUD
    renderHUD();
}

function renderHUD() {
    const pct = player.health / player.maxHealth;

    // Barra de vida (esquina sup-derecha)
    const bx = WIDTH - 330, by = 28, bw = 300, bh = 22;
    ctx.fillStyle = 'rgba(30,30,30,0.8)';
    ctx.beginPath();
    ctx.roundRect(bx - 4, by - 4, bw + 8, bh + 8, 8);
    ctx.fill();

    ctx.fillStyle = pct > 0.5 ? '#22ee44' : pct > 0.25 ? '#ffcc00' : '#ff3322';
    ctx.beginPath();
    ctx.roundRect(bx, by, bw * pct, bh, 6);
    ctx.fill();

    ctx.strokeStyle = '#ffffff44';
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    ctx.roundRect(bx, by, bw, bh, 6);
    ctx.stroke();

    ctx.fillStyle = '#fff';
    ctx.font = 'bold 14px "Segoe UI", sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(`HP: ${player.health} / ${player.maxHealth}`, bx + bw / 2, by + bh / 2);

    // Score
    ctx.textAlign = 'left';
    ctx.textBaseline = 'top';
    ctx.font = 'bold 38px "Segoe UI", sans-serif';
    ctx.fillStyle = '#ffffff';
    ctx.shadowColor = '#00ffff';
    ctx.shadowBlur = 10;
    ctx.fillText(`Puntaje: ${score}`, 30, 28);
    ctx.shadowBlur = 0;

    // Debug info
    if (showDebug) {
        ctx.fillStyle = 'rgba(0,0,0,0.55)';
        ctx.fillRect(25, 80, 320, 90);
        ctx.fillStyle = '#aaffaa';
        ctx.font = '20px monospace';
        ctx.fillText(`Entidades: ${entities.length}`, 35, 90);
        ctx.fillText(`Checks Colisión: ${collisionChecks}`, 35, 114);
        ctx.fillText(`Quadtree: ${useQuadtree ? 'ON' : 'OFF (fuerza bruta)'}`, 35, 138);
    }

    // Indicadores de PowerUp activos
    if (player.hasTripleShot()) renderPUIndicator('TRIPLE SHOT ✦', '#ff44ff', 0);
    if (player.isInvulnerable()) renderPUIndicator('SHIELD 🛡', '#44aaff', 1);

    // Bot indicator
    if (autoPilot) {
        ctx.fillStyle = 'rgba(0,255,255,0.15)';
        ctx.beginPath();
        ctx.roundRect(WIDTH / 2 - 80, HEIGHT - 60, 160, 36, 10);
        ctx.fill();
        ctx.fillStyle = '#0ff';
        ctx.font = 'bold 20px "Segoe UI", sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('🤖 BOT ACTIVO', WIDTH / 2, HEIGHT - 42);
    }
}

function renderPUIndicator(label, color, index) {
    const x = WIDTH - 330 + (index * 180);
    const y = 62;
    ctx.fillStyle = color + '33';
    ctx.beginPath();
    ctx.roundRect(x, y, 170, 28, 7);
    ctx.fill();
    ctx.fillStyle = color;
    ctx.font = 'bold 16px "Segoe UI", sans-serif';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(label, x + 85, y + 14);
}

function renderGameOver() {
    // Overlay oscuro
    ctx.fillStyle = 'rgba(0,0,0,0.65)';
    ctx.fillRect(0, 0, WIDTH, HEIGHT);

    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';

    // GAME OVER
    ctx.shadowColor = '#ff0000';
    ctx.shadowBlur = 50;
    ctx.fillStyle = '#ff3333';
    ctx.font = 'bold 110px "Segoe UI", sans-serif';
    ctx.fillText('GAME OVER', WIDTH / 2, HEIGHT / 2 - 80);
    ctx.shadowBlur = 0;

    // Stats
    ctx.fillStyle = '#ffffff';
    ctx.font = '34px "Segoe UI", sans-serif';
    ctx.fillText(`Bacterias eliminadas: ${bacteriaKilled}`, WIDTH / 2, HEIGHT / 2 + 50);
    ctx.fillText(`Puntaje Total: ${score}`, WIDTH / 2, HEIGHT / 2 + 100);

    // Reiniciar
    const blink = (Math.sin(menuFadeTimer * 6) + 1) / 2;
    ctx.globalAlpha = 0.5 + blink * 0.5;
    ctx.fillStyle = '#ffffff';
    ctx.font = '44px "Segoe UI", sans-serif';
    ctx.fillText("Presiona 'R' para Reiniciar", WIDTH / 2, HEIGHT / 2 + 175);
    ctx.globalAlpha = 1;
}

// ----------- ARRANCAR -----------
initGame();
requestAnimationFrame(ts => { lastTime = ts; requestAnimationFrame(gameLoop); });
