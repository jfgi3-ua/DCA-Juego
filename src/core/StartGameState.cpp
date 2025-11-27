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
    Vector2 mousePos = GetMousePosition();
    
    // Configurar rectángulos de botones (igual que en render)
    float buttonWidth = 150;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;
    
    Rectangle playButton = {startX, startY, buttonWidth, buttonHeight};
    Rectangle exitButton = {startX, startY + 100, buttonWidth, buttonHeight};
    
    // Detectar hover con ratón
    if (CheckCollisionPointRec(mousePos, playButton)) {
        selectedOption = 0;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::cout << "Clic en JUGAR" << std::endl;
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
    
    // Cambiar selección menu con teclado
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
        std::cout << "Tecla arriba/abajo presionada." << std::endl;
        selectedOption = !selectedOption;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        std::cout << "Tecla enter presionada." << std::endl;
        if(selectedOption){
            this->state_machine->set_game_ending(true);
        }else{
            this->state_machine->add_state(std::make_unique<MainGameState>(), true);
        }
    }
}

void StartGameState::update(float) {
}

void StartGameState::render() {
    ClearBackground(backgroundColor);
    Vector2 mousePos = GetMousePosition();

     // --- Botones ---
    float buttonWidth = 150;
    float buttonHeight = 40;
    float startX = WINDOW_WIDTH / 2.0f - buttonWidth / 2.0f;
    float startY = WINDOW_HEIGHT / 2.0f - buttonHeight;

    // Botón JUGAR
    Rectangle playButton = {startX, startY, buttonWidth, buttonHeight};
    bool playHover = CheckCollisionPointRec(mousePos, playButton);
    Color playColor = (selectedOption == 0 || playHover) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(playButton, playColor);
    DrawRectangleLinesEx(playButton, 2.0f, playHover ? YELLOW : BLACK);
    DrawText("JUGAR", startX + 30, startY + 5, 30, WHITE);

    // Botón SALIR
    startY += 100;
    Rectangle exitButton = {startX, startY, buttonWidth, buttonHeight };
    bool exitHover = CheckCollisionPointRec(mousePos, exitButton);
    Color exitColor = (selectedOption == 1 || exitHover) ? LIGHTGRAY : DARKGRAY;
    DrawRectangleRec(exitButton, exitColor);
    DrawRectangleLinesEx(exitButton, 2.0f, exitHover ? YELLOW : BLACK);
    DrawText("SALIR", startX + 30, startY + 5, 30, WHITE);
}
