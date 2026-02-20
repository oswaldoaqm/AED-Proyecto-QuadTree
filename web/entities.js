// ============================================================
//  entities.js – Puerto de Player, Enemy, Projectile, PowerUp,
//                Particle y FloatingText
// ============================================================

let _idCounter = 1;
function nextId() { return _idCounter++; }
function resetIdCounter() { _idCounter = 1; }

// ---- Tipos / Enums ----------------------------------------
const EntityType = { PLAYER: 'PLAYER', ENEMY: 'ENEMY', BOSS: 'BOSS', PROJECTILE: 'PROJECTILE', POWERUP: 'POWERUP' };
const EnemyVariant = { NORMAL: 'NORMAL', KAMIKAZE: 'KAMIKAZE', TANK: 'TANK' };
const PowerUpType = { HEAL: 0, SHIELD: 1, TRIPLE: 2, BOMB: 3 };

// ---- Helpers color ----------------------------------------
function rgba(r, g, b, a = 1) { return `rgba(${r},${g},${b},${a})`; }
function randomFloat(min, max) { return min + Math.random() * (max - min); }

// ===========================================================
//  PLAYER
// ===========================================================
class Player {
    constructor(x, y) {
        this.id = 0;
        this.type = EntityType.PLAYER;
        this.active = true;
        this.bounds = new AABB(x, y, 20, 20);
        this.speed = 500;
        this.health = 100;
        this.maxHealth = 100;
        this.invulnerableTimer = 0;
        this.tripleShotTimer = 0;
        this.rotation = 0;
        this.glowPhase = 0;
    }

    isInvulnerable() { return this.invulnerableTimer > 0; }
    hasTripleShot() { return this.tripleShotTimer > 0; }

    takeDamage(amount) {
        if (this.isInvulnerable() || !this.active) return;
        this.health -= amount;
        if (this.health <= 0) { this.health = 0; this.active = false; }
        this.invulnerableTimer = 1.0;
    }

    update(dt, keys, WIDTH, HEIGHT) {
        if (this.invulnerableTimer > 0) this.invulnerableTimer -= dt;
        if (this.tripleShotTimer > 0) this.tripleShotTimer -= dt;

        let dx = 0, dy = 0;
        if (keys['w'] || keys['arrowup']) dy -= 1;
        if (keys['s'] || keys['arrowdown']) dy += 1;
        if (keys['a'] || keys['arrowleft']) dx -= 1;
        if (keys['d'] || keys['arrowright']) dx += 1;

        if (dx !== 0 || dy !== 0) {
            const len = Math.sqrt(dx * dx + dy * dy);
            this.bounds.x += (dx / len) * this.speed * dt;
            this.bounds.y += (dy / len) * this.speed * dt;
        }

        const r = this.bounds.halfW;
        this.bounds.x = Math.max(r, Math.min(WIDTH - r, this.bounds.x));
        this.bounds.y = Math.max(r, Math.min(HEIGHT - r, this.bounds.y));

        this.glowPhase += dt * 3;
    }

    render(ctx) {
        if (!this.active) return;
        const { x, y, halfW } = this.bounds;
        const r = halfW;

        // Glow externo
        const glowAlpha = this.isInvulnerable() ? 0.6 + Math.sin(this.invulnerableTimer * 20) * 0.4 : 0.25 + Math.sin(this.glowPhase) * 0.1;
        const grd = ctx.createRadialGradient(x, y, r * 0.4, x, y, r * 2.2);
        grd.addColorStop(0, `rgba(180, 200, 255, ${glowAlpha})`);
        grd.addColorStop(1, 'rgba(180, 200, 255, 0)');
        ctx.fillStyle = grd;
        ctx.beginPath();
        ctx.arc(x, y, r * 2.2, 0, Math.PI * 2);
        ctx.fill();

        // Cuerpo con parpadeo si invulnerable
        const blink = this.isInvulnerable() ? (Math.floor(this.invulnerableTimer * 15) % 2 === 0 ? 0.45 : 1.0) : 1.0;
        ctx.globalAlpha = blink;

        ctx.fillStyle = rgba(245, 245, 255, 0.9);
        ctx.beginPath();
        ctx.arc(x, y, r, 0, Math.PI * 2);
        ctx.fill();

        ctx.strokeStyle = rgba(180, 200, 255, 0.9);
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(x, y, r, 0, Math.PI * 2);
        ctx.stroke();

        ctx.globalAlpha = 1;
    }

