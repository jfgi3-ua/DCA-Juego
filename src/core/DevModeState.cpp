#include "DevModeState.hpp"
#include "StateMachine.hpp"
#include "../objects/Player.hpp"
#include "MainGameState.hpp"
#include "GameOverState.hpp"
#include <iostream>

extern "C" {
    #include <raylib.h>
}

DevModeState::DevModeState(Player* player, float* levelTime, bool* freezeEnemies, bool* infiniteTime, bool* keyGivenByCheating, int currentLevel)
    : player_(player), levelTime_(levelTime), freezeEnemies_(freezeEnemies), 
      infiniteTime_(infiniteTime), keyGivenByCheating_(keyGivenByCheating), currentLevel_(currentLevel),
      awaitingPassword_(true), selectedOption_(-1), authenticated_(false),
      godMode_(false), noClip_(false)
{
}

DevModeState::~DevModeState()
{
}

void DevModeState::init()
{
    awaitingPassword_ = true;
    authenticated_ = false;
    passwordInput_.clear();
    selectedOption_ = -1;
    
    // Sincronizar estado de cheats con el Player
    godMode_ = player_->isGodMode();
    noClip_ = player_->isNoClip();
    
    // Configurar las opciones de cheats
    setupCheatOptions();
}

void DevModeState::setupCheatOptions()
{
    cheatOptions_.clear();
    
    // Verificar que los punteros son válidos
    if (!player_ || !freezeEnemies_ || !infiniteTime_ || !keyGivenByCheating_) {
        std::cerr << "ERROR: Punteros nulos en setupCheatOptions!" << std::endl;
        return;
    }
    
    // 0. God Mode
    cheatOptions_.push_back({
        "1. God Mode (Invulnerabilidad)",
        [this]() {
            godMode_ = !godMode_;
            if (player_) player_->setGodMode(godMode_);
            std::cout << "God Mode: " << (godMode_ ? "ON" : "OFF") << std::endl;
        },
        [this]() { return godMode_ ? "[ON]" : "[OFF]"; }
    });
    
    // 1. Congelar Enemigos
    cheatOptions_.push_back({
        "2. Congelar Enemigos",
        [this]() {
            if (freezeEnemies_) {
                *freezeEnemies_ = !(*freezeEnemies_);
                std::cout << "Freeze Enemies: " << (*freezeEnemies_ ? "ON" : "OFF") << std::endl;
            }
        },
        [this]() { return (freezeEnemies_ && *freezeEnemies_) ? "[ON]" : "[OFF]"; }
    });
    
    // 2. Tiempo Infinito
    cheatOptions_.push_back({
        "3. Tiempo Infinito",
        [this]() {
            if (infiniteTime_) {
                *infiniteTime_ = !(*infiniteTime_);
                std::cout << "Infinite Time: " << (*infiniteTime_ ? "ON" : "OFF") << std::endl;
            }
        },
        [this]() { return (infiniteTime_ && *infiniteTime_) ? "[ON]" : "[OFF]"; }
    });
    
    // 3. NoClip
    cheatOptions_.push_back({
        "4. NoClip (Atravesar Paredes)",
        [this]() {
            noClip_ = !noClip_;
            if (player_) player_->setNoClip(noClip_);
            std::cout << "NoClip: " << (noClip_ ? "ON" : "OFF") << std::endl;
        },
        [this]() { return noClip_ ? "[ON]" : "[OFF]"; }
    });
    
    // 4. Añadir vida
    cheatOptions_.push_back({
        "5. Anadir +1 Vida",
        [this]() {
            if (player_) {
                player_->addLife();
                std::cout << "Vida anadida. Vidas: " << player_->getLives() << std::endl;
            }
        },
        [this]() { return ""; }
    });
    
    // 5. Vidas máximas
    cheatOptions_.push_back({
        "6. Vidas Maximas (10)",
        [this]() {
            if (player_) {
                player_->setMaxLives();
                std::cout << "Vidas al maximo (10)" << std::endl;
            }
        },
        [this]() { return ""; }
    });
    
    // 6. Dar llave
    cheatOptions_.push_back({
        "7. Obtener Llave",
        [this]() {
            if (player_) {
                if (!player_->hasKey()) {
                    if (keyGivenByCheating_) *keyGivenByCheating_ = true;
                    player_->setKeyCount(999);
                    std::cout << "Llave obtenida (por cheat)" << std::endl;
                } else {
                    std::cout << "Ya tienes la llave" << std::endl;
                }
            }
        },
        [this]() { return (player_ && player_->hasKey()) ? "[SI]" : "[NO]"; }
    });
    
    // 7. Añadir tiempo
    cheatOptions_.push_back({
        "8. Anadir +30 Segundos",
        [this]() {
            if (levelTime_) {
                *levelTime_ += 30.0f;
                std::cout << "30 segundos anadidos" << std::endl;
            }
        },
        [this]() { return ""; }
    });
    
    // 8. Saltar nivel
    cheatOptions_.push_back({
        "9. Saltar Nivel",
        [this]() {
            if (this->state_machine) {
                this->state_machine->remove_overlay_state();
                // Si estamos en el nivel 6 o superior, mostrar pantalla de victoria
                if (currentLevel_ >= 6) {
                    std::cout << "Nivel final completado - Mostrando pantalla de victoria..." << std::endl;
                    this->state_machine->add_state(std::make_unique<GameOverState>(6, false, 0.0f, true), true);
                } else {
                    std::cout << "Saltando al siguiente nivel..." << std::endl;
                    this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel_ + 1), true);
                }
            }
        },
        [this]() { return ""; }
    });
    
    // 9. Resetear cheats
    cheatOptions_.push_back({
        "0. Resetear Todos los Cheats",
        [this]() {
            godMode_ = false;
            if (freezeEnemies_) *freezeEnemies_ = false;
            if (infiniteTime_) *infiniteTime_ = false;
            noClip_ = false;
            if (player_) {
                player_->setGodMode(false);
                player_->setNoClip(false);
                player_->resetLives();
            }
            
            if (keyGivenByCheating_ && *keyGivenByCheating_ && player_) {
                player_->setKeyCount(0);
                *keyGivenByCheating_ = false;
                std::cout << "Llave quitada (era de cheat)" << std::endl;
            }
            
            std::cout << "Todos los cheats reseteados" << std::endl;
        },
        [this]() { return ""; }
    });
}

