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
            CloseWindow();
        }else{
            //si opcion == 0 volvemos al juego
            this->state_machine->add_state(std::make_unique<MainGameState>(), true);

            //this->state_machine->add_state(std::make_unique<MainGameState>(currentLevel), true);
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

    std::string title = dead ? "GAME OVER" : "NIVEL COMPLETADO";
    DrawText(title.c_str(), 50, 120, 40, WHITE);

    if (dead) {
        DrawText("Has muerto", 150, 180, 20, WHITE);
    } else {
        std::string timeMsg = "Tiempo restante: " + std::to_string((int)remainingTime) + " s";
        DrawText(timeMsg.c_str(), 150, 180, 20, WHITE);
    }

    // --- Botones ---
    float buttonWidth = 150;
    float buttonHeight = 40;
    float startX = 200;
    float startY = 250;

    // Botón JUGAR
    Rectangle playButton = {startX, startY, buttonWidth, buttonHeight};
    Color playColor = (selectedOption == 0) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(playButton, playColor);
    DrawText("JUGAR", startX + 30, startY + 5, 30, WHITE);

    // Botón SALIR
    startY += 100;
    Rectangle exitButton = {startX, startY, buttonWidth, buttonHeight };
    Color exitColor = (selectedOption == 1) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(exitButton, exitColor);
    DrawText("SALIR", startX + 30, startY + 5, 30, WHITE);

    EndDrawing();
}