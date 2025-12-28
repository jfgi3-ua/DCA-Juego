#pragma once
#include "GameState.hpp"
#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include "ResourceManager.hpp"
#include <iostream>
#include <deque> 

extern "C" {
    #include <raylib.h>
}

struct spritesPaths {
    std::vector<std::string> victorySprites_ = {
        "sprites/menus/background_congratulations.png",
        "sprites/menus/items_congratulations.png",
        "sprites/icons/boton_reiniciar.png",
        "sprites/icons/boton_salir.png"
    };
    std::vector<std::string> deathSprites_ = {
        "sprites/menus/game_over_background.png",
        "sprites/menus/game_over.png",
        "sprites/icons/boton_reiniciar.png",
        "sprites/icons/boton_salir.png"
    };
    std::vector<std::string> levelCompletedSprites_ = {
        "sprites/menus/background_pasar_nivel.png",
        "sprites/icons/boton_siguiente_nivel.png",
        "sprites/icons/boton_salir.png"
    };
};

class GameOverState : public GameState
{
    public:
        GameOverState(int score, bool die, float time, bool isVictory = false);
        ~GameOverState() override {};

        void init() override;
        void handleInput() override;
        void update(float dt) override;
        void render() override;

        void pause() override {}
        void resume() override {}

    private:
        int currentLevel_ = 1;
        Color backgroundColor_ = GRAY;
        float remainingTime_;
        int selectedOption_ = 0;
        bool isDead_ = false;     // true si el jugador murió
        bool isVictory_ = false;  // true si es victoria total (completó todos los niveles)
        
        spritesPaths spritesPaths_;

        //para cuando tengamos sprites
        const Texture2D* titulo_ = nullptr;
        const Texture2D* background_ = nullptr;

        // Funciones auxiliares
        void loadSprites_(const std::vector<std::string>& sprites);
};