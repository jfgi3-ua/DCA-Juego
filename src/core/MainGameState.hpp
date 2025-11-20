#pragma once
#include "GameState.hpp"
#include <iostream>
#include "objects/Map.hpp"
#include "objects/Player.hpp"
#include <vector>
#include "Config.hpp"
#include "objects/Spikes.hpp"
#include "objects/Mechanism.hpp"
extern "C" {
  #include <raylib.h>
}

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

        void activeMechanisms();


    private:
        char entered_key;
        // Mapa del juego
        Map map_;
        int tile_ = 32;
        int level_ = 1;

        // Jugador
        Player player_;

        // Enemigo
        std::vector<Vector2> enemiesPos_;

        //Pinchos
        Spikes spikes_;

        //Mecanismos
        std::vector<Mechanism> mechanisms_;
        std::vector<Vector2> activeMechanisms_;
        // Temporizador del nivel (segundos). 30s base + 30s por nivel
        float levelTime_ = 30.0f;
};