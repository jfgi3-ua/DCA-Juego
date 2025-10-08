#include "MainGameState.hpp"
#include "StateMachine.hpp"
#include "GameOverState.hpp"
#include <iostream>
extern "C" {
  #include <raylib.h>
}

MainGameState::MainGameState()
{
}

void MainGameState::init()
{
    std::cout << "You are in the Main Game State" << std::endl;
}

void MainGameState::handleInput()
{
    if(IsKeyPressed(KEY_ENTER)){
        this->state_machine->add_state(std::make_unique<GameOverState>(1, 1, 1.0f), true);
    }
}

void MainGameState::update(float deltaTime)
{
    // this->handleInput();
}

void MainGameState::render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const char* msg = "Bienvenido a Escape del Laberinto!";
    int fontSize = 19;
    int textWidth = MeasureText(msg, fontSize);

    // void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
    DrawText(
        msg,
        (GetScreenWidth() - textWidth) / 2,   // centrado horizontal
        GetScreenHeight() / 3,                // aprox. un tercio desde arriba
        fontSize,
        BLACK
    );

    EndDrawing();
}