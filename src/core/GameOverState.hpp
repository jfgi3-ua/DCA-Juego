#pragma once

#include <iostream>
#include <string>
#include <functional>  

#include "GameState.hpp"
#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include "ResourceManager.hpp"
#include "Localization.hpp"
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
        int _currentLevel = 1;
        Color _backgroundColor = GRAY;
        float _remainingTime;
        int _selectedOption = 0;

        bool _isDead = false;     // true si el jugador murió
        bool _isVictory = false;  // true si es victoria total (completó todos los niveles)
        
        // Struct para rutas de sprites
        spritesPaths _spritesPaths;

        // Configuraciones de botones
        ButtonConfig _levelConfig = {400, 120, 100, 380, 50, 30};
        ButtonConfig _otherConfig = {350, 100, 80, WINDOW_HEIGHT - 200, 40, 20};

        // Para cuando tengamos sprites
        const Texture2D* _titulo = nullptr;
        const Texture2D* _background = nullptr;

        // Idioma actual (para detectar cambios y recargar sprites)
        std::string currentLang_ = "";
        // Rectángulo destino al dibujar el background (permite mantener tamaño consistente entre idiomas)
        Rectangle backgroundDrawRect_ = {0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
        // Constantes para renderizado
        static constexpr float _CONGRATS_SCALE = 0.35f;
        static constexpr float _GAME_OVER_SCALE = 0.45f;
        static constexpr int _CONGRATS_Y = 100;
        static constexpr int _GAME_OVER_Y = 80;
        static constexpr int _TIME_TEXT_Y = 250;
        static constexpr int _TIME_FONT_SIZE = 35;
        static constexpr int _TEXT_OFFSET = 2;

        // Texturas de botones
        static const std::string _TEX_BUTTON_NEXT;
        static const std::string _TEX_BUTTON_EXIT;
        static const std::string _TEX_BUTTON_RESTART;

        // Funciones auxiliares
        void _loadSprites(const std::vector<std::string>& sprites);
        void _handleButtons(const Rectangle& button1Area, const Rectangle& button2Area, std::function<void()> action1, std::function<void()> action2);
        void _handleConfig(const ButtonConfig& config, std::function<void()> action1, std::function<void()> action2);
        void _renderButtons(const ButtonConfig& config, const std::string& tex1, const std::string& tex2, bool useHover);
        void _handleLangButtonInput();
        void _renderLangButton() const;
        Rectangle _getLangButtonRect() const;

};