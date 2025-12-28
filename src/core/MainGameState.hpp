#pragma once
#include <iostream>
#include <vector>
#include "Config.hpp"
#include "GameState.hpp"
#include "objects/Map.hpp"
#include "objects/Player.hpp"
#include "objects/Spikes.hpp"
extern "C" {
  #include <raylib.h>
}
#include <entt/entt.hpp>  // Librería ECS

class MainGameState : public GameState
{
    public:
        MainGameState(int level = 1);
        ~MainGameState() = default;

        void init() override;
        void handleInput() override;
        void update(float deltaTime) override;
        void render() override;

        void pause(){};
        void resume(){};

        void _loadLevelEntities();

    private:
        char _entered_key;
        // Mapa del juego
        Map _map;
        int _tile = 32;
        int _level = 1;

        // Temporizador del nivel (segundos). 30s base + 30s por nivel
        float _levelTime = 30.0f;

        // Total de llaves en el mapa (guardado al inicio, no cambia)
        int _totalKeysInMap = 0;

        // ECS registry
        entt::registry _registry;

        // ========== DEVELOPER MODE ==========
        bool _freezeEnemies = false;     // Enemigos congelados
        bool _infiniteTime = false;      // Tiempo infinito
        bool _keyGivenByCheating = false; // Track si la llave fue obtenida por cheat
};
