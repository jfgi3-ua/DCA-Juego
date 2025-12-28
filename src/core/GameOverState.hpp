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
        int currentLevel = 1;
        Color backgroundColor = GRAY;
        bool dead;
        float remainingTime;
        int selectedOption = 0;
        bool isVictory_ = false;  // true si es victoria total (completó todos los niveles)
        
        spritesPaths spritesPaths_;

        //para cuando tengamos sprites
        const Texture2D* titulo = nullptr;
        const Texture2D* background = nullptr;

        // Funciones auxiliares
        void loadSprites_(const std::vector<std::string>& sprites);
};