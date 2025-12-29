#pragma once

#include <iostream>
#include <string>
#include <functional>  

#include "GameState.hpp"
#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include "ResourceManager.hpp"
#include <deque> 

extern "C" {
    #include <raylib.h>
}

struct spritesPaths {
    std::vector<std::string> victorySprites = {
        "sprites/menus/background_congratulations.png",
        "sprites/menus/items_congratulations.png",
        "sprites/icons/boton_reiniciar.png",
        "sprites/icons/boton_salir.png"
    };
    std::vector<std::string> deathSprites = {
        "sprites/menus/game_over_background.png",
        "sprites/menus/game_over.png",
        "sprites/icons/boton_reiniciar.png",
        "sprites/icons/boton_salir.png"
    };
    std::vector<std::string> levelCompletedSprites = {
        "sprites/menus/background_pasar_nivel.png",
        "sprites/icons/boton_siguiente_nivel.png",
        "sprites/icons/boton_salir.png"
    };
};

struct ButtonConfig {
    float buttonWidth;
    float buttonHeight;
    float spacing;
    float yPos;
    float clickPaddingX;
    float clickPaddingY;
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
        // Variables de estado
        int currentLevel_ = 1;
        Color backgroundColor_ = GRAY;
        float remainingTime_;
        int selectedOption_ = 0;

        bool isDead_ = false;     // true si el jugador murió
        bool isVictory_ = false;  // true si es victoria total (completó todos los niveles)
        
        // Struct para rutas de sprites
        spritesPaths spritesPaths_;

        // Configuraciones de botones
        ButtonConfig levelConfig_ = {400, 120, 100, 380, 50, 30};
        ButtonConfig otherConfig_ = {350, 100, 80, WINDOW_HEIGHT - 200, 40, 20};

        // Para cuando tengamos sprites
        const Texture2D* titulo_ = nullptr;
        const Texture2D* background_ = nullptr;

        // Idioma actual (para detectar cambios y recargar sprites)
        std::string currentLang_ = "";
        // Rectángulo destino al dibujar el background (permite mantener tamaño consistente entre idiomas)
        Rectangle backgroundDrawRect_ = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
        // Constantes para renderizado
        static constexpr float CONGRATS_SCALE = 0.35f;
        static constexpr float GAME_OVER_SCALE = 0.45f;
        static constexpr int CONGRATS_Y = 100;
        static constexpr int GAME_OVER_Y = 80;
        static constexpr int TIME_TEXT_Y = 250;
        static constexpr int TIME_FONT_SIZE = 35;
        static constexpr int TEXT_OFFSET = 2;

        // Texturas de botones
        static const std::string TEX_BUTTON_NEXT;
        static const std::string TEX_BUTTON_EXIT;
        static const std::string TEX_BUTTON_RESTART;

        // Funciones auxiliares
        void loadSprites_(const std::vector<std::string>& sprites);
        void handleButtons_(const Rectangle& button1Area, const Rectangle& button2Area, std::function<void()> action1, std::function<void()> action2);
        void handleConfig_(const ButtonConfig& config, std::function<void()> action1, std::function<void()> action2);
        void renderButtons_(const ButtonConfig& config, const std::string& tex1, const std::string& tex2, bool useHover);
};