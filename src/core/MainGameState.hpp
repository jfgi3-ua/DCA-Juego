#pragma once
#include <iostream>
#include <vector>

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

        void loadLevelEntities();

    private:
        // Mapa del juego
        Map map_;
        int tile_ = 32;
        int level_ = 1;

        // Temporizador del nivel (segundos). 30s base + 30s por nivel
        float levelTime_ = 30.0f;

        // Total de llaves en el mapa (guardado al inicio, no cambia)
        int totalKeysInMap_ = 0;

        // ECS registry
        entt::registry registry;

        // ========== DEVELOPER MODE ==========
        bool freezeEnemies_ = false;     // Enemigos congelados
        bool infiniteTime_ = false;      // Tiempo infinito
        bool keyGivenByCheating_ = false; // Track si la llave fue obtenida por cheat

        // Métodos privados para renderizado
        void renderMap();
        void renderHUD();
        void renderPlayerHUD(const Rectangle& bagHud, const Rectangle& livesHud, float baseY);
        void renderTimerAndLevel();

        // Método para verificar condiciones de fin de juego
        void checkGameEndConditions();
};
