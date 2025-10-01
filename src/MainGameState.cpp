#include <MainGameState.hpp>
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
    std::cout << "Enter a key: ";
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