    getPosition() { return { x: this.bounds.x, y: this.bounds.y }; }
}

// ===========================================================
//  ENEMY
// ===========================================================
class Enemy {
    constructor(x, y, tx, ty, generation = 0, isBoss = false, variant = EnemyVariant.NORMAL) {
        this.id = nextId();
        this.type = isBoss ? EntityType.BOSS : EntityType.ENEMY;
        this.active = true;
        this.variant = variant;
        this.generation = generation;
        this.targetX = tx;
        this.targetY = ty;
        this.rotation = 0;

        let size = 40;
        if (isBoss) {
            size = 150;
            this.speed = 50;
            this.health = 50;
            this.color = rgba(128, 0, 128, 0.9);
            this.outlineColor = rgba(200, 0, 255, 0.9);
            this.rotSpeed = 30;
        } else {
            switch (variant) {
                case EnemyVariant.KAMIKAZE:
                    size = 25; this.speed = 350; this.health = 2;
                    this.color = rgba(255, 50, 50, 0.85);
                    this.outlineColor = rgba(255, 150, 150, 0.9);
                    this.rotSpeed = 360;
                    break;
                case EnemyVariant.TANK:
                    size = 60; this.speed = 80; this.health = 20;
                    this.color = rgba(100, 0, 150, 0.9);
                    this.outlineColor = rgba(200, 50, 255, 0.9);
                    this.rotSpeed = 90;
                    break;
                default: // NORMAL
                    size = 40 / (generation + 1);
                    this.speed = (120 + randomFloat(0, 80)) * (1 + generation * 0.4);
                    this.health = generation === 0 ? 4 : (generation === 1 ? 2 : 1);
                    if (generation === 0) this.color = rgba(50, 200, 50, 0.75);
                    else if (generation === 1) this.color = rgba(100, 255, 100, 0.85);
                    else this.color = rgba(180, 255, 180, 1.0);
                    this.outlineColor = rgba(0, 255, 100, 0.6);
                    this.rotSpeed = 90;
                    break;
            }
        }

        this.maxHealth = this.health;
        this.size = size;
        this.bounds = new AABB(x, y, size / 2, size / 2);
    }

    setTarget(tx, ty) { this.targetX = tx; this.targetY = ty; }

    update(dt) {
        const dx = this.targetX - this.bounds.x;
        const dy = this.targetY - this.bounds.y;
        const dist = Math.sqrt(dx * dx + dy * dy);

        if (dist > 0.5) {
            const moveDist = this.speed * dt;
            const actual = Math.min(moveDist, dist);
            this.bounds.x += (dx / dist) * actual;
            this.bounds.y += (dy / dist) * actual;
        }

        this.rotation += this.rotSpeed * dt;
    }

    render(ctx) {
        if (!this.active) return;
        const { x, y } = this.bounds;
        const s = this.size;

        ctx.save();
        ctx.translate(x, y);
        ctx.rotate((this.rotation * Math.PI) / 180);

        // Cuerpo
        ctx.fillStyle = this.color;
        ctx.beginPath();
        ctx.roundRect(-s / 2, -s / 2, s, s, s * 0.18);
        ctx.fill();

        // Borde
        ctx.strokeStyle = this.outlineColor;
        ctx.lineWidth = this.type === EntityType.BOSS ? 4 : 2;
        ctx.beginPath();
        ctx.roundRect(-s / 2, -s / 2, s, s, s * 0.18);
        ctx.stroke();

        // Barra de salud (si tiene más de 1 HP)
        if (this.maxHealth > 1) {
            const barW = s;
            const pct = this.health / this.maxHealth;
            ctx.fillStyle = 'rgba(60,60,60,0.7)';
            ctx.fillRect(-barW / 2, s / 2 + 4, barW, 5);
            ctx.fillStyle = pct > 0.5 ? rgba(50, 220, 50, 0.9) : pct > 0.25 ? rgba(255, 200, 0, 0.9) : rgba(255, 50, 50, 0.9);
            ctx.fillRect(-barW / 2, s / 2 + 4, barW * pct, 5);
        }

        ctx.restore();
    }
}

