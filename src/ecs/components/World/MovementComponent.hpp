#pragma once
extern "C" {
  #include <raylib.h>
}

// Logica de movimiento por casillas
struct MovementComponent {
    float speed;
    bool isMoving;

    Vector2 startPos;
    Vector2 targetPos;

    float progress;
    float duration;

    MovementComponent(float spd = 150.0f)
        : speed(spd), isMoving(false), startPos{0,0}, targetPos{0,0}, progress(0.0f), duration(0.12f) {}
};