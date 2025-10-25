#pragma once
#include "Map.hpp"
extern "C" {
  #include <raylib.h>
}

class Enemy {
public:
    int x = 0;
    int y = 0;
    int targetX = 0;
    int targetY = 0;
    float px = 0.0f;
    float py = 0.0f;
    float speed = 0.0f;
    float moveCooldown = 0.0f;
    float timer = 0.0f;
    bool moving = false;

    float bboxW = 0.0f;
    float bboxH = 0.0f;

    bool collidesWithPlayer(float playerPx, float playerPy, float playerRadius) const;


    Enemy() = default;
    Enemy(int x_, int y_, int tileSize) : x(x_), y(y_), targetX(x_), targetY(y_) {
        px = x * tileSize + tileSize * 0.5f;
        py = y * tileSize + tileSize * 0.5f;

        speed = tileSize * 1.5f;
        moveCooldown = 0.0f;
        timer = 0.0f;
        moving = false;
        bboxW = tileSize * 0.7f;
        bboxH = tileSize * 0.7f;
    }

    void update(const Map &map, float dt, int tileSize);

    // void draw(int tileSize, Color color = RED) const; // método de dibujado sin offset --- IGNORE ---
    void draw(int tileSize, int ox, int oy, Color color = RED) const; // método de dibujado con offset (ox, oy) ¡¡Hugh!! Este es el que hay que usar ahora
};