// ===========================================================
//  PROJECTILE
// ===========================================================
class Projectile {
    constructor(x, y, dirX, dirY) {
        this.id = nextId();
        this.type = EntityType.PROJECTILE;
        this.active = true;
        this.bounds = new AABB(x, y, 8, 8);
        const speed = 750;
        const len = Math.sqrt(dirX * dirX + dirY * dirY) || 1;
        this.vx = (dirX / len) * speed;
        this.vy = (dirY / len) * speed;
        this.trailPoints = [];
    }

    update(dt, WIDTH, HEIGHT) {
        if (!this.active) return;
        this.trailPoints.unshift({ x: this.bounds.x, y: this.bounds.y });
        if (this.trailPoints.length > 6) this.trailPoints.pop();

        this.bounds.x += this.vx * dt;
        this.bounds.y += this.vy * dt;

        const m = 60;
        if (this.bounds.x < -m || this.bounds.x > WIDTH + m ||
            this.bounds.y < -m || this.bounds.y > HEIGHT + m) {
            this.active = false;
        }
    }

    render(ctx) {
        if (!this.active) return;
        const { x, y } = this.bounds;

        // Estela
        for (let i = 0; i < this.trailPoints.length; i++) {
            const t = this.trailPoints[i];
            const alpha = (1 - i / this.trailPoints.length) * 0.4;
            const radius = (8 - i) * 0.7;
            ctx.fillStyle = `rgba(255, 230, 0, ${alpha})`;
            ctx.beginPath();
            ctx.arc(t.x, t.y, Math.max(1, radius), 0, Math.PI * 2);
            ctx.fill();
        }

        // Bola principal
        const grd = ctx.createRadialGradient(x, y, 0, x, y, 8);
        grd.addColorStop(0, 'rgba(255, 255, 200, 1)');
        grd.addColorStop(1, 'rgba(255, 200, 0, 0.8)');
        ctx.fillStyle = grd;
        ctx.beginPath();
        ctx.arc(x, y, 8, 0, Math.PI * 2);
        ctx.fill();
    }
}

// ===========================================================
//  POWERUP
// ===========================================================
class PowerUp {
    constructor(x, y, puType) {
        this.id = nextId();
        this.type = EntityType.POWERUP;
        this.active = true;
        this.puType = puType;
        this.bounds = new AABB(x, y, 18, 18);
        this.lifetime = 10.0;
        this.rotation = 0;
        this.phase = 0;

        switch (puType) {
            case PowerUpType.HEAL: this.color = '#00ee44'; this.label = '❤'; break;
            case PowerUpType.SHIELD: this.color = '#44aaff'; this.label = '🛡'; break;
            case PowerUpType.TRIPLE: this.color = '#ff44ff'; this.label = '✦'; break;
            case PowerUpType.BOMB: this.color = '#aa00ff'; this.label = '💥'; break;
        }
    }

    getPUType() { return this.puType; }

    update(dt) {
        if (!this.active) return;
        this.lifetime -= dt;
        if (this.lifetime <= 0) { this.active = false; return; }
        this.rotation += 45 * dt;
        this.phase += dt * 5;
    }

