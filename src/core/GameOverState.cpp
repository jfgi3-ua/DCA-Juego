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
    BeginDrawing();
    //DrawTexture(background, 0, 0, WHITE);
    ClearBackground(backgroundColor);

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
        // Convertir tiempo restante a minutos:segundos
        int minutes = (int)remainingTime / 60;
        int seconds = (int)remainingTime % 60;
        subtitle = "Tiempo restante: " + std::to_string(minutes) + ":" + 
                  (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }

    int subFontSize = 25;
    int subWidth = MeasureText(subtitle.c_str(), subFontSize);
    int subX = (WINDOW_WIDTH - subWidth) / 2;
    int subY = titleY + 70;
    DrawText(subtitle.c_str(), subX, subY, subFontSize, WHITE);

    // --- Botones ---
    float buttonWidth = 200;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;

    // Texto del botón principal cambia según el estado
    const char* mainButtonText = dead ? "REINTENTAR" : "SIGUIENTE NIVEL";

    // Botón principal (REINTENTAR o SIGUIENTE NIVEL)
    Rectangle playButton = {startX, startY, buttonWidth, buttonHeight};
    Color playColor = (selectedOption == 0) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(playButton, playColor);
    int mainTextWidth = MeasureText(mainButtonText, 24);
    DrawText(mainButtonText, startX + (buttonWidth - mainTextWidth) / 2, startY + 8, 24, WHITE);

    // Botón SALIR
    startY += 100;
    Rectangle exitButton = {startX, startY, buttonWidth, buttonHeight };
    Color exitColor = (selectedOption == 1) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(exitButton, exitColor);
    int exitTextWidth = MeasureText("SALIR", 24);
    DrawText("SALIR", startX + (buttonWidth - exitTextWidth) / 2, startY + 8, 24, WHITE);

    EndDrawing();
}