void DevModeState::handleInput()
{
    Vector2 mousePos = GetMousePosition();
    
    if (awaitingPassword_)
    {
        // Capturar entrada de teclado
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && passwordInput_.length() < 20)
            {
                passwordInput_ += (char)key;
            }
            key = GetCharPressed();
        }
        
        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE) && passwordInput_.length() > 0)
        {
            passwordInput_.pop_back();
        }
        
        // Botones
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;
        Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
        Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };
        
        bool confirmClicked = CheckCollisionPointRec(mousePos, confirmBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        bool cancelClicked = CheckCollisionPointRec(mousePos, cancelBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        if (IsKeyPressed(KEY_ENTER) || confirmClicked)
        {
            if (passwordInput_ == "developer")
            {
                authenticated_ = true;
                awaitingPassword_ = false;
                selectedOption_ = -1;
                passwordInput_.clear();
                std::cout << "Developer Mode activado!" << std::endl;
            }
            else
            {
                passwordInput_.clear();
                std::cout << "Contraseña incorrecta" << std::endl;
            }
        }
        
        if (IsKeyPressed(KEY_ESCAPE) || cancelClicked)
        {
            // Cerrar el overlay
            this->state_machine->remove_overlay_state();
        }
    }
    else if (authenticated_)
    {
        // Menú de opciones
        int menuX = GetScreenWidth() / 2 - 250;
        int menuY = 100;
        
        // Detectar hover y clic en opciones
        for (int i = 0; i < static_cast<int>(cheatOptions_.size()); i++)
        {
            Rectangle optionRect = { 
                (float)(menuX + 30), 
                (float)(menuY + 100 + i * 38), 
                440.0f, 
                35.0f 
            };
            
            if (CheckCollisionPointRec(mousePos, optionRect))
            {
                selectedOption_ = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    cheatOptions_[i].action();
                }
            }
        }
        
        // Navegación por teclado
        if (IsKeyPressed(KEY_UP))
        {
            selectedOption_--;
            if (selectedOption_ < 0) selectedOption_ = static_cast<int>(cheatOptions_.size()) - 1;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            selectedOption_++;
            if (selectedOption_ >= static_cast<int>(cheatOptions_.size())) selectedOption_ = 0;
        }
        
        if (IsKeyPressed(KEY_ENTER) && selectedOption_ >= 0 && selectedOption_ < static_cast<int>(cheatOptions_.size()))
        {
            cheatOptions_[selectedOption_].action();
        }
        
        // Cerrar menú
        Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
        bool closeBtnClicked = CheckCollisionPointRec(mousePos, closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        if (IsKeyPressed(KEY_ESCAPE) || closeBtnClicked)
        {
            this->state_machine->remove_overlay_state();
        }
    }
}

void DevModeState::update(float /*deltaTime*/)
{
    // Este estado no actualiza lógica, solo maneja UI
}

void DevModeState::render()
{
    if (awaitingPassword_)
    {
        renderPasswordScreen();
    }
    else if (authenticated_)
    {
        renderDevMenu();
    }
}

