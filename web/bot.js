// ============================================================
//  bot.js – Puerto del BotController de main.cpp
// ============================================================

class BotController {
    constructor() {
        this.perceptionRadius = 450;
        this.safeDistance = 250;
        this.speed = 500;
        this.timeSinceLastShot = 0;
        this.fireRate = 0.15;
    }

    update(player, quadtree, entities, dt, WIDTH, HEIGHT) {
        const px = player.bounds.x;
        const py = player.bounds.y;
        const pr = this.perceptionRadius;

        const perceptionBox = new AABB(px, py, pr, pr);
        const threats = [];
        quadtree.query(perceptionBox, threats);

        let moveX = 0, moveY = 0;
        let threatCount = 0;
        let nearestTarget = null;
        let minDistSq = 1e8;

        for (const e of threats) {
            if (!e.active || e.id === player.id) continue;

            const dx = px - e.bounds.x;
            const dy = py - e.bounds.y;
            const distSq = dx * dx + dy * dy;

            if (e.type === EntityType.ENEMY || e.type === EntityType.BOSS) {
                if (distSq < pr * pr) {
                    const dist = Math.sqrt(distSq);
                    const strength = (pr - dist) / pr;
                    moveX += (dx / dist) * strength * 8;
                    moveY += (dy / dist) * strength * 8;
                    threatCount++;
                }

                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    nearestTarget = e;
                }
            } else if (e.type === EntityType.POWERUP) {
                const dist = Math.sqrt(distSq);
                const attraction = (player.health < 50) ? 4 : 1.5;
                moveX -= (dx / dist) * attraction;
                moveY -= (dy / dist) * attraction;
            }
        }

        // Mantener alejado de los bordes
        const margin = 150;
        if (px < margin) moveX += 3 * ((margin - px) / margin);
        if (px > WIDTH - margin) moveX -= 3 * ((px - (WIDTH - margin)) / margin);
        if (py < margin) moveY += 3 * ((margin - py) / margin);
        if (py > HEIGHT - margin) moveY -= 3 * ((py - (HEIGHT - margin)) / margin);

        // Volver al centro si no hay amenazas
        if (threatCount === 0) {
            moveX += (WIDTH / 2 - px) * 0.001;
            moveY += (HEIGHT / 2 - py) * 0.001;
        }

        const length = Math.sqrt(moveX * moveX + moveY * moveY);
        if (length > 0.05) {
            const inputMag = Math.min(length, 1);
            const dirX = moveX / length;
            const dirY = moveY / length;
            player.bounds.x += dirX * inputMag * this.speed * dt;
            player.bounds.y += dirY * inputMag * this.speed * dt;
        }

        // Clamp dentro del mapa
        const r = player.bounds.halfW;
        player.bounds.x = Math.max(r, Math.min(WIDTH - r, player.bounds.x));
        player.bounds.y = Math.max(r, Math.min(HEIGHT - r, player.bounds.y));

        // Disparo
        this.timeSinceLastShot += dt;
        const panicMode = (threatCount > 15);
        let shouldShoot = true;

        if (panicMode && nearestTarget && nearestTarget.type === EntityType.ENEMY) {
            if (nearestTarget.variant === EnemyVariant.NORMAL && !player.hasTripleShot()) {
                shouldShoot = false;
            }
        }

        if (nearestTarget && this.timeSinceLastShot >= this.fireRate && shouldShoot) {
            this.timeSinceLastShot = 0;
            const dx = nearestTarget.bounds.x - px;
            const dy = nearestTarget.bounds.y - py;

            entities.push(new Projectile(px, py, dx, dy));
            if (player.hasTripleShot()) {
                entities.push(new Projectile(px, py, dx + dy * 0.2, dy - dx * 0.2));
                entities.push(new Projectile(px, py, dx - dy * 0.2, dy + dx * 0.2));
            }
        }
    }
}
