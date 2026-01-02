#pragma once

#include "Config.hpp"
#include "GameState.hpp"
#include "objects/Map.hpp"
#include "objects/Player.hpp"
#include "objects/Spikes.hpp"
#include "GameOverState.hpp"
#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "ResourceManager.hpp"
#include "ecs/Ecs.hpp"

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

    private:
        // Mapa del juego
        Map _map;
        int _tile = 32;
        int _level = 1;

        float levelTime_ = 60.0f;

        // Total de llaves en el mapa (guardado al inicio, no cambia)
        int _totalKeysInMap = 0;

        // ECS registry
        entt::registry _registry;

        // ========== DEVELOPER MODE ==========
        bool _freezeEnemies = false;     // Enemigos congelados
        bool _infiniteTime = false;      // Tiempo infinito
        bool _keyGivenByCheating = false; // Track si la llave fue obtenida por cheat

        // Métodos privados para renderizado
        void _renderMap();
        void _renderHUD();
        void _renderPlayerHUD(const Rectangle& bagHud, const Rectangle& livesHud, float baseY);
        void _renderTimerAndLevel();

        // Método para verificar condiciones de fin de juego
        void _checkGameEndConditions();
};