void DevModeState::renderPasswordScreen()
{
    Vector2 mousePos = GetMousePosition();
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
    
    int menuX = GetScreenWidth() / 2 - 250;
    int menuY = 100;
    int menuWidth = 500;
    int menuHeight = 400;
    
    // Panel del menú
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKGRAY);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);
    
    DrawText("DEVELOPER MODE", menuX + 100, menuY + 20, 30, GOLD);
    DrawText("Introduce la contrasena:", menuX + 80, menuY + 80, 20, WHITE);
    
    // Campo de contraseña
    char masked[32] = {0};
    for (size_t i = 0; i < passwordInput_.length() && i < 31; i++)
    {
        masked[i] = '*';
    }
    DrawRectangle(menuX + 50, menuY + 120, 400, 40, LIGHTGRAY);
    DrawRectangleLines(menuX + 50, menuY + 120, 400, 40, DARKGRAY);
    DrawText(masked, menuX + 60, menuY + 128, 25, BLACK);
    
    // Botones
    Rectangle confirmBtn = { (float)(menuX + 150), (float)(menuY + 240), 200.0f, 40.0f };
    Rectangle cancelBtn = { (float)(menuX + 150), (float)(menuY + 290), 200.0f, 40.0f };
    
    bool confirmHover = CheckCollisionPointRec(mousePos, confirmBtn);
    bool cancelHover = CheckCollisionPointRec(mousePos, cancelBtn);
    
    DrawRectangleRec(confirmBtn, confirmHover ? DARKGREEN : GREEN);
    DrawRectangleLinesEx(confirmBtn, 2.0f, BLACK);
    DrawText("CONFIRMAR", menuX + 175, menuY + 248, 20, WHITE);
    
    DrawRectangleRec(cancelBtn, cancelHover ? MAROON : RED);
    DrawRectangleLinesEx(cancelBtn, 2.0f, BLACK);
    DrawText("CANCELAR", menuX + 180, menuY + 298, 20, WHITE);
    
    DrawText("Usa teclado o haz clic en los botones", menuX + 90, menuY + 350, 16, LIGHTGRAY);
}

void DevModeState::renderDevMenu()
{
    Vector2 mousePos = GetMousePosition();
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
    
    int menuX = GetScreenWidth() / 2 - 250;
    int menuY = 100;
    int menuWidth = 500;
    int menuHeight = 520;
    
    // Panel del menú
    DrawRectangle(menuX, menuY, menuWidth, menuHeight, DARKGRAY);
    DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);
    
    DrawText("DEVELOPER MENU", menuX + 120, menuY + 20, 30, GOLD);
    DrawText("Haz clic o usa UP/DOWN y ENTER", menuX + 80, menuY + 60, 16, LIGHTGRAY);
    
    // Mostrar estado GOD global (cualquier cheat activo)
    bool isGodActive = godMode_ || noClip_ || (*freezeEnemies_) || (*infiniteTime_);
    if (isGodActive) {
        DrawRectangle(menuX + 10, menuY + 70, menuWidth - 20, 20, Fade(GOLD, 0.3f));
        DrawText("*** GOD MODE ACTIVO ***", menuX + 140, menuY + 73, 16, GOLD);
    }
    
    for (int i = 0; i < static_cast<int>(cheatOptions_.size()); i++)
    {
        Rectangle optionRect = { 
            (float)(menuX + 30), 
            (float)(menuY + 100 + i * 38), 
            440.0f, 
            35.0f 
        };
        
        bool isHovered = CheckCollisionPointRec(mousePos, optionRect);
        bool isSelected = (i == selectedOption_);
        
        if (isHovered || isSelected)
        {
            DrawRectangleRec(optionRect, Fade(YELLOW, 0.2f));
        }
        
        Color textColor = (isHovered || isSelected) ? YELLOW : WHITE;
        Color stateColor = (isHovered || isSelected) ? GOLD : LIGHTGRAY;
        
        DrawText(cheatOptions_[i].label, menuX + 35, menuY + 102 + i * 38, 20, textColor);
        
        // Estado
        const char* state = cheatOptions_[i].getState();
        DrawText(state, menuX + 400, menuY + 102 + i * 38, 20, stateColor);
    }
    
    // Botón cerrar
    Rectangle closeBtn = { (float)(menuX + 150), (float)(menuY + 490), 200.0f, 25.0f };
    bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);
    
    DrawRectangleRec(closeBtn, closeHover ? Fade(RED, 0.3f) : Fade(DARKGRAY, 0.3f));
    DrawRectangleLines(menuX + 150, menuY + 490, 200, 25, closeHover ? RED : LIGHTGRAY);
    DrawText("CERRAR (ESC)", menuX + 170, menuY + 493, 18, closeHover ? RED : LIGHTGRAY);
}

