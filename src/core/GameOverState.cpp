#include "GameOverState.hpp"
#include <iostream>
#include "StateMachine.hpp"
#include "MainGameState.hpp"
#include "ResourceManager.hpp"

extern "C" {
    #include <raylib.h>
}

GameOverState::GameOverState(int nivel, bool die, float time, bool isVictory)
{
    dead = die;
    remainingTime = time;
    isVictory_ = isVictory;
    backgroundColor = isVictory_ ? GOLD : (dead ? RED : DARKGREEN); //si añadimos sprites eliminar color de fondo
    currentLevel = nivel;
}

GameOverState::~GameOverState() {
}

void GameOverState::init() {
    auto& rm = ResourceManager::Get();

    // Dependiendo del estado de victoria, muerte o nivel completado cargamos recursos distintos
    if (isVictory_) {
        titulo = &rm.GetTexture("background-day.png");
        background = &rm.GetTexture("background-day.png");
    } else if (dead) {
        titulo = &rm.GetTexture("background-day.png");
        background = &rm.GetTexture("background-day.png");

    } else {
        titulo = &rm.GetTexture("background-day.png");
        background = &rm.GetTexture("background-day.png");
    }
}


void GameOverState::handleInput() {
    

    Vector2 mousePos = GetMousePosition();
    
    // Configurar rectángulos de botones (igual que en render)
    float buttonWidth = 250;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = isVictory_ ? WINDOW_HEIGHT / 2.0f + 50 : WINDOW_HEIGHT / 2.0f - buttonHeight;
    
    Rectangle retryButton = {startX, startY, buttonWidth, buttonHeight};
    Rectangle exitButton = {startX, startY + 100, buttonWidth, buttonHeight};
    
    // Detectar hover con ratón
    if (CheckCollisionPointRec(mousePos, retryButton)) {
        selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (isVictory_) {
                std::cout << "Clic en EMPEZAR DE NUEVO" << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(1), true);
            } else if (!dead && currentLevel >= 6) {
                // Completó el último nivel, mostrar pantalla de victoria
                this->state_machine->add_state(std::make_unique<GameOverState>(6, false, remainingTime, true), true);
            } else {
                std::cout << "Clic en REINTENTAR/SIGUIENTE" << std::endl;
                this->state_machine->add_state(std::make_unique<MainGameState>(), true);
            }
            return;
        }
    }
    
    if (CheckCollisionPointRec(mousePos, exitButton)) {
        selectedOption = 1;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en SALIR" << std::endl;
            this->state_machine->set_game_ending(true);
            return;
        }
    }
    
    // Cambiar selección menu
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
        selectedOption = !selectedOption; // alterna entre 0 y 1
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if(selectedOption){
            //si opcion == 1 salimos del juego
            this->state_machine->set_game_ending(true);
        }else{
            //si opcion == 0
            if (isVictory_) {
                // Victoria total: empezar de nuevo desde nivel 1
                this->state_machine->add_state(std::make_unique<MainGameState>(1), true);
            } else if (dead) {
                // Si murió, reiniciar el mismo nivel
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel), true);
            } else if (currentLevel >= 6) {
                // Si completó el nivel 6 (último), mostrar pantalla de victoria
                this->state_machine->add_state(std::make_unique<GameOverState>(6, false, remainingTime, true), true);
            } else {
                // Si completó el nivel, pasar al siguiente
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel + 1), true);
            }
        }
    }
}

void GameOverState::update(float) {
}

void GameOverState::render()
{
    ClearBackground(backgroundColor);
    //DrawTexture(*background, 0, 0, WHITE);
    Vector2 mousePos = GetMousePosition();

    // --- TÍTULO PRINCIPAL ---

    //se cambiara por sprite cuando lo tengamos
    //el sprite ya se determina en el init segun si es victoria, muerte o nivel completado NO HACEN FALTA CONDICIONALES AQUI
    //DrawTexture(*titulo, 0, 0, WHITE);

    std::string title;
    if (isVictory_) {
        title = "¡FELICIDADES!";
    } else {
        title = dead ? "GAME OVER" : "NIVEL " + std::to_string(currentLevel) + " COMPLETADO";
    }

    int titleFontSize = isVictory_ ? 60 : 50;
    int titleWidth = MeasureText(title.c_str(), titleFontSize);
    int titleX = (WINDOW_WIDTH - titleWidth) / 2;
    int titleY = isVictory_ ? WINDOW_HEIGHT / 5 : WINDOW_HEIGHT / 4;

    DrawText(title.c_str(), titleX, titleY, titleFontSize, isVictory_ ? DARKGREEN : WHITE);

    // --- SUBTÍTULO ---
    std::string subtitle;
    if (isVictory_) {
        subtitle = "Has completado todos los niveles";
    } else if (dead) {
        subtitle = "Has muerto";
    } else {
        // Convertir tiempo a minutos:segundos
        int totalSeconds = (int)remainingTime;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
        subtitle = "Tiempo restante: " + std::string(timeStr);
    }

    int subFontSize = isVictory_ ? 30 : 25;
    int subWidth = MeasureText(subtitle.c_str(), subFontSize);
    int subX = (WINDOW_WIDTH - subWidth) / 2;
    int subY = titleY + (isVictory_ ? 80 : 70);
    DrawText(subtitle.c_str(), subX, subY, subFontSize, isVictory_ ? DARKBROWN : WHITE);
    
    // --- MENSAJE ADICIONAL (solo en victoria) ---
    if (isVictory_) {
        std::string message = "¡Eres un maestro del laberinto!";
        int msgFontSize = 22;
        int msgWidth = MeasureText(message.c_str(), msgFontSize);
        int msgX = (WINDOW_WIDTH - msgWidth) / 2;
        int msgY = subY + 50;
        DrawText(message.c_str(), msgX, msgY, msgFontSize, BROWN);
    }

    // --- Botones ---
    float buttonWidth = 250;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = isVictory_ ? WINDOW_HEIGHT / 2.0f + 50 : WINDOW_HEIGHT / 2.0f - buttonHeight;

    // Botón REINTENTAR/SIGUIENTE NIVEL/EMPEZAR DE NUEVO
    Rectangle retryButton = {startX, startY, buttonWidth, buttonHeight};
    bool retryHover = CheckCollisionPointRec(mousePos, retryButton);
    Color retryColor = (selectedOption == 0 || retryHover) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(retryButton, retryColor);
    DrawRectangleLinesEx(retryButton, 2.0f, retryHover ? YELLOW : BLACK);
    
    std::string retryText = isVictory_ ? "EMPEZAR DE NUEVO" : (dead ? "REINTENTAR" : "SIGUIENTE NIVEL");
    int retryTextWidth = MeasureText(retryText.c_str(), 25);
    DrawText(retryText.c_str(), startX + (buttonWidth - retryTextWidth) / 2, startY + 7, 25, WHITE);

    // Botón SALIR
    startY += 100;
    Rectangle exitButton = {startX, startY, buttonWidth, buttonHeight};
    bool exitHover = CheckCollisionPointRec(mousePos, exitButton);
    Color exitColor = (selectedOption == 1 || exitHover) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(exitButton, exitColor);
    DrawRectangleLinesEx(exitButton, 2.0f, exitHover ? YELLOW : BLACK);
    DrawText("SALIR", startX + 90, startY + 7, 25, WHITE);
}