#pragma once
extern "C" {
  #include <raylib.h>
}

// Etiqueta para el jugador
struct PlayerInputComponent {};

struct PlayerCheatComponent {
    bool godMode;
    bool noClip;

    PlayerCheatComponent(bool god = false, bool noclip = false)
        : godMode(god), noClip(noclip) {}
};

struct PlayerStateComponent {
    Vector2 lastTilePos;
    float invulnerableTimer;
    float invulnerableDuration;

    PlayerStateComponent(Vector2 startPos = {0.0f, 0.0f}, float duration = 1.5f)
        : lastTilePos(startPos), invulnerableTimer(0.0f), invulnerableDuration(duration) {}
};

// Componente de Estadisticas (jugador)
struct StatsComponent {
    int lives;
    int keysCollected;

    StatsComponent(int l = 5) : lives(l), keysCollected(0) {}
};
