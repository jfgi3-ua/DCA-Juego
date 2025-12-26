#pragma once
extern "C" {
  #include <raylib.h>
}

struct PlayerStateComponent {
    Vector2 lastTilePos;
    float invulnerableTimer;
    float invulnerableDuration;

    PlayerStateComponent(Vector2 startPos = {0.0f, 0.0f}, float duration = 1.5f)
        : lastTilePos(startPos), invulnerableTimer(0.0f), invulnerableDuration(duration) {}
};