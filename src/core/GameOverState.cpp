#include "GameOverState.hpp"
#include <iostream>
#include "StateMachine.hpp"
#include "MainGameState.hpp"

extern "C" {
    #include <raylib.h>
}

GameOverState::GameOverState(int nivel, bool die, float time)
{
    dead = die;
    remainingTime = time;
    backgroundColor = dead ? RED : DARKGREEN;
    currentLevel = nivel; //queremos enseñar el nivel?, pasar de nivel se gestiona en main o en esta clase?
    //en caso de gestionar el nivel en GameOver
    //currentLevel = dead ? nivel : nivel++;
}

GameOverState::~GameOverState() {
    //UnloadTexture(gameOverText);
}

void GameOverState::init() {
    //gameOverText = LoadTexture("assets/sprites/gameover.png");
}

void GameOverState::handleInput() {
    

    Vector2 mousePos = GetMousePosition();
    
    // Configurar rectángulos de botones (igual que en render)
    float buttonWidth = 250;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;
    
    Rectangle retryButton = {startX, startY, buttonWidth, buttonHeight};
    Rectangle exitButton = {startX, startY + 100, buttonWidth, buttonHeight};
    
    // Detectar hover con ratón
    if (CheckCollisionPointRec(mousePos, retryButton)) {
        selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en REINTENTAR/SIGUIENTE" << std::endl;
            this->state_machine->add_state(std::make_unique<MainGameState>(), true);
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
            if (dead) {
                // Si murió, reiniciar el mismo nivel
                this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel), true);
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
    Vector2 mousePos = GetMousePosition();

    //CASO 0, PASAMOS NIVEL dead = 0
    //CASO 1, MORIMOS dead = 1

    // --- TÍTULO PRINCIPAL ---
    std::string title = dead
        ? "GAME OVER"
        : "NIVEL " + std::to_string(currentLevel) + " COMPLETADO";

    int titleFontSize = 50;
    int titleWidth = MeasureText(title.c_str(), titleFontSize);
    int titleX = (WINDOW_WIDTH - titleWidth) / 2;
    int titleY = WINDOW_HEIGHT / 4;

    DrawText(title.c_str(), titleX, titleY, titleFontSize, WHITE);

    // --- SUBTÍTULO ---
    std::string subtitle;
    if (dead) {
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

    int subFontSize = 25;
    int subWidth = MeasureText(subtitle.c_str(), subFontSize);
    int subX = (WINDOW_WIDTH - subWidth) / 2;
    int subY = titleY + 70;
    DrawText(subtitle.c_str(), subX, subY, subFontSize, WHITE);

    // --- Botones ---
    float buttonWidth = 250;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;

    // Botón REINTENTAR/SIGUIENTE NIVEL
    Rectangle retryButton = {startX, startY, buttonWidth, buttonHeight};
    bool retryHover = CheckCollisionPointRec(mousePos, retryButton);
    Color retryColor = (selectedOption == 0 || retryHover) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(retryButton, retryColor);
    DrawRectangleLinesEx(retryButton, 2.0f, retryHover ? YELLOW : BLACK);
    
    std::string retryText = dead ? "REINTENTAR" : "SIGUIENTE NIVEL";
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