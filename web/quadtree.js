// ============================================================
//  quadtree.js – Puerto de Quadtree.cpp y DataStructures.h
// ============================================================

class AABB {
    constructor(x, y, halfW, halfH) {
        this.x = x;
        this.y = y;
        this.halfW = halfW;
        this.halfH = halfH;
    }

    intersects(other) {
        if (Math.abs(this.x - other.x) > (this.halfW + other.halfW)) return false;
        if (Math.abs(this.y - other.y) > (this.halfH + other.halfH)) return false;
        return true;
    }

    contains(other) {
        return (
            other.x - other.halfW >= this.x - this.halfW &&
            other.x + other.halfW <= this.x + this.halfW &&
            other.y - other.halfH >= this.y - this.halfH &&
            other.y + other.halfH <= this.y + this.halfH
        );
    }
}

const QT_CAPACITY = 4;
const QT_MAX_DEPTH = 6;

class Quadtree {
    constructor(bounds, level = 0) {
        this.boundary = bounds;
        this.level = level;
        this.divided = false;
        this.objects = [];
        this.children = [null, null, null, null];
    }

    clear() {
        this.objects = [];
        if (this.divided) {
            for (let i = 0; i < 4; i++) {
                if (this.children[i]) {
                    this.children[i].clear();
                    this.children[i] = null;
                }
            }
            this.divided = false;
        }
    }

    subdivide() {
        const x = this.boundary.x;
        const y = this.boundary.y;
        const qHW = this.boundary.halfW / 2;
        const qHH = this.boundary.halfH / 2;

        this.children[0] = new Quadtree(new AABB(x - qHW, y - qHH, qHW, qHH), this.level + 1); // NW
        this.children[1] = new Quadtree(new AABB(x + qHW, y - qHH, qHW, qHH), this.level + 1); // NE
        this.children[2] = new Quadtree(new AABB(x - qHW, y + qHH, qHW, qHH), this.level + 1); // SW
        this.children[3] = new Quadtree(new AABB(x + qHW, y + qHH, qHW, qHH), this.level + 1); // SE
        this.divided = true;
    }

    _getIndex(itemBounds) {
        const vMid = this.boundary.x;
        const hMid = this.boundary.y;

        const top    = (itemBounds.y + itemBounds.halfH) < hMid;
        const bottom = (itemBounds.y - itemBounds.halfH) > hMid;
        const left   = (itemBounds.x + itemBounds.halfW) < vMid;
        const right  = (itemBounds.x - itemBounds.halfW) > vMid;

        if (left) {
            if (top)    return 0;
            if (bottom) return 2;
        } else if (right) {
            if (top)    return 1;
            if (bottom) return 3;
        }
        return -1;
    }

    insert(entity) {
        if (!this.boundary.intersects(entity.bounds)) return false;

        if (this.divided) {
            const idx = this._getIndex(entity.bounds);
            if (idx !== -1) return this.children[idx].insert(entity);
        }

        this.objects.push(entity);

        if (this.objects.length > QT_CAPACITY && this.level < QT_MAX_DEPTH) {
            if (!this.divided) this.subdivide();

            this.objects = this.objects.filter(obj => {
                const idx = this._getIndex(obj.bounds);
                if (idx !== -1) {
                    this.children[idx].insert(obj);
                    return false;
                }
                return true;
            });
        }
        return true;
    }

    query(range, found) {
        if (!this.boundary.intersects(range)) return;

        for (const obj of this.objects) {
            if (obj.active && range.intersects(obj.bounds)) {
                found.push(obj);
            }
        }

        if (this.divided) {
            for (let i = 0; i < 4; i++) {
                this.children[i].query(range, found);
            }
        }
    }

    debugRender(ctx) {
        const b = this.boundary;
        const intensity = Math.min(255, 50 + this.level * 25);
        ctx.strokeStyle = `rgba(0, ${intensity}, 255, 0.6)`;
        ctx.lineWidth = Math.max(0.3, 1 / (this.level + 1));
        ctx.strokeRect(b.x - b.halfW, b.y - b.halfH, b.halfW * 2, b.halfH * 2);

        if (this.divided) {
            for (let i = 0; i < 4; i++) {
                this.children[i].debugRender(ctx);
            }
        }
    }
}
