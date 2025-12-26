#pragma once
extern "C" {
  #include <raylib.h>
}

// Componente de Estadisticas (jugador)
struct PlayerStatsComponent {
    int lives;
    int keysCollected;

    PlayerStatsComponent(int l = 5) : lives(l), keysCollected(0) {}
};