    render(ctx) {
        if (!this.active) return;
        const { x, y } = this.bounds;
        const scale = 1 + Math.sin(this.phase) * 0.12;
        const r = 15 * scale;
        const fade = Math.min(1, this.lifetime);

        ctx.save();
        ctx.globalAlpha = fade;
        ctx.translate(x, y);
        ctx.rotate((this.rotation * Math.PI) / 180);

        // Glow
        const grd = ctx.createRadialGradient(0, 0, 2, 0, 0, r * 2.2);
        grd.addColorStop(0, this.color + 'aa');
        grd.addColorStop(1, this.color + '00');
        ctx.fillStyle = grd;
        ctx.beginPath();
        ctx.arc(0, 0, r * 2.2, 0, Math.PI * 2);
        ctx.fill();

        // Cuerpo
        ctx.fillStyle = this.color;
        ctx.strokeStyle = '#ffffff88';
        ctx.lineWidth = 2.5;
        ctx.beginPath();
        ctx.arc(0, 0, r, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();

        // Icono
        ctx.rotate(-(this.rotation * Math.PI) / 180);
        ctx.font = `${Math.round(r * 0.95)}px serif`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(this.label, 0, 1);

        ctx.globalAlpha = 1;
        ctx.restore();
    }
}

// ===========================================================
//  PARTICLE
// ===========================================================
class Particle {
    constructor(x, y, speed, angleDeg, color) {
        this.x = x; this.y = y;
        const rad = (angleDeg * Math.PI) / 180;
        this.vx = Math.cos(rad) * speed;
        this.vy = Math.sin(rad) * speed;
        this.life = 1.0;
        this.decay = randomFloat(1.2, 2.5);
        this.radius = randomFloat(2.5, 5.5);
        this.color = color; // { r, g, b }
    }

    update(dt) {
        this.x += this.vx * dt;
        this.y += this.vy * dt;
        this.vx *= 0.95;
        this.vy *= 0.95;
        this.life -= this.decay * dt;
        return this.life > 0;
    }

    render(ctx) {
        if (this.life <= 0) return;
        const { r, g, b } = this.color;
        ctx.fillStyle = `rgba(${r},${g},${b},${this.life.toFixed(2)})`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius * this.life, 0, Math.PI * 2);
        ctx.fill();
    }
}

// ===========================================================
//  FLOATING TEXT
// ===========================================================
class FloatingText {
    constructor(text, x, y, color) {
        this.text = text;
        this.x = x; this.y = y;
        this.vy = -60;
        this.life = 1.0;
        this.decay = 1.0;
        this.active = true;
        this.color = color; // 'rgba(...)' string
    }

    update(dt) {
        this.y += this.vy * dt;
        this.life -= this.decay * dt;
        if (this.life <= 0) { this.active = false; }
    }

    render(ctx) {
        if (!this.active) return;
        ctx.save();
        ctx.globalAlpha = Math.max(0, this.life);
        ctx.font = 'bold 22px "Segoe UI", sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = this.color;
        ctx.shadowColor = this.color;
        ctx.shadowBlur = 8;
        ctx.fillText(this.text, this.x, this.y);
        ctx.restore();
    }
}

// ===========================================================
//  BACKGROUND CELL (ambiente biológico)
// ===========================================================
class BackgroundCell {
    constructor(WIDTH, HEIGHT) {
        this.reset(WIDTH, HEIGHT, true);
    }

    reset(WIDTH, HEIGHT, initial = false) {
        this.x = randomFloat(0, WIDTH);
        this.y = initial ? randomFloat(0, HEIGHT) : -50;
        this.radius = randomFloat(10, 30);
        this.speed = randomFloat(30, 80);
        this.amplitude = randomFloat(0.5, 1.5);
        this.offset = randomFloat(0, Math.PI * 2);
        this.alpha = randomFloat(0.05, 0.18);
    }

    update(dt, t, WIDTH, HEIGHT) {
        this.y += this.speed * dt;
        this.x += Math.sin(t + this.offset) * this.amplitude;
        if (this.y > HEIGHT + 60) this.reset(WIDTH, HEIGHT);
    }

    render(ctx) {
        ctx.fillStyle = `rgba(150, 20, 40, ${this.alpha})`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
    }
}
