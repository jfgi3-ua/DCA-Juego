#pragma once
#include "GameState.hpp"
#include <iostream>
#include "Map.hpp"
#include "Player.hpp"
#include <vector>
#include "Config.hpp"
#include "Spikes.hpp"
#include "Mechanism.hpp"
extern "C" {
  #include <raylib.h>
}

class MainGameState : public GameState
{
    public:
        MainGameState();
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

        // Jugador
        Player player_;

        // Enemigo
        std::vector<Vector2> enemiesPos_;

        //Pinchos
        Spikes spikes_;

        //Mecanismos
        std::vector<Mechanism> mechanisms_;
        std::vector<Vector2> activeMechanisms_;
};