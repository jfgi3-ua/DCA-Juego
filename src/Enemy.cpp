#include "Enemy.hpp"
extern "C" {
  #include <raylib.h>
}
#include <algorithm>
#include <cmath>

static void shuffledNeighbors(int out[4]) {
    out[0]=0; out[1]=1; out[2]=2; out[3]=3;
    for (int i = 3; i > 0; --i) {
        int j = GetRandomValue(0, i);
        std::swap(out[i], out[j]);
    }
}

void Enemy::update(const Map &map, float dt, int tileSize)
{
    // timer opcional si quieres retardo entre decisiones
    timer += dt;
    //if (moveCooldown > 0.0f && timer < moveCooldown) {
    //}

    if (!moving && (moveCooldown == 0.0f || timer >= moveCooldown)) {
        const int dx[4] = {0, 0, 1, -1};
        const int dy[4] = {1, -1, 0, 0};

        int order[4];
        shuffledNeighbors(order);

        bool found = false;
        for (int k = 0; k < 4; ++k) {
            int i = order[k];
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (map.isWalkable(nx, ny)) {
                
                targetX = nx;
                targetY = ny;
                moving = true;
                timer = 0.0f; 
                found = true;
                break;
            }
        }
        if (!found) {
            moving = false;
        }
    }

    if (moving) {
        float targetPx = targetX * tileSize + tileSize * 0.5f;
        float targetPy = targetY * tileSize + tileSize * 0.5f;

        float dx = targetPx - px;
        float dy = targetPy - py;
        float dist = std::sqrt(dx*dx + dy*dy);

        if (dist <= 0.0001f) {
            px = targetPx;
            py = targetPy;
            x = targetX;
            y = targetY;
            moving = false;
            timer = 0.0f;
            return;
        }

        float step = speed * dt;
        if (step >= dist) {
            px = targetPx;
            py = targetPy;
            x = targetX;
            y = targetY;
            moving = false;
            timer = 0.0f;
        } else {
            px += dx / dist * step;
            py += dy / dist * step;
        }
    }
}

void Enemy::draw(int tileSize, Color color) const
{
    float s = tileSize * 0.7f;
    Rectangle r{ px - s/2.0f, py - s/2.0f, s, s };
    DrawRectangleRec(r, color);

}
