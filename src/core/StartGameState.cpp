#include "StartGameState.hpp"
#include "StateMachine.hpp"
#include <iostream>

extern "C" {
    #include <raylib.h>
}

StartGameState::StartGameState() = default;

StartGameState::~StartGameState() {
    //UnloadTexture(background);
}

void StartGameState::init() {
    //background = LoadTexture("assets/sprites/background.png");
}

void StartGameState::handleInput() {
    // Cambiar selección menu
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
        std::cout << "Tecla arriba/abajo presionada." << std::endl;
        selectedOption = !selectedOption; // alterna entre 0 y 1
    }

    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "Tecla enter presionada." << std::endl;
        if(selectedOption){
            //si opcion == 1 salimos del juego
            this->state_machine->set_game_ending(true);
        }else{
            //si opcion == 0 pasamos al juego
            this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        }
    }
}

void StartGameState::update(float) {
}

void StartGameState::render() {
    BeginDrawing();

    //DrawTexture(background, 0, 0, WHITE);
    ClearBackground(backgroundColor);

     // --- Botones ---
    float buttonWidth = 150;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;